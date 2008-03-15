/* $Id: m5e_cluster.hh,v 1.1 2008-03-15 19:26:15 dpatti Exp $ */

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

#ifndef __M5ELEMENTS_M5E_CLUSTER_HH_
#define __M5ELEMENTS_M5E_CLUSTER_HH_

#include <set>
#include <deque>
#include <vector>

using std::deque;
using std::vector;
using std::set;

#include "m5elements/mem_types.hh"
#include "base/statistics.hh"
#include "mem/functional/functional.hh"
#include "mem/mem_interface.hh"
#include "sim/eventq.hh"
#include "sim/sim_exit.hh"
#include "sim/sim_object.hh"
#include "sim/stats.hh"

class ExecContext;
class M5E_Cluster : public SimObject
{
  public:

    M5E_Cluster(const std::string &name,
            int _cluster_id,
	    MemInterface *_icache_interface,
	    MemInterface *_dcache_interface,
            FunctionalMemory *main_mem,
	    unsigned _memorySize,
	    unsigned _percentReads,
	    unsigned _progressInterval,
	    Addr _traceAddr,
	    Counter _max_loads);

    // register statistics
    virtual void regStats();
    // main simulation loop (one cycle)
    void tick();

    bool isPresentInIcache(M5E_mem_req req);
    int probeDcache(M5E_mem_req simu_req);

    bool isDcacheBlocked();
    long issueIcacheRequestUponTick(M5E_mem_req req);
    long issueDcacheRequestUponTick(M5E_mem_req req);
    vector<M5E_mem_req> getCompletedRequests();

  protected:
    class TickEvent : public Event
    {
      private:
	M5E_Cluster *cpu;
      public:
	TickEvent(M5E_Cluster *c)
	    : Event(&mainEventQueue, CPU_Tick_Pri), cpu(c) {}
	void process() {cpu->tick();}
	virtual const char *description() { return "tick event"; }
    };

    TickEvent tickEvent;

    int cluster_id;
    MemInterface *icacheInterface;
    MemInterface *dcacheInterface;
    FunctionalMemory *mainMem;
    ExecContext *xc;

    unsigned size;		// size of testing memory region

    unsigned percentReads;	// target percentage of read accesses

    int id;

    // looks unused:
    //std::set<unsigned> outstandingAddrs;
    int numOutstandingReqs;

    deque<M5E_mem_req> blockedIcacheReqs;
    deque<M5E_mem_req> blockedDcacheReqs;
    vector<M5E_mem_req> issuedIcacheReqsThisCycle;
    vector<M5E_mem_req> issuedDcacheReqsThisCycle;

    Tick completedReqsCycleNum;
    bool completedReqsUnread;
    vector<M5E_mem_req> completedReqsThisCycle;


    unsigned blockSize;

    Addr blockAddrMask;

    Addr blockAddr(Addr addr)
    {
	return (addr & ~blockAddrMask);
    }

    Addr traceBlockAddr;

    Addr baseAddr1;		// fix this to option
    Addr baseAddr2;		// fix this to option

    unsigned progressInterval;	// frequency of progress reports
    Tick nextProgressMessage;	// access # for next progress report


    Tick noResponseCycles;

    uint64_t numDcacheReads;
    uint64_t maxLoads;
    Stats::Scalar<> numDcacheReadsStat;
    Stats::Scalar<> numWritesStat;

    /* M5E_Cluster::completeRequest()
     *     req       the M5 memory request object
     *     data      data returned by M5
     *     dcache    true for d-cache, false for i-cache
     *
     * called by M5E_MemCompleteEvent::process()
     */
    void completeRequest(MemReqPtr &req, uint8_t *data, bool dcache);

    void issueIcacheRequestNow(M5E_mem_req req);
    void issueDcacheRequestNow(M5E_mem_req req);

    MemReqPtr convertToM5Request(M5E_mem_req req);

    friend class M5E_MemCompleteEvent;
};



class M5E_MemCompleteEvent : public Event
{
    MemReqPtr req;
    uint8_t *data;
    M5E_Cluster *tester;
    bool dcache; // true = d-cache event, false = i-cache event 

  public:

    M5E_MemCompleteEvent(MemReqPtr &_req, uint8_t *_data, M5E_Cluster *_tester, 
	bool _dcache)
	: Event(&mainEventQueue),
	  req(_req), data(_data), tester(_tester),
	  dcache(_dcache)
    {
    }

    void process();

    virtual const char *description();
};

#endif /* __M5ELEMENTS_M5E_CLUSTER_HH_ */



