
// ********************************************************************
//   GA-based exploration
// ********************************************************************

//******************************************************************************************//
#include "explorer.h"
#include "common.h"
#include "paramspace.h"
#include <values.h>

void Explorer::start_PARAMSPACE(double alpha, int k, int max_eras)
{
    // mpi parallel implementation currently unsupported on PARAMSPACE
    int myrank = get_mpi_rank();
    assert(myrank == 0);

    // setup some not-algorithm-related stuff before starting
    current_algo="PARAMSPACE";
    string logfile = get_base_dir()+string(EE_LOG_PATH);

    Exploration_stats stats;
    stats.space_size = get_space_size();
    stats.start_time = time(NULL);
    reset_sim_counter();

    string file_name;

    file_name = Options.benchmark+"_"+current_algo+"_"+current_space;

    string header = "["+Options.benchmark+"_"+current_algo+"] ";
    string message = header+"Building random space for " + to_string(k) + " simulations...";
    write_to_log(myrank,logfile,message);


    int era = 0;

    // create a space of k randomly chosen configurations
    vector<Configuration> initial_space;

    int valid = 0 ;

    for(int i=0;i<k;i++)
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

	compiler.tcc_region.set_random();
        compiler.max_unroll_allowed.set_random();
        compiler.regroup_only.set_random();	
    	compiler.do_classic_opti.set_random();
    	compiler.do_prepass_scalar_scheduling.set_random();
    	compiler.do_postpass_scalar_scheduling.set_random();
    	compiler.do_modulo_scheduling.set_random();	 
    	compiler.memvr_profiled.set_random();	

	Configuration temp_conf = create_configuration(processor,mem_hierarchy,compiler);

	if (temp_conf.is_feasible())
	{
	    valid++;
	    initial_space.push_back(temp_conf);
	}
    }

    message = header+ "Valid configurations:" + to_string(valid) + " of "+to_string(k)+" requested";
    write_to_log(myrank,logfile,message);



    while ( era++ < max_eras )
    {
	write_to_log(myrank,logfile,"Iteration " + era);

	vector<Simulation> current_sims = simulate_space(initial_space);
	vector<Simulation> pareto_set = get_pareto(current_sims);

	char temp[30];
	sprintf(temp, "_%d", era);

	save_simulations(current_sims,Options.benchmark+"_PARAMSPACE"+current_space+"_gen"+string(temp)+".exp");
	save_simulations(pareto_set,Options.benchmark+"_PARAMSPACE"+current_space+"_gen"+string(temp)+".pareto.exp");
    }

    // save statistics
    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats, file_name+".stat");
}



/*************************************************************************/
