#ifndef _REDUCTION_H
#define _REDUCTION_H

#include "Population.h"

using namespace std;

class Reducer {
  friend class MOEA;
 public:
  MOEADefineIdentity("Reducer", "R", 500);

  Reducer() : ppop1(NULL), ppop2(NULL) { }
  virtual ~Reducer() { }

  virtual void initialize(POP* p1, int sz1, POP* p2=0, int sz2=0) { 
    ppop1=p1; ppop2=p2; pop1_maxSz = sz1; pop2_maxSz = sz2;
  } 

  virtual void reduce() = 0;

 protected:
  POP* ppop1;
  POP* ppop2;
  int pop1_maxSz;   // population 1 allowed size.
  int pop2_maxSz;   // population 2 allowed size.
};



class ClusteringReducer : public Reducer{
 public:
  MOEADefineIdentity("ClusteringReducer", "cR", 501);

  ClusteringReducer(int chunkSize = 100);
  ~ClusteringReducer() {delete[] clusters;delete[] clusterList;delete[] selectedSolutions; }

  virtual void reduce() { 
    if(ppop1 && ppop1->currentSize() > pop1_maxSz) 
      reduce(*ppop1, pop1_maxSz); 
    if(ppop2 && ppop2->currentSize() > pop2_maxSz) 
      reduce(*ppop2, pop2_maxSz); 
  }

 private:
  void reduce(POP& pop, int maxSz);

 private:
  double _ClusterDistance(int  cluster1, int  cluster2, const POP& pop);
  void _JoinClusters(int  cluster1, int  cluster2, int&  numOfClusters);
  int  _ClusterCentroid(int  cluster, const POP& pop);

 private:
  int    N;
  int    chunkSz;

  int*   clusters;      
  int*   clusterList;   
  int*   selectedSolutions;  
};
#endif
