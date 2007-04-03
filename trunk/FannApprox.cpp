// Artificial Neural Network Approximation 
#include "FannApprox.h"

  CAnnFunctionApproximation::CAnnFunctionApproximation() {};
  CAnnFunctionApproximation::~CAnnFunctionApproximation() {};

  bool CAnnFunctionApproximation::Init(REAL _threshold, int min, int max, int nouts) {};
  bool CAnnFunctionApproximation::StartUp(int MaxNumberOfEpochs, REAL threshold, int _min, int _max) {};
  bool CAnnFunctionApproximation::Learn(Configuration conf,Simulation sim,Processor& p,Mem_hierarchy& mem) {};
  bool CAnnFunctionApproximation::Learn(const Configuration& conf,const Dynamic_stats& dyn) {};
  Simulation CAnnFunctionApproximation::Estimate1(Configuration conf,Processor& p,Mem_hierarchy& mem) {};
  Dynamic_stats CAnnFunctionApproximation::Estimate2(Configuration conf) {};
  bool CAnnFunctionApproximation::Reliable() {};
  int CAnnFunctionApproximation::GetRules() {};
  void CAnnFunctionApproximation::Clean() {};

