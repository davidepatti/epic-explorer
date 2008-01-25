//Virtual Class Function Approximation
#ifndef FunctionApprox_H
#define FunctionApprox_H
#include <vector>
#include "common.h"
#include "processor.h"
#include "mem_hierarchy.h"

#define ERR_MEMORY 20

class CFunctionApproximation {

public:
  //CFunctionApproximation();
  ~CFunctionApproximation() {};

  virtual bool Init(double _threshold, int min, int max, int nouts) = 0;
  virtual bool StartUp(int MaxNumberOf_, double threshold, int _min, int _max) = 0;
  virtual bool Learn(Configuration conf,Simulation sim,Processor& p,Mem_hierarchy& mem) = 0;
  virtual bool Learn(const Configuration& conf,const Dynamic_stats& dyn) = 0;
  virtual Simulation Estimate1(Configuration conf,Processor& p,Mem_hierarchy& mem) = 0;
  virtual Dynamic_stats Estimate2(Configuration conf) = 0;
  virtual bool Reliable() = 0;
  virtual int GetSystem() = 0;
  virtual void Clean() = 0;
  
  // For fuzzy only
  virtual bool FuzzySetsInit(const vector<pair<int,int> >& min_max) = 0;

};
#endif
