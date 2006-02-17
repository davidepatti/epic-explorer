#include "FuzzyApprox.h"

using namespace std;
bool CFuzzyFunctionApproximation::Init(REAL _threshold, 
				       const vector<pair<int,int> >& min_max, 
				       int nouts)
{
  // verifica se la classe è già stata inizializzata e cancella tutto

  int nins = min_max.size();

  REAL *InMin = new REAL[nins];
  REAL *InMax = new REAL[nins];

  for (int i=0; i<nins; i++)
	{
		InMin[i] = min_max[i].first;
		InMax[i] = min_max[i].second;
	}

  int InSets[18] = {5,5,5,5,	3,3,3,3,3,	9,4,3,	9,4,3,	9,4,3};
  
  
  //if (GetRules() > 0) Clean();

  if (!GenerateInputFuzzySets(nins, InSets, InMin, InMax)) return (false);

  OutDim = nouts;

  //  if (!StartUp(10000, _threshold)) return (false);
  if (!StartUp(10000, 5)) return (false);

  return (true);
}

bool CFuzzyFunctionApproximation::Learn(const Configuration& conf,const Dynamic_stats& dyn)
{
}

bool CFuzzyFunctionApproximation::Learn(Configuration conf,Simulation sim) {
	appoggio[0] = REAL(conf.integer_units);
	appoggio[1] = REAL(conf.float_units);
	appoggio[2] = REAL(conf.branch_units);
	appoggio[3] = REAL(conf.memory_units);
	appoggio[4] = REAL(conf.gpr_static_size);
	appoggio[5] = REAL(conf.fpr_static_size);
	appoggio[6] = REAL(conf.pr_static_size);
	appoggio[7] = REAL(conf.cr_static_size);
	appoggio[8] = REAL(conf.btr_static_size);
	appoggio[9] = REAL(conf.L1D_size);
	appoggio[10] = REAL(conf.L1D_block);
	appoggio[11] = REAL(conf.L1D_assoc);
	appoggio[12] = REAL(conf.L1I_size);
	appoggio[13] = REAL(conf.L1I_block);
	appoggio[14] = REAL(conf.L1I_assoc);
	appoggio[15] = REAL(conf.L2U_size);
	appoggio[16] = REAL(conf.L2U_block);
	appoggio[17] = REAL(conf.L2U_assoc);
	appoggio[18] = REAL(sim.energy); //energy?
	appoggio[19] = REAL(sim.exec_time);
	return (Learn(appoggio,&(appoggio[18])));
}

Dynamic_stats CFuzzyFunctionApproximation::EstimateG(Configuration conf) {
    Dynamic_stats dyn;
    /*
	Simulation sim;
	appoggio[0] = REAL(conf.integer_units);
	appoggio[1] = REAL(conf.float_units);
	appoggio[2] = REAL(conf.branch_units);
	appoggio[3] = REAL(conf.memory_units);
	appoggio[4] = REAL(conf.gpr_static_size);
	appoggio[5] = REAL(conf.fpr_static_size);
	appoggio[6] = REAL(conf.pr_static_size);
	appoggio[7] = REAL(conf.cr_static_size);
	appoggio[8] = REAL(conf.btr_static_size);
	appoggio[9] = REAL(conf.L1D_size);
	appoggio[10] = REAL(conf.L1D_block);
	appoggio[11] = REAL(conf.L1D_assoc);
	appoggio[12] = REAL(conf.L1I_size);
	appoggio[13] = REAL(conf.L1I_block);
	appoggio[14] = REAL(conf.L1I_assoc);
	appoggio[15] = REAL(conf.L2U_size);
	appoggio[16] = REAL(conf.L2U_block);
	appoggio[17] = REAL(conf.L2U_assoc);
	appoggio[18] = 0.0f;
	appoggio[19] = 0.0f;
	EstimateG(appoggio,&(appoggio[18]));
	sim.config.branch_units = conf.branch_units;
	sim.config.btr_static_size = conf.btr_static_size;
	sim.config.cr_static_size = conf.cr_static_size;
	sim.config.float_units = conf.float_units;
	sim.config.fpr_static_size = conf.fpr_static_size;
	sim.config.gpr_static_size = conf.gpr_static_size;
	sim.config.integer_units = conf.integer_units;
	sim.config.L1D_assoc = conf.L1D_assoc;
	sim.config.L1D_block = conf.L1D_block;
	sim.config.L1D_size = conf.L1D_size;
	sim.config.L1I_assoc = conf.L1I_assoc;
	sim.config.L1I_block = conf.L1I_block;
	sim.config.L1I_size = conf.L1I_size;
	sim.config.L2U_assoc = conf.L2U_assoc;
	sim.config.L2U_block = conf.L2U_block;
	sim.config.L2U_size = conf.L2U_size;
	sim.config.memory_units = conf.memory_units;
	sim.config.pr_static_size = conf.pr_static_size;
	sim.area = -1.0f;
	sim.clock_freq = -1.0f;
	sim.energy = double(appoggio[18]);
	sim.exec_time = double(appoggio[19]);
	return (sim);
	*/
    return dyn;
}
