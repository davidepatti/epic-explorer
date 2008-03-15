/* File:     cache.cc
 * Author:   Steve Lieberman <lieberm@umich.edu>
 * Project:  CCCP - M5elements Cache Simulator
 * Date:     March 2005
 * Descr:    M5elements is a library-based interface to M5's memory system
 *           to allow its incorporation into SIMU or other simulators.
 *           Some code in this file is a modified version of M5's
 *           .../sim/main.cc 
 */

/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004
 * The Regents of The University of Michigan
 * All Rights Reserved
 *
 * This code is part of the M5 simulator, developed by Nathan Binkert,
 * Erik Hallnor, Steve Raasch, and Steve Reinhardt, with contributions
 * from Ron Dreslinski, Dave Greene, Lisa Hsu, Ali Saidi, and Andrew
 * Schultz.
 *
 * Permission is granted to use, copy, create derivative works and
 * redistribute this software and such derivative works for any
 * purpose, so long as the copyright notice above, this grant of
 * permission, and the disclaimer below appear in all copies made; and
 * so long as the name of The University of Michigan is not used in
 * any advertising or publicity pertaining to the use or distribution
 * of this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE
 * UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND
 * WITHOUT WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE
 * LIABLE FOR ANY DAMAGES, INCLUDING DIRECT, SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
 * IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGES.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <signal.h>

#include <string>

#include "base/copyright.hh"
#include "base/embedfile.hh"
#include "base/inifile.hh"
#include "base/misc.hh"
#include "base/output.hh"
#include "base/pollevent.hh"
#include "base/statistics.hh"
#include "base/str.hh"
#include "base/time.hh"
#include "m5elements/cache.hh"
#include "m5elements/mem_types.hh"
#include "m5elements/req_by_cluster.hh"
#include "python/pyconfig.hh"
#include "sim/async.hh"
#include "sim/builder.hh"
#include "sim/configfile.hh"
#include "sim/host.hh"
#include "sim/root.hh"
#include "sim/sim_events.hh"
#include "sim/sim_exit.hh"
#include "sim/sim_object.hh"
#include "sim/stat_control.hh"
#include "sim/stats.hh"


using namespace std;

static const string PY_SRC_DIR = "$M5_ROOT/m5/python/";
static const string PY_BUILD_DIR = "$M5_ROOT/m5/build/ALPHA/python/";
static const string M5_OUTPUT_DIR = ".";
static const string MPY_CONFIG_FILE = getenv("M5_CONFIG_FILE");
static const int MAX_MEM_REQ_BUFFER_SIZE = 1024;

/* GLOBALS */
static int            _num_clusters = -1;
static M5E_mem_req _mem_req_obj_buffer[MAX_MEM_REQ_BUFFER_SIZE];
static long           _mem_req_buffer[MAX_MEM_REQ_BUFFER_SIZE];
static int            _mem_req_load_buffer[MAX_MEM_REQ_BUFFER_SIZE];
static int            _outstanding_requests = 0;


// See async.h.
volatile bool async_event = false;
volatile bool async_dump = false;
volatile bool async_dumpreset = false;
volatile bool async_exit = false;
volatile bool async_io = false;
volatile bool async_alarm = false;

static void sayHello(ostream &out);
static void dumpStatsHandler(int sigtype);
static void dumprstStatsHandler(int sigtype);
static void exitNowHandler(int sigtype);
static void abortHandler(int sigtype);

/// Print welcome message.
static void
sayHello(ostream &out)
{
    extern const char *compileDate;	// from date.cc

    ccprintf(out, "M5 Simulator System (via M5elements library)\n");
    // display copyright
    ccprintf(out, "%s\n", briefCopyright);
    ccprintf(out, "M5 compiled on %d\n", compileDate);

    char *host = getenv("HOSTNAME");
    if (!host)
	host = getenv("HOST");

    if (host)
	ccprintf(out, "M5 executing on %s\n", host);

    ccprintf(out, "M5 simulation started %s\n", Time::start);
}

/// Stats signal handler.
static void
dumpStatsHandler(int sigtype)
{
    async_event = true;
    async_dump = true;
}

static void
dumprstStatsHandler(int sigtype)
{
    async_event = true;
    async_dumpreset = true;
}

/// Exit signal handler.
static void
exitNowHandler(int sigtype)
{
    async_event = true;
    async_exit = true;
}

/// Abort signal handler.
static void
abortHandler(int sigtype)
{
    cerr << "Program aborted at cycle " << curTick << endl;
    
#if TRACING_ON
    // dump trace buffer, if there is one
    Trace::theLog.dump(cerr);
#endif
}

/*
int
main(int argc, char **argv)
{
	M5E_init_system(4);

	M5E_destruct_system();
}
*/

extern int
M5E_is_present_in_icache(M5E_mem_req req, int cid)
{
  M5E_Cluster *c = m5eClusters[cid];
  return c->isPresentInIcache(req);
}

extern int
M5E_probe_dcache(M5E_mem_req req, int cid)
{
  M5E_Cluster *c = m5eClusters[cid];
  return c->probeDcache(req);
}

extern long 
M5E_send_icache_request(M5E_mem_req req, int cid)
{
	M5E_Cluster *c = m5eClusters[cid];
	assert(c);
#ifdef DEBUG
	cerr << "send I-cache request: type=" << req.type << ",addr=" << req.addr << ",data=" << req.data << ",size=" << req.size << ",is_valid=" << req.is_valid
	  << ",id=" << req.id << ",register_id=" << req.register_id << endl;
#endif
	_outstanding_requests++;
	return c->issueIcacheRequestUponTick(req);
}

extern long 
M5E_send_dcache_request(M5E_mem_req req, int cid)
{
	M5E_Cluster *c = m5eClusters[cid];
	assert(c);
#ifdef DEBUG
	cerr << "send D-cache request: type=" << req.type << ",addr=" << req.addr << ",data=" << req.data << ",size=" << req.size << ",is_valid=" << req.is_valid
	  << ",id=" << req.id << ",register_id=" << req.register_id << endl;
#endif
	_outstanding_requests++;
	return c->issueDcacheRequestUponTick(req);
}

/* M5E_ready_loads()
*      returns an array of register ids indicating the loads that *just*
*      completed in the current cycle, terminated by the id -1
*/
extern int*
M5E_ready_loads()
{
	int i = 0;

	for (int cid = 0; cid < _num_clusters; cid++) {
		M5E_Cluster* c = m5eClusters[cid];
		vector<M5E_mem_req> vect = c->getCompletedRequests();
		vector<M5E_mem_req>::iterator vecti;

		for (vecti = vect.begin(); vecti != vect.end(); vecti++) {
			M5E_mem_req req = *vecti;

			assert(i != MAX_MEM_REQ_BUFFER_SIZE);
			
			if (req.type == M5E_LOAD)
				_mem_req_load_buffer[i++] = req.register_id;		
			_outstanding_requests--;
		}
	}

	assert(i != MAX_MEM_REQ_BUFFER_SIZE);
	//_mem_req_load_buffer[i++] =  INT_MIN;   /* sentinel value */
	_mem_req_load_buffer[i++] =  -1;   /* sentinel value */

	return _mem_req_load_buffer;
}


/* M5E_ready_requests()
*      returns an array of request ids of all requests that *just*
*      completed in the current cycle, terminated by the id -1
*/
extern long*
M5E_ready_requests()
{
	int i = 0;

	for (int cid = 0; cid < _num_clusters; cid++) {
		M5E_Cluster* c = m5eClusters[cid];
		vector<M5E_mem_req> vect = c->getCompletedRequests();
		vector<M5E_mem_req>::iterator vecti;

		for (vecti = vect.begin(); vecti != vect.end(); vecti++) {
			M5E_mem_req req = *vecti;

			assert(i != MAX_MEM_REQ_BUFFER_SIZE);
			_mem_req_buffer[i++] = req.id;		
		}
	}

	assert(i != MAX_MEM_REQ_BUFFER_SIZE);
	_mem_req_buffer[i++] =  -1;   /* sentinel value */

	return _mem_req_buffer;
}



extern M5E_mem_req*
M5E_ready_requests_as_obj()
{
	int i = 0;

	
	for (int cid = 0; cid < _num_clusters; cid++) {
		M5E_Cluster* c = m5eClusters[cid];
		vector<M5E_mem_req> vect = c->getCompletedRequests();
		vector<M5E_mem_req>::iterator vecti;

		for (vecti = vect.begin(); vecti != vect.end(); vecti++) {
			M5E_mem_req req = *vecti;

			assert(i != MAX_MEM_REQ_BUFFER_SIZE);
			_mem_req_obj_buffer[i++] = req;		
		}
	}

	M5E_mem_req null_req;
	null_req.is_valid = 0;

	assert(i != MAX_MEM_REQ_BUFFER_SIZE);
	_mem_req_obj_buffer[i++] = null_req;

	return _mem_req_obj_buffer;
}

extern void
M5E_debug_dump(M5E_addr addr)
{
	cerr << "cache.cc: M5E_debug_dump(" << hex << addr << ")" << dec 
	  << endl;
}


extern int
M5E_is_dcache_request_queue_full(int cid)
{
#ifdef DEBUG
	if (m5eClusters[cid]->isDcacheBlocked()) {
		cerr << "M5E_is_dcache_request_queue_full(" << cid << "): "
		     << ( m5eClusters[cid]->isDcacheBlocked() ? "true" : "false")
		     << endl;
	}
#endif
	return m5eClusters[cid]->isDcacheBlocked();
}

extern void
M5E_destruct_system()
{
  exitNow("M5elements terminated normally.", 0, false);
}

extern int
M5E_get_outstanding_requests()
{
  assert(_outstanding_requests >= 0);
  return _outstanding_requests;
}


extern void
M5E_init_system(int num_clusters)
{
    assert(num_clusters > 0);

    _num_clusters = num_clusters;


    signal(SIGFPE, SIG_IGN);		// may occur on misspeculated paths
    signal(SIGTRAP, SIG_IGN);
    signal(SIGUSR1, dumpStatsHandler);		// dump intermediate stats
    signal(SIGUSR2, dumprstStatsHandler);	// dump and reset stats
    signal(SIGINT, exitNowHandler);		// dump final stats and exit
    signal(SIGABRT, abortHandler);

#ifdef DEBUG
    sayHello(cerr);
#endif
    string outdir;

    if (outdir.empty()) {
	char *env = getenv("M5_OUTPUT_DIR");
	outdir = env ? env : M5_OUTPUT_DIR;
    }

    simout.setDirectory(outdir);

    char *env = getenv("CONFIG_OUTPUT");
    if (!env)
	env = "config.out";
    configStream = simout.find(env);

    IniFile inifile;
    string pyFile(MPY_CONFIG_FILE);
    PythonConfig pyconfig;
    pyconfig.load(pyFile);
    
    if (!pyconfig.output(inifile))
      panic("Error processing python code");

    // The configuration database is now complete; start processing it.

    // Initialize statistics database
    Stats::InitSimStats();

    // Now process the configuration hierarchy and create the SimObjects.
    ConfigHierarchy configHierarchy(inifile);
    configHierarchy.build();
    configHierarchy.createSimObjects();

    // Parse and check all non-config-hierarchy parameters.
    ParamContext::parseAllContexts(inifile);
    ParamContext::checkAllContexts();

    // Print hello message to stats file if it's actually a file.  If
    // it's not (i.e. it's cout or cerr) then we already did it above.
    if (simout.isFile(*outputStream))
	sayHello(*outputStream);

    // Echo command line and all parameter settings to stats file as well.
    //echoCommandLine(argc, argv, *outputStream);
    ParamContext::showAllContexts(*configStream);

    // Do a second pass to finish initializing the sim objects
    SimObject::initAll();

    // Restore checkpointed state, if any.
    configHierarchy.unserializeSimObjects();

    // Done processing the configuration database.
    // Check for unreferenced entries.
    if (inifile.printUnreferenced())
	panic("unreferenced sections/entries in the intermediate ini file");

    SimObject::regAllStats();

    // uncomment the following to get PC-based execution-time profile
#ifdef DO_PROFILE
    init_profile((char *)&_init, (char *)&_fini);
#endif

    // Check to make sure that the stats package is properly initialized
    Stats::check();

    // Reset to put the stats in a consistent state.
    Stats::reset();

#ifdef DEBUG
    warn("Entering event queue.  Starting simulation...\n");
#endif
    SimStartup();

}

/* M5E_advance_clock()
 *      moves the M5 clock forward to the next cycle. note this also clears
 *      the list of requests that completed this cycle.
 */
extern void
M5E_advance_clock()
{

	assert(curTick <= mainEventQueue.nextTick() &&
	"event scheduled in the past");

	// forward current cycle to the time of the first event on the
	// queue
	curTick = mainEventQueue.nextTick();

#ifdef DEBUG
	//cerr << "================================== TIME " << curTick
	//     << " ===================================" << endl;
#endif

	while (!mainEventQueue.empty()) {

		if (mainEventQueue.nextTick() != curTick) {
			// we are done with this clock tick, exit func
			return;
		}

		mainEventQueue.serviceOne();

		if (async_event) {
			async_event = false;
			if (async_dump) {
				async_dump = false;

				using namespace Stats;
				SetupEvent(Dump, curTick);
			}

			if (async_dumpreset) {
				async_dumpreset = false;

				using namespace Stats;
				SetupEvent(Dump | Reset, curTick);
			}

			if (async_exit) {
				async_exit = false;
				new SimExitEvent("User requested STOP");
			}

			if (async_io || async_alarm) {
				async_io = false;
				async_alarm = false;
				pollQueue.service();
			}
		}
	}

	// This should never happen... every conceivable way for the
	// simulation to terminate (hit max cycles/insts, signal,
	// simulated system halts/exits) generates an exit event, so we
	// should never run out of events on the queue.
	exitNow("no events on event loop!  All CPUs must be idle.", 1);
}

