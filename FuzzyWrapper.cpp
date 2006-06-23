#include "FuzzyApprox.h"
//#include "common.h"

using namespace std;
bool CFuzzyFunctionApproximation::Init(REAL _threshold, int _min, int _max, int nouts)
{

  cout << "\n--------------------------------------------------------";
  cout << "\n Fuzzy Function Approximation Initializated";
  cout << "\n--------------------------------------------------------";

  // verifica se la classe è già stata inizializzata e cancella tutto

  //if (GetRules() > 0) Clean();

  OutDim = nouts;
  InDim = 18;

  //  if (!StartUp(10000, _threshold)) return (false);
  if (!StartUp(10000, _threshold,_min,_max)) return (false);
 
  return (true);
}

bool CFuzzyFunctionApproximation::FuzzySetsInit(const vector<pair<int,int> >& min_max) {
    
  
  int nins = min_max.size();

  REAL *InMin = new REAL[nins];
  REAL *InMax = new REAL[nins];
  int *InSets = new int[nins];

  for (int i=0; i<nins; i++)
  {
		InMin[i] = min_max[i].first;
		InMax[i] = min_max[i].second;
		InSets[i] = int(InMax[i]);
  }

  //int InSets[18] = {5,5,2,3,	3,3,3,3,3,	9,4,3,	9,4,3,	9,4,3};
  
  if (!GenerateInputFuzzySets(nins, InSets, InMin, InMax)) return (false);
  delete InMin;
  delete InMax;
  return (true);
}

bool CFuzzyFunctionApproximation::Learn(const Configuration& conf,const Dynamic_stats& dyn)
{
    return false;
}

bool CFuzzyFunctionApproximation::Learn(Configuration conf,Simulation sim,Processor& p,Mem_hierarchy& mem) {
	REAL appoggio[20];
    	appoggio[0] = REAL(p.integer_units.get_pos(conf.integer_units));
	appoggio[1] = REAL(p.float_units.get_pos(conf.float_units));
	appoggio[2] = REAL(p.branch_units.get_pos(conf.branch_units));
	appoggio[3] = REAL(p.memory_units.get_pos(conf.memory_units));
	appoggio[4] = REAL(p.gpr_static_size.get_pos(conf.gpr_static_size));
	appoggio[5] = REAL(p.fpr_static_size.get_pos(conf.fpr_static_size));
	appoggio[6] = REAL(p.pr_static_size.get_pos(conf.pr_static_size));
	appoggio[7] = REAL(p.cr_static_size.get_pos(conf.cr_static_size));
	appoggio[8] = REAL(p.btr_static_size.get_pos(conf.btr_static_size));
	appoggio[9] = REAL(mem.L1D.size.get_pos(conf. L1D_size  ));
	appoggio[10] = REAL(mem.L1D.block_size.get_pos(conf.L1D_block));
	appoggio[11] = REAL(mem.L1D.associativity.get_pos(conf.L1D_assoc));
	appoggio[12] = REAL(mem.L1I.size.get_pos(conf.L1I_size  ));
	appoggio[13] = REAL(mem.L1I.block_size.get_pos(conf.L1I_block));
	appoggio[14] = REAL(mem.L1I.associativity.get_pos(conf.L1I_assoc));
	appoggio[15] = REAL(mem.L2U.size.get_pos(conf.L2U_size  ));
	appoggio[16] = REAL(mem.L2U.block_size.get_pos(conf.L2U_block));
	appoggio[17] = REAL(mem.L2U.associativity.get_pos(conf.L2U_assoc));
	appoggio[18] = REAL(sim.energy); 
	appoggio[19] = REAL(sim.exec_time);
	return (Learn(appoggio,&(appoggio[18])));
}

Simulation CFuzzyFunctionApproximation::Estimate1(Configuration conf,Processor& p,Mem_hierarchy& mem) {
	Simulation sim;
	REAL appoggio[20];
    	appoggio[0] = REAL(p.integer_units.get_pos(conf.integer_units));
	appoggio[1] = REAL(p.float_units.get_pos(conf.float_units));
	appoggio[2] = REAL(p.branch_units.get_pos(conf.branch_units));
	appoggio[3] = REAL(p.memory_units.get_pos(conf.memory_units));
	appoggio[4] = REAL(p.gpr_static_size.get_pos(conf.gpr_static_size));
	appoggio[5] = REAL(p.fpr_static_size.get_pos(conf.fpr_static_size));
	appoggio[6] = REAL(p.pr_static_size.get_pos(conf.pr_static_size));
	appoggio[7] = REAL(p.cr_static_size.get_pos(conf.cr_static_size));
	appoggio[8] = REAL(p.btr_static_size.get_pos(conf.btr_static_size));
	appoggio[9] = REAL(mem.L1D.size.get_pos(conf. L1D_size  ));
	appoggio[10] = REAL(mem.L1D.block_size.get_pos(conf.L1D_block));
	appoggio[11] = REAL(mem.L1D.associativity.get_pos(conf.L1D_assoc));
	appoggio[12] = REAL(mem.L1I.size.get_pos(conf.L1I_size  ));
	appoggio[13] = REAL(mem.L1I.block_size.get_pos(conf.L1I_block));
	appoggio[14] = REAL(mem.L1I.associativity.get_pos(conf.L1I_assoc));
	appoggio[15] = REAL(mem.L2U.size.get_pos(conf.L2U_size  ));
	appoggio[16] = REAL(mem.L2U.block_size.get_pos(conf.L2U_block));
	appoggio[17] = REAL(mem.L2U.associativity.get_pos(conf.L2U_assoc));
	appoggio[18] = 0.0f;
	appoggio[19] = 0.0f;
	EstimateG(appoggio,&(appoggio[18]));
	sim.config = conf;
	sim.area = -1.0f;
	sim.clock_freq = 0.0f;
	sim.energy = double(appoggio[18]);
	sim.exec_time = double(appoggio[19]);
	sim.simulated = false;
	//cout << "\n-----------Estimate 1 : " << sim.energy << " __ " << sim.exec_time;
	return (sim);
	}

Dynamic_stats CFuzzyFunctionApproximation::Estimate2(Configuration conf) {
    Dynamic_stats dyn;
    return dyn;
}
