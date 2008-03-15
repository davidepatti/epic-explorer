
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "m5elements/cache.hh"

int main()
{
  	int present;
	int i;

	M5E_init_system(4);
	M5E_advance_clock();
	
	M5E_mem_req req;
	req.type = M5E_STORE;
	req.addr = 0xFEEDBEEF;
	req.data = 0xDEADDEED;
	req.is_valid = 1;
	req.size = 4;
	req.id = 5000;
	req.register_id = -313;

	req.type = M5E_LOAD;
	req.addr = 0xFADE0000;
	req.id = 4999;
	req.register_id = 780;

	present = M5E_is_present_in_icache(req, 0);
	printf("(1) is_present? %d\n", present);

	req.type = M5E_STORE;
	req.addr = 0xFEEDBEEF;
	req.data = 0xDEADDEED;
	req.register_id = -313;

	M5E_send_dcache_request(req, 2);

	long* larr = NULL;

	do {
		M5E_advance_clock();
		larr = M5E_ready_requests();
	} while ((*larr) == -1);

	for (i = 0; i < 100; i++) {
	  M5E_advance_clock();
	  larr = M5E_ready_requests();
	}

	req.type = M5E_LOAD;
	req.data = -1;
	req.id = 5001;
	req.register_id = 711;

	M5E_send_icache_request(req, 3);

	int* arr = NULL;

	do {
		M5E_advance_clock();
		arr = M5E_ready_loads();
		printf("outstanding_requests: %d\n", 
		    M5E_get_outstanding_requests());
	} while ((*arr) == -1);

	printf("done with data request: register: %d\n", *arr);

	req.type = M5E_LOAD;
	req.addr = 0xFEEDBEEF;
	req.id = 5002;
	req.register_id = 750;

#if 0
	for (i = 0; i < 1000; i++)
	  M5E_advance_clock();
#endif

	present = M5E_is_present_in_icache(req, 3);
	printf("(2) is_present? %d\n", present);

	M5E_destruct_system();
	return 0;
}
