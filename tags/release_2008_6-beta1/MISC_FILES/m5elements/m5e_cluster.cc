/* $Id: m5e_cluster.cc,v 1.1 2008-03-15 19:26:15 dpatti Exp $ */

/*
 * Copyright (c) 2002, 2003, 2004
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

// FIX ME: make trackBlkAddr use blocksize from actual cache, not hard coded

#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "base/misc.hh"
#include "base/statistics.hh"
#include "cpu/exec_context.hh"
#include "m5elements/m5e_cluster.hh"
#include "m5elements/req_by_cluster.hh"
#include "mem/cache/base_cache.hh"
//#include "mem/functional_mem/main_memory.hh"
#include "sim/builder.hh"
#include "sim/sim_events.hh"
#include "sim/stats.hh"

using namespace std;

//static int DVLIW_TESTER_ALLOCATOR=0;

M5E_Cluster::M5E_Cluster(const string &name,
                 int _cluster_id,
		 MemInterface *_icache_interface,
		 MemInterface *_dcache_interface,
		 FunctionalMemory *main_mem,
		 unsigned _memorySize,
		 unsigned _percentReads,
		 unsigned _progressInterval,
		 Addr _traceAddr,
		 Counter _max_loads)
    : SimObject(name),
      tickEvent(this),
      cluster_id(_cluster_id),
      icacheInterface(_icache_interface),
      dcacheInterface(_dcache_interface),
      mainMem(main_mem),
      size(_memorySize),
      percentReads(_percentReads),
      completedReqsCycleNum(0),
      completedReqsUnread(false),
      progressInterval(_progressInterval),
      nextProgressMessage(_progressInterval),
      maxLoads(_max_loads)
{
	vector<string> cmd;
	cmd.push_back("/bin/ls");
	vector<string> null_vec;
	xc = new ExecContext(NULL, 0, mainMem, 0);

	traceBlockAddr = blockAddr(_traceAddr);
	noResponseCycles = 0;
	numOutstandingReqs = 0;

	tickEvent.schedule(0);

	if (m5eClusters.size() < cluster_id)
		for (int i = 0; i < cluster_id; i++)
			m5eClusters.push_back(NULL);
	else if (m5eClusters.size() == cluster_id)
		m5eClusters.push_back(this);
	else	
		m5eClusters[cluster_id] = this;
	//id = DVLIW_TESTER_ALLOCATOR++;
}

#ifdef DEBUG
static void
printData(ostream &os, uint8_t *data, int nbytes)
{
    os << hex << setfill('0');
    // assume little-endian: print bytes from highest address to lowest
    for (uint8_t *dp = data + nbytes - 1; dp >= data; --dp) {
	os << setw(2) << (unsigned)*dp;
    }
    os << dec;
}
#endif


void
M5E_Cluster::regStats()
{
    using namespace Stats;

    
    numDcacheReadsStat
	.name(name() + ".num_dcache_reads")
	.desc("number of D-cache read accesses completed (using MemReqs)")
	;

    numWritesStat
	.name(name() + ".num_writes")
	.desc("number of write accesses completed (using MemReqs)")
	;

}

long
M5E_Cluster::issueIcacheRequestUponTick(M5E_mem_req simu_req)
{
	//HACK:
	// removing this assertion means that we are assuming infinite
	// request queues -- processor does not have to stall when request
	// queues fill                    <lieberm 12/08/2005>
	//assert(!icacheInterface->isBlocked()); 

	long reqId = (curTick * 1000000) + (nextReqId++);
	simu_req.id = reqId;

	issuedIcacheReqsThisCycle.push_back(simu_req);

	return reqId;
}

long
M5E_Cluster::issueDcacheRequestUponTick(M5E_mem_req simu_req)
{
	//HACK:
	// removing this assertion means that we are assuming infinite
	// request queues -- processor does not have to stall when request
	// queues fill                    <lieberm 12/08/2005>
	//assert(!dcacheInterface->isBlocked()); 

	long reqId = (curTick * 1000000) + (nextReqId++);
	simu_req.id = reqId;

	issuedDcacheReqsThisCycle.push_back(simu_req);

	return reqId;
}

MemReqPtr
M5E_Cluster::convertToM5Request(M5E_mem_req simu_req)
{
  MemReqPtr req = new MemReq();	
  req->size = simu_req.size; //1 << 0;          // TODO 
  req->data = new uint8_t[req->size];
  req->paddr = simu_req.addr;
  req->paddr &= ~(req->size - 1);
  req->time = curTick;
  req->xc = xc;
  req->pc = simu_req.id;
  req->register_id = simu_req.register_id;

  if (simu_req.type == M5E_LOAD) {
    req->cmd = Read;
  } else if (simu_req.type == M5E_STORE) {
    req->cmd = Write;
    assert(req->data);
    memcpy(req->data, (uint8_t*) &(simu_req.data), req->size);

#ifdef DEBUG
    cerr << "M5E_Cluster::convertToM5Request(): addr"
      << setfill('0') << hex << req->paddr << " req.data=";
    printData(cerr, req->data, req->size);
    cerr << " simu_req.data=" << setfill('0') << hex << simu_req.data
      << "/";
    printData(cerr, (uint8_t*) &(simu_req.data), req->size);
    cerr << endl << dec;
#endif

  } else {
    assert(0);
  }
  return req;
}

bool
M5E_Cluster::isPresentInIcache(M5E_mem_req simu_req)
{
  MemReqPtr req = convertToM5Request(simu_req);
  assert(req->cmd == Read);
  Tick cycle = icacheInterface->probe(req);
#ifdef DEBUG
  cerr << "M5E_Cluster::isPresentInIcache(): curr cycle: " << curTick
     << " for addr " << simu_req.addr << " would come back in " 
     << cycle << endl;
#endif

  return (cycle != 0 && cycle == curTick);
}

int
M5E_Cluster::probeDcache(M5E_mem_req simu_req)
{
  MemReqPtr req = convertToM5Request(simu_req);
  Tick cycle = dcacheInterface->probe(req);
  return cycle;
}

void
M5E_Cluster::issueIcacheRequestNow(M5E_mem_req simu_req)
{
  // cache could have become blocked since we may
  // have issued several requests this cycle
  if (icacheInterface->isBlocked()) {
    blockedIcacheReqs.push_back(simu_req);
    return;
  }

  MemReqPtr req = convertToM5Request(simu_req);

#ifdef DEBUG
  if (blockAddr(req->paddr) == traceBlockAddr) {
    cerr << name() << ": initiating "
      << ((req->cmd == Read) ? "read" :
	  (req->cmd == Write) ? "write" : "UNKNOWN")
      << " access of "
      << dec << req->size << " bytes (value = 0x";
    printData(cerr, req->data, req->size);
    cerr << ") to addr 0x"
      << hex << req->paddr 
      << " (0x" << hex << blockAddr(req->paddr) << ")"
      << " at cycle "
      << dec << curTick << endl;
  }
#endif

  req->completionEvent = new M5E_MemCompleteEvent(req, NULL, this, false);
  icacheInterface->access(req);
  numOutstandingReqs++;
}

void
M5E_Cluster::issueDcacheRequestNow(M5E_mem_req simu_req)
{
	// cache could have become blocked since we may
	// have issued several requests this cycle
	if (dcacheInterface->isBlocked()) {
		blockedDcacheReqs.push_back(simu_req);
		return;
	}


	MemReqPtr req = convertToM5Request(simu_req);

#ifdef DEBUG
	if (blockAddr(req->paddr) == traceBlockAddr) {
	    cerr << name() << ": initiating "
	         << ((req->cmd == Read) ? "read" :
                    (req->cmd == Write) ? "write" : "UNKNOWN")
		 << " access of "
		 << dec << req->size << " bytes (value = 0x";
	    printData(cerr, req->data, req->size);
	    cerr << ") to addr 0x"
		 << hex << req->paddr 
		 << " (0x" << hex << blockAddr(req->paddr) << ")"
		 << " at cycle "
		 << dec << curTick << endl;
	}
#endif

	req->completionEvent = new M5E_MemCompleteEvent(req, NULL, this, true);
	dcacheInterface->access(req);
	numOutstandingReqs++;
}

vector<M5E_mem_req>
M5E_Cluster::getCompletedRequests()
{
	if (completedReqsUnread) {
		assert(curTick == completedReqsCycleNum);
	} else {
		if (curTick != completedReqsCycleNum) {
			//requests were already read, OK to discard them
			completedReqsThisCycle.clear();
		}
	}
	completedReqsUnread = false;
	return completedReqsThisCycle;
}

bool
M5E_Cluster::isDcacheBlocked()
{
	return (dcacheInterface->isBlocked() || blockedDcacheReqs.size() > 0);
}


void
M5E_Cluster::tick()
{
#ifdef DEBUG
	//cerr << "M5E_Cluster::tick(): cluster " << cluster_id << endl;
#endif

	if (!tickEvent.scheduled())
		tickEvent.schedule(curTick + 1);

#ifdef DEBUG
	if (numOutstandingReqs > 0) {
	  if (++noResponseCycles >= 10000) {
	    cerr << name() << ": deadlocked at cycle " << curTick << endl;
	    fatal("");
	  }
	}
#endif

	while (!blockedIcacheReqs.empty() && !icacheInterface->isBlocked()) {
		issueIcacheRequestNow(blockedIcacheReqs.front());
		blockedIcacheReqs.pop_front();
	}

	while (!blockedDcacheReqs.empty() && !dcacheInterface->isBlocked()) {
		issueDcacheRequestNow(blockedDcacheReqs.front());
		blockedDcacheReqs.pop_front();
	}

	vector<M5E_mem_req>::iterator i = issuedIcacheReqsThisCycle.begin();
	for ( ; i != issuedIcacheReqsThisCycle.end(); i++) {
		M5E_mem_req& req = *i;
		issueIcacheRequestNow(req);
	}

	vector<M5E_mem_req>::iterator j = issuedDcacheReqsThisCycle.begin();
	for ( ; j != issuedDcacheReqsThisCycle.end(); j++) {
		M5E_mem_req& req = *j;
		issueDcacheRequestNow(req);
	}

	issuedIcacheReqsThisCycle.clear();
	issuedDcacheReqsThisCycle.clear();
}

/* M5E_Cluster::completeRequest()
 *     req       the M5 memory request object
 *     data      data returned by M5
 *     dcache    true for d-cache, false for i-cache
 *
 * called by M5E_MemCompleteEvent::process()
 */
void
M5E_Cluster::completeRequest(MemReqPtr &req, uint8_t *data, bool dcache)
{
#ifdef DEBUG
  cerr << "M5E_Cluster::completeRequest(): entered function.." << endl;
#endif
  switch (req->cmd) {
    case Read:
      if (dcache) {
	numDcacheReads++;
	numDcacheReadsStat++;
      }

#ifdef DEBUG
      if (dcache) {
	if (numDcacheReads == nextProgressMessage) {
	  ccprintf(cerr,
	      "%s: completed %d read accesses @%d\n",
	      name(), numDcacheReads, curTick);
	  nextProgressMessage += progressInterval;
	}
      }
#endif
      break;

    case Write:
      numWritesStat++;
      break;

    case Copy:
      panic("saw invalid use of copy instr!");
    default:
      panic("invalid command");
  }

#ifdef DEBUG
  if (blockAddr(req->paddr) == traceBlockAddr) {
    cerr << name() << ": completed "
      << (req->cmd.isWrite() ? "write" : "read") 
      << " access of "
      << dec << req->size << " bytes at address 0x"
      << hex << req->paddr 
      << " (0x" << hex << blockAddr(req->paddr) << ")"
      << ", value = 0x";
    printData(cerr, req->data, req->size);
    cerr << " @ cycle " << dec << curTick;
    cerr << endl;
  }
#endif

  M5E_mem_req simu_req;
  assert(req->cmd == Read || req->cmd == Write);
  simu_req.type = (req->cmd == Read) ? M5E_LOAD : M5E_STORE;
  simu_req.addr = req->paddr;
  simu_req.id   = req->pc;
  simu_req.register_id = req->register_id;
  simu_req.is_valid = 1;
  simu_req.size = req->size;             /* TODO */	

  memcpy((uint8_t*) &(simu_req.data), req->data, req->size);

#ifdef DEBUG
  cerr << "M5E_Cluster::completeRequest(): addr"
					   << setfill('0') << hex << req->paddr << " req.data=";
  printData(cerr, req->data, req->size);
  cerr << " simu_req.data=" << setfill('0') << hex << simu_req.data
    << "/";
  printData(cerr, (uint8_t*) &(simu_req.data), req->size);
  cerr << endl << dec;
#endif

  noResponseCycles = 0;
  numOutstandingReqs--;

  if (data != NULL)
    delete [] data;


  if (curTick != completedReqsCycleNum) {
#ifdef DEBUG
    cerr << "M5E_Cluster::completeRequest(): cluster " << cluster_id
					     << " clearing! " << "cycle " << curTick << " last was "
					       << completedReqsCycleNum << endl;
#endif
    assert(!completedReqsUnread); //somebody should have read them
    completedReqsThisCycle.clear();
    completedReqsCycleNum = curTick;
    assert(completedReqsCycleNum == curTick);
  }
  completedReqsUnread = true;
  completedReqsThisCycle.push_back(simu_req);
}


void
M5E_MemCompleteEvent::process()
{
    tester->completeRequest(req, data, dcache);
    delete this;
}


const char *
M5E_MemCompleteEvent::description()
{
    return "memory access completion";
}


BEGIN_DECLARE_SIM_OBJECT_PARAMS(M5E_Cluster)

    Param<int> cluster_id;
    SimObjectParam<BaseCache *> icache;
    SimObjectParam<BaseCache *> dcache;
    SimObjectParam<FunctionalMemory *> main_mem;
    Param<unsigned> memory_size;
    Param<unsigned> percent_reads;
    Param<unsigned> progress_interval;
    Param<Addr> trace_addr;
    Param<Counter> max_loads;

END_DECLARE_SIM_OBJECT_PARAMS(M5E_Cluster)


BEGIN_INIT_SIM_OBJECT_PARAMS(M5E_Cluster)

    INIT_PARAM(cluster_id, "cluster ID"),
    INIT_PARAM(icache, "L1 I-cache"),
    INIT_PARAM(dcache, "L1 D-cache"),
    INIT_PARAM(main_mem, "hierarchical memory"),
    INIT_PARAM(memory_size, "memory size"),
    INIT_PARAM(percent_reads, "target read percentage"),
    INIT_PARAM(progress_interval, "progress report interval (in accesses)"),
    INIT_PARAM(trace_addr, "address to trace"),
    INIT_PARAM(max_loads, "terminate when we have reached this load count")

END_INIT_SIM_OBJECT_PARAMS(M5E_Cluster)


CREATE_SIM_OBJECT(M5E_Cluster)
{
    return new M5E_Cluster(getInstanceName(), cluster_id,
                       icache->getInterface(),
                       dcache->getInterface(), main_mem,
		       memory_size, percent_reads, 
		       progress_interval,
		       trace_addr, max_loads);
}

REGISTER_SIM_OBJECT("M5E_Cluster", M5E_Cluster)

