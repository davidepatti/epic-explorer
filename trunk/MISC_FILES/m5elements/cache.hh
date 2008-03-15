/* File:     cache.hh
 * Author:   Steve Lieberman <lieberm@umich.edu>
 * Project:  CCCP - M5elements Cache Simulator
 * Date:     February-March 2005
 * Descr:    M5elements is a library-based interface to M5's memory system
 *           to allow its incorporation into SIMU or other simulators.
 */

#ifndef _M5E_CACHE_HH
#define _M5E_CACHE_HH

#include "m5elements/mem_types.hh"

#ifdef __cplusplus
extern "C" {
#endif

  extern void M5E_init_system(int num_clusters);
  extern int M5E_is_dcache_request_queue_full(int cid);
  extern int M5E_is_present_in_icache(M5E_mem_req req, int cid); 
  extern int M5E_probe_dcache(M5E_mem_req req, int cid);
  extern long M5E_send_icache_request(M5E_mem_req req, int cid);
  extern long M5E_send_dcache_request(M5E_mem_req req, int cid);

  /* M5E_advance_clock()
   *      moves the M5 clock forward to the next cycle. note this also clears
   *      the list of requests that completed this cycle.
   */
  extern void M5E_advance_clock();

  /* M5E_ready_loads()
   *      returns an array of register ids indicating the loads that *just*
   *      completed in the current cycle, terminated by the id -1.
   *
   *      returns both I-cache and D-cache requests.
   */
  extern int* M5E_ready_loads();

  /* M5E_ready_requests()
   *      returns an array of request ids of all requests that *just*
   *      completed in the current cycle, terminated by the id -1.
   *
   *      returns both I-cache and D-cache requests.
   */
  extern long* M5E_ready_requests();

  extern M5E_mem_req* M5E_ready_requests_as_obj();
  extern void M5E_debug_dump(M5E_addr addr);
  extern void M5E_destruct_system();

  extern int M5E_get_outstanding_requests();

#ifdef __cplusplus
}
#endif

#endif /* _M5E_CACHE_HH */
