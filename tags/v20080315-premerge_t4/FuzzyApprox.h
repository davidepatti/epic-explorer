//Fuzzy Function Approximation
#ifndef FuzzyApprox_H
#define FuzzyApprox_H
#include <vector>
#include "common.h"
#include "processor.h"
#include "mem_hierarchy.h"
#include "RuleList.h"
#include "FunctionApprox.h"

#include <time.h>

#define ERR_MEMORY 20

class CFuzzyFunctionApproximation : public CFunctionApproximation {

public:
  CFuzzyFunctionApproximation();
  ~CFuzzyFunctionApproximation();

  bool Init(double _threshold, int min, int max, int nouts);
  
  bool FuzzySetsInit(const vector<pair<int,int> >& min_max);
  bool StartUp(int MaxNumberOfRules, double threshold, int _min, int _max);
 
  bool Learn(double* InputValues, double* OutputValues);
  bool Learn(Configuration conf,Simulation sim,Processor& p,Mem_hierarchy& mem);
  bool Learn(const Configuration& conf,const Dynamic_stats& dyn);
  bool GenerateInputFuzzySets(int Dimensionality, int* InputFuzzySetsNumberVector, double *InMinimumValuesVector, double *InMaximumValuesVector);
  bool EstimateG(double* InputValues,double* OutputVector);
  Simulation Estimate1(Configuration conf,Processor& p,Mem_hierarchy& mem);
  Dynamic_stats Estimate2(Configuration conf);
  bool Reliable();
  int GetSystem();
  void Clean();

private:
  int position();
  int InDim,OutDim;
  int *InputSetsNumber;
  double *InputsMin;
  double *InputsMax;
  double *InputCenters;
  CRuleList *RuleTable;
  Rule newRule;
  double *estimatedValues;
  double *degrees;
  double *alpha;
  int *Sets;
  int count;
  bool calcola;
  double *errore;
  double **errmatrix;
  int prove;
  double *stima;
  double threshold;
  int min_sims,max_sims;
  int posx;
  FILE* fuzzy_log;
  FILE* fuzzy_error;
};
#endif
