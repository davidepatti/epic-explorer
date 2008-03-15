#include "explorer.h"
#include "common.h"
#include <ctime>
// ********************************************************************
//  mono-objective sensivity based approach
// ********************************************************************

void Explorer::start_SAP()
{
    current_algo = "SAP";
    Exploration_stats stats;
    reset_sim_counter();
    stats.space_size = get_space_size();
    stats.feasible_size = get_feasible_size();
    stats.start_time = time(NULL);

    int n_par = 18;

    string file_name = Options.benchmark+"_SAP_"+current_space;
    vector<double> sens;
    vector<int> sorted_index;

    Configuration base_conf = create_configuration();

    vector<Space_mask> parameter_masks;

    // for each parameter is created a mask with only one value set to
    // true. These masks will allow to explore all values of each
    // parameter without changing the other
    for (int m=0;m<n_par;m++)
    {
	vector<bool> boolean_mask(n_par,false);
	boolean_mask[m] = true;
	Space_mask temp_mask = create_space_mask(boolean_mask);
	parameter_masks.push_back(temp_mask);

	// note NO_L2_CHECK option, cache size parameters should
	// be considered in all their range while testing sensivity
	vector<Configuration> space = build_space(temp_mask,base_conf);

	vector<Simulation> sims = simulate_space(space);

	double temp = get_sensivity_energydelay(sims);

	sens.push_back(temp);
    }

    string path = get_base_dir()+"/trimaran-workspace/epic-explorer/";
    string name;
    if (Options.hyperblock) name = "H_";
    name = path + name + Options.benchmark + "_SAP_"+current_space+"_sensitivity.stat";
    FILE * fp = fopen(name.c_str(),"w");

    fprintf(fp,"\n%.14f        %% L1D_size",sens[0]);        
    fprintf(fp,"\n%.14f        %% L1D_block",sens[1]);       
    fprintf(fp,"\n%.14f        %% L1D_assoc",sens[2]);       
    fprintf(fp,"\n%.14f        %% L1I_size",sens[3]);        
    fprintf(fp,"\n%.14f        %% L1I_block",sens[4]);       
    fprintf(fp,"\n%.14f        %% L1I_assoc",sens[5]);       
    fprintf(fp,"\n%.14f        %% L2U_size",sens[6]);        
    fprintf(fp,"\n%.14f        %% L2U_block",sens[7]);       
    fprintf(fp,"\n%.14f        %% L2U_assoc",sens[8]);       
    fprintf(fp,"\n%.14f        %% integer_units",sens[9]);   
    fprintf(fp,"\n%.14f        %% float_units",sens[10]);     
    fprintf(fp,"\n%.14f        %% memory_units",sens[11]);    
    fprintf(fp,"\n%.14f        %% branch_units",sens[12]);    
    fprintf(fp,"\n%.14f        %% gpr_static_size",sens[13]); 
    fprintf(fp,"\n%.14f        %% fpr_static_size",sens[14]); 
    fprintf(fp,"\n%.14f        %% pr_static_size",sens[15]);  
    fprintf(fp,"\n%.14f        %% cr_static_size",sens[16]);  
    fprintf(fp,"\n%.14f        %% btr_static_size",sens[17]); 
    fprintf(fp,"\n%.14f        %% num_clusters",sens[18]); 

    fclose(fp);

    // sort indexes by decreasing sensitivity
  
    for (unsigned int j=0;j<sens.size();j++)
    {
	int max_index;
	double max_sens = -1;

	for (unsigned int i=0;i<sens.size();i++)
	{
	    if (sens[i]>max_sens)
	    {
		max_sens = sens[i];
		max_index = i;
	    }
	}
	sens[max_index] = -1;
	sorted_index.push_back(max_index);
    }

    for (unsigned int i=0;i<sorted_index.size();i++)
    {
	int index = sorted_index[i];

	// this time there isn't the NO_L2_CHECK option, optimal
	// parameters values must be searched only in the feasible
	// configurations
	vector<Configuration> space = build_space(parameter_masks[index],base_conf);

	vector<Simulation> sims = simulate_space(space);
	vector<Simulation> ordered_sims = sort_by_energydelay_product(sims);

	// base_conf must be updated, considering the optimal value
	// of the the last parameter explored 
	// base_conf should already have i-1 parameters set to their
	// optimal val, so the following line should change  the
	// last explored parameter

	base_conf = ordered_sims[0].config;

	char temp[10];
	sprintf(temp,"%d_",i);        

	save_simulations(ordered_sims,file_name+string(temp)+".exp");
    }
    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,file_name+".stat");
}

// modified version of SAP, sensivity order of parameters is
// recomputed after a new optimal parameter values is discovered

//**************************************************************

void Explorer::start_SAPMOD()
{
    current_algo = "SAPMOD";
    Exploration_stats stats;
    reset_sim_counter();
    stats.space_size = get_space_size();
    stats.feasible_size = get_feasible_size();
    stats.start_time = time(NULL);

    vector<Simulation> max_sens_sims;

    int max_sens_index;
    double max_sens;
    Configuration base_conf = create_configuration();

    Space_mask empty_mask = get_space_mask(UNSET_ALL);
    Space_mask temp_mask;

    vector<Space_mask> parameter_masks;

    int n_par = 19;

    // for each parameter is created a mask with only one value set to
    // true. These masks will allow to explore all values of each
    // parameter without changing the other
    for (int m=0;m<n_par;m++)
    {
	vector<bool> boolean_mask(n_par,false);
	boolean_mask[m] = true;
	temp_mask = create_space_mask(boolean_mask);
	parameter_masks.push_back(temp_mask);
    }

    string path = get_base_dir()+"/trimaran-workspace/epic-explorer/";
    string file_name = Options.benchmark+"_SAP_"+current_space;
    char file[50];
    double sensitivity;

    while (parameter_masks.size()>0)
    {
	max_sens = 0;

	for (unsigned int i = 0;i<parameter_masks.size();i++)
	{
	    vector<Configuration> space= build_space(parameter_masks[i],base_conf);
	    vector<Simulation> sims = simulate_space(space);

	    sensitivity = get_sensivity_energydelay(sims);

	    if ( sensitivity>max_sens ) 
	    {
		max_sens = sensitivity;
		max_sens_sims.clear();
		append_simulations(max_sens_sims,sims);
		max_sens_index = i;
	    }
	}
	// max_sens_index is the index of parameter_mask corresponding
	// to the parameter which has the greatest influence on
	// objective function energy-delay product

	// the first is the min product
	vector<Simulation> ordered_sims = sort_by_energydelay_product(max_sens_sims);

	sprintf(file,"/SAPMOD_%d.exp",parameter_masks.size());        
	save_simulations(ordered_sims,path+file);

	// change base_conf assigning most sensitive parameter to its
	// optimal value, the one that minimizes energy*delay
	base_conf = ordered_sims[0].config;

	//we must remove the mask corresponding the parameter with max
	//sentivity value, because it will be no longer examined.
	parameter_masks.erase(parameter_masks.begin()+max_sens_index);
    }
    // last base_conf should be the optimal configuration
    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,file_name+".stat");
}
// ********************************************************************
//   Pareto-based sensivity analysis
// ********************************************************************

void Explorer::start_PBSA()
{
    current_algo = "PBSA";
    Exploration_stats stats;
    reset_sim_counter();

    stats.space_size = get_space_size();
    stats.feasible_size = get_feasible_size();

    stats.start_time = time(NULL);

    int n_par = 19;

    vector<double> sens;

    Configuration base_conf = create_configuration();

    vector<Space_mask> parameter_masks;

    vector<vector<Simulation> > parameters_sim_collection;

    vector<Simulation> all_parameter_sims;

    // for each parameter is created a mask with only one value set to
    // true. These masks will allow to explore all values of each
    // parameter without changing the other
    for (int m=0;m<n_par;m++)
    {
	vector<bool> boolean_mask(n_par,false);
	boolean_mask[m] = true;
	Space_mask temp_mask = create_space_mask(boolean_mask);
	parameter_masks.push_back(temp_mask);

	// note NO_L2_CHECK option, cache size parameters should
	// be considered in all their range while testing sensivity

	vector<Configuration> space = build_space(temp_mask);
	vector<Simulation> sims = simulate_space(space);

	// cumulate simulations for each parameter
	// this is needed by get_sensivity_PBSA(...)
	append_simulations(all_parameter_sims,sims);
	parameters_sim_collection.push_back(sims);
    }

    int pippo;
    //each element of parameters_sim_collection is a vector of sims
    //where only a particulare parameter is made to vary in its values
    //range
    for(unsigned int p=0;p<parameters_sim_collection.size();p++)
    {
	double temp = get_sensivity_PBSA(parameters_sim_collection[p],all_parameter_sims);
	sens.push_back(temp);
	cout << EE_TAG << "sensivity " << p << ": " << sens[p];
    }

    string name = get_base_dir()+"/trimaran-workspace/epic-explorer/";
    if (Options.hyperblock) name+="H_";
    name+= Options.benchmark+"_PBSA_"+current_space+"_sensitivity.stat";

    FILE * fp = fopen(name.c_str(),"w");

    fprintf(fp,"\n%.14f        %% L1D_size",sens[0]);        
    fprintf(fp,"\n%.14f        %% L1D_block",sens[1]);       
    fprintf(fp,"\n%.14f        %% L1D_assoc",sens[2]);       
    fprintf(fp,"\n%.14f        %% L1I_size",sens[3]);        
    fprintf(fp,"\n%.14f        %% L1I_block",sens[4]);       
    fprintf(fp,"\n%.14f        %% L1I_assoc",sens[5]);       
    fprintf(fp,"\n%.14f        %% L2U_size",sens[6]);        
    fprintf(fp,"\n%.14f        %% L2U_block",sens[7]);       
    fprintf(fp,"\n%.14f        %% L2U_assoc",sens[8]);       
    fprintf(fp,"\n%.14f        %% integer_units",sens[9]);   
    fprintf(fp,"\n%.14f        %% float_units",sens[10]);     
    fprintf(fp,"\n%.14f        %% memory_units",sens[11]);    
    fprintf(fp,"\n%.14f        %% branch_units",sens[12]);    
    fprintf(fp,"\n%.14f        %% gpr_static_size",sens[13]); 
    fprintf(fp,"\n%.14f        %% fpr_static_size",sens[14]); 
    fprintf(fp,"\n%.14f        %% pr_static_size",sens[15]);  
    fprintf(fp,"\n%.14f        %% cr_static_size",sens[16]);  
    fprintf(fp,"\n%.14f        %% btr_static_size",sens[17]); 
    fprintf(fp,"\n%.14f        %% num_clusters",sens[18]); 

    fclose(fp);

    // sort indexes by decreasing sensitivity
  
    vector<int> sorted_index;

    for (unsigned int j=0;j<sens.size();j++)
    {
	int max_index;
	double max_sens = -1;

	for (unsigned int i=0;i<sens.size();i++)
	{
	    if (sens[i]>max_sens)
	    {
		max_sens = sens[i];
		max_index = i;
	    }
	}
	sens[max_index] = -1;
	sorted_index.push_back(max_index);
    }
    
    // Initialization of pareto set simulations.
    // Create a space with only the default configuration, simulate it
    // and put the result in the pareto set simulation.

    vector<Configuration> initial_space ;
    initial_space.push_back(base_conf);

    // initial space has only one element ! (SIM_SPACE_18)
    vector<Simulation> pareto_set = simulate_space(initial_space);

    for (unsigned int i=0;i<sorted_index.size();i++)
    {
	int index = sorted_index[i];

	// we must combine two configuration spaces:
	
	// a space where only one parameter varies on its range
	vector<Configuration> param_space = build_space(parameter_masks[index],NO_L2_CHECK);

	// ...and the space of pareto set 
	vector<Configuration> pareto_space = extract_space(pareto_set);

	// we must combine each single value of parameter that is
	// modified in the first space with all configuration of the
	// second space
	
	Space_mask other_params = negate_mask(parameter_masks[index]);

	vector<Configuration> new_space = build_space_cross_merge(param_space,
								  pareto_space,
		                                                  parameter_masks[index],
								  other_params );
	vector<Simulation> sims = simulate_space(new_space);

	append_simulations(pareto_set,sims);
	remove_dominated_simulations(pareto_set);

	char temp[10];
	sprintf(temp,"%d_",i);
	string file_name = Options.benchmark+"_PBSA_stage"+string(temp)+current_space;
	save_simulations(pareto_set,file_name+".exp");
    }

    string file_name = Options.benchmark+"_PBSA_"+current_space;
    save_simulations(pareto_set,file_name+".pareto.exp");

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,file_name+".stat");
}
// ********************************************************************
// multiobjective sensivity 
// ********************************************************************
double Explorer::get_sensivity_PBSA(const vector<Simulation>& simulations,const vector<Simulation>& all_sims)
{
    vector<Simulation> energy_sorted = sort_by_energy(all_sims);
    vector<Simulation> exec_time_sorted = sort_by_exec_time(all_sims);
    vector<Simulation> area_sorted = sort_by_area(all_sims);


    double max_energy = energy_sorted[energy_sorted.size()-1].energy;
    double max_exec_time = exec_time_sorted[exec_time_sorted.size()-1].exec_time;
    double max_area = area_sorted[area_sorted.size()-1].area;

    // copy simulations as normalized_sims, then each element of
    // normalized_sims will be overwritten with its normalized value
    vector<Simulation> normalized_sims;
    append_simulations(normalized_sims,simulations);

    for (unsigned i=0;i<normalized_sims.size();i++)
    {
	normalized_sims[i].energy = normalized_sims[i].energy/max_energy;
	normalized_sims[i].exec_time = normalized_sims[i].exec_time/max_exec_time;
	normalized_sims[i].area = normalized_sims[i].area/max_area;
    }

    double current_distance;
    double max_distance = 0;

    // depending on which objectives has been selected, 'distance'
    // function will use an appropriate distance metric
    for (unsigned int i=0;i<normalized_sims.size();i++)
    {
	for (unsigned int j=0;j<normalized_sims.size();j++)
	{
	    current_distance = distance(normalized_sims[i],normalized_sims[j]);
	    if (current_distance>max_distance) max_distance = current_distance;
	}
    }

    return max_distance;
}
//**************************************************************
double Explorer::get_sensivity_energydelay(const vector<Simulation>& sims)
{
    vector<Simulation> temp = sort_by_energydelay_product(sims);
   
    double min_product = (temp[0].energy)*(temp[0].exec_time);
    double max_product = (temp[temp.size()-1].energy)*(temp[temp.size()-1].exec_time);

    return max_product-min_product;
}
