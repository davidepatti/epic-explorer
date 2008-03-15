
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "m5elements/cache.hh"
#include "m5elements/mem_types.hh"


void
usage(char* argv0)
{
	printf("usage: %s num_clusters num_cycles req_per_cycle\n",
		argv0);
	exit(1);
}


inline int
any_cluster_full(int num_clusters)
{
	int i;
	int ret_val = 0;

	for (i = 0; i < num_clusters; i++)
		ret_val |= M5E_is_dcache_request_queue_full(i);

	return ret_val;
}

int
main(int argc, char** argv)
{
	int num_clusters;
	int num_cycles;
	int req_per_cycle;
	int actual_cycles = 0;
	int actual_requests = 0;
	int c;
	int r;
	int req_this_cycle;

	if (argc != 4)
		usage(argv[0]);

	num_clusters  = atoi(argv[1]);
	num_cycles    = atoi(argv[2]);
	req_per_cycle = atoi(argv[3]);	

	if (num_clusters <= 0 || num_cycles <= 0 || req_per_cycle <= 0)
		usage(argv[0]);

	printf("speed_test_init, clusters, %d, cycles, %d, req_per_cycle, %d\n",
		num_clusters, num_cycles, req_per_cycle); 

	M5E_init_system(num_clusters);	

	for (c = 0; c < num_cycles; c++) {

		M5E_ready_loads();    /* speed of exec includes this */
		M5E_advance_clock();
		actual_cycles++;

		req_this_cycle = rand() % (2 * req_per_cycle);

		for (r = 0; r < req_this_cycle; r++) {
			M5E_mem_req req;			
			int cid = rand() % num_clusters;

			if (any_cluster_full(num_clusters)) {
				break;
			}

			/* must be a real request to actually test sys */
			req.type = (rand() % 2) ? M5E_LOAD : M5E_STORE;
			req.data = rand();
			req.size = 4;
			req.is_valid = 1;
			req.addr = rand(); /* TODO: should do this better */

			M5E_send_dcache_request(req, cid);	
			actual_requests++;
		}
	}

	while (M5E_get_outstanding_requests()) {
	  M5E_ready_loads();    /* speed of exec includes this */
	  M5E_advance_clock();
	  actual_cycles++;
	}



	printf("speed_test_complete, actual_cycles, %d, actual_requests, %d\n",
		actual_cycles, actual_requests); 

	M5E_destruct_system();
	return 0;
}

