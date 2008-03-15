/* File:     req_by_cluster.hh
 * Author:   Steve Lieberman <lieberm@umich.edu>
 * Project:  CCCP - M5elements Cache Simulator
 * Date:     March 2005
 * Descr:    M5elements is a library-based interface to M5's memory system
 *           to allow its incorporation into SIMU or other simulators.
 */

#ifndef _REQ_BY_CLUSTER_HH
#define _REQ_BY_CLUSTER_HH

#include <vector>
#include "m5elements/mem_types.hh"
#include "m5elements/m5e_cluster.hh"

using namespace std;

extern long nextReqId;
extern vector<M5E_Cluster*> m5eClusters;

#endif /* _REQ_BY_CLUSTER_HH */


