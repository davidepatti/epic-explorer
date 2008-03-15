/* File:     req_by_cluster.cc
 * Author:   Steve Lieberman <lieberm@umich.edu>
 * Project:  CCCP - M5elements Cache Simulator
 * Date:     March 2005
 * Descr:    M5elements is a library-based interface to M5's memory system
 *           to allow its incorporation into SIMU or other simulators.
 */

#include "m5elements/req_by_cluster.hh"

long nextReqId = 0;
vector<M5E_Cluster*> m5eClusters;




