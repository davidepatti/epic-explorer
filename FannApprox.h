//ANN Function Approximation
#ifndef AnnFunctionApprox_H
#define AnnFunctionApprox_H
#include <vector>
#include "common.h"
#include "processor.h"
#include "mem_hierarchy.h"
#include "FunctionApprox.h"
#include "fann.h"
#define ERR_MEMORY 20

class CAnnFunctionApproximation : public CFunctionApproximation{

public:
  CAnnFunctionApproximation();
  ~CAnnFunctionApproximation();

  bool Init(REAL _threshold, int min, int max, int nouts);
  bool StartUp(int MaxNumberOfEpochs, REAL threshold, int _min, int _max);
  bool Learn(Configuration conf,Simulation sim,Processor& p,Mem_hierarchy& mem);
  bool Learn(const Configuration& conf,const Dynamic_stats& dyn);
  Simulation Estimate1(Configuration conf,Processor& p,Mem_hierarchy& mem);
  Dynamic_stats Estimate2(Configuration conf);
  bool Reliable();
  int GetRules();
  void Clean();

};
#endif
