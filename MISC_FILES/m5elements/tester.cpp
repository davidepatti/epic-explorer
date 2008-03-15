
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <list>

using namespace std;

#include "m5elements/cache.hh"
#include "m5elements/mem_types.hh"

#define NUM_CLUSTERS 4


/* memory addresses already being accessed this cycle, so we can ensure
 * no more than one cluster is accessing an address during a cycle --
 * this is a guarantee made by the compiler.
 */
set<M5E_addr> addresses_this_cycle;

/* requests that were issued in the current cycle, to be added to
 * requests_by_cycle */
list<M5E_mem_req> requests_this_cycle;

/* requests that need to be completed before we can move forward in
 * requests_by_cycle, when receiving completed requests
 */
list<M5E_mem_req> incomplete_requests;

/* requests in order of the cycle they were given, so that we can ensure
 * they were committed in the same order, when we receive completed
 * requests.
 */
list<list<M5E_mem_req> > requests_by_cycle;

/* all addresses that have ever been used -- this allows us to make
 * tester more stressful by hitting up the same addresses more often.
 */
vector<M5E_addr> active_addresses;

/* our store of requests we have issued and the request IDs assigned by the
 * dcache subsystem.
 */
map<int, M5E_mem_req> requests;

/* holds the state memory should be in if the system had one memory.
 * this is updated when requests are received as complete from the
 * dcache subsystem, and requests are checked against it.
 */
map<M5E_addr, M5E_data> correct_mem;

int cycles = 0;
int stall_cycles = 0;

static bool list_find_and_remove(list<M5E_mem_req>& l, const M5E_mem_req& tgt);

extern void
print_mem_req(const M5E_mem_req* req, int newline)
{
	fprintf(stderr, "request=[");

	if (req->type == M5E_LOAD)
		fprintf(stderr, "M5E_LOAD");
	else if (req->type == M5E_STORE)
		fprintf(stderr, "M5E_STORE");
	else
		assert(0);

	fprintf(stderr, ",addr=%08x,data=%08x,size=%d,id=%d]",
		(int) req->addr,
		(int) req->data,
		req->size,
		(int) req->id);

	if (newline)
		fprintf(stderr, "\n");
}

static void
print_requests_by_cycle()
{
	list<list<M5E_mem_req> >::iterator i;
	list<M5E_mem_req>::iterator j;
	int cycle = 1;
	
	fprintf(stderr, "tester.cpp: requests_by_cycle[incomplete]:\n");
	j = incomplete_requests.begin();
	for ( ; j != incomplete_requests.end(); j++) {
		fprintf(stderr, "                ");
		print_mem_req(&(*j), true);
	}

	i = requests_by_cycle.begin();
	for ( ; i != requests_by_cycle.end(); i++) {

		fprintf(stderr, 
			"tester.cpp: requests_by_cycle[incomplete + %d]:\n",
			cycle++);
		j = (*i).begin();
		for ( ; j != (*i).end(); j++) {
			fprintf(stderr, "                ");
			print_mem_req(&(*j), true);
		}
	}

}

static bool
operator==(const M5E_mem_req& a, const M5E_mem_req& b)
{
	bool eq = (a.id == b.id);

	if (eq) {
		assert(a.addr == b.addr);
		assert(a.size == b.size);
		assert(a.is_valid == b.is_valid);
		assert(a.type == M5E_LOAD || (a.data == b.data));
	}

	return eq;
/*
	return ((a.type == b.type)
	     && (a.addr == b.addr)
	     && (a.size == b.size)
	     && (a.is_valid == b.is_valid)
	     && (a.type == M5E_LOAD || (a.data == b.data)));
*/
}

static bool
list_find_and_remove(list<M5E_mem_req>& l, const M5E_mem_req& tgt)
{
	list<M5E_mem_req>::iterator i;

	for (i = l.begin(); i != l.end(); i++) {
		if (*i == tgt) {
			l.erase(i);
			return true;
		}
	}

	return false;
}

/* check_request():
 *     find request in requests_by_cycle and assume all previous requests 
 *     have completed
 */
static void
check_request(M5E_mem_req req)
{
	int prev_size = incomplete_requests.size();

	if (list_find_and_remove(incomplete_requests, req)) {
		assert(incomplete_requests.size() == prev_size - 1);
		//printf("                             ");
		//printf("FOUND in incomplete_requests.\n");

		if (req.type == M5E_LOAD) {
			/* data checking occurs here */
			M5E_data expected = correct_mem[req.addr];
			M5E_data actual = req.data;

			if (actual != expected) {
				M5E_debug_dump(req.addr);
				fprintf(stderr,
					"ERROR! expecting %08x received %08x\n",
					(int) expected, (int) actual);
				assert(0);
				exit(1);
			}
		} else if (req.type == M5E_STORE) {
			correct_mem[req.addr] = req.data;

		} else {
			assert(0);
		}
	} else {
		assert(incomplete_requests.size() == prev_size);
		//printf("                             ");
		//printf("NOT found in incomplete_requests.\n");
		/* pop the next cycle's requests of the queue and
		 * start evaluation of them
		 */

		/* if we fail assert here the requests may have been 
		 * processed out of order
		 */
		assert(requests_by_cycle.size() > 0);

		list<M5E_mem_req> req_set = requests_by_cycle.front();
		requests_by_cycle.pop_front();

		list<M5E_mem_req>::iterator i;
		for (i = req_set.begin(); i != req_set.end(); i++) {
			incomplete_requests.push_back(*i);
		}

		/* now that we have more incomplete_requests, loop
		 * back to beginning of function
		 */
		check_request(req);
		
	}
}


static void
tick_clock()
{
	cerr << "tester.cpp: tick_clock(): about to check ready req" << endl;	

	/* new clock cycle, but first check completed req */
	M5E_mem_req* ready_arr = M5E_ready_requests_as_obj();

	/* checks completed requests for accuracy */
	while (ready_arr->is_valid) {
		fprintf(stderr, "tester.cpp: check_request(): ");
		print_mem_req(ready_arr, true);
		if (rand() == 13) // (just to supress compiler warning)
			print_requests_by_cycle();
		check_request(*ready_arr);

		// now we're only removing requests from this when
		// they have been completed:
		addresses_this_cycle.erase(ready_arr->addr);

		ready_arr++;
	}

	
	M5E_advance_clock();
	requests_by_cycle.push_back(requests_this_cycle);
	requests_this_cycle.clear();

	// let's instead remove requests from this when they have
	// been completed
	//addresses_this_cycle.clear();
}

static void
issue_request()
{
	const int REQ_SIZE = 4;
	M5E_mem_req req;
	bool already_exists = false;
	M5E_addr addr;
	//active_addresses.push_back(0xFEEDBEEF);

	do {
		if (active_addresses.size() > 0 && ((rand() % 100) < 90)) {
			/* use existing address */
			int rand_idx = rand() % active_addresses.size();
			addr = active_addresses[rand_idx];
			req.type = (rand() % 2) ? M5E_LOAD : M5E_STORE;
		} else {
			/* use new address */
			addr = rand() % INT_MAX;
			addr &= ~(REQ_SIZE - 1); 
			req.type = M5E_STORE;
		}

		already_exists = addresses_this_cycle.find(addr)
			!= addresses_this_cycle.end();
		
	} while (already_exists);

	addresses_this_cycle.insert(addr);
	
	req.addr = addr;
	req.data = (req.type == M5E_LOAD) ? -1 : rand();
	req.size = REQ_SIZE;
	req.is_valid = 1;
	
	int cid = rand() % NUM_CLUSTERS;
	if ((rand() % 4) == 0)
		cid = 0;  /* overload one cluster to make more unfair */
	req.id = M5E_send_dcache_request(req, cid);

	active_addresses.push_back(req.addr);
	requests[req.id] = req;
	requests_this_cycle.push_back(req);
}

int
main()
{
	srand(252352326); /* TODO: set to a real random seed after more testing */

	M5E_init_system(NUM_CLUSTERS);
	M5E_advance_clock();

	for (int i = 0; i < 100000; i++) {


		bool stall = false;
		for (int c = 0; c < NUM_CLUSTERS; c++)
			stall = stall || M5E_is_dcache_request_queue_full(c);

		if (!stall)	
			issue_request();

		if ((rand() % 100) < 30) {
			if (stall)
				stall_cycles++;
			cycles++;
			
			tick_clock();
		}

	}
	fprintf(stderr, "SUCCESS: stall cycles: %d    total cycles: %d\n",
		stall_cycles, cycles);
	
	M5E_destruct_system();
	return 0;
}

