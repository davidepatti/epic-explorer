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
  bool FuzzySetsInit(const vector<pair<int,int> >& min_max) {};
  bool Reliable();
  int GetSystem();
  void Clean();

private:
  fann_type *calc_out;
  unsigned int num_input;
  unsigned int num_output;
  unsigned int num_layers;
  float desired_error;
  unsigned int max_epochs;
  unsigned int epochs_between_reports;
  struct fann *ann;
  struct fann_train_data *train_data, *test_data;
  int min_sims, max_sims;
  unsigned int simulations_done;
  unsigned int train_,test_;
  FILE *train_file, *test_file, *log_file;
  int i;
  bool trained;

};
#endif
