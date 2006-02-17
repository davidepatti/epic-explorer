//Fuzzy Function Approximation
#ifndef FuzzyApprox_H
#define FuzzyApprox_H
#include <vector>
#include "common.h"
#include "RuleList.h"

using namespace std;

class CFuzzyFunctionApproximation {

public:
  CFuzzyFunctionApproximation() {};
  ~CFuzzyFunctionApproximation();

  bool Init(REAL _threshold, const vector<pair<int,int> >& min_max, int nouts);

  bool StartUp(int MaxNumberOfRules, float threshold);

  bool Learn(REAL* InputValues, REAL* OutputValues);
  bool Learn(Configuration conf,Simulation sim);
  bool Learn(const Configuration& conf,const Dynamic_stats& dyn);
  bool GenerateInputFuzzySets(int Dimensionality, int* InputFuzzySetsNumberVector, REAL *InMinimumValuesVector, REAL *InMaximumValuesVector);
  bool EstimateG(REAL* InputValues,REAL* OutputVector);
  //Simulation EstimateG(Configuration conf);
  Dynamic_stats EstimateG(Configuration conf);
  bool Reliable();
  int GetRules();
  void Clean();

private:
  int InDim,OutDim;
  int *InputSetsNumber;
  REAL *appoggio;
  REAL *InputsMin;
  REAL *InputsMax;
  REAL *InputCenters;
  CRuleList *RuleTable;
  Rule newRule;
  REAL *estimatedValues;
  REAL *degrees;
  REAL *alpha;
  int *Sets;
  int count;
  bool calcola;
  REAL *errore;
  double *errmedio;
  int prove;
  REAL *stima;
  REAL threshold;
};
#endif
