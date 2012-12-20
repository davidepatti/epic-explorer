
// ********************************************************************
//   GA-based exploration
// ********************************************************************

//******************************************************************************************//
#include "explorer.h"
#include "common.h"
#include "paramspace.h"
#include "parameter.h"
#include <values.h>

using namespace std;

// utilities 
// TBD: what to put inside/outside Region class ??
bool check_interval_contiguity(const Edges& e1, const Edges& e2,const Parameter& p)
{
    // Parameter::get_interval(....)
    return false;
}

int get_splitting_direction(const Region& r)
{
    // 

    // Parameter::get_index();
    // (internamente associa ad una label un indice, poi utilizza
    // per selezione l'Edge

    // N_PARAMS = numero totale di parametri
    return 0;
}



// put outside for coherence with merge_regions() ?
vector<Region> Region::split_region()
{
    // we just want to split in 2, but leaving vector for general case

    Region r1,r2;
    vector<Region> regions;

    int dir = get_splitting_direction(*this);

    // use direction in some way....

    regions.push_back(r1);
    regions.push_back(r2);

    return regions;
}

Region merge_regions(const Region& r1,const Region& r2)
{
    // TBD: what returns when merging is not possible ?
    Region r;

    // return a region with valid=false if not possible merge
    return r;
}

double separation(const Simulation& s1, const Simulation& s2)
{
    return 0.0;
}

double get_innovation_score(const Simulation& s, vector<Simulation> pareto)
{
    return 0.0;
}

double get_innovation_score(const Region& r,vector<Simulation> pareto)
{
    return 0.0;
}

void sort_by_innovation(vector<Region> * regions)
{
    // actually modifies the main set of regions...
}

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
    vector<Region> regions;


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
	write_to_log(myrank,logfile,"Era " + era);

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

