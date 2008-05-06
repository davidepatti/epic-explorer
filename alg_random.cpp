#include "explorer.h"
#include "common.h"
//********************************************************************
// N Random  explorations of configuration space
//*******************************************************************
void Explorer::start_RAND(int n)
{

    current_algo = "RAND";
    vector<Configuration> random_space;

    int valid = 0;
    reset_sim_counter();

    cout << "\n Building random space for " << n << " simulations...";

    for(int i=0;i<n;i++)
    {
	processor.num_clusters.set_random();
	processor.integer_units.set_random();
	processor.float_units.set_random();
	processor.branch_units.set_random();
	processor.memory_units.set_random();
	processor.gpr_static_size.set_random();
	processor.fpr_static_size.set_random();
	processor.pr_static_size.set_random();
	processor.cr_static_size.set_random();
	processor.btr_static_size.set_random();
	mem_hierarchy.L1D.size.set_random();
	mem_hierarchy.L1D.block_size.set_random();
	mem_hierarchy.L1D.associativity.set_random();
	mem_hierarchy.L1I.size.set_random();
	mem_hierarchy.L1I.block_size.set_random();
	mem_hierarchy.L1I.associativity.set_random();
	mem_hierarchy.L2U.size.set_random();
	mem_hierarchy.L2U.block_size.set_random();
	mem_hierarchy.L2U.associativity.set_random();

	compiler.tcc_region.set_random();	//db
        compiler.max_unroll_allowed.set_random();	 	//db
        compiler.regroup_only.set_random();	 	//db
    	compiler.do_classic_opti.set_random();	 	//db
    	compiler.do_prepass_scalar_scheduling.set_random();	 	//db
    	compiler.do_postpass_scalar_scheduling.set_random();	 	//db
    	compiler.do_modulo_scheduling.set_random();	 	//db
    	compiler.memvr_profiled.set_random();	 	//db

	Configuration temp_conf = create_configuration(processor,mem_hierarchy,compiler);

	if (mem_hierarchy.test_valid_config())
	{
	    valid++;
	    random_space.push_back(temp_conf);
	}
    }

    cout << "\n Ok, created random space " ;
    cout << "\n\n Valid configurations:" << valid << " of " << n << " requested";


    vector<Simulation> rand_sims = simulate_space(random_space);
    vector<Simulation> pareto_set = get_pareto(rand_sims);

    save_simulations(rand_sims,Options.benchmark+"_RAND_"+current_space+".exp");
    save_simulations(pareto_set,Options.benchmark+"_RAND_"+current_space+".pareto.exp");
    
}
