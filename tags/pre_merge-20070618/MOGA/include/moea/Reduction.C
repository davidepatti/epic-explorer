#include "Reduction.h"

ClusteringReducer :: ClusteringReducer(int chunkSize) : 
  N(chunkSize), chunkSz(chunkSize), Reducer() {
  clusters = new int [N];   
  clusterList = new int [N];      
  selectedSolutions = new int [N];
}

void  ClusteringReducer :: reduce(POP& pop, int maxSz) {
  int curSz = pop.currentSize();
  if(curSz > N) {
    while(curSz > N) N += chunkSz;
    delete[] clusters; delete clusterList; delete[] selectedSolutions;
    clusters = new int [N];   
    clusterList = new int [N];      
    selectedSolutions = new int [N];
  }

  int  numOfClusters = curSz;  //define one-ele-clusters.
  for (int  i = 0 ; i < numOfClusters; i++)
    clusters[i] = i, clusterList[i] = -1;

  /* join clusters using average linkage method */
  while (numOfClusters > maxSz) {
    /* find cluster pair with minimal distance */
    double  minDist = DOUBLE_LARGE;
    int     join1, join2;
    for (int  c = 0; c < numOfClusters; c++) {
      for (int  d = c + 1; d < numOfClusters; d++) {
	double  dist = _ClusterDistance(c, d, pop);
	if (dist < minDist) {
	  minDist = dist;
	  join1 = c, join2 = d;
	}
      }
    }
    _JoinClusters(join1, join2, numOfClusters);
  }

  for (int i = 0; i < curSz; i++)
    selectedSolutions[i] = false;
  for (int i = 0; i < numOfClusters; i++)
    selectedSolutions[_ClusterCentroid(i, pop)] = true;

  // del unselected & move the selected to be continous in position.
  int idx_r = curSz-1;  // current rightmost index.
  int idx_l = 0;        // curren leftmost index which should be deleted.
  for(int deleted=0; deleted < curSz-numOfClusters; deleted++) {
    if(!selectedSolutions[idx_r]) pop.del(idx_r);
    else {
      while(selectedSolutions[idx_l]) idx_l++;
      pop.replace(idx_l, pop.remove(idx_r));
    }
    --idx_r;
  }
}


double ClusteringReducer :: _ClusterDistance(int  cluster1, int  cluster2, const POP& pop) {
  double  sum = 0;
  int  numOfPairs = 0;
  int  c1 = clusters[cluster1];
  while (c1 >= 0) {
    int  c2 = clusters[cluster2];
    while (c2 >= 0) {
      sum += pop[c1]->distance(*pop[c2]);
      numOfPairs++;
      c2 = clusterList[c2];
    }
    c1 = clusterList[c1];
  }
  return (sum / double(numOfPairs));
} /* ClusterDistance */


void ClusteringReducer ::  _JoinClusters(int  cluster1, int  cluster2, int&  numOfClusters) {
  int  c1 = clusters[cluster1];
  while (clusterList[c1] >= 0)
    c1 = clusterList[c1];
  clusterList[c1] = clusters[cluster2];
  numOfClusters--;
  clusters[cluster2] = clusters[numOfClusters]; //move last list to the deleted position.
} /* JoinClusters */

 
int ClusteringReducer :: _ClusterCentroid(int  cluster, const POP& pop) {
  double  minSum = DOUBLE_LARGE;  int  minIndex;
  for(int c1=clusters[cluster]; c1 >= 0; c1=clusterList[c1]) {
    double  sum = 0;
    for(int c2=clusters[cluster]; c2 >= 0; c2=clusterList[c2])
      sum += pop[c1]->distance(*pop[c2]);
    if (sum < minSum)
      minSum = sum, minIndex = c1;
  }
  return minIndex;
}

