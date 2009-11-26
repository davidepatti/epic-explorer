/***************************************************************************
                          explorer.cpp  -  description
                             -------------------
    begin                : Sat Aug 24 2002
    copyright            : (C) 2002 by Davide Patti
    email                : dpatti@diit.unict.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "explorer.h"
#include "common.h"
#ifdef EPIC_MPI
#include "mpi.h"
#endif


//********************************************************************
Explorer::Explorer(Trimaran_interface * ti)
{
    this->trimaran_interface = ti;
    // when simulation is started these values will be calculated
    // to be more realistic

    average_compilation_time = 15;
    average_exec_time = 5;

    // by default objectives include exec_time and average power 
    Options.objective_area = false;
    Options.objective_power = true;
    Options.objective_energy = false;
    Options.objective_exec_time = true;

    Options.save_spaces = false;
    Options.save_estimation = false;

    Options.save_objectives_details = false;
    force_simulation = false;
    function_approx = NULL;
    base_dir = string(getenv(BASE_DIR));

    current_space = "NOT_SET";
    string space_dir = get_base_dir()+"/trimaran-workspace/epic-explorer/SUBSPACES/";
    string space_file = space_dir+"current.sub";

    char target_space_file[140];
    int l = readlink(space_file.c_str(),target_space_file,140);
    if (l!=-1)
    {
	load_space_file(space_file);
	target_space_file[l] = '\0';
	set_space_name(target_space_file);
    }
    else
    {
	cout << "\nError while setting subspace " << target_space_file << ". Check that symbolic link";
	cout << "\n'current.sub' in " << space_dir << " it's properly set to point to a valid subspace file!";
    }
}

//********************************************************************

Explorer::~Explorer()
{
    string logfile = get_base_dir()+string(EE_LOG_PATH);
    write_to_log(get_mpi_rank(),logfile,"Destroying explorer class");
}


//********************************************************************
void Explorer::set_options(const struct User_Settings& user_settings)
{
    Options = user_settings;

    estimator.set_autoclock(user_settings.auto_clock);

    // Number of objectives 
    n_obj = 0;

    if (Options.objective_power) n_obj++;
    if (Options.objective_exec_time) n_obj++;
    if (Options.objective_area) n_obj++;
    if (Options.objective_energy) n_obj++;

#ifdef EPIC_MPI
    if (!Options.multidir)
    {
	cout << "\n WARNING !!!!";
	cout << "\n****************";
	cout << "\n EPIC_MPI compilation detected, save/restore simulations NOT enabled!";
	cout << "\n Currently, MPI works ONLY if save/restore simulations (multidir) otion is enabled.";
	cout << "\n SEE the VERY IMPORTANT NOTE on file MPI.README (section III).";
	wait_key();
    }
#endif

}

////////////////////////////////////////////////////////////////////////////
// The following  are utility functions used by exploring functions
////////////////////////////////////////////////////////////////////////////
//********************************************************************

Configuration Explorer::create_configuration(const Processor& p,const Mem_hierarchy& mem, const Compiler& comp) //db
{
    Configuration conf;

    conf.gpr_static_size = processor.gpr_static_size.get_val();
    conf.fpr_static_size = processor.fpr_static_size.get_val();
    conf.pr_static_size = processor.pr_static_size.get_val();
    conf.cr_static_size = processor.cr_static_size.get_val();
    conf.btr_static_size = processor.btr_static_size.get_val();

    conf.num_clusters = processor.num_clusters.get_val();
    conf.integer_units = processor.integer_units.get_val();
    conf.float_units = processor.float_units.get_val();
    conf.branch_units = processor.branch_units.get_val();
    conf.memory_units = processor.memory_units.get_val();

    conf.L1D_size = mem.L1D.size.get_val();
    conf.L1D_block = mem.L1D.block_size.get_val();
    conf.L1D_assoc = mem.L1D.associativity.get_val();

    conf.L1I_size = mem.L1I.size.get_val();
    conf.L1I_block = mem.L1I.block_size.get_val();
    conf.L1I_assoc = mem.L1I.associativity.get_val();

    conf.L2U_size = mem.L2U.size.get_val();
    conf.L2U_block = mem.L2U.block_size.get_val();
    conf.L2U_assoc = mem.L2U.associativity.get_val();

    conf.tcc_region = comp.tcc_region.get_val();	//db
    conf.max_unroll_allowed = comp.max_unroll_allowed.get_val();	 	//db
    conf.regroup_only = comp.regroup_only.get_val();	 	//db
    conf.do_classic_opti = comp.do_classic_opti.get_val();	 	//db
    conf.do_prepass_scalar_scheduling = comp.do_prepass_scalar_scheduling.get_val();	 	//db
    conf.do_postpass_scalar_scheduling = comp.do_postpass_scalar_scheduling.get_val();	 	//db
    conf.do_modulo_scheduling = comp.do_modulo_scheduling.get_val();	 	//db
    conf.memvr_profiled = comp.memvr_profiled.get_val();	 	//db


    return conf;
}

////////////////////////////////////////////////////////////////////////////
Configuration Explorer::create_configuration() const
{

     Configuration default_config;

     default_config.gpr_static_size = processor.gpr_static_size.get_default();
     default_config.fpr_static_size = processor.fpr_static_size.get_default();
     default_config.pr_static_size = processor.pr_static_size.get_default();
     default_config.cr_static_size = processor.cr_static_size.get_default();
     default_config.btr_static_size = processor.btr_static_size.get_default();

     default_config.num_clusters = processor.num_clusters.get_default();
     default_config.integer_units = processor.integer_units.get_default();
     default_config.float_units = processor.float_units.get_default();
     default_config.memory_units = processor.memory_units.get_default();
     default_config.branch_units = processor.branch_units.get_default();

     default_config.L1D_size = mem_hierarchy.L1D.size.get_default();
     default_config.L1D_block = mem_hierarchy.L1D.block_size.get_default();
     default_config.L1D_assoc = mem_hierarchy.L1D.associativity.get_default();
     
     default_config.L1I_size = mem_hierarchy.L1I.size.get_default();
     default_config.L1I_block = mem_hierarchy.L1I.block_size.get_default();
     default_config.L1I_assoc = mem_hierarchy.L1I.associativity.get_default();

     default_config.L2U_size = mem_hierarchy.L2U.size.get_default();
     default_config.L2U_block = mem_hierarchy.L2U.block_size.get_default();
     default_config.L2U_assoc = mem_hierarchy.L2U.associativity.get_default();

     default_config.tcc_region = compiler.tcc_region.get_default();	//db
     default_config.max_unroll_allowed = compiler.max_unroll_allowed.get_default();	 	//db
     default_config.regroup_only = compiler.regroup_only.get_default();	 	//db
     default_config.do_classic_opti = compiler.do_classic_opti.get_default();	 	//db
     default_config.do_prepass_scalar_scheduling = compiler.do_prepass_scalar_scheduling.get_default();	 	//db
     default_config.do_postpass_scalar_scheduling = compiler.do_postpass_scalar_scheduling.get_default();	 	//db
     default_config.do_modulo_scheduling = compiler.do_modulo_scheduling.get_default();	 	//db
     default_config.memvr_profiled = compiler.memvr_profiled.get_default();	 	//db

     return default_config;
}

////////////////////////////////////////////////////////////////////////////
Configuration Explorer::create_configuration(const Space_mask& mask,const Configuration& base) const 
{
    Configuration config = create_configuration();


    if (mask.L1D_size) config.L1D_size=base.L1D_size;
    if (mask.L1D_block) config.L1D_block=base.L1D_block;
    if (mask.L1D_assoc) config.L1D_assoc=base.L1D_assoc;
    if (mask.L1I_size) config.L1I_size=base.L1I_size;
    if (mask.L1I_block) config.L1I_block=base.L1I_block;
    if (mask.L1I_assoc) config.L1I_assoc=base.L1I_assoc;
    if (mask.L2U_size) config.L2U_size=base.L2U_size;
    if (mask.L2U_block) config.L2U_block=base.L2U_block;
    if (mask.L2U_assoc) config.L2U_assoc=base.L2U_assoc;
    if (mask.num_clusters) config.num_clusters=base.num_clusters;
    if (mask.integer_units) config.integer_units=base.integer_units;
    if (mask.float_units) config.float_units=base.float_units;
    if (mask.memory_units) config.memory_units=base.memory_units;
    if (mask.branch_units) config.branch_units=base.branch_units;
    if (mask.gpr_static_size) config.gpr_static_size=base.gpr_static_size;
    if (mask.fpr_static_size) config.fpr_static_size=base.fpr_static_size;
    if (mask.pr_static_size) config.pr_static_size=base.pr_static_size;
    if (mask.cr_static_size) config.cr_static_size=base.cr_static_size;
    if (mask.btr_static_size) config.btr_static_size=base.btr_static_size;


    if (mask.tcc_region) config.tcc_region=base.tcc_region;	//db
    if (mask.max_unroll_allowed) config.max_unroll_allowed=base.max_unroll_allowed;	 	//db
    if (mask.regroup_only) config.regroup_only=base.regroup_only;	 	//db
    if (mask.do_classic_opti) config.do_classic_opti=base.do_classic_opti;	 	//db
    if (mask.do_prepass_scalar_scheduling) config.do_prepass_scalar_scheduling=base.do_prepass_scalar_scheduling;	 	//db
    if (mask.do_postpass_scalar_scheduling) config.do_postpass_scalar_scheduling=base.do_postpass_scalar_scheduling;	 	//db
    if (mask.do_modulo_scheduling) config.do_modulo_scheduling=base.do_modulo_scheduling;	 	//db
    if (mask.memvr_profiled) config.memvr_profiled=base.memvr_profiled;	 	//db

    return config;
}
////////////////////////////////////////////////////////////////////////////
Space_mask Explorer::negate_mask(Space_mask mask)
{
    vector<bool> boolean_mask = get_boolean_mask(mask);

    for (unsigned int i=0;i<boolean_mask.size();i++) boolean_mask[i]=!(boolean_mask[i]);

    return create_space_mask(boolean_mask);
}

//********************************************************************
// Remove dominated simulations using the appropriate get_pareto(...)
// function, depending on the objectives selected in Options
void Explorer::remove_dominated_simulations(vector<Simulation>& sims)
{
    vector<Simulation> pareto_set = get_pareto(sims);
    sims.clear();
    append_simulations(sims,pareto_set);
}


// ********************************************************************
//  Euclidean distance between objective function values
// ********************************************************************
double Explorer::distance(const Simulation& s1, const Simulation& s2)
{
    if ( (Options.objective_area) && (Options.objective_power) && (Options.objective_exec_time) )
	    return sqrt(pow(s1.area-s2.area,2)+ pow(s1.energy-s2.energy,2) + pow(double(s1.exec_time-s2.exec_time),2) );

    if ( (Options.objective_power) && (Options.objective_exec_time) )
	    return sqrt(pow(s1.energy-s2.energy,2) + pow(double(s1.exec_time-s2.exec_time),2) );

    if ( (Options.objective_energy) && (Options.objective_exec_time) )
	    return sqrt(pow(s1.energy-s2.energy,2) + pow(double(s1.exec_time-s2.exec_time),2) );

    if ( (Options.objective_area) && (Options.objective_power) )
	    return sqrt(pow(s1.energy-s2.energy,2) + pow(double(s1.area-s2.area),2) );

    if ( (Options.objective_area) && (Options.objective_exec_time) )
	    return sqrt(pow(s1.area-s2.area,2) + pow(double(s1.exec_time-s2.exec_time),2) );

    cout << "\n\n The selected combination of objectives is not supported from";
    cout << "\n get_distance(...) function in explorer.cpp ";
    exit(0);
    
    return 0;
}


//*********************************************************************
//    Ehaustive exploration of all parameters 
// ********************************************************************
void Explorer::start_EXHA()
{
    current_algo = "EXHA";
    Exploration_stats stats;
    reset_sim_counter();
    stats.space_size = get_space_size();
    stats.start_time=time(NULL);

    Space_mask mask = get_space_mask(SET_ALL);

    vector<Configuration> space = build_space(mask);

    // all parameters are explored in all their values 
    vector<Simulation> simulations = simulate_space(space);
    vector<Simulation> pareto_set = get_pareto(simulations);

    string filename = Options.benchmark+"_EXHA_"+current_space;
    save_simulations(simulations,filename+".exp");
    save_simulations(pareto_set,filename+".pareto.exp");

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,filename+".stat");
}


//**************************************************************

vector<Simulation> Explorer::normalize(const vector<Simulation>& sims)
{
    vector<Simulation> sorted1 = sort_by_exec_time(sims);
    vector<Simulation> sorted2 = sort_by_energy(sims);
    vector<Simulation> sorted3 = sort_by_area(sims);

    double min_exec_time = sorted1[0].exec_time;
    double max_exec_time = sorted1[sorted1.size()-1].exec_time;

    double min_energy = sorted2[0].energy;
    double max_energy = sorted2[sorted2.size()-1].energy;

    double min_area = sorted3[0].area;
    double max_area = sorted3[sorted3.size()-1].area;

    vector<Simulation> normalized;

    for (unsigned int i=0;i<sorted1.size();i++)
    {
	Simulation temp_sim = sims[i];

	temp_sim.energy = (temp_sim.energy)/(max_energy);
	temp_sim.exec_time = (temp_sim.exec_time)/(max_exec_time);
	temp_sim.area   = (temp_sim.area)/(max_area);

	normalized.push_back(temp_sim);
    }
    return normalized;
}

//**************************************************************
// from min product to max product
vector<Simulation> Explorer::sort_by_energydelay_product(vector<Simulation> sims)
{
    int min_index;
    double min_product;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
	min_product = 1000000000000000.0;

	for (unsigned int i=0;i<sims.size();i++)
	{
	    if ( (sims[i].exec_time)*(sims[i].energy)  <= min_product) 
	    {
		//cout << "\nDEBUG:(i="<<i<< ") " << sims[i].exec_time << " <= " << min_exec_time;
		min_product= sims[i].exec_time * sims[i].energy;
		min_index = i;
	    }
	}
	temp.push_back(sims[min_index]);
	sims.erase(sims.begin()+min_index);
    }
    return temp;
}

// ********************************************************************
//   Re-Evaluation of Pareto exploration
// ********************************************************************
void Explorer::start_REP(const string& file){
    current_algo="REP";


    REP_source_file = file.substr(file.find_last_of('/') + 1);
    vector<Configuration> vconf;
    ifstream in(file.c_str());

    string logfile = get_base_dir()+string(EE_LOG_PATH);
    write_to_log(get_mpi_rank(),logfile,"Starting Re-Evaluate pareto on file "+file);

    while(in.good()){
        string s;
        getline(in, s);
        char comment = '%';
        string delimiters = " /";
        // Ignore empty or comment lines
        if(s.size() > 0 && s[0] != comment){
            string::size_type start = s.find(comment);
            // Start after comment
            string str = s.substr(++start);
            // Skip delimiters at beginning
            string::size_type lastPos = str.find_first_not_of(delimiters, 0);
            // Find first "non-delimiter"
            string::size_type pos = str.find_first_of(delimiters, lastPos);
            vector<int> tmp;
            //cout << endl;
            while (string::npos != pos || string::npos != lastPos){
                // Found a token, add it to the vector.
                istringstream iss(str.substr(lastPos, pos - lastPos));
                int value;
                iss >> value; // convert to int
                //cout << value << " ";
                tmp.push_back(value);
                // Skip delimiters
                lastPos = str.find_first_not_of(delimiters, pos);
                // Find next "non-delimiter"
                pos = str.find_first_of(delimiters, lastPos);
            }
	    //cout << endl;
            Configuration config;
	    // Same order as Explorer::save_simulations / Config::to_string()
	    config.num_clusters =    tmp[0];
	    config.integer_units =   tmp[1];
	    config.float_units =     tmp[2];
	    config.branch_units =    tmp[3];
	    config.memory_units =    tmp[4];
	    config.gpr_static_size = tmp[5];
	    config.fpr_static_size = tmp[6];
	    config.pr_static_size =  tmp[7];
	    config.cr_static_size =  tmp[8];
	    config.btr_static_size = tmp[9];
	    config.L1D_size =        tmp[10];
	    config.L1D_block =       tmp[11];
	    config.L1D_assoc =       tmp[12];
	    config.L1I_size =        tmp[13];
	    config.L1I_block =       tmp[14];
	    config.L1I_assoc =       tmp[15];
	    config.L2U_size =        tmp[16];
	    config.L2U_block =       tmp[17];
	    config.L2U_assoc =       tmp[18];

            //cout << config.to_string();
	    vconf.push_back(config);
        }
    }
    in.close();
    write_to_log(get_mpi_rank(),logfile,"Read "+to_string(vconf.size())+" pareto configurations into memory. Starting simulations!");

    vector<Simulation> vsim = simulate_space(vconf);
    string file_name = Options.benchmark+"_"+current_algo+"_"+current_space;
    vector<Simulation> pareto = get_pareto(vsim);
    save_simulations(pareto, file_name+".pareto.exp");
    save_simulations(vsim, file_name+".history.stat");
}

vector<Simulation> Explorer::sort_by_exec_time(vector<Simulation> sims)
{
    int min_index;
    double min_exec_time;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
	min_exec_time = 1000000000000000.0;

	for (unsigned int i=0;i<sims.size();i++)
	{
	    if (sims[i].exec_time<=min_exec_time) 
	    {
		min_exec_time=sims[i].exec_time;
		min_index = i;
	    }
	}
	temp.push_back(sims[min_index]);
	sims.erase(sims.begin()+min_index);
    }

#ifdef VERBOSE
    for (unsigned int i = 0;i<temp.size();i++)
	cout << EE_TAG << "DEBUG temp(ordinato): " << temp[i].exec_time;
#endif

    return temp;
}

//********************************************************************
vector<Simulation> Explorer::sort_by_energy(vector<Simulation> sims)
{
    int min_index;
    double min_energy;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
	min_energy = 1000000000000000.0;

	for (unsigned int i=0;i<sims.size();i++)
	{
	    if (sims[i].energy<=min_energy) 
	    {
		min_energy=sims[i].energy;
		min_index = i;
	    }
	}
	temp.push_back(sims[min_index]);

	sims.erase(sims.begin()+min_index);
    }

#ifdef VERBOSE
    for (unsigned int i = 0;i<temp.size();i++)
	cout << EE_TAG << "DEBUG (orderer E): " << temp[i].energy;
#endif
    return temp;
}
////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::sort_by_area(vector<Simulation> sims)
{
    int min_index;
    double min_area;
    vector<Simulation> temp;

    while (sims.size()>0)
    {
	min_area = 1000000000000000.0;

	for (unsigned int i=0;i<sims.size();i++)
	{
	    if (sims[i].area<=min_area) 
	    {
		min_area=sims[i].area;
		min_index = i;
	    }
	}
	temp.push_back(sims[min_index]);
	sims.erase(sims.begin()+min_index);
    }

#ifdef VERBOSE
    for (unsigned int i = 0;i<temp.size();i++)
	cout << EE_TAG << "DEBUG (orderer Area): " << temp[i].area;
#endif

    return temp;
}

/////////////////////////////////////////////////////////////////////////////
bool isDominated(Simulation sim, const vector<Simulation>& simulations)
{
    
    for(int i=0;i<simulations.size();++i)
    {
	if ((sim.energy>=simulations[i].energy) && (sim.exec_time>=simulations[i].exec_time))
		return (true);
    }

    return (false);

}
// wrap function which determines the right pareto function to be 
// called 
vector<Simulation> Explorer::get_pareto(const vector<Simulation>& simulations)
{
	// Note that if we consider energy instead of power, the same function is called 
	// since the field sim.energy is used both for energy and power
    if ( (Options.objective_area) && (Options.objective_power) && (Options.objective_exec_time) )
	return get_pareto3d(simulations);

    if ( (Options.objective_power) && (Options.objective_exec_time) )
	return get_pareto_CyclesPower(simulations);

    if ( (Options.objective_energy) && (Options.objective_exec_time) )
	return get_pareto_CyclesPower(simulations);

    if ( (Options.objective_area) && (Options.objective_power) )
	return get_pareto_AreaPower(simulations);

    if ( (Options.objective_area) && (Options.objective_exec_time) )
	return get_pareto_AreaCycles(simulations);
}
////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::get_pareto_CyclesPower(const vector<Simulation>& simulations)
{
    double min_e = 1000000000000000.0;

    vector<Simulation> pareto_set;
    vector<Simulation> sorted = sort_by_exec_time(simulations);

    while (sorted.size()>0)
    {
	if (sorted[0].energy<=min_e)
	{
	    if ( (pareto_set.size()>0) && (pareto_set.back().exec_time==sorted[0].exec_time))
		pareto_set.pop_back();

	    min_e = sorted[0].energy;
	    pareto_set.push_back(sorted[0]);
	}
	sorted.erase(sorted.begin());
    }
    return pareto_set;
}

////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::get_pareto_AreaCycles(const vector<Simulation>& simulations)
{
    double min_area = 1000000000000000.0;

    vector<Simulation> pareto_set;
    vector<Simulation> sorted = sort_by_exec_time(simulations);

    while (sorted.size()>0)
    {
	if (sorted[0].area<=min_area)
	{
	    if ( (pareto_set.size()>0) && (pareto_set.back().exec_time==sorted[0].exec_time))
		pareto_set.pop_back();

	    min_area = sorted[0].area;
	    pareto_set.push_back(sorted[0]);
	}
	sorted.erase(sorted.begin());
    }
    return pareto_set;
}
////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::get_pareto_AreaPower(const vector<Simulation>& simulations)
{
    double min_area = 1000000000000000.0;

    vector<Simulation> pareto_set;
    vector<Simulation> sorted = sort_by_energy(simulations);

    while (sorted.size()>0)
    {
	if (sorted[0].area<=min_area)
	{
	    if ( (pareto_set.size()>0) && (pareto_set.back().energy==sorted[0].energy))
		pareto_set.pop_back();

	    min_area = sorted[0].area;
	    pareto_set.push_back(sorted[0]);
	}
	sorted.erase(sorted.begin());
    }
    return pareto_set;
}

////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::get_pareto3d(const vector<Simulation>& simulations)
{
  vector<Simulation> pareto_set;

  for (int i=0; i<simulations.size(); i++)
  {
      bool dominated = false;

      for (int j=0; j<simulations.size() && !dominated; j++)

	if (   // if it is dominated...
	     (simulations[j].energy <= simulations[i].energy &&
	      simulations[j].area <= simulations[i].area &&
	      simulations[j].exec_time <= simulations[i].exec_time) 
	    && 
	     ( // ...but not from an identical sim
	      simulations[j].energy != simulations[i].energy ||
	      simulations[j].area != simulations[i].area ||
	      simulations[j].exec_time != simulations[i].exec_time) 
	   )
	    dominated = true;

      // avoid repeated pareto configs
      if ( (!dominated) && (simulation_present(simulations[i],pareto_set) == -1) )
	pareto_set.push_back(simulations[i]);
  }

  return pareto_set;
}
// power/area pareto set



int Explorer::count_needed_recompilations(const vector<Configuration>& space)
{
    bool processor_changed;
    int recompilations = 0;

    Configuration c;
    c.invalidate();

    Space_mask mask = get_space_mask(SET_PROCESSOR);

    for (unsigned int i=0;i<space.size();i++)
    {
	processor_changed = c.check_difference(space[i],mask);

	if (processor_changed)  recompilations++;

	c = space[i];
    }
    return recompilations;
}

////////////////////////////////////////////////////////////////////////////
void Explorer::save_configurations(const vector<Configuration>& space, const string& filename)
{
    vector<Simulation> pseudo_sims;
    Simulation pseudo_sim;

    for (unsigned int i = 0;i< space.size();i++)
    {
	pseudo_sim.config = space[i];
	pseudo_sim.area = 0.0;
	pseudo_sim.exec_time = 0.0;
	pseudo_sim.energy = 0.0;
	pseudo_sim.simulated = false;
	pseudo_sims.push_back(pseudo_sim);
    }
    save_simulations(pseudo_sims,filename);
}
////////////////////////////////////////////////////////////////////////////
void Explorer::save_simulations(const vector<Simulation>& simulations, const string& filename)
{
    double area,energy;
    double energydelay;
    double exec_time;
    
    FILE * fp;

    string path = get_base_dir()+"/trimaran-workspace/epic-explorer/";

    string file_path = path+filename;

    time_t now = time(NULL);
    string uora = string(asctime(localtime(&now)));
    string pretitle = "\n%% Epic Explorer simulation file - created on " + uora;
    if(current_algo=="REP")
        pretitle += "\n%% REP source file: " + REP_source_file;

    string pretitle2 ="\n%% Objectives: ";
    //G

    if (Options.objective_area) pretitle2+="Area, ";
    if (Options.objective_energy) pretitle2+="Energy, ";
    if (Options.objective_power) pretitle2+= "Average Power, ";
    if (Options.objective_exec_time) pretitle2+="Execution Time";

    string pretitle3 ="\n%% Benchmark: ";
    if(Options.multibench)
        for(int i=0; i<benchmarks.size(); i++)
            pretitle3 += benchmarks.at(i) + " ";
    else
        pretitle3 += Options.benchmark;

    string pretitle4 = "\n%% Compiler: tcc_region, max_unroll_allowed, regroup_only, do_classic_opti, do_prepass_scalar_scheduling, do_postpass_scalar_scheduling, do_modulo_scheduling, memvr_profiled";

    string title = "\n\n%% ";

    // currently, energy and power are mutually exclusive objectives
    if (Options.objective_energy) title+="Area(cm^2)\tEnergy(J)\tTime(ms)";
    else
	title+="Area(cm^2)  Power(W)     Time(ms)   / MHz";

    title+=" / cl / I F B M / gpr fpr pr cr btr / L1D(CBA) / L1I(CBA) / L2U(CBA) / Compiler";

    fp=fopen(file_path.c_str(),"w");
    
    fprintf(fp,"\n%% ----------------------------------------------");
    fprintf(fp,pretitle.c_str());
    fprintf(fp,pretitle2.c_str());
    fprintf(fp,pretitle3.c_str());
    fprintf(fp,pretitle4.c_str());
    fprintf(fp,title.c_str());
    fprintf(fp,"\n%% ----------------------------------------------");

    for (unsigned int i =0;i<simulations.size();i++)
    {
	area = simulations[i].area;
	energy = simulations[i].energy;
	exec_time = simulations[i].exec_time*1000;
	
	string conf_string = simulations[i].config.get_header();

	int mhz = int(simulations[i].clock_freq/1e6);
	   
	char ch = ' ';
	if (!simulations[i].simulated) ch = '*';

	fprintf(fp,"\n%.9f  %.9f  %.9f %%/ %d / %s %c",area,energy,exec_time,mhz,conf_string.c_str(),ch);
    }
    fclose(fp);
}

void Explorer::save_objectives_details(const Dynamic_stats& dyn,const Configuration& config, const string filename) const
{
    FILE * fp;
    fp=fopen(filename.c_str(),"a");

    string c = config.get_header();

    fprintf(fp,"\n %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %s",
		    					    dyn.compute_cycles,
	                                                    dyn.ialu,
							    dyn.falu,
							    dyn.branch,
							    dyn.load,
							    dyn.store,
							    dyn.total_dynamic_operations,
							    dyn.L1D_r_hit,
							    dyn.L1D_w_hit,
							    dyn.L1D_r_miss,
							    dyn.L1D_w_miss,
							    dyn.L1I_hit,
							    dyn.L1I_miss,
							    dyn.L2U_hit,
							    dyn.L2U_miss,
							    c.c_str());

    fclose(fp);
}

////////////////////////////////////////////////////////////////////////////
void Explorer::save_stats(const Exploration_stats& stats,const string& file)
{
    FILE * fp;
    string file_path = get_base_dir()+"/trimaran-workspace/epic-explorer/"+file;

    fp = fopen(file_path.c_str(),"w");

    // we want the total size of the space of all possible
    // configurations

    int elapsed_time = (int)difftime(stats.end_time,stats.start_time)/60;

    fprintf(fp,"\n Space size: %g",stats.space_size);
    fprintf(fp,"\n simulations: %d ",stats.n_sim);
    fprintf(fp,"\n total exploration time: %d minutes ",elapsed_time);
    fprintf(fp,"\n simulation start: %s ",asctime(localtime(&stats.start_time)));
    fprintf(fp,"\n simulation end: %s ",asctime(localtime(&stats.end_time)));

    fclose(fp);
}

/////////////////////////////////////////////////////////////////
void Explorer::prepare_explorer(const string& application, const Configuration& config)
{
    // Note that this fuction does NOT create any directory, but
    // simply sets explorer class members according to the app and configuration
    epic_dir = get_base_dir()+"/trimaran-workspace/epic-explorer/";
    application_dir = epic_dir + Options.benchmark;
    exe_dir = application_dir+"/"+config.get_executable_dir();
    cache_dir_name = config.get_mem_dir();
    mem_hierarchy_dir = exe_dir+"/"+cache_dir_name+"/";
    machine_dir = exe_dir+"/machines";
    m5_dir = mem_hierarchy_dir+"./m5elements";

    mem_hierarchy_filename = m5_dir+"/default.py";
    hmdes_filename = machine_dir +"/"+ EXPLORER_HMDES2;
    comp_filename = epic_dir+"/machines/"+COMPILER_PARAM;  //db

    bench_executable = Options.benchmark;
    if (config.tcc_region == 1) bench_executable +="_O";  //db	
    if (config.tcc_region == 2) bench_executable +="_HS";  //db
    if (config.tcc_region == 3) bench_executable +="_S";  //db

    pd_stats_file = mem_hierarchy_dir+"/PD_STATS";
#ifdef TEST
    cout <<  EE_TAG << " Assuming the following setup:";
    cout <<  EE_TAG << "epic_dir: " << epic_dir;
    cout <<  EE_TAG << "application_dir: " << application_dir;
    cout <<  EE_TAG << "exe_dir: " << exe_dir;
    cout <<  EE_TAG << "mem_hierarchy_dir: " << mem_hierarchy_dir;
    cout <<  EE_TAG << "machine_dir: " << machine_dir;
    cout <<  EE_TAG << "m5_dir: " << m5_dir;
    cout <<  EE_TAG << "hmdes_filename: " << hmdes_filename;
    cout <<  EE_TAG << "mem_hierarchy_filename: " << mem_hierarchy_filename;
    cout <<  EE_TAG << "bench_executable: " << bench_executable;
    cout <<  EE_TAG << "pd_stats_file: " << pd_stats_file;
    wait_key();
#endif
}

// --------------------------------------------------------------//
// get_explorer_status 
// if a exe/mem_hierarchy_dir is already present the are two
// possibilities:
//  - the required bench exe and PD_STATS files have been previously created
//  - another process is working on that dir
//
// EXPLORER_NOTHING_DONE: must compile/execute benchmark
// EXPLORER_BINARY_DONE: only memory simulation required
// EXPLORER_ALL_DONE: all steps done, can simply read statistics

int Explorer::get_explorer_status() const
{
    int myrank = get_mpi_rank();
    int mysize = get_mpi_size();
    string logfile = get_base_dir()+string(EE_LOG_PATH);

    bool must_execute = false;
    bool must_compile = false;
    string cmd;
    
    mode_t mode = 0770;

    if (!file_exists(application_dir)) 
    {
	write_to_log(myrank,logfile,"Creating application dir " + application_dir);
	mkdir(application_dir.c_str(),mode);
    }

    // check compilation status 

    if (file_exists(exe_dir))  
    {
	// another process created the exe directory, must wait
	// for benchmark executable to be available

	write_to_log(myrank,logfile,"Found " + exe_dir + ", checking for executable...");

	while (!file_exists(exe_dir+"/simu_intermediate/"+bench_executable))
	    sleep(EXPLORER_RETRY_TIME);

	write_to_log(myrank,logfile,"Ok, found " + bench_executable);
    }
    else
    {
	// the exe directory must be created, but we should
	// avoid race condition with other processes trying to create
	// same directory

	if (mkdir(exe_dir.c_str(),mode)==0)
	{   // dir successifully created, this process is responsible for compiling!
	    write_to_log(myrank,logfile,"Created " + exe_dir);
	    must_compile = true; 
	    cmd = "cp -R "+get_base_dir()+"/trimaran-workspace/epic-explorer/machines/ "+exe_dir;
	    system(cmd.c_str());
	}
	else
	{
	    write_to_log(myrank,logfile," Warning: Could not create exe dir, waiting for exe in "+exe_dir+"...");

	    while (!file_exists(exe_dir+"/simu_intermediate/"+bench_executable))
		sleep(EXPLORER_RETRY_TIME);

	    write_to_log(myrank,logfile,"Ok, found " + bench_executable);
	}
    }

    // check execution status 

    if (file_exists(mem_hierarchy_dir)) 
    {

	write_to_log(myrank,logfile,"Found mem dir " + mem_hierarchy_dir +", checking for PD_STATS...");

	while (!file_exists(pd_stats_file))
	    sleep(EXPLORER_RETRY_TIME);

	write_to_log(myrank,logfile,"Ok, found "+pd_stats_file);
    }
    else
    {

	if (mkdir(mem_hierarchy_dir.c_str(),mode)==0)
	{
	    write_to_log(myrank,logfile,"Created " + mem_hierarchy_dir);
	    must_execute = true; // this process is responsible for executing!
	    cmd = "cp -R "+get_base_dir()+"/trimaran-workspace/epic-explorer/m5elements/ "+mem_hierarchy_dir;
	    system(cmd.c_str());
	}
	else
	{
	   write_to_log(myrank,logfile," Warning: could not create mem dir, Waiting for " + pd_stats_file +"...");
	    while (!file_exists(pd_stats_file))
		sleep(EXPLORER_RETRY_TIME);
	    write_to_log(myrank,logfile," OK, can proceed on PD_STATS file");
	}
    }

    if (must_compile) 
    {
	write_to_log(myrank,logfile,"Epic explorer status -> Required recompilation and execution");
	return EXPLORER_NOTHING_DONE;
    }

    if (must_execute) 
    {
	write_to_log(myrank,logfile,"Epic explorer status -> Found application exe, only re-execution required!");
	return EXPLORER_BINARY_DONE;
    }

    write_to_log(myrank,logfile,"Epic explorer status -> Found PS_STATS file, all simulation steps already done!");
    return EXPLORER_ALL_DONE;
}

////////////////////////////////////////////////////////////////////////////
void Explorer::save_estimation_file( const Dynamic_stats& dynamic_stats, 
				         const Estimate& estimate, 
					 Processor& processor, 
					 Mem_hierarchy& mem,
					 Compiler& comp,
					 string& filename) const
{
    string file_path;
    file_path = get_base_dir()+"/trimaran-workspace/epic-explorer/";
    file_path += filename;

    std::ofstream output_file(file_path.c_str());

    if (!output_file)
    {
	cout << "\n Error while saving " << file_path ;
	wait_key();
    }
    else
    {
	output_file << "\n >>>>>>>> EPIC Explorer estimation file: " << filename;
	output_file << "\n";
	output_file << "\n **************************************************";
	output_file << "\n # of clusters: " <<  processor.num_clusters.get_val();
	output_file << "\n GPR,FPR,PR,CR,BTR = " <<processor.gpr_static_size.get_val();
	output_file << "," <<  processor.fpr_static_size.get_val();
	output_file << "," <<  processor.pr_static_size.get_val();
	output_file << "," <<  processor.cr_static_size.get_val();
	output_file << "," <<  processor.btr_static_size.get_val();

	output_file << "\n Per-cluster IU,FPR,MU,BU  =  " << processor.integer_units.get_val();
	output_file << "," <<   processor.float_units.get_val();
	output_file << "," <<  processor.memory_units.get_val();
	output_file << "," <<  processor.branch_units.get_val();

	output_file <<"\n";
	output_file <<"\n L2U Size,BSize,Assoc = " << mem.L2U.size.get_val() << "," << mem.L2U.block_size.get_val() << "," << mem.L2U.associativity.get_val();
	output_file <<"\n L1I Size,BSize,Assoc = " << mem.L1I.size.get_val() << "," << mem.L1I.block_size.get_val() << "," << mem.L1I.associativity.get_val();
	output_file <<"\n L1D Size,BSize,Assoc = " << mem.L1D.size.get_val() << "," << mem.L1D.block_size.get_val() << "," << mem.L1D.associativity.get_val();
	output_file << "\n " << comp.tcc_region.get_label() << ": "<< comp.tcc_region.get_val();
	output_file << "\n " << comp.max_unroll_allowed.get_label()<< ": " << comp.max_unroll_allowed.get_val();
	output_file << "\n " << comp.do_classic_opti.get_label()<< ": " << comp.do_classic_opti.get_val(); 
	output_file << "\n " << comp.do_prepass_scalar_scheduling.get_label()<< ": " << comp.do_prepass_scalar_scheduling.get_val();
	output_file << "\n " << comp.do_postpass_scalar_scheduling.get_label()<< ": " << comp.do_postpass_scalar_scheduling.get_val();
	output_file << "\n " << comp.do_modulo_scheduling.get_label()<< ": " << comp.do_modulo_scheduling.get_val();
	output_file << "\n " << comp.memvr_profiled.get_label()<< ": " << comp.memvr_profiled.get_val(); //db
	output_file << "\n ****************************************************";
	output_file << "\n";
	output_file << "\n  P e r f o r m a n c e ";
	output_file << "\n ----------------------------------------------------";
	output_file << "\n     Total cycles: " << estimate.execution_cycles;
	output_file << "\n   Compute cycles: " << estimate.compute_cycles;
	output_file << "\n     Stall cycles: " << estimate.stall_cycles;
	output_file << "\n   Execution time (ms): " << estimate.execution_time*1000 << " (clock: " << estimate.clock_freq/1e6 << "MHz)";
	if (Options.auto_clock)
	{
	    output_file << "\n   L1D access time (ns): " << estimate.L1D_access_time*1e9;
	    output_file << "\n   L1I access time (ns): " << estimate.L1I_access_time*1e9;
	}

	output_file << "\n   IPC: " << estimate.IPC << " ops/cycle ";
	output_file << "\n   IPC (no stalls): " << dynamic_stats.average_issued_ops_compute_cycles << " ops/cycle";
	output_file << "\n";
	output_file << "\n  E n e r g y  &  P o w e r ";
	output_file << "\n ----------------------------------------------------";
	output_file << "\n   L1D cache energy (mJ): " << estimate.L1D_energy*1000;
	output_file << "\n   L1I cache energy (mJ): " << estimate.L1I_energy*1000;
	output_file << "\n   L2U cache energy (mJ): " << estimate.L2U_energy*1000;
	output_file << "\n-->  Total Cache energy (mJ) " << estimate.total_cache_energy*1000;
	output_file << "\n   Main Memory energy (mJ): " << estimate.main_memory_energy*1000;
	output_file << "\n   Processor energy (mJ): " << estimate.total_processor_energy*1000;
	output_file << "\n\n--> Total System Energy (mJ): "<< estimate.total_system_energy*1000;
	output_file << "\n--> Average System Power (W):"<< estimate.total_average_power;
	output_file << "\n\n   Total System Energy (no stalls) (mJ): "<< estimate.NO_MEM_system_energy*1000;
	output_file << "\n   Average System Power (no stalls) (W):"<< estimate.NO_MEM_system_energy/(estimate.clock_T*dynamic_stats.total_cycles);

	output_file << "\n";
	output_file << "\n I n s t r u c t i o n  M i x";
	output_file << "\n ----------------------------------------------------";

	output_file << "\n                Branches: " << dynamic_stats.branch;
	output_file << "\n                    Load: " << dynamic_stats.load;
	output_file << "\n                   Store: " << dynamic_stats.store;
	output_file << "\n             Integer alu: " << dynamic_stats.ialu;
	output_file << "\n               Float alu: " << dynamic_stats.falu;
	output_file << "\n    Compare to predicate: " << dynamic_stats.cmp;
	output_file << "\n       Prepare to branch: " << dynamic_stats.pbr;
	output_file << "\n          Spill Restores: " << dynamic_stats.spills_restores;
	output_file << "\n";
	output_file << "\n  M e m o r y ";
	output_file << "\n ----------------------------------------------------";
	output_file << "\n L1I total fetches: " << dynamic_stats.L1I_fetches;
	output_file << "\n L1I (total hit/miss): ("<< dynamic_stats.L1I_hit <<"/"<< dynamic_stats.L1I_miss<<")";
	output_file << "\n";
	output_file << "\n L1D (read hit/miss): ("<< dynamic_stats.L1D_r_hit<<"/"<<dynamic_stats.L1D_r_miss<<")";
	output_file << "\n L1D (write hit/miss): ("<< dynamic_stats.L1D_w_hit<<"/"<<dynamic_stats.L1D_w_miss<<")";

	output_file << "\n";
	output_file << "\n L2U (hit/miss): ("<< dynamic_stats.L2U_hit<<"/" << dynamic_stats.L2U_miss<<")";
	output_file << "\n";
	output_file << "\n L1D bus transition prob: " << estimate.L1D_transition_p;
	output_file << "\n L1I bus transition prob: " << estimate.L1I_transition_p;
    }
}

////////////////////////////////////////////////////////////////////////////

bool Explorer::equivalent_spaces(const vector<Configuration>& s1,const vector<Configuration>& s2) const
{
    if (s1.size()!=s2.size()) return false;

    for (int i=0;i<s1.size();i++)
    {
	if (!configuration_present(s1[i],s2)) return false;
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////
// return one of the commonly used configuration masks available if user
// don't need to create a particular one

Space_mask Explorer::get_space_mask(Mask_type mask_type) const
{
    Space_mask mask;

    // sets all values false, like an none_free mask
    mask.gpr_static_size = false;
    mask.fpr_static_size = false;
    mask.pr_static_size = false;
    mask.cr_static_size = false;
    mask.btr_static_size = false;

    mask.num_clusters = false;
    mask.integer_units = false;
    mask.float_units = false;
    mask.memory_units = false;
    mask.branch_units = false;

    mask.L1D_size = false;
    mask.L1D_block = false;
    mask.L1D_assoc = false;

    mask.L1I_size = false;
    mask.L1I_block = false;
    mask.L1I_assoc = false;

    mask.L2U_size = false;
    mask.L2U_block = false;
    mask.L2U_assoc = false;


    mask.tcc_region = false;	//db
    mask.max_unroll_allowed = false;  //db
    mask.regroup_only = false;	//db
    mask.do_classic_opti = false;	//db
    mask.do_prepass_scalar_scheduling = false;	//db
    mask.do_postpass_scalar_scheduling = false;	//db
    mask.do_modulo_scheduling = false;	//db
    mask.memvr_profiled = false; 	//db


    switch (mask_type)
    {
	case SET_ALL:
	    mask.gpr_static_size = true;
	    mask.fpr_static_size = true;
	    mask.pr_static_size = true;
	    mask.cr_static_size = true;
	    mask.btr_static_size = true;

	    mask.num_clusters = true;
	    mask.integer_units = true;
	    mask.float_units = true;
	    mask.memory_units = true;
	    mask.branch_units = true;

	    mask.L1D_size = true;
	    mask.L1D_block = true;
	    mask.L1D_assoc = true;

	    mask.L1I_size = true;
	    mask.L1I_block = true;
	    mask.L1I_assoc = true;

	    mask.L2U_size = true;
	    mask.L2U_block = true;
	    mask.L2U_assoc = true;

	    mask.tcc_region = true;  //db
	    mask.max_unroll_allowed = true;  //db
	    mask.regroup_only = true;	//db
	    mask.do_classic_opti = true;	//db
	    mask.do_prepass_scalar_scheduling = true;	//db
	    mask.do_postpass_scalar_scheduling = true;	//db
	    mask.do_modulo_scheduling = true;	//db
	    mask.memvr_profiled = true; 	//db
	    break;

	case UNSET_ALL:
	    // do nothing, mask is already set!
	    break;

	case SET_L1D:
	    mask.L1D_size = true;
	    mask.L1D_block = true;
	    mask.L1D_assoc = true;
	    break;

	case SET_L1I:
	    mask.L1I_size = true;
	    mask.L1I_block = true;
	    mask.L1I_assoc = true;
	    break;

	case SET_L2U:
	    mask.L2U_size = true;
	    mask.L2U_block = true;
	    mask.L2U_assoc = true;
	    break;

	case SET_PROCESSOR_UNITS:
	    mask.num_clusters = true;
	    mask.integer_units = true;
	    mask.float_units = true;
	    mask.memory_units = true;
	    mask.branch_units = true;
	    break;

	case SET_PROCESSOR:

	    mask.num_clusters = true;
	    mask.gpr_static_size = true;
	    mask.fpr_static_size = true;
	    mask.pr_static_size = true;
	    mask.cr_static_size = true;
	    mask.btr_static_size = true;

	    mask.integer_units = true;
	    mask.float_units = true;
	    mask.memory_units = true;
	    mask.branch_units = true;
	    break;

	case SET_COMPILER:

	    mask.tcc_region = true;  //db
	    mask.max_unroll_allowed = true;  //db
	    mask.regroup_only = true;	//db
	    mask.do_classic_opti = true;	//db
	    mask.do_prepass_scalar_scheduling = true;	//db
	    mask.do_postpass_scalar_scheduling = true;	//db
	    mask.do_modulo_scheduling = true;	//db
	    mask.memvr_profiled = true; 	//db
	    break;
	default:
	    assert(false);
    }

    return mask;
}

////////////////////////////////////////////////////////////////////////////
vector<bool> Explorer::get_boolean_mask(const Space_mask& mask)
{
    vector<bool> b;

    b.push_back(mask.L1D_size);
    b.push_back(mask.L1D_block);
    b.push_back(mask.L1D_assoc);
    b.push_back(mask.L1I_size);
    b.push_back(mask.L1I_block);
    b.push_back(mask.L1I_assoc);
    b.push_back(mask.L2U_size);
    b.push_back(mask.L2U_block);
    b.push_back(mask.L2U_assoc);
    b.push_back(mask.integer_units);
    b.push_back(mask.float_units);
    b.push_back(mask.memory_units);
    b.push_back(mask.branch_units);
    b.push_back(mask.gpr_static_size);
    b.push_back(mask.fpr_static_size);
    b.push_back(mask.pr_static_size);
    b.push_back(mask.cr_static_size);
    b.push_back(mask.btr_static_size);
    b.push_back(mask.num_clusters);
    
    b.push_back(mask.tcc_region);  //db
    b.push_back(mask.max_unroll_allowed);  //db
    b.push_back(mask.regroup_only);	//db
    b.push_back(mask.do_classic_opti);	//db
    b.push_back(mask.do_prepass_scalar_scheduling);	//db
    b.push_back(mask.do_postpass_scalar_scheduling);	//db
    b.push_back(mask.do_modulo_scheduling);	//db
    b.push_back(mask.memvr_profiled); 	//db


    return b;
}

////////////////////////////////////////////////////////////////////////////
Space_mask Explorer::create_space_mask(const vector<bool>& boolean_mask)
{
    Space_mask mask;

    mask.L1D_size = boolean_mask[0];
    mask.L1D_block = boolean_mask[1];
    mask.L1D_assoc = boolean_mask[2];

    mask.L1I_size = boolean_mask[3];
    mask.L1I_block = boolean_mask[4];
    mask.L1I_assoc = boolean_mask[5];

    mask.L2U_size = boolean_mask[6];
    mask.L2U_block = boolean_mask[7];
    mask.L2U_assoc = boolean_mask[8];

    mask.integer_units = boolean_mask[9];
    mask.float_units = boolean_mask[10];
    mask.memory_units = boolean_mask[11];
    mask.branch_units = boolean_mask[12];

    mask.gpr_static_size = boolean_mask[13];
    mask.fpr_static_size = boolean_mask[14];
    mask.pr_static_size = boolean_mask[15];
    mask.cr_static_size = boolean_mask[16];
    mask.btr_static_size = boolean_mask[17];

    mask.num_clusters = boolean_mask[18];

    mask.tcc_region = boolean_mask[19];  //db
    mask.max_unroll_allowed = boolean_mask[20];  //db
    mask.regroup_only = boolean_mask[21];	//db
    mask.do_classic_opti = boolean_mask[22];	//db
    mask.do_prepass_scalar_scheduling = boolean_mask[23];	//db
    mask.do_postpass_scalar_scheduling = boolean_mask[24];	//db
    mask.do_modulo_scheduling = boolean_mask[25];	//db
    mask.memvr_profiled = boolean_mask[26]; 	//db
    return mask;
}

///////////////////////////////////////////////////////////////////////////////
//
// Build space functions 
//----------------------------------------------------------------------------
//
//  These function help Explorer user to create vector<Configuration>
//  spaces needed by 
//  vector<Simulation> simulate_space(vector<Configuration>& custom_space);
//

//----------------------------------------------------------------------------
// build a configuration space from a vector of simulations .
// The return value is the configuration space where simulation were
// performed
//----------------------------------------------------------------------------
vector<Configuration> Explorer::extract_space(const vector<Simulation>& sims) const
{
    vector<Configuration> temp;

    for (unsigned int i=0;i<sims.size();i++) temp.push_back(sims[i].config);

    return temp;
}

//----------------------------------------------------------------------------
// build a configuration space where parameters are explored
// exhaustively .
// Parameters with Space_mask value set to false are
// considered fixed to the default config passed as argument
//----------------------------------------------------------------------------

vector<Configuration> Explorer::build_space(const Space_mask& mask,Configuration base_conf, Space_opt opt)
{
    vector<Configuration> space;

    processor.num_clusters.set_to_first();
    do {
	if (mask.num_clusters) base_conf.num_clusters=processor.num_clusters.get_val();

	processor.integer_units.set_to_first();
	do {
	    if (mask.integer_units) base_conf.integer_units=processor.integer_units.get_val();

	    processor.float_units.set_to_first();
	    do {
		if (mask.float_units) base_conf.float_units=processor.float_units.get_val();

		processor.memory_units.set_to_first();
		do {
		    if (mask.memory_units) base_conf.memory_units=processor.memory_units.get_val();

		    processor.branch_units.set_to_first();
		    do {
			if (mask.branch_units) base_conf.branch_units=processor.branch_units.get_val();

			processor.gpr_static_size.set_to_first();
			do {
			    if (mask.gpr_static_size) base_conf.gpr_static_size=processor.gpr_static_size.get_val();

			    processor.fpr_static_size.set_to_first();
			    do {
				if (mask.fpr_static_size) base_conf.fpr_static_size=processor.fpr_static_size.get_val();

				processor.pr_static_size.set_to_first();
				do {
				    if (mask.pr_static_size) base_conf.pr_static_size=processor.pr_static_size.get_val();

				    processor.cr_static_size.set_to_first();
				    do {
					if (mask.cr_static_size) base_conf.cr_static_size=processor.cr_static_size.get_val();

					processor.btr_static_size.set_to_first();
					do {
					    if (mask.btr_static_size) base_conf.btr_static_size=processor.btr_static_size.get_val();

					    compiler.tcc_region.set_to_first();	//db
					    do{	//db
						if (mask.tcc_region) base_conf.tcc_region=compiler.tcc_region.get_val();	//db
						compiler.max_unroll_allowed.set_to_first();	//db
						do{	//db
						    if (mask.max_unroll_allowed) base_conf.max_unroll_allowed=compiler.max_unroll_allowed.get_val();	//db
						    compiler.regroup_only.set_to_first();	//db
						    do{	//db
							if (mask.regroup_only) base_conf.regroup_only=compiler.regroup_only.get_val();	//db
							compiler.do_classic_opti.set_to_first();	//db
							do{	//db
							    if (mask.do_classic_opti) base_conf.do_classic_opti=compiler.do_classic_opti.get_val();	//db
							    compiler.do_prepass_scalar_scheduling.set_to_first();	//db
							    do{	//db
								if (mask.do_prepass_scalar_scheduling) base_conf.do_prepass_scalar_scheduling=compiler.do_prepass_scalar_scheduling.get_val();	//db
								compiler.do_postpass_scalar_scheduling.set_to_first();	//db
								do{	//db
								    if (mask.do_postpass_scalar_scheduling) base_conf.do_postpass_scalar_scheduling=compiler.do_postpass_scalar_scheduling.get_val();	//db
								    compiler.do_modulo_scheduling.set_to_first();	//db
								    do{	//db
									if (mask.do_modulo_scheduling) base_conf.do_modulo_scheduling=compiler.do_modulo_scheduling.get_val();	//db
									compiler.memvr_profiled.set_to_first();	//db
									do{	//db
									    if (mask.memvr_profiled) base_conf.memvr_profiled=compiler.memvr_profiled.get_val();	//db

									    mem_hierarchy.L1D.size.set_to_first();
									    do {
										if (mask.L1D_size) base_conf.L1D_size=mem_hierarchy.L1D.size.get_val();

										mem_hierarchy.L1D.block_size.set_to_first();
										do {
										    if (mask.L1D_block) base_conf.L1D_block=mem_hierarchy.L1D.block_size.get_val();

										    mem_hierarchy.L1D.associativity.set_to_first();
										    do {
											if (mask.L1D_assoc) base_conf.L1D_assoc=mem_hierarchy.L1D.associativity.get_val();

											mem_hierarchy.L1I.size.set_to_first();
											do {
											    if (mask.L1I_size) base_conf.L1I_size=mem_hierarchy.L1I.size.get_val();

											    mem_hierarchy.L1I.block_size.set_to_first();
											    do {
												if (mask.L1I_block) base_conf.L1I_block=mem_hierarchy.L1I.block_size.get_val();

												mem_hierarchy.L1I.associativity.set_to_first();
												do {
												    if (mask.L1I_assoc) base_conf.L1I_assoc=mem_hierarchy.L1I.associativity.get_val();

												    mem_hierarchy.L2U.size.set_to_first();
												    do {
													if (mask.L2U_size) base_conf.L2U_size=mem_hierarchy.L2U.size.get_val();

													mem_hierarchy.L2U.block_size.set_to_first();
													do {
													    if (mask.L2U_block) base_conf.L2U_block=mem_hierarchy.L2U.block_size.get_val();

													    mem_hierarchy.L2U.associativity.set_to_first();
													    do {
														if (mask.L2U_assoc) base_conf.L2U_assoc=mem_hierarchy.L2U.associativity.get_val();

														//////////////////////////////////////////////////////////////////////////////////
														//  inner loop

														// if all cache parameters are valid
														if (base_conf.is_feasible()) space.push_back(base_conf);

														//////////////////////////////////////////////////////////////////////////////////
													    }while ( (mask.L2U_assoc)&&(mem_hierarchy.L2U.associativity.increase()));
													}while ( (mask.L2U_block)&&(mem_hierarchy.L2U.block_size.increase() ));
												    }while ( (mask.L2U_size)&&(mem_hierarchy.L2U.size.increase()));
												}while ( (mask.L1I_assoc)&&(mem_hierarchy.L1I.associativity.increase()));
											    }while ( (mask.L1I_block)&&(mem_hierarchy.L1I.block_size.increase() ));
											}while ( (mask.L1I_size)&&(mem_hierarchy.L1I.size.increase()));
										    } while ( (mask.L1D_assoc)&&(mem_hierarchy.L1D.associativity.increase()));
										} while ( (mask.L1D_block)&&(mem_hierarchy.L1D.block_size.increase() ));
									    } while ( (mask.L1D_size)&&(mem_hierarchy.L1D.size.increase()));
									}while ( (mask.memvr_profiled)&&(compiler.memvr_profiled.increase()));	//db
								    }while ( (mask.do_modulo_scheduling)&&(compiler.do_modulo_scheduling.increase()));	//db
								}while ( (mask.do_postpass_scalar_scheduling)&&(compiler.do_postpass_scalar_scheduling.increase()));	//db
							    }while ( (mask.do_prepass_scalar_scheduling)&&(compiler.do_prepass_scalar_scheduling.increase()));	//db
							}while ( (mask.do_classic_opti)&&(compiler.do_classic_opti.increase()));	//db
						    }while ( (mask.regroup_only)&&(compiler.regroup_only.increase()));	//db
						}while ( (mask.max_unroll_allowed)&&(compiler.max_unroll_allowed.increase()));	//db
					    }while ( (mask.tcc_region)&&(compiler.tcc_region.increase()));	//db
					}while ( (mask.btr_static_size) && (processor.btr_static_size.increase() ) );
				    }while ( (mask.cr_static_size) && (processor.cr_static_size.increase() ) );
				}while ( (mask.pr_static_size) && (processor.pr_static_size.increase() ) );
			    } while ( (mask.fpr_static_size) && (processor.fpr_static_size.increase() ) );
			} while ( (mask.gpr_static_size) && (processor.gpr_static_size.increase() ) );
		    } while ( (mask.branch_units) && (processor.branch_units.increase()) );
		} while ( (mask.memory_units) && (processor.memory_units.increase()) ) ;
	    } while( (mask.float_units) && (processor.float_units.increase()) );
	} while ( (mask.integer_units) && (processor.integer_units.increase() ));
    } while ( (mask.num_clusters) && (processor.num_clusters.increase() ));

    return space;

}


//----------------------------------------------------------------------------
// build a configuration space where parameters are explored
// exhaustively .
// Parameters with Space_mask value set to false are
// considered fixed to their default value and will not be explored.
//----------------------------------------------------------------------------

vector<Configuration> Explorer::build_space(const Space_mask& mask) 
{
    Configuration default_conf = create_configuration();

    return build_space(mask,default_conf,STANDARD);
}

////////////////////////////////////////////////////////////////////////////
vector<Configuration> Explorer::build_space(const Space_mask& mask,Configuration base_conf) 
{
    return build_space(mask,base_conf,STANDARD);
}

////////////////////////////////////////////////////////////////////////////
vector<Configuration> Explorer::build_space(const Space_mask& mask,Space_opt opt) 
{
    Configuration default_conf = create_configuration();

    return build_space(mask,default_conf,opt);
}

////////////////////////////////////////////////////////////////////////////////
// build a new space cross merging two pre-existing spaces .
// Each configuration of s1 space will be merged with each of the
// configurations of s2 . 
// Space_masks specifies which parameters of each space must be
// considered . Note that a parameter mask value cannot be set true in
// both mask1 and mask2.
// The parameters that are unset in both mask1 and mask2 will be
// considered to their default value.
// Example:
// s1 = { a1, b1, c1 }     s2 { a3, b3, c3 }
//      { a2, b2, c2 }        { a4, b4, c4 }
//
//      mask1 = { 1,0,0}   mask2 = {0,0,1}
//
// s12 = { a1,def,c3 }
//       { a1,def,c4 }
//       { a2,def,c3 }
//       { a2,def,c4 }
//
vector<Configuration> Explorer::build_space_cross_merge(const vector<Configuration>& s1,
	                                                const vector<Configuration>& s2,
							const Space_mask& mask1,
							const Space_mask& mask2 ) const
{
    Configuration base_conf = create_configuration();
    
    vector<Configuration> merged_space;

    for (unsigned int n1=0;n1<s1.size();n1++)
    {
	if (mask1.gpr_static_size) base_conf.gpr_static_size=s1[n1].gpr_static_size;
	if (mask1.fpr_static_size) base_conf.fpr_static_size=s1[n1].fpr_static_size;
	if (mask1.pr_static_size) base_conf.pr_static_size=s1[n1].pr_static_size;
	if (mask1.cr_static_size) base_conf.cr_static_size=s1[n1].cr_static_size;
	if (mask1.btr_static_size) base_conf.btr_static_size=s1[n1].btr_static_size;

	if (mask1.num_clusters) base_conf.num_clusters=s1[n1].num_clusters;
	if (mask1.integer_units) base_conf.integer_units=s1[n1].integer_units;
	if (mask1.float_units) base_conf.float_units=s1[n1].float_units;
	if (mask1.memory_units) base_conf.memory_units=s1[n1].memory_units;
	if (mask1.branch_units) base_conf.branch_units=s1[n1].branch_units;

	if (mask1.L1D_size) base_conf.L1D_size = s1[n1].L1D_size;
	if (mask1.L1D_block) base_conf.L1D_block = s1[n1].L1D_block;
	if (mask1.L1D_assoc) base_conf.L1D_assoc = s1[n1].L1D_assoc;

	if (mask1.L1I_size) base_conf.L1I_size = s1[n1].L1I_size;
	if (mask1.L1I_block) base_conf.L1I_block = s1[n1].L1I_block;
	if (mask1.L1I_assoc) base_conf.L1I_assoc = s1[n1].L1I_assoc;

	if (mask1.L2U_size) base_conf.L2U_size = s1[n1].L2U_size;
	if (mask1.L2U_block) base_conf.L2U_block = s1[n1].L2U_block;
	if (mask1.L2U_assoc) base_conf.L2U_assoc = s1[n1].L2U_assoc;

	if (mask1.tcc_region) base_conf.tcc_region = s1[n1].tcc_region;	//db
	if (mask1.max_unroll_allowed) base_conf.max_unroll_allowed = s1[n1].max_unroll_allowed;	//db
	if (mask1.regroup_only) base_conf.regroup_only = s1[n1].regroup_only;	//db
	if (mask1.do_classic_opti) base_conf.do_classic_opti = s1[n1].do_classic_opti;	//db
	if (mask1.do_prepass_scalar_scheduling) base_conf.do_prepass_scalar_scheduling = s1[n1].do_prepass_scalar_scheduling;	//db
	if (mask1.do_postpass_scalar_scheduling) base_conf.do_postpass_scalar_scheduling = s1[n1].do_postpass_scalar_scheduling;	//db
	if (mask1.do_modulo_scheduling) base_conf.do_modulo_scheduling = s1[n1].do_modulo_scheduling;	//db
	if (mask1.memvr_profiled) base_conf.memvr_profiled = s1[n1].memvr_profiled;	//db

	for(unsigned int n2=0;n2<s2.size();n2++)
	{
	    if (mask2.gpr_static_size) base_conf.gpr_static_size=s2[n2].gpr_static_size;
	    if (mask2.fpr_static_size) base_conf.fpr_static_size=s2[n2].fpr_static_size;
	    if (mask2.pr_static_size) base_conf.pr_static_size=s2[n2].pr_static_size;
	    if (mask2.cr_static_size) base_conf.cr_static_size=s2[n2].cr_static_size;
	    if (mask2.btr_static_size) base_conf.btr_static_size=s2[n2].btr_static_size;

	    if (mask2.num_clusters) base_conf.num_clusters=s2[n2].num_clusters;
	    if (mask2.integer_units) base_conf.integer_units=s2[n2].integer_units;
	    if (mask2.float_units) base_conf.float_units=s2[n2].float_units;
	    if (mask2.memory_units) base_conf.memory_units=s2[n2].memory_units;
	    if (mask2.branch_units) base_conf.branch_units=s2[n2].branch_units;

	    if (mask2.L1D_size) base_conf.L1D_size = s2[n2].L1D_size;
	    if (mask2.L1D_block) base_conf.L1D_block = s2[n2].L1D_block;
	    if (mask2.L1D_assoc) base_conf.L1D_assoc = s2[n2].L1D_assoc;

	    if (mask2.L1I_size) base_conf.L1I_size = s2[n2].L1I_size;
	    if (mask2.L1I_block) base_conf.L1I_block = s2[n2].L1I_block;
	    if (mask2.L1I_assoc) base_conf.L1I_assoc = s2[n2].L1I_assoc;

	    if (mask2.L2U_size) base_conf.L2U_size = s2[n2].L2U_size;
	    if (mask2.L2U_block) base_conf.L2U_block = s2[n2].L2U_block;
	    if (mask2.L2U_assoc) base_conf.L2U_assoc = s2[n2].L2U_assoc;

	    if (mask2.tcc_region) base_conf.tcc_region = s2[n2].tcc_region;	//db
	    if (mask2.max_unroll_allowed) base_conf.max_unroll_allowed = s2[n2].max_unroll_allowed;	//db
	    if (mask2.regroup_only) base_conf.regroup_only = s2[n2].regroup_only;	//db
	    if (mask2.do_classic_opti) base_conf.do_classic_opti = s2[n2].do_classic_opti;	//db
	    if (mask2.do_prepass_scalar_scheduling) base_conf.do_prepass_scalar_scheduling = s2[n2].do_prepass_scalar_scheduling;	//db
	    if (mask2.do_postpass_scalar_scheduling) base_conf.do_postpass_scalar_scheduling = s2[n2].do_postpass_scalar_scheduling;	//db
	    if (mask2.do_modulo_scheduling) base_conf.do_modulo_scheduling = s2[n2].do_modulo_scheduling;	//db
	    if (mask2.memvr_profiled) base_conf.memvr_profiled = s2[n2].memvr_profiled;	//db
	    // resulting combination of two configurations may result
	    // in not feasible config

    if ( base_conf.is_feasible() && !configuration_present(base_conf,merged_space) ) 
	merged_space.push_back(base_conf);

	}
    }
    return merged_space;
}

////////////////////////////////////////////////////////////////////////////
bool Explorer::configuration_present(const Configuration& conf, const vector<Configuration>& space) const
{

    if (space.size()==0) return false;

    for(unsigned int i = 0;i<space.size();i++)
    {
	if(   (conf.gpr_static_size==space[i].gpr_static_size)
	    &&(conf.fpr_static_size==space[i].fpr_static_size)
	    &&(conf.pr_static_size==space[i].pr_static_size)
	    &&(conf.cr_static_size==space[i].cr_static_size)
	    &&(conf.btr_static_size==space[i].btr_static_size)
	    &&(conf.num_clusters == space[i].num_clusters)
	    &&(conf.integer_units == space[i].integer_units)
	    &&(conf.float_units == space[i].float_units)
	    &&(conf.memory_units == space[i].memory_units)
	    &&(conf.branch_units == space[i].branch_units)
	    &&(conf.L1D_size == space[i].L1D_size)
	    &&(conf.L1D_block == space[i].L1D_block)
	    &&(conf.L1D_assoc == space[i].L1D_assoc)
	    &&(conf.L1I_size == space[i].L1I_size)
	    &&(conf.L1I_block == space[i].L1I_block)
	    &&(conf.L1I_assoc == space[i].L1I_assoc)
	    &&(conf.L2U_size == space[i].L2U_size)
	    &&(conf.L2U_block == space[i].L2U_block)
	    &&(conf.L2U_assoc == space[i].L2U_assoc)
	    &&(conf.tcc_region == space[i].tcc_region) 		//db
	    &&(conf.max_unroll_allowed == space[i].max_unroll_allowed)	 	//db
	    &&(conf.regroup_only == space[i].regroup_only)	 	//db
	    &&(conf.do_classic_opti == space[i].do_classic_opti)	 	//db
	    &&(conf.do_prepass_scalar_scheduling == space[i].do_prepass_scalar_scheduling)	 	//db
	    &&(conf.do_postpass_scalar_scheduling == space[i].do_postpass_scalar_scheduling)	 	//db
            &&(conf.do_modulo_scheduling == space[i].do_modulo_scheduling)	 	//db
	    &&(conf.memvr_profiled == space[i].memvr_profiled))	 	//db
	    return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////
int Explorer::simulation_present(const Simulation& sim,const vector<Simulation>& simulations) const
{
    if (simulations.size()==0) return (-1);

    for (int i=0;i<simulations.size();++i)
    {
	if ( 
		(simulations[i].area == sim.area) &&
		(simulations[i].energy == sim.energy) &&
		(simulations[i].exec_time == sim.exec_time) 
	   )
	    return (i);
    }
    return (-1);
}

////////////////////////////////////////////////////////////////////////////
Space_mask Explorer::mask_union(Space_mask& m1,Space_mask& m2) const
{
    Space_mask mask;
    mask.gpr_static_size = (m1.gpr_static_size) || (m2.gpr_static_size);
    mask.fpr_static_size = (m1.fpr_static_size) || (m2.fpr_static_size);
    mask.pr_static_size = (m1.pr_static_size) || (m2.pr_static_size);
    mask.cr_static_size = (m1.cr_static_size) || (m2.cr_static_size);
    mask.btr_static_size = (m1.btr_static_size) || (m2.btr_static_size);

    mask.num_clusters = (m1.num_clusters) || (m2.num_clusters);
    mask.integer_units = (m1.integer_units) || (m2.integer_units);
    mask.float_units = (m1.float_units) || (m2.float_units);
    mask.memory_units = (m1.memory_units) || (m2.memory_units);
    mask.branch_units = (m1.branch_units) || (m2.branch_units);

    mask.L1D_size = (m1.L1D_size) || (m2.L1D_size);
    mask.L1D_block = (m1.L1D_block) || (m2.L1D_block);
    mask.L1D_assoc =(m1.L1D_assoc) || (m2.L1D_assoc);

    mask.L1I_size = (m1.L1I_size) || (m2.L1I_size);
    mask.L1I_block = (m1.L1I_block) || (m2.L1I_block);
    mask.L1I_assoc =(m1.L1I_assoc) || (m2.L1I_assoc);

    mask.L2U_size = (m1.L2U_size) || (m2.L2U_size);
    mask.L2U_block = (m1.L2U_block) || (m2.L2U_block);
    mask.L2U_assoc =(m1.L2U_assoc) || (m2.L2U_assoc);

    mask.tcc_region = (m1.tcc_region)||(m2.tcc_region);  //db
    mask.max_unroll_allowed = (m1.max_unroll_allowed)||(m2.max_unroll_allowed);  //db
    mask.regroup_only = (m1.regroup_only)||(m2.regroup_only);	//db
    mask.do_classic_opti = (m1.do_classic_opti)||(m2.do_classic_opti);	//db
    mask.do_prepass_scalar_scheduling = (m1.do_prepass_scalar_scheduling)||(m2.do_prepass_scalar_scheduling);	//db
    mask.do_postpass_scalar_scheduling = (m1.do_postpass_scalar_scheduling)||(m2.do_postpass_scalar_scheduling);	//db
    mask.do_modulo_scheduling = (m1.do_modulo_scheduling)||(m2.do_modulo_scheduling);	//db
    mask.memvr_profiled = (m1.memvr_profiled)||(m2.memvr_profiled); 	//db
    return mask;

}

////////////////////////////////////////////////////////////////////////////
// compute the total number of configurations considering all
// possible values of paremeters whose mask boolean is set true

long double Explorer::get_space_size(const Space_mask& mask) const
{
    double size = 1;


    if (mask.gpr_static_size) size = size*(processor.gpr_static_size.get_size());
    if (mask.fpr_static_size) size = size*(processor.fpr_static_size.get_size());
    if (mask.pr_static_size) size = size*(processor.pr_static_size.get_size());
    if (mask.cr_static_size) size = size*(processor.cr_static_size.get_size());
    if (mask.btr_static_size) size = size*(processor.btr_static_size.get_size());

    if (mask.num_clusters) size = size*(processor.num_clusters.get_size());
    if (mask.integer_units) size = size*(processor.integer_units.get_size());
    if (mask.float_units) size = size*(processor.float_units.get_size());
    if (mask.memory_units) size = size*(processor.memory_units.get_size());
    if (mask.branch_units) size = size*(processor.branch_units.get_size());

    if (mask.L1D_size) size = size*(mem_hierarchy.L1D.size.get_size());
    if (mask.L1D_block) size = size*(mem_hierarchy.L1D.block_size.get_size());
    if (mask.L1D_assoc) size = size*(mem_hierarchy.L1D.associativity.get_size());

    if (mask.L1I_size) size = size*(mem_hierarchy.L1I.size.get_size());
    if (mask.L1I_block) size = size*(mem_hierarchy.L1I.block_size.get_size());
    if (mask.L1I_assoc) size = size*(mem_hierarchy.L1I.associativity.get_size());

    if (mask.L2U_size) size = size*(mem_hierarchy.L2U.size.get_size());
    if (mask.L2U_block) size = size*(mem_hierarchy.L2U.block_size.get_size());
    if (mask.L2U_assoc) size = size*(mem_hierarchy.L2U.associativity.get_size());

    if (mask.tcc_region) size = size*(compiler.tcc_region.get_size());	//db
    if (mask.max_unroll_allowed) size = size*(compiler.max_unroll_allowed.get_size());	//db
    if (mask.regroup_only) size = size*(compiler.regroup_only.get_size());	//db
    if (mask.do_classic_opti) size = size*(compiler.do_classic_opti.get_size());	//db
    if (mask.do_prepass_scalar_scheduling) size = size*(compiler.do_prepass_scalar_scheduling.get_size());	//db
    if (mask.do_postpass_scalar_scheduling) size = size*(compiler.do_postpass_scalar_scheduling.get_size());	//db
    if (mask.do_modulo_scheduling) size = size*(compiler.do_modulo_scheduling.get_size());	//db
    if (mask.memvr_profiled) size = size*(compiler.memvr_profiled.get_size());	//db

    return size;
}

////////////////////////////////////////////////////////////////////////////
// if no mask is specified, total configuration space size is returned
long double Explorer::get_space_size() const
{
    Space_mask all_parm = get_space_mask(SET_ALL);
    return get_space_size(all_parm);

}


///////////////////////////////////////////////////////////////////
//  Misc Functions 
// ---------------------------------------------------------------------------
// functions used by explorer methods
//
void Explorer::append_simulations(vector<Simulation>& dest,const vector<Simulation>& new_sims)
{
    for (unsigned int i=0;i<new_sims.size();i++) 
    {
	if (simulation_present(new_sims[i],dest) < 0) dest.push_back(new_sims[i]);
    }
}

////////////////////////////////////////////////////////////////////////////
int Explorer::get_sim_counter() const
{
    return sim_counter;
}

////////////////////////////////////////////////////////////////////////////
void Explorer::reset_sim_counter() 
{
    sim_counter=0;
}

////////////////////////////////////////////////////////////////////////////
vector<pair<int,int> > Explorer::getParametersNumber()
{

  vector<pair<int,int> > v;
  
  v.push_back(pair<int,int>(1, processor.integer_units.get_size()) );
  v.push_back(pair<int,int>(1, processor.float_units.get_size()));
  v.push_back(pair<int,int>(1, processor.branch_units.get_size()));
  v.push_back(pair<int,int>(1, processor.memory_units.get_size()));
  v.push_back(pair<int,int>(1, processor.gpr_static_size.get_size()));
  v.push_back(pair<int,int>(1, processor.fpr_static_size.get_size()));
  v.push_back(pair<int,int>(1, processor.pr_static_size.get_size()));
  v.push_back(pair<int,int>(1, processor.cr_static_size.get_size()));
  v.push_back(pair<int,int>(1, processor.btr_static_size.get_size()));
  v.push_back(pair<int,int>(1, mem_hierarchy.L1D.size.get_size()));
  v.push_back(pair<int,int>(1, mem_hierarchy.L1D.block_size.get_size()));
  v.push_back(pair<int,int>(1, mem_hierarchy.L1D.associativity.get_size()));
  v.push_back(pair<int,int>(1, mem_hierarchy.L1I.size.get_size()));
  v.push_back(pair<int,int>(1, mem_hierarchy.L1I.block_size.get_size()));
  v.push_back(pair<int,int>(1, mem_hierarchy.L1I.associativity.get_size()));
  v.push_back(pair<int,int>(1, mem_hierarchy.L2U.size.get_size()));
  v.push_back(pair<int,int>(1, mem_hierarchy.L2U.block_size.get_size()));
  v.push_back(pair<int,int>(1, mem_hierarchy.L2U.associativity.get_size()));
  v.push_back(pair<int,int>(1, processor.num_clusters.get_size()));
  v.push_back(pair<int,int>(1,compiler.tcc_region.get_size()));	//db
  v.push_back(pair<int,int>(1,compiler.max_unroll_allowed.get_size()));	//db
  v.push_back(pair<int,int>(1,compiler.regroup_only.get_size()));	//db
  v.push_back(pair<int,int>(1,compiler.do_classic_opti.get_size()));	//db
  v.push_back(pair<int,int>(1,compiler.do_prepass_scalar_scheduling.get_size()));	//db
  v.push_back(pair<int,int>(1,compiler.do_postpass_scalar_scheduling.get_size()));	//db
  v.push_back(pair<int,int>(1,compiler.do_modulo_scheduling.get_size()));	//db
  v.push_back(pair<int,int>(1,compiler.memvr_profiled.get_size()));	//db
  return v;
}
	
////////////////////////////////////////////////////////////////////////////
vector<pair<int,int> > Explorer::getParameterRanges()
{
  vector<pair<int,int> > v;

  v.push_back(pair<int,int>(processor.integer_units.get_first(), processor.integer_units.get_last()));
  v.push_back(pair<int,int>(processor.float_units.get_first(), processor.float_units.get_last()));
  v.push_back(pair<int,int>(processor.branch_units.get_first(), processor.branch_units.get_last()));
  v.push_back(pair<int,int>(processor.memory_units.get_first(), processor.memory_units.get_last()));
  v.push_back(pair<int,int>(processor.gpr_static_size.get_first(), processor.gpr_static_size.get_last()));
  v.push_back(pair<int,int>(processor.fpr_static_size.get_first(), processor.fpr_static_size.get_last()));
  v.push_back(pair<int,int>(processor.pr_static_size.get_first(), processor.pr_static_size.get_last()));
  v.push_back(pair<int,int>(processor.cr_static_size.get_first(), processor.cr_static_size.get_last()));
  v.push_back(pair<int,int>(processor.btr_static_size.get_first(), processor.btr_static_size.get_last()));
  v.push_back(pair<int,int>(mem_hierarchy.L1D.size.get_first(), mem_hierarchy.L1D.size.get_last()));
  v.push_back(pair<int,int>(mem_hierarchy.L1D.block_size.get_first(), mem_hierarchy.L1D.block_size.get_last()));
  v.push_back(pair<int,int>(mem_hierarchy.L1D.associativity.get_first(), mem_hierarchy.L1D.associativity.get_last()));
  v.push_back(pair<int,int>(mem_hierarchy.L1I.size.get_first(), mem_hierarchy.L1I.size.get_last()));
  v.push_back(pair<int,int>(mem_hierarchy.L1I.block_size.get_first(), mem_hierarchy.L1I.block_size.get_last()));
  v.push_back(pair<int,int>(mem_hierarchy.L1I.associativity.get_first(), mem_hierarchy.L1I.associativity.get_last()));
  v.push_back(pair<int,int>(mem_hierarchy.L2U.size.get_first(), mem_hierarchy.L2U.size.get_last()));
  v.push_back(pair<int,int>(mem_hierarchy.L2U.block_size.get_first(), mem_hierarchy.L2U.block_size.get_last()));
  v.push_back(pair<int,int>(mem_hierarchy.L2U.associativity.get_first(), mem_hierarchy.L2U.associativity.get_last()));
  v.push_back(pair<int,int>(processor.num_clusters.get_first(), processor.num_clusters.get_last()));
  v.push_back(pair<int,int>(compiler.tcc_region.get_first(),compiler.tcc_region.get_last()));	//db
  v.push_back(pair<int,int>(compiler.max_unroll_allowed.get_first(),compiler.max_unroll_allowed.get_last()));	//db
  v.push_back(pair<int,int>(compiler.regroup_only.get_first(),compiler.regroup_only.get_last()));	//db
  v.push_back(pair<int,int>(compiler.do_classic_opti.get_first(),compiler.do_classic_opti.get_last()));	//db
  v.push_back(pair<int,int>(compiler.do_prepass_scalar_scheduling.get_first(),compiler.do_prepass_scalar_scheduling.get_last()));	//db
  v.push_back(pair<int,int>(compiler.do_postpass_scalar_scheduling.get_first(),compiler.do_postpass_scalar_scheduling.get_last()));	//db
  v.push_back(pair<int,int>(compiler.do_modulo_scheduling.get_first(),compiler.do_modulo_scheduling.get_last()));	//db
  v.push_back(pair<int,int>(compiler.memvr_profiled.get_first(),compiler.memvr_profiled.get_last()));	//db

  return v;
}

////////////////////////////////////////////////////////////////////////////

void Explorer::set_force_simulation(bool s)
{
    force_simulation = s;
}

void Explorer::set_space_name(const string& spacename)
{
    current_space = spacename;
}

string Explorer::get_space_name() const
{
    return current_space;
}
void Explorer::load_space_file(const string& filename)
{
   std::ifstream input_file (filename.c_str());
   string word;

   if (!input_file)
   {
       cout << "\n Error while reading " << filename;
       wait_key();
   }
   else
   {

	go_until("[BEGIN_SPACE]",input_file);

	int val;
	vector<int> values;

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	mem_hierarchy.L1D.size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	mem_hierarchy.L1D.block_size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	mem_hierarchy.L1D.associativity.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	mem_hierarchy.L1I.size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	mem_hierarchy.L1I.block_size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	mem_hierarchy.L1I.associativity.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	mem_hierarchy.L2U.size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	mem_hierarchy.L2U.block_size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	mem_hierarchy.L2U.associativity.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	processor.integer_units.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	processor.float_units.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	processor.memory_units.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	processor.branch_units.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	processor.gpr_static_size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	processor.fpr_static_size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	processor.pr_static_size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	processor.cr_static_size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	processor.btr_static_size.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	processor.num_clusters.set_values(values,val);

	processor.set_to_default();
	//trimaran_interface->save_processor_config(processor,hmdes_filename);

//add db	
	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	compiler.tcc_region.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	compiler.max_unroll_allowed.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	compiler.regroup_only.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	compiler.do_classic_opti.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	compiler.do_prepass_scalar_scheduling.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	compiler.do_postpass_scalar_scheduling.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	compiler.do_modulo_scheduling.set_values(values,val);

	values.clear();
	input_file >> word; // skip parameter label
	while ( (input_file>>val) && (val!=0)) { values.push_back(val); } // reads val until 0
	input_file >> word; // skip default label
	input_file >> val;  // get default value
	compiler.memvr_profiled.set_values(values,val);

//end add db
   }

}

void Explorer::save_space_file(const string& filename)
{
    std::ofstream output_file(filename.c_str());
    if (!output_file)
    {
	cout << "\n Error while saving " << filename ;
	wait_key();
    }
    else
    {
	output_file << "\n\n [BEGIN_SPACE]";

	processor.integer_units.set_to_first();
	processor.float_units.set_to_first();
	processor.branch_units.set_to_first();
	processor.memory_units.set_to_first();
	processor.gpr_static_size.set_to_first();
	processor.fpr_static_size.set_to_first();
	processor.pr_static_size.set_to_first();
	processor.cr_static_size.set_to_first();
	processor.btr_static_size.set_to_first();
	processor.num_clusters.set_to_first();

	mem_hierarchy.L1D.size.set_to_first();
	mem_hierarchy.L1D.block_size.set_to_first();
	mem_hierarchy.L1D.associativity.set_to_first();

	mem_hierarchy.L1I.size.set_to_first();
	mem_hierarchy.L1I.block_size.set_to_first();
	mem_hierarchy.L1I.associativity.set_to_first();

	mem_hierarchy.L2U.size.set_to_first();
	mem_hierarchy.L2U.block_size.set_to_first();
	mem_hierarchy.L2U.associativity.set_to_first();

	compiler.tcc_region.set_to_first();	//db
	compiler.max_unroll_allowed.set_to_first();	//db
	compiler.regroup_only.set_to_first();	//db
	compiler.do_classic_opti.set_to_first();	//db
	compiler.do_prepass_scalar_scheduling.set_to_first();	//db
	compiler.do_postpass_scalar_scheduling.set_to_first();	//db
	compiler.do_modulo_scheduling.set_to_first();	//db
	compiler.memvr_profiled.set_to_first();	//db

	output_file << "\nL1D_size " ;
	do { output_file << mem_hierarchy.L1D.size.get_val() << " "; } while (mem_hierarchy.L1D.size.increase()); 
	output_file << "0";
	output_file << "\n DEFAULT " << mem_hierarchy.L1D.size.get_default();

	output_file << "\nL1D_block_size " ;
	do { output_file << mem_hierarchy.L1D.block_size.get_val() << " "; } while (mem_hierarchy.L1D.block_size.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << mem_hierarchy.L1D.block_size.get_default();

	output_file << "\nL1D_associativity " ;
	do { output_file << mem_hierarchy.L1D.associativity.get_val() << " "; } while (mem_hierarchy.L1D.associativity.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << mem_hierarchy.L1D.associativity.get_default();

	output_file << "\nL1I_size " ;
	do { output_file << mem_hierarchy.L1I.size.get_val() << " "; } while (mem_hierarchy.L1I.size.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << mem_hierarchy.L1I.size.get_default();

	output_file << "\nL1I_block_size " ;
	do { output_file << mem_hierarchy.L1I.block_size.get_val() << " "; } while (mem_hierarchy.L1I.block_size.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << mem_hierarchy.L1I.block_size.get_default();

	output_file << "\nL1I_associativity " ;
	do { output_file << mem_hierarchy.L1I.associativity.get_val() << " "; } while (mem_hierarchy.L1I.associativity.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << mem_hierarchy.L1I.associativity.get_default();

	output_file << "\nL2U_size " ;
	do { output_file << mem_hierarchy.L2U.size.get_val() << " "; } while (mem_hierarchy.L2U.size.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << mem_hierarchy.L2U.size.get_default();

	output_file << "\nL2U_block_size " ;
	do { output_file << mem_hierarchy.L2U.block_size.get_val() << " "; } while (mem_hierarchy.L2U.block_size.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << mem_hierarchy.L2U.block_size.get_default();

	output_file << "\nL2U_associativity " ;
	do { output_file << mem_hierarchy.L2U.associativity.get_val() << " "; } while (mem_hierarchy.L2U.associativity.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << mem_hierarchy.L2U.associativity.get_default();

	output_file << "\ninteger_units ";
	do { output_file << processor.integer_units.get_val() << " "; } while (processor.integer_units.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << processor.integer_units.get_default();

	output_file << "\nfloat_units ";
	do { output_file << processor.float_units.get_val() << " "; } while (processor.float_units.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << processor.float_units.get_default();

	output_file << "\nmemory_units ";
	do { output_file << processor.memory_units.get_val() << " "; } while (processor.memory_units.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << processor.memory_units.get_default();

	output_file << "\nbranch_units ";
	do { output_file << processor.branch_units.get_val() << " "; } while (processor.branch_units.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << processor.branch_units.get_default();

	output_file << "\ngpr_static_size ";
	do { output_file << processor.gpr_static_size.get_val() << " "; } while (processor.gpr_static_size.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << processor.gpr_static_size.get_default();

	output_file << "\nfpr_static_size ";
	do { output_file << processor.fpr_static_size.get_val() << " "; } while (processor.fpr_static_size.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << processor.fpr_static_size.get_default();

	output_file << "\npr_static_size ";
	do { output_file << processor.pr_static_size.get_val() << " "; } while (processor.pr_static_size.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << processor.pr_static_size.get_default();

	output_file << "\ncr_static_size ";
	do { output_file << processor.cr_static_size.get_val() << " "; } while (processor.cr_static_size.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << processor.cr_static_size.get_default();

	output_file << "\nbtr_static_size ";
	do { output_file << processor.btr_static_size.get_val() << " "; } while (processor.btr_static_size.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << processor.btr_static_size.get_default();

	output_file << "\nnum_clusters ";
	do { output_file << processor.num_clusters.get_val() << " "; } while (processor.num_clusters.increase());
	output_file << "0";
	output_file << "\n DEFAULT " << processor.num_clusters.get_default();

	output_file <<"\ntcc_region "; 	//db
	do { output_file << compiler.tcc_region.get_val() << " "; } while (compiler.tcc_region.increase()); //db
	output_file << "0";	//db
	output_file << "\n DEFAULT " << compiler.tcc_region.get_default();	//db

	output_file <<"\nmax_unroll_allowed "; 	//db
	do { output_file << compiler.max_unroll_allowed.get_val() << " "; } while (compiler.max_unroll_allowed.increase()); //db
	output_file << "0";	//db
	output_file << "\n DEFAULT " << compiler.max_unroll_allowed.get_default();	//db

	output_file <<"\nregroup_only "; 	//db
	do { output_file << compiler.regroup_only.get_val() << " "; } while (compiler.regroup_only.increase()); //db
	output_file << "0";	//db
	output_file << "\n DEFAULT " << compiler.regroup_only.get_default();	//db

	output_file <<"\ndo_classic_opti "; 	//db
	do { output_file << compiler.do_classic_opti.get_val() << " "; } while (compiler.do_classic_opti.increase()); //db
	output_file << "0";	//db
	output_file << "\n DEFAULT " << compiler.do_classic_opti.get_default();	//db

	output_file <<"\ndo_prepass_scalar_scheduling "; 	//db
	do { output_file << compiler.do_prepass_scalar_scheduling.get_val() << " "; } while (compiler.do_prepass_scalar_scheduling.increase()); //db
	output_file << "0";	//db
	output_file << "\n DEFAULT " << compiler.do_prepass_scalar_scheduling.get_default();	//db

	output_file <<"\ndo_postpass_scalar_scheduling "; 	//db
	do { output_file << compiler.do_postpass_scalar_scheduling.get_val() << " "; } while (compiler.do_postpass_scalar_scheduling.increase()); //db
	output_file << "0";	//db
	output_file << "\n DEFAULT " << compiler.do_postpass_scalar_scheduling.get_default();	//db

	output_file <<"\ndo_modulo_scheduling "; 	//db
	do { output_file << compiler.do_modulo_scheduling.get_val() << " "; } while (compiler.do_modulo_scheduling.increase()); //db
	output_file << "0";	//db
	output_file << "\n DEFAULT " << compiler.do_modulo_scheduling.get_default();	//db

	output_file <<"\nmemvr_profiled "; 	//db
	do { output_file << compiler.memvr_profiled.get_val() << " "; } while (compiler.memvr_profiled.increase()); //db
	output_file << "0";	//db
	output_file << "\n DEFAULT " << compiler.memvr_profiled.get_default();	//db

	output_file << "\n\n [END_SPACE]";

    }

}

void Explorer::set_base_dir(const string& dir)
{
    base_dir = dir;
    trimaran_interface->set_environment(dir);
}

// TODO: shouldn't get it from trimaran interface ?
string Explorer::get_base_dir() const
{
    return base_dir;
}
////////////////////////////////////////////////////////////////////////////
void Explorer::test()
{
    // PLACE CODE HERE TO QUICKLY TEST YOUR OWN ALGORITHM

    /* Example 1 ******************************************
     * using a predefined mask to make an exhaustive
     * exploration of a subset of parameters
     * ****************************************************
    Space_mask mask = get_space_mask(SET_L1D);
    vector<Configuration> space = build_space(mask);
    vector<Simulation> res = simulate_space(space);

    string filename = "L1D_EXHA";

    save_simulations(res,filename+".exp");
    vector<Simulation> pareto_set = get_pareto(res);

    save_simulations(pareto_set,filename+".pareto.exp");
    */

}

void Explorer::init_approximation()
{
    if (Options.approx_settings.enabled==1)
    {
	cout << "\nFuzzy Approximation Enabled\n";
	if (function_approx != NULL) free(function_approx);
	function_approx = new CFuzzyFunctionApproximation();
	function_approx->FuzzySetsInit(getParametersNumber());
	function_approx->Init(Options.approx_settings.threshold,Options.approx_settings.min, Options.approx_settings.max,n_objectives());
    } 
    else if (Options.approx_settings.enabled==2)
    {

	cout << "\nArtificial Neural Network Approximation is not available in this release\n";
	cout << "\nFuzzy Approximation will be enabled instead\n";
	Options.approx_settings.enabled = 1;
	init_approximation();
	
	/*
	
	cout << "\nArtificial Neural Network Approximation Enabled\n";
	if (function_approx != NULL) free(function_approx);
	function_approx = new CAnnFunctionApproximation();
	function_approx->Init(Options.approx_settings.threshold,Options.approx_settings.min, Options.approx_settings.max,n_objectives());
    */
    } 

}
