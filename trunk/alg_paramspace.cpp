
// ********************************************************************
//   GA-based exploration
// ********************************************************************

//******************************************************************************************//
#include "explorer.h"
#include "common.h"
#include "paramspace.h"
#include "parameter.h"
#include <values.h>
#include <list>
#include <cstdlib> //for rand()

using namespace std;


string logfile;
int myrank;

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

//TODO: si potrebbe evitare di passare expl se getParameterRanges fosse statico
void transform_to_root_region(Region* r, Explorer* expl)
{
	vector<pair<int,int> > ranges = expl->getParameterRanges();
	
	if (ranges.size() != N_PARAMS)
	{
		string message = "alg_paramspace.cpp:"+ to_string(__LINE__) +": ERROR: N_PARAMS=" + 
			to_string(N_PARAMS) + " whereas getParameterRanges()"
			+" has returned "+ to_string(ranges.size() ) + "elements";
		write_to_log(myrank,logfile,message);
		return;
	}
	
	for(int i=0; i<N_PARAMS; i++)
	{
		Parameter par = expl->getParameter( (EParameterType)i );
		Edges e; e.a=par.get_first(); e.b=par.get_last();
		r->edges[i] = e;
	}
	return;
}

Region* select_region(vector<Region*> regions, double total_innovation_score)
{
	double r =  (double)rand()/(RAND_MAX);
    double random_point = (double)(r*total_innovation_score);
    double partial_innovation_score;
    
    int i; for (i=0; partial_innovation_score < random_point; i++)
    	partial_innovation_score += regions[i]->innovation_score;
    
    string message = "alg_paramspace.cpp: region " + to_string(i) + " is selected";
    write_to_log(myrank,logfile,message);	
    
    return regions[i];
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
    myrank = get_mpi_rank();
    assert(myrank == 0);

    // setup some not-algorithm-related stuff before starting
    current_algo="PARAMSPACE";
    logfile = get_base_dir()+string(EE_LOG_PATH);

    Exploration_stats stats;
    stats.space_size = get_space_size();
    stats.start_time = time(NULL);
    reset_sim_counter();

    string file_name;

    file_name = Options.benchmark+"_"+current_algo+"_"+current_space;

    string header = "["+Options.benchmark+"_"+current_algo+"] ";
    string message = header+"Building random space for " + to_string(k) + " simulations...";
    write_to_log(myrank,logfile,message);
    
    
    
    //DA QUI COMINCIA
    int era = 0;
    int valid = 0 ;
    
    //The sum of the innovation_score of all regions
    double total_innovation_score=0;
    
    vector<Region*> regions;
    Region* root_region = new Region();
    transform_to_root_region(root_region, this);
    regions.push_back(root_region);
    
    // create a space of k randomly chosen configurations
    vector<Configuration> configs_to_simulate;

    
    
    // Use the budget of k simulations
    for(int i=0;i<k;i++)
    {
    	Region* selected_region = select_region(regions, total_innovation_score);
    	Configuration temp_conf;
    	
    	for (int j=0; j<N_PARAMS; j++)
    	{
    		Parameter par = getParameter( (EParameterType)j );
    		Edges edges = (selected_region->edges)[j];
    		par.set_random(edges.a, edges.b );
    		fix_parameter(&temp_conf, (EParameterType)j, par.get_val() );
    	}
    	//Now all parameters of temp_conf are fixed.

		if (temp_conf.is_feasible())
		{
			valid++;
			configs_to_simulate.push_back(temp_conf);
		}
    	
    }


    message = header+ "Valid configurations:" + to_string(valid) + " of "+to_string(k)+" requested";
    write_to_log(myrank,logfile,message);



    while ( era++ < max_eras )
    {
		write_to_log(myrank,logfile,"Era " + era);

		vector<Simulation> current_sims = simulate_space(configs_to_simulate);
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

