/***************************************************************************
                          explorer.cpp  -  description
                             -------------------
    begin                : Sat Aug 24 2002
    copyright            : (C) 2002 by Davide Patti
    email                : davidepatti@email.it
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
#include "FuzzyApprox.h"

#include <moea/AlleleStr.h>

#include "common.h"

#include <ctime>
#include <values.h>

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
    Options.save_PD_STATS = false;
    Options.save_estimation = false;

    Options.save_objectives_details = false;
    fuzzy_enabled = false;
    force_simulation = false;

    current_space = "SPACE_NOT_SET";

    set_base_dir(ti->get_base_dir());
}

//********************************************************************

Explorer::~Explorer()
{
    cout << "\n Destroying explorer...";
}

void Explorer::set_fuzzy(bool v)
{
    fuzzy_enabled = v;
}


//********************************************************************
void Explorer::set_options(const struct User_Settings& user_settings)
{
    Options.hyperblock = user_settings.hyperblock;
    Options.objective_area = user_settings.objective_area;
    Options.objective_power = user_settings.objective_power;
    Options.objective_energy = user_settings.objective_energy;
    Options.objective_exec_time = user_settings.objective_exec_time;
    Options.save_spaces = user_settings.save_spaces;
    Options.save_PD_STATS = user_settings.save_PD_STATS;
    Options.save_estimation = user_settings.save_estimation;
    Options.benchmark = user_settings.benchmark;

    Options.auto_clock = user_settings.auto_clock;

    estimator.set_autoclock(user_settings.auto_clock);
    Options.hyperblock = user_settings.hyperblock;

    // Number of objectives 
    n_obj = 0;

    if (Options.objective_power) n_obj++;
    if (Options.objective_exec_time) n_obj++;
    if (Options.objective_area) n_obj++;
    if (Options.objective_energy) n_obj++;

}


//********************************************************************
// Exploration Algorithms
//********************************************************************

//********************************************************************
// N Random  explorations of configuration space
//********************************************************************
void Explorer::start_RAND(int n)
{

    current_algo = "RAND";
    vector<Configuration> random_space;

    int valid = 0;
    reset_sim_counter();

    cout << "\n Building random space for " << n << " simulations...";

    for(int i=0;i<n;i++)
    {
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

	Configuration temp_conf = create_configuration(processor,mem_hierarchy);

	if (mem_hierarchy.test_valid_config())
	{
	    valid++;
	    random_space.push_back(temp_conf);
	}
    }

    cout << "\n Ok, created random space " ;
    cout << "\n\n Valid configurations :" << valid << " of " << n << " requested";


    vector<Simulation> rand_sims = simulate_space(random_space);
    vector<Simulation> pareto_set = get_pareto(rand_sims);

    save_simulations(rand_sims,Options.benchmark+"_RAND_"+current_space+".exp",SHOW_ALL);
    save_simulations(pareto_set,Options.benchmark+"_RAND_"+current_space+".pareto.exp",SHOW_ALL);
    
}
//********************************************************************

void Explorer::start_DEP2()
{
    current_algo = "DEP2";
    reset_sim_counter();
    Exploration_stats stats;

    //stats.feasible_size = get_feasible_size();
    stats.space_size = get_space_size();

    stats.start_time = time(NULL);

    char mess[55];
    int t;

    vector<Simulation> pareto_set;

    string filename = Options.benchmark+"_DEP2_"+current_space;

    write_log("  -> Starting DEP2 simulation ");

    sprintf(mess,"Space size : %d  , feasible %d ",(int)stats.space_size,(int)stats.feasible_size);
    write_log(mess);

    write_log("  -> Starting c5 exploration ");

    vector<bool> boolean_mask(18,false);
    boolean_mask[13] = true; // GPR
    Space_mask c5_mask = create_space_mask(boolean_mask);

    vector<Configuration> c5_space = build_space(c5_mask);

    vector<Simulation> c5_sims = simulate_space(c5_space);
    vector<Simulation> c5_sims_pareto = get_pareto(c5_sims);

    vector<Configuration> c5_space_pareto = extract_space(c5_sims_pareto);

    Space_mask c1_mask = get_space_mask(SET_L1D);

    vector<Configuration> c1_space = build_space(c1_mask);


    vector<Configuration> c5p_c1_space = build_space_cross_merge(c5_space_pareto,
	                                                        c1_space,
								c5_mask,
								c1_mask);

    vector<Simulation> c5p_c1_sims = simulate_space(c5p_c1_space);

    vector<Simulation> c5p_c1_sims_pareto = get_pareto(c5p_c1_sims);
    vector<Configuration> c1_space_pareto = extract_space(c5p_c1_sims_pareto);

    Space_mask c2_mask = get_space_mask(SET_L1I);
    vector<Configuration> c2_space = build_space(c2_mask);
    vector<Simulation> c2_sims = simulate_space(c2_space);
    vector<Simulation> c2_sims_pareto = get_pareto(c2_sims);

    vector<Configuration> c2_space_pareto = extract_space(c2_sims_pareto);


    vector<Configuration> c1p_c2p_space = build_space_cross_merge(c1_space_pareto,
	                                                        c2_space_pareto,
								c1_mask,
								c2_mask);
    /////////////////////////////////////////////////////////////////
    write_log("Starting c3 exploration ");

    Space_mask c3_mask = get_space_mask(SET_L2U);
    Space_mask c1_c2_mask = mask_union(c1_mask,c2_mask);

    vector<Configuration> original_c3_space = build_space(c3_mask);

    vector<Configuration> c3_space = build_space_cross_merge(c1p_c2p_space,
	                                                     original_c3_space,
							     c1_c2_mask,
							     c3_mask);

    vector<Simulation> c3_sims = simulate_space(c3_space);

    vector<Simulation> c3_sims_pareto = get_pareto(c3_sims);

    save_simulations(c3_sims,filename+"_c3.exp",SHOW_ALL);
    save_simulations(c3_sims_pareto,filename+"_c3.pareto.exp",SHOW_ALL);

    vector<Configuration> c3_space_pareto = extract_space(c3_sims_pareto);



    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged c1 and c2 clusters... ");

    vector<Configuration> c1to2_space = build_space_cross_merge(c1_space_pareto,
	                                                        c2_space_pareto,
							        c1_mask,
							        c2_mask);
    vector<Simulation> c1to2_sims =  simulate_space(c1to2_space);
    append_simulations(pareto_set,c1to2_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to2_space_pareto = extract_space(pareto_set);

    save_simulations(c1to2_sims,filename+"_c1to2.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to2.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged c1to2 and c3 clusters... ");

    vector<Configuration> c1to3_space = build_space_cross_merge(c1to2_space_pareto,
	                                                        c3_space_pareto,
							        c1_c2_mask,
							        c3_mask);
    vector<Simulation> c1to3_sims =  simulate_space(c1to3_space);
    append_simulations(pareto_set,c1to3_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to3_space_pareto = extract_space(pareto_set);

    save_simulations(c1to3_sims,filename+"_c1to3.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to3.pareto.exp",SHOW_ALL);

    Space_mask c1to3_mask = mask_union(c1_c2_mask,c3_mask);

    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged c1to3 and c5 clusters... ");

    vector<Configuration> c1to5_space = build_space_cross_merge(c1to3_space_pareto,
	                                                        c5_space_pareto,
							        c1to3_mask,
							        c5_mask);
    vector<Simulation> c1to5_sims =  simulate_space(c1to5_space);
    append_simulations(pareto_set,c1to5_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to5_space_pareto = extract_space(pareto_set);

    save_simulations(c1to5_sims,filename+"_c1to5.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to5.pareto.exp",SHOW_ALL);

    save_stats(stats,filename+".stat");
    save_simulations(pareto_set,filename+".pareto.exp",SHOW_ALL);

}

//////////////////////////////////////////////////////////////////////
//  Dependency Algorithm - DEP
/// Implementation of system level exploration using clusters .
//
// List of clusters :
//
//  c1 : L1 data ( size, block_size, associativity)
//  c2 : L1 instruction ( size, block_size, associativity)
//  c3 : L2 unified cache ( size, block_size, associativity)
//  c4 : Functional units ( IU,FPU,BU,MU )
//  c5 : GPR , c6 : FPR , c7 : PR , c8 : CR , c9 : BTR

//********************************************************************
void Explorer::start_DEP()
{
    current_algo = "DEP";
    reset_sim_counter();
    Exploration_stats stats;

    stats.feasible_size = get_feasible_size();
    stats.space_size = get_space_size();

    stats.start_time = time(NULL);

    char mess[55];
    int t;

    string filename = Options.benchmark+"_DEP_"+current_space;

    
    // explore c1 and c2 , separately .
    // all other parameters are set to arbitrary values because c1 and
    // c2 pareto optimal values are assumed indipendent from them
    //
    //////////////////////////////////////////////////////////////////////


    write_log("  -> Starting DEP simulation ");

    sprintf(mess,"Space size : %d  , feasible %d ",(int)stats.space_size,(int)stats.feasible_size);
    write_log(mess);

    write_log("  -> Starting c1 exploration ");

    Space_mask c1_mask = get_space_mask(SET_L1D);

    vector<Configuration> c1_space = build_space(c1_mask);
    
    t = (c1_space.size() * average_exec_time)/60;
    sprintf(mess," c1 : %d explorations, estimated time %d minutes ",c1_space.size(),t);
    write_log(mess);

    vector<Simulation> c1_sims = simulate_space(c1_space);
    vector<Simulation> c1_sims_pareto = get_pareto(c1_sims);

    vector<Configuration> c1_space_pareto = extract_space(c1_sims_pareto);
    int n_ott_c1 = c1_space_pareto.size();

    save_simulations(c1_sims,filename+"_c1.exp",SHOW_ALL);
    save_simulations(c1_sims_pareto,filename+"_c1.pareto.exp",SHOW_ALL);

    //////////////////////////////////////////////////////////////////////
    write_log("Starting c2 exploration ");

    Space_mask c2_mask = get_space_mask(SET_L1I);

    vector<Configuration> c2_space = build_space(c2_mask);

    t = (c2_space.size() * average_exec_time)/60;
    sprintf(mess," (%d explorations, estimated time %d minutes ",c2_space.size(),t);
    write_log(mess);

    vector<Simulation> c2_sims = simulate_space(c2_space);
    vector<Simulation> c2_sims_pareto = get_pareto(c2_sims);

    vector<Configuration> c2_space_pareto = extract_space(c2_sims_pareto);
    int n_ott_c2 = c2_space_pareto.size();

    save_simulations(c2_sims,filename+"_c2.exp",SHOW_ALL);
    save_simulations(c2_sims_pareto,filename+"_c2.pareto.exp",SHOW_ALL);

    ////////////////////////////////////////////////////
    // a first estimation of exploration time can be done here
    // note we are assuming a arbitrary val for n pareto c4
    //
    // Error is proportional to error in n_ott_c4_assumed

    int est_n_ott_c4 = (n_ott_c1 + n_ott_c2);
    int min_estimated_time = DEP_phase1_time(n_ott_c1,n_ott_c2,2);
    int max_estimated_time = DEP_phase1_time(n_ott_c1,n_ott_c2,est_n_ott_c4);

    sprintf(mess," Approx exploration time : (%d - %d) minutes ",min_estimated_time,max_estimated_time);

    write_log(mess);

    // now c3 and c4 must be explored , for each combination of c1 and
    // c2 pareto-optimal configs. All other parameters are set to
    // arbitrary values.
    // Order is not important , but c4 exploration is performed for
    // first in order to obtain n_ott_c4 to estimate phase 1
    // algorithm time as soon as possible
    //////////////////////////////////////////////////////////////////

    write_log("Starting c4 exploration ");

    // first of all , we cross merge c1 and c2

    vector<Configuration> c1p_c2p_space = build_space_cross_merge(c1_space_pareto,
	                                                        c2_space_pareto,
								c1_mask,
								c2_mask);
    Space_mask c1_c2_mask = mask_union(c1_mask,c2_mask);

    Space_mask c4_mask = get_space_mask(SET_PROCESSOR_UNITS);

    vector<Configuration> original_c4_space = build_space(c4_mask);

    vector<Configuration> c4_space = build_space_cross_merge(c1p_c2p_space,
	                                                     original_c4_space,
							     c1_c2_mask,
							     c4_mask);

    t = (c4_space.size() * (average_exec_time+average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c4_space.size(),t);
    write_log(mess);

    vector<Simulation> c4_sims = simulate_space(c4_space);
    vector<Simulation> c4_sims_pareto = get_pareto(c4_sims);

    save_simulations(c4_sims,filename+"_c4.exp",SHOW_ALL);
    save_simulations(c4_sims_pareto,filename+"_c4.pareto.exp",SHOW_ALL);

    vector<Configuration> c4_space_pareto = extract_space(c4_sims_pareto);
    int n_ott_c4 = c4_space_pareto.size();

    /////////////////////////////////////////////////////////////////
    // Now it's possible to computer the total time necessary to
    // perform the first phase of algorithm , because we have n_ott_c1
    // , n_ott_c2 and n_ott_c4

    int estimated_time = DEP_phase1_time(n_ott_c1,n_ott_c2,n_ott_c4);
    sprintf(mess," Estimated Phase 1 time : %d minutes ...",estimated_time);

    write_log(mess);
    /////////////////////////////////////////////////////////////////
    write_log("Starting c3 exploration ");

    Space_mask c3_mask = get_space_mask(SET_L2U);

    vector<Configuration> original_c3_space = build_space(c3_mask);

    vector<Configuration> c3_space = build_space_cross_merge(c1p_c2p_space,
	                                                     original_c3_space,
							     c1_c2_mask,
							     c3_mask);

    t = (c3_space.size() * average_exec_time)/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c3_space.size(),t);
    write_log(mess);

    vector<Simulation> c3_sims = simulate_space(c3_space);

    vector<Simulation> c3_sims_pareto = get_pareto(c3_sims);

    save_simulations(c3_sims,filename+"_c3.exp",SHOW_ALL);
    save_simulations(c3_sims_pareto,filename+"_c3.pareto.exp",SHOW_ALL);

    vector<Configuration> c3_space_pareto = extract_space(c3_sims_pareto);

    ///////////////////////////////////////////////////////////////////////
    // now we must perform an exhaustive exploration of c5..c9
    // clusters for each of the pareto config of c4
    //
    // clusters from c5 to c9 depend on c4 . So we must explore each
    // of them combining their original exhaustive space with every
    // pareto val of c4 exploration
    //
    // First of all, we create these cross merged spaces for each of
    // c5..c9 parameters

    Space_mask c124_mask = mask_union(c1_c2_mask,c4_mask);
    Space_mask c5_mask = get_space_mask(UNSET_ALL);
    c5_mask.gpr_static_size = true;

    vector <Configuration> original_c5_space = build_space(c5_mask);

    vector <Configuration> c5_space = build_space_cross_merge(original_c5_space,
	                                                      c4_space_pareto,
							      c5_mask,
							      c124_mask);

    ///////////////////////////////////////////////////////////////////////

    Space_mask c6_mask = get_space_mask(UNSET_ALL);
    c6_mask.fpr_static_size = true;
	                                                      
    vector <Configuration> original_c6_space = build_space(c6_mask);

    vector <Configuration> c6_space = build_space_cross_merge(original_c6_space,
	                                                      c4_space_pareto,
							      c6_mask,
							      c124_mask);
    ///////////////////////////////////////////////////////////////////////

    Space_mask c7_mask = get_space_mask(UNSET_ALL);
    c7_mask.pr_static_size = true;
	                                                      
    vector <Configuration> original_c7_space = build_space(c7_mask);

    vector <Configuration> c7_space = build_space_cross_merge(original_c7_space,
	                                                      c4_space_pareto,
							      c7_mask,
							      c124_mask);
    ///////////////////////////////////////////////////////////////////////

    Space_mask c8_mask = get_space_mask(UNSET_ALL);
    c8_mask.cr_static_size = true;
	                                                      
    vector <Configuration> original_c8_space = build_space(c8_mask);

    vector <Configuration> c8_space = build_space_cross_merge(original_c8_space,
	                                                      c4_space_pareto,
							      c8_mask,
							      c124_mask);
    ///////////////////////////////////////////////////////////////////////

    Space_mask c9_mask = get_space_mask(UNSET_ALL);
    c9_mask.btr_static_size = true;
	                                                      
    vector <Configuration> original_c9_space = build_space(c9_mask);

    vector <Configuration> c9_space = build_space_cross_merge(original_c9_space,
	                                                      c4_space_pareto,
							      c9_mask,
							      c124_mask);
    ///////////////////////////////////////////////////////////////////////
    // All c(i)_space are are ready to be explored 
    //////////////////////////////////////////////////////////////////////
    write_log("  -> Started c5 exploration ");

    t = (c5_space.size() * average_exec_time + average_compilation_time)/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c5_space.size(),t);
    write_log(mess);

    vector<Simulation> c5_sims = simulate_space(c5_space);
    vector<Simulation> c5_sims_pareto = get_pareto(c5_sims);
    save_simulations(c5_sims,filename+"_c5.exp",SHOW_ALL);
    save_simulations(c5_sims_pareto,filename+"_c5.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c6 exploration ");

    t = (c6_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c6_space.size(),t);
    write_log(mess);

    vector<Simulation> c6_sims = simulate_space(c6_space);
    vector<Simulation> c6_sims_pareto = get_pareto(c6_sims);
    save_simulations(c6_sims,filename+"_c6.exp",SHOW_ALL);
    save_simulations(c6_sims_pareto,filename+"_c6.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c7 exploration ");

    t = (c7_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c7_space.size(),t);
    write_log(mess);

    vector<Simulation> c7_sims = simulate_space(c7_space);
    vector<Simulation> c7_sims_pareto = get_pareto(c7_sims);
    save_simulations(c7_sims,filename+"_c7.exp",SHOW_ALL);
    save_simulations(c7_sims_pareto,filename+"_c7.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c8 exploration ");

    t = (c8_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c8_space.size(),t);
    write_log(mess);

    vector<Simulation> c8_sims = simulate_space(c8_space);
    vector<Simulation> c8_sims_pareto = get_pareto(c8_sims);
    save_simulations(c8_sims,filename+"_c8.exp",SHOW_ALL);
    save_simulations(c8_sims_pareto,filename+"_c8.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c9 exploration ");

    t = (c9_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c9_space.size(),t);
    write_log(mess);

    vector<Simulation> c9_sims = simulate_space(c9_space);
    vector<Simulation> c9_sims_pareto = get_pareto(c9_sims);
    save_simulations(c9_sims,filename+"_c9.exp",SHOW_ALL);
    save_simulations(c9_sims_pareto,filename+"_c9.pareto.exp",SHOW_ALL);

    // Phase 1 of alg is finished , we have the pareto optimal
    // configuration of all clusters

    // Once we have explored c5..c9 considering each pareto of c4, we
    // can compute pareto of the union collected simulations
    vector<Simulation> all_sims;
    append_simulations(all_sims,c1_sims_pareto);
    append_simulations(all_sims,c2_sims_pareto);
    append_simulations(all_sims,c3_sims_pareto);
    append_simulations(all_sims,c4_sims_pareto);
    append_simulations(all_sims,c5_sims_pareto);
    append_simulations(all_sims,c6_sims_pareto);
    append_simulations(all_sims,c7_sims_pareto);
    append_simulations(all_sims,c8_sims_pareto);
    append_simulations(all_sims,c9_sims_pareto);

    remove_dominated_simulations(all_sims);

    save_simulations(all_sims,filename+"_phase1.pareto.exp",SHOW_ALL);

    write_log("DEP simulation Phase1 ended. ");

    /////////////////////////////////////////////////////////////////////////////
    /// PHASE 2 - merging all clusters , couple by couple . 
    //
    // c1toN_space :
    // resulting space from cross merging c1toN-1_space and cN_space

    write_log("Starting Phase 2 . ");

    vector<Configuration> c5_space_pareto = extract_space(c5_sims_pareto);
    vector<Configuration> c6_space_pareto = extract_space(c6_sims_pareto);
    vector<Configuration> c7_space_pareto = extract_space(c7_sims_pareto);
    vector<Configuration> c8_space_pareto = extract_space(c8_sims_pareto);
    vector<Configuration> c9_space_pareto = extract_space(c9_sims_pareto);

    // this is the vector where pareto simulations will be stored
    vector<Simulation> pareto_set;

    append_simulations(pareto_set,c1_sims_pareto);

    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged c1 and c2 clusters... ");

    vector<Configuration> c1to2_space = build_space_cross_merge(c1_space_pareto,
	                                                        c2_space_pareto,
							        c1_mask,
							        c2_mask);
    vector<Simulation> c1to2_sims =  simulate_space(c1to2_space);
    append_simulations(pareto_set,c1to2_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to2_space_pareto = extract_space(pareto_set);

    save_simulations(c1to2_sims,filename+"_c1to2.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to2.pareto.exp",SHOW_ALL);
    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to2) and c3 clusters... ");

    vector<Configuration> c1to3_space = build_space_cross_merge(c1to2_space_pareto,
	                                                       c3_space_pareto,
							       c1_c2_mask,
							       c3_mask);
    vector<Simulation> c1to3_sims =  simulate_space(c1to3_space);
    append_simulations(pareto_set,c1to3_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to3_space_pareto = extract_space(pareto_set);

    save_simulations(c1to3_sims,filename+"_c1to3.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to3.pareto.exp",SHOW_ALL);
    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to3) and c4 clusters... ");

    Space_mask c1to3_mask = mask_union(c1_c2_mask,c3_mask);

    vector<Configuration> c1to4_space = build_space_cross_merge(c1to3_space_pareto,
	                                                       c4_space_pareto,
							       c1to3_mask,
							       c4_mask);
    vector<Simulation> c1to4_sims =  simulate_space(c1to4_space);
    append_simulations(pareto_set,c1to4_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to4_space_pareto = extract_space(pareto_set);

    save_simulations(c1to4_sims,filename+"_c1to4.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to4.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////

    write_log("Simulating merged (c1to4) and c5 clusters... ");

    Space_mask c1to4_mask = mask_union(c1to3_mask,c4_mask);

    vector<Configuration> c1to5_space = build_space_cross_merge(c1to4_space_pareto,
	                                                       c5_space_pareto,
							       c1to4_mask,
							       c5_mask);
    vector<Simulation> c1to5_sims =  simulate_space(c1to5_space);
    append_simulations(pareto_set,c1to5_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to5_space_pareto = extract_space(pareto_set);

    save_simulations(c1to5_sims,filename+"_c1to5.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to5.pareto.exp",SHOW_ALL);
    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to5) and c6 clusters... ");

    Space_mask c1to5_mask = mask_union(c1to4_mask,c5_mask);

    vector<Configuration> c1to6_space = build_space_cross_merge(c1to5_space_pareto,
	                                                       c6_space_pareto,
							       c1to5_mask,
							       c6_mask);
    vector<Simulation> c1to6_sims =  simulate_space(c1to6_space);
    append_simulations(pareto_set,c1to6_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to6_space_pareto = extract_space(pareto_set);

    save_simulations(c1to6_sims,filename+"_c1to6.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to6.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to6) and c7 clusters... ");

    Space_mask c1to6_mask = mask_union(c1to5_mask,c6_mask);

    vector<Configuration> c1to7_space = build_space_cross_merge(c1to6_space_pareto,
	                                                       c7_space_pareto,
							       c1to6_mask,
							       c7_mask);
    vector<Simulation> c1to7_sims =  simulate_space(c1to7_space);
    append_simulations(pareto_set,c1to7_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to7_space_pareto = extract_space(pareto_set);

    save_simulations(c1to7_sims,filename+"_c1to7.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to7.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to7) and c8 clusters... ");

    Space_mask c1to7_mask = mask_union(c1to6_mask,c7_mask);

    vector<Configuration> c1to8_space = build_space_cross_merge(c1to7_space_pareto,
	                                                       c8_space_pareto,
							       c1to7_mask,
							       c8_mask);
    vector<Simulation> c1to8_sims =  simulate_space(c1to8_space);
    append_simulations(pareto_set,c1to8_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to8_space_pareto = extract_space(pareto_set);

    save_simulations(c1to8_sims,filename+"_c1to8.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to8.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to8) and c9 clusters... ");

    Space_mask c1to8_mask = mask_union(c1to7_mask,c8_mask);

    vector<Configuration> c1to9_space = build_space_cross_merge(c1to8_space_pareto,
	                                                       c9_space_pareto,
							       c1to8_mask,
							       c9_mask);
    vector<Simulation> c1to9_sims =  simulate_space(c1to9_space);
    append_simulations(pareto_set,c1to9_sims);
    remove_dominated_simulations(pareto_set);

    save_simulations(c1to9_sims,filename+"_c1to9.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_pareto.exp",SHOW_ALL);

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,filename+".stat");

    write_log("End of Phase 2 ");
    write_log("End of DEP simulation ");
}


// ********************************************************************
// simplified DEP : do not consider monodirectional dependencies
// ********************************************************************
void Explorer::start_DEPMOD()
{
    current_algo = "DEPMOD";
    reset_sim_counter();
    Exploration_stats stats;

 
    stats.start_time = time(NULL);

    char mess[55];
    int t;

    string filename = Options.benchmark+"_DEPMOD_"+current_space;

    
    // explore c1 and c2 , separately .
    // all other parameters are set to arbitrary values because c1 and
    // c2 pareto optimal values are assumed indipendent from them
    //
    //////////////////////////////////////////////////////////////////////
    write_log("  -> Starting c1 exploration ");

    Space_mask c1_mask = get_space_mask(SET_L1D);

    vector<Configuration> c1_space = build_space(c1_mask);
    
    t = (c1_space.size() * average_exec_time)/60;
    sprintf(mess," c1 : %d explorations, estimated time %d minutes ",c1_space.size(),t);
    write_log(mess);

    vector<Simulation> c1_sims = simulate_space(c1_space);
    vector<Simulation> c1_sims_pareto = get_pareto(c1_sims);

    vector<Configuration> c1_space_pareto = extract_space(c1_sims_pareto);
    int n_ott_c1 = c1_space_pareto.size();

    save_simulations(c1_sims,filename+"_c1.exp",SHOW_ALL);
    save_simulations(c1_sims_pareto,filename+"_c1.pareto.exp",SHOW_ALL);

    //////////////////////////////////////////////////////////////////////
    write_log("Starting c2 exploration ");

    Space_mask c2_mask = get_space_mask(SET_L1I);

    vector<Configuration> c2_space = build_space(c2_mask);

    t = (c2_space.size() * average_exec_time)/60;
    sprintf(mess," (%d explorations, estimated time %d minutes ",c2_space.size(),t);
    write_log(mess);

    vector<Simulation> c2_sims = simulate_space(c2_space);
    vector<Simulation> c2_sims_pareto = get_pareto(c2_sims);

    vector<Configuration> c2_space_pareto = extract_space(c2_sims_pareto);
    int n_ott_c2 = c2_space_pareto.size();

    save_simulations(c2_sims,filename+"_c2.exp",SHOW_ALL);
    save_simulations(c2_sims_pareto,filename+"_c2.pareto.exp",SHOW_ALL);

    write_log("Starting c4 exploration ");

    // first of all , we cross merge c1 and c2

    Space_mask c4_mask = get_space_mask(SET_PROCESSOR_UNITS);

    vector<Configuration> c4_space = build_space(c4_mask);

    t = (c4_space.size() * (average_exec_time+average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c4_space.size(),t);
    write_log(mess);

    vector<Simulation> c4_sims = simulate_space(c4_space);
    vector<Simulation> c4_sims_pareto = get_pareto(c4_sims);

    save_simulations(c4_sims,filename+"_c4.exp",SHOW_ALL);
    save_simulations(c4_sims_pareto,filename+"_c4.pareto.exp",SHOW_ALL);

    vector<Configuration> c4_space_pareto = extract_space(c4_sims_pareto);
    int n_ott_c4 = c4_space_pareto.size();

    /////////////////////////////////////////////////////////////////
    write_log("Starting c3 exploration ");

    Space_mask c3_mask = get_space_mask(SET_L2U);

    vector<Configuration> c3_space = build_space(c3_mask);

    t = (c3_space.size() * average_exec_time)/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c3_space.size(),t);
    write_log(mess);

    vector<Simulation> c3_sims = simulate_space(c3_space);

    vector<Simulation> c3_sims_pareto = get_pareto(c3_sims);

    save_simulations(c3_sims,filename+"_c3.exp",SHOW_ALL);
    save_simulations(c3_sims_pareto,filename+"_c3.pareto.exp",SHOW_ALL);

    vector<Configuration> c3_space_pareto = extract_space(c3_sims_pareto);

    ///////////////////////////////////////////////////////////////////////
    // now we must perform an exhaustive exploration of c5..c9

    Space_mask c5_mask = get_space_mask(UNSET_ALL);
    c5_mask.gpr_static_size = true;
    vector <Configuration> c5_space = build_space(c5_mask);

    ///////////////////////////////////////////////////////////////////////

    Space_mask c6_mask = get_space_mask(UNSET_ALL);
    c6_mask.fpr_static_size = true;
	                                                      
    vector <Configuration> c6_space = build_space(c6_mask);

    ///////////////////////////////////////////////////////////////////////

    Space_mask c7_mask = get_space_mask(UNSET_ALL);
    c7_mask.pr_static_size = true;
	                                                      
    vector <Configuration> c7_space = build_space(c7_mask);

    ///////////////////////////////////////////////////////////////////////

    Space_mask c8_mask = get_space_mask(UNSET_ALL);
    c8_mask.cr_static_size = true;
	                                                      
    vector <Configuration> c8_space = build_space(c8_mask);

    ///////////////////////////////////////////////////////////////////////

    Space_mask c9_mask = get_space_mask(UNSET_ALL);
    c9_mask.btr_static_size = true;
	                                                      
    vector <Configuration> c9_space = build_space(c9_mask);

    ///////////////////////////////////////////////////////////////////////
    // All c(i)_space are are ready to be explored 
    //////////////////////////////////////////////////////////////////////
    write_log("  -> Started c5 exploration ");

    t = (c5_space.size() * average_exec_time + average_compilation_time)/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c5_space.size(),t);
    write_log(mess);

    vector<Simulation> c5_sims = simulate_space(c5_space);
    vector<Simulation> c5_sims_pareto = get_pareto(c5_sims);
    save_simulations(c5_sims,filename+"_c5.exp",SHOW_ALL);
    save_simulations(c5_sims_pareto,filename+"_c5.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c6 exploration ");

    t = (c6_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c6_space.size(),t);
    write_log(mess);

    vector<Simulation> c6_sims = simulate_space(c6_space);
    vector<Simulation> c6_sims_pareto = get_pareto(c6_sims);
    save_simulations(c6_sims,filename+"_c6.exp",SHOW_ALL);
    save_simulations(c6_sims_pareto,filename+"_c6.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c7 exploration ");

    t = (c7_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c7_space.size(),t);
    write_log(mess);

    vector<Simulation> c7_sims = simulate_space(c7_space);
    vector<Simulation> c7_sims_pareto = get_pareto(c7_sims);
    save_simulations(c7_sims,filename+"_c7.exp",SHOW_ALL);
    save_simulations(c7_sims_pareto,filename+"_c7.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c8 exploration ");

    t = (c8_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c8_space.size(),t);
    write_log(mess);

    vector<Simulation> c8_sims = simulate_space(c8_space);
    vector<Simulation> c8_sims_pareto = get_pareto(c8_sims);
    save_simulations(c8_sims,filename+"_c8.exp",SHOW_ALL);
    save_simulations(c8_sims_pareto,filename+"_c8.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c9 exploration ");

    t = (c9_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c9_space.size(),t);
    write_log(mess);

    vector<Simulation> c9_sims = simulate_space(c9_space);
    vector<Simulation> c9_sims_pareto = get_pareto(c9_sims);
    save_simulations(c9_sims,filename+"_c9.exp",SHOW_ALL);
    save_simulations(c9_sims_pareto,filename+"_c9.pareto.exp",SHOW_ALL);

    // Phase 1 of alg is finished , we have the pareto optimal
    // configuration of all clusters

    // Once we have explored c5..c9 considering each pareto of c4, we
    // can compute pareto of the union collected simulations
    vector<Simulation> all_sims;
    append_simulations(all_sims,c1_sims_pareto);
    append_simulations(all_sims,c2_sims_pareto);
    append_simulations(all_sims,c3_sims_pareto);
    append_simulations(all_sims,c4_sims_pareto);
    append_simulations(all_sims,c5_sims_pareto);
    append_simulations(all_sims,c6_sims_pareto);
    append_simulations(all_sims,c7_sims_pareto);
    append_simulations(all_sims,c8_sims_pareto);
    append_simulations(all_sims,c9_sims_pareto);

    remove_dominated_simulations(all_sims);

    save_simulations(all_sims,filename+"_GLOBAL_no_merge.pareto.exp",SHOW_ALL);

    write_log("DEPMOD simulation Phase1 ended. ");

    /////////////////////////////////////////////////////////////////////////////
    /// PHASE 2 - merging all clusters , couple by couple . 
    //
    // c1toN_space :
    // resulting space from cross merging c1toN-1_space and cN_space

    write_log("Starting Phase 2 . ");

    vector<Configuration> c5_space_pareto = extract_space(c5_sims_pareto);
    vector<Configuration> c6_space_pareto = extract_space(c6_sims_pareto);
    vector<Configuration> c7_space_pareto = extract_space(c7_sims_pareto);
    vector<Configuration> c8_space_pareto = extract_space(c8_sims_pareto);
    vector<Configuration> c9_space_pareto = extract_space(c9_sims_pareto);

    // this is the vector where pareto simulations will be stored
    vector<Simulation> pareto_set;

    append_simulations(pareto_set,c1_sims_pareto);

    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged c1 and c2 clusters... ");

    vector<Configuration> c1to2_space = build_space_cross_merge(c1_space_pareto,
	                                                        c2_space_pareto,
							        c1_mask,
							        c2_mask);
    vector<Simulation> c1to2_sims =  simulate_space(c1to2_space);
    append_simulations(pareto_set,c1to2_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to2_space_pareto = extract_space(pareto_set);

    save_simulations(c1to2_sims,filename+"_c1to2.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to2.pareto.exp",SHOW_ALL);
    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to2) and c3 clusters... ");

    Space_mask c1_c2_mask = mask_union(c1_mask,c2_mask);
    vector<Configuration> c1to3_space = build_space_cross_merge(c1to2_space_pareto,
	                                                       c3_space_pareto,
							       c1_c2_mask,
							       c3_mask);
    vector<Simulation> c1to3_sims =  simulate_space(c1to3_space);
    append_simulations(pareto_set,c1to3_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to3_space_pareto = extract_space(pareto_set);

    save_simulations(c1to3_sims,filename+"_c1to3.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to3.pareto.exp",SHOW_ALL);
    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to3) and c4 clusters... ");

    Space_mask c1to3_mask = mask_union(c1_c2_mask,c3_mask);

    vector<Configuration> c1to4_space = build_space_cross_merge(c1to3_space_pareto,
	                                                       c4_space_pareto,
							       c1to3_mask,
							       c4_mask);
    vector<Simulation> c1to4_sims =  simulate_space(c1to4_space);
    append_simulations(pareto_set,c1to4_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to4_space_pareto = extract_space(pareto_set);

    save_simulations(c1to4_sims,filename+"_c1to4.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to4.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////

    write_log("Simulating merged (c1to4) and c5 clusters... ");

    Space_mask c1to4_mask = mask_union(c1to3_mask,c4_mask);

    vector<Configuration> c1to5_space = build_space_cross_merge(c1to4_space_pareto,
	                                                       c5_space_pareto,
							       c1to4_mask,
							       c5_mask);
    vector<Simulation> c1to5_sims =  simulate_space(c1to5_space);
    append_simulations(pareto_set,c1to5_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to5_space_pareto = extract_space(pareto_set);

    save_simulations(c1to5_sims,filename+"_c1to5.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to5.pareto.exp",SHOW_ALL);
    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to5) and c6 clusters... ");

    Space_mask c1to5_mask = mask_union(c1to4_mask,c5_mask);

    vector<Configuration> c1to6_space = build_space_cross_merge(c1to5_space_pareto,
	                                                       c6_space_pareto,
							       c1to5_mask,
							       c6_mask);
    vector<Simulation> c1to6_sims =  simulate_space(c1to6_space);
    append_simulations(pareto_set,c1to6_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to6_space_pareto = extract_space(pareto_set);

    save_simulations(c1to6_sims,filename+"_c1to6.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to6.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to6) and c7 clusters... ");

    Space_mask c1to6_mask = mask_union(c1to5_mask,c6_mask);

    vector<Configuration> c1to7_space = build_space_cross_merge(c1to6_space_pareto,
	                                                       c7_space_pareto,
							       c1to6_mask,
							       c7_mask);
    vector<Simulation> c1to7_sims =  simulate_space(c1to7_space);
    append_simulations(pareto_set,c1to7_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to7_space_pareto = extract_space(pareto_set);

    save_simulations(c1to7_sims,filename+"_c1to7.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to7.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to7) and c8 clusters... ");

    Space_mask c1to7_mask = mask_union(c1to6_mask,c7_mask);

    vector<Configuration> c1to8_space = build_space_cross_merge(c1to7_space_pareto,
	                                                       c8_space_pareto,
							       c1to7_mask,
							       c8_mask);
    vector<Simulation> c1to8_sims =  simulate_space(c1to8_space);
    append_simulations(pareto_set,c1to8_sims);
    remove_dominated_simulations(pareto_set);
    vector<Configuration> c1to8_space_pareto = extract_space(pareto_set);

    save_simulations(c1to8_sims,filename+"_c1to8.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_c1to8.pareto.exp",SHOW_ALL);

    ///////////////////////////////////////////////////////////////
    write_log("Simulating merged (c1to8) and c9 clusters... ");

    Space_mask c1to8_mask = mask_union(c1to7_mask,c8_mask);

    vector<Configuration> c1to9_space = build_space_cross_merge(c1to8_space_pareto,
	                                                       c9_space_pareto,
							       c1to8_mask,
							       c9_mask);
    vector<Simulation> c1to9_sims =  simulate_space(c1to9_space);
    append_simulations(pareto_set,c1to9_sims);
    remove_dominated_simulations(pareto_set);

    save_simulations(c1to9_sims,filename+"_c1to9.exp",SHOW_ALL);
    save_simulations(pareto_set,filename+"_pareto.exp",SHOW_ALL);

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,filename+".stat");

    write_log("End of Phase 2 ");
    write_log("End of DEPMOD simulation ");
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

    int n_par = 18;

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

	// note NO_L2_SIZE_CHECK option, cache size parameters should
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
	cout << "\n sens " << p << ": " << sens[p];
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

	// we must combine two configuration spaces :
	
	// a space where only one parameter varies on its range
	vector<Configuration> param_space = build_space(parameter_masks[index],NO_L2_SIZE_CHECK);

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
	save_simulations(pareto_set,file_name+".exp",SHOW_ALL);
    }

    string file_name = Options.benchmark+"_PBSA_"+current_space;
    save_simulations(pareto_set,file_name+".pareto.exp",SHOW_ALL);

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,file_name+".stat");
}

//********************************************************************
bool Explorer::same_intorno(const Simulation& sim1,const Simulation& sim2,double r)
{
    return (distance(sim1,sim2)<=r);
}
/*
// **************************************************************************
// computes how many points of 'pareto_set'  may be considered 
// approximated by 'sim' points, given a certain tolerance error
// 'err'.  'err' is expressed as % error on variation range of pareto_set.
// **************************************************************************
double Explorer::get_pareto_distance(vector<Simulation>& pareto_set,vector<Simulation>& sim,double err)
{
    // normalize the two sets in the same range .
    // pareto_set is assumed to contain pareto_set simulations so it
    // should have the greates variation in energy and cyles values.
    double max_energy,min_energy;
    double max_exec_time,min_exec_time;

    // to be more general ,we make no assumption on how pareto_set is ordered,
    // althought get_pareto() or remove dominated() always order
    // from greatest energy to smallest energy.

    vector<Simulation> e_sorted = sort_by_energy(pareto_set);

    max_energy = e_sorted[e_sorted.size()-1].energy;
    min_energy = e_sorted[0].energy;
    // to smallest energy corresponds greated exec_time
    max_exec_time = e_sorted[0].exec_time;
    min_exec_time = e_sorted[e_sorted.size()-1].exec_time;

    cout << "\n e : " << min_energy << "   " << max_energy;
    cout << "\n c : " << min_exec_time << "   " << max_exec_time;

    for (unsigned int i=0;i<pareto_set.size();i++)
    {
	pareto_set[i].energy = (pareto_set[i].energy-min_energy)/(max_energy-min_energy);
	pareto_set[i].exec_time = (pareto_set[i].exec_time-min_exec_time)/(max_exec_time-min_exec_time);
    }

    for (unsigned int i=0;i<sim.size();i++)
    {
	sim[i].energy = (sim[i].energy-min_energy)/(max_energy-min_energy);
	sim[i].exec_time = (sim[i].exec_time-min_exec_time)/(max_exec_time-min_exec_time);
    }

    string path = get_base_dir();
    save_simulations(pareto_set,path+"/PROVA.pareto.exp_norm",MATLAB);
    save_simulations(sim,path+"/PROVA_sim_norm",MATLAB);

    vector<bool> marked(pareto_set.size(),false);

    double n_approx = 0;

    // for each point of pareto_set , check if the at least on point
    // from the other set that approximates it with a certain error %
    // on total variation range of pareto.

    for (unsigned int i1 = 0;i1<pareto_set.size();i1++)
    {
	for (unsigned int i2=0;i2<sim.size();i2++)
	{
	    double d = distance(pareto_set[i1],sim[i2]);
	    if (d<err)
	    {
		cout << "\n ho trovato due vicini : "
		cout << "\n pareto("<< pareto_set[i1].energy << "," << pareto_set[i1].exec_time << ")";
		cout << "\n    sim("<< sim[i2].energy << "," << sim[i2].exec_time << ")";

		if (!marked[i1]) 
		{
		    n_approx++;
		    cout << "\n ed era non ancora approssimato ";
		    marked[i1] = true;
		}
		else
		{
		    cout << "\n  ma e' gia' stato approssimato ! ";
		}
	    }
	}
    }

    cout << "\n Total point of pareto_set approximated by sim " << n_approx;

    return n_approx;

}
    */

//********************************************************************
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
// ********************************************************************
//  Euclidean distance between objective function values
// ********************************************************************
inline double Explorer::distance(const Simulation& s1, const Simulation& s2)
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

	// note NO_L2_SIZE_CHECK option, cache size parameters should
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

	// this time there isn't the NO_L2_SIZE_CHECK option, optimal
	// parameters values must be searched only in the feasible
	// configurations
	vector<Configuration> space = build_space(parameter_masks[index],base_conf);

	vector<Simulation> sims = simulate_space(space);
	vector<Simulation> ordered_sims = sort_by_energydelay_product(sims);

	// base_conf must be updated , considering the optimal value
	// of the the last parameter explored 
	// base_conf should already have i-1 parameters set to their
	// optimal val, so the following line should change  the
	// last explored parameter

	base_conf = ordered_sims[0].config;

	char temp[10];
	sprintf(temp,"%d_",i);        

	save_simulations(ordered_sims,file_name+string(temp)+".exp",SHOW_ALL);
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

    int n_par = 18;

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
	save_simulations(ordered_sims,path+file,SHOW_ALL);

	// change base_conf assigning most sensitive parameter to its
	// optimal value , the one that minimizes energy*delay
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

//*********************************************************************
//    Ehaustive exploration of all parameters 
// ********************************************************************
void Explorer::start_EXHA()
{
    current_algo = "EXHA";
    Exploration_stats stats;
    reset_sim_counter();
    stats.space_size = get_space_size();
    stats.feasible_size = get_feasible_size();
    stats.start_time=time(NULL);

    Space_mask mask = get_space_mask(SET_ALL);

    vector<Configuration> space = build_space(mask);

    // all parameters are explored in all their values 
    vector<Simulation> simulations = simulate_space(space);
    vector<Simulation> pareto_set = get_pareto(simulations);

    string filename = Options.benchmark+"_EXHA_"+current_space;
    save_simulations(simulations,filename+".exp",SHOW_ALL);
    save_simulations(pareto_set,filename+".pareto.exp",SHOW_ALL);

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,filename+".stat");
}

//**************************************************************
double Explorer::get_sensivity_energydelay(const vector<Simulation>& sims)
{
    vector<Simulation> temp = sort_by_energydelay_product(sims);
   
    double min_product = (temp[0].energy)*(temp[0].exec_time);
    double max_product = (temp[temp.size()-1].energy)*(temp[temp.size()-1].exec_time);

    return max_product-min_product;
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
//   GA-based exploration
// ********************************************************************
// two different strategies:
//  - classic MOGA  start_GA(...)
//  - hybrid genetic fuzzy start_GA_Fuzzy(...)
#define DEF_HASH_TABLE_SIZE   128

//#define DEF_POP1_SIZE          30 // 50
//#define DEF_POP2_SIZE          30 // 50
//#define DEF_PXOVER 0.8
//#define DEF_PMUTATION 0.01
//#define DEF_MAX_GENERATIONS 50

//#define REPORT_PARETO_STEP      5

#define IDX_ENERGY              0
#define IDX_CYCLES              1
#define IDX_AREA                2

#define BIG_CYCLES      MAXDOUBLE
#define BIG_ENERGY      MAXDOUBLE
#define BIG_AREA        MAXDOUBLE

//**************************************************************
void Explorer::start_GA(const GA_parameters& parameters)
{
  current_algo="GA";
  HashGA ht_ga(DEF_HASH_TABLE_SIZE); // maybe static?
  SPEA   ga;

  static ExportUserData eud;
  eud.explorer = this;
  eud.ht_ga = &ht_ga;
  eud.history.clear();

  static GA_parameters ga_parameters;
  ga_parameters.population_size = parameters.population_size;
  ga_parameters.pcrossover = parameters.pcrossover;
  ga_parameters.pmutation = parameters.pmutation;
  ga_parameters.max_generations = parameters.max_generations;

  Exploration_stats stats;
  stats.space_size = get_space_size();
  stats.feasible_size = get_feasible_size();
  stats.start_time = time(NULL);
  reset_sim_counter();

  init_GA(ga, &eud, &ga_parameters);

  string file_name = Options.benchmark+"_GA_"+current_space;

  while ( !ga.done() )
    {
      ga.step(); // evolve

      // save Pareto-set every report_pareto_step generations
      if (ga.currentGeneration() % parameters.report_pareto_step == 0)
      	{
      	  vector<Simulation> pareto = get_pareto(eud.history);

	  char temp[10];
	  sprintf(temp, "_%d",ga.currentGeneration());
	  save_simulations(pareto, file_name+string(temp)+".pareto.exp", SHOW_ALL);

	  ga_show_info(ga, eud, file_name+".info.stat");
	}
    }

  // save history
  save_simulations(eud.history, file_name+".history.stat", SHOW_ALL);

  // save statistics
  stats.end_time = time(NULL);
  stats.n_sim = get_sim_counter();
  save_stats(stats, file_name+".stat");
}

//******************************************************************************************//

void Explorer::start_GA_Fuzzy(const GA_parameters& parameters)
{
  current_algo="GA_Fuzzy";
  HashGA ht_ga(DEF_HASH_TABLE_SIZE); // maybe static?
  HashGA ht_hy(DEF_HASH_TABLE_SIZE);
  SPEA   ga;

  static ExportUserData eud;
  eud.explorer = this;
  eud.ht_ga = &ht_ga;
  eud.ht_hy = &ht_hy;
  eud.history.clear();

  /*
  vector <Simulation> vsim = load_simulations("history_cache.old");

  for(int i=0; i<vsim.size(); ++i) {
	vsim[i].exec_time = vsim[i].exec_time / 1000;
  	eud.ht_hy->addT(vsim[i]);
  }
  */

  static GA_parameters ga_parameters;

  ga_parameters.population_size = parameters.population_size;
  ga_parameters.pcrossover = parameters.pcrossover;
  ga_parameters.pmutation = parameters.pmutation;
  ga_parameters.max_generations = parameters.max_generations;

  Exploration_stats stats;
  stats.space_size = get_space_size();
  stats.feasible_size = get_feasible_size();
  stats.start_time = time(NULL);
  reset_sim_counter();

  init_GA_Fuzzy(ga, &eud, &ga_parameters);

  string file_name = Options.benchmark+"_FUZZY_"+current_space;

  //---- FuzzyApprox
  fuzzy_approx.Init(2.0f, getParameterRanges(), 2);
  // 2.0f = threshold
  // 2 = Number of objectives
  SimulateBestWorst(eud);

  vector<Simulation> pareto;
  eud.pareto = pareto;
  //-----
  
  while ( !ga.done() )
    {
      ga.step(); // evolve

      // save Pareto-set every report_pareto_step generations
      if (ga.currentGeneration() % parameters.report_pareto_step == 0)
      	{
      	  //pareto = get_pareto(eud.history);
	  pareto = eud.pareto;

	  char temp[10];
	  sprintf(temp, "_%d",ga.currentGeneration());
	  save_simulations(pareto, file_name+string(temp)+".pareto.exp", SHOW_ALL);

  	  save_simulations(eud.history, file_name+".history.stat", SHOW_ALL);
	  ga_show_info(ga, eud, file_name+".info.stat");
	}
    }

  // save history
  //save_simulations(eud.history, file_name+".history.stat", SHOW_ALL);

  // save statistics
  stats.end_time = time(NULL);
  stats.n_sim = get_sim_counter();
  save_stats(stats, file_name+".stat");
}


//********************************************************************

void Explorer::init_GA(SPEA& ga, ExportUserData* eud,GA_parameters* ga_parameters)
{

  ga.objectiveDimensions(n_obj); 

  ga.minimize(); /* minimization of the objectives */
  vector<vector<AlleleString::Allele> > alleles;
  // memory hierarchy parameters
  alleles.push_back(values2alleles(mem_hierarchy.L1D.block_size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L1D.size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L1D.associativity.get_values()));

  alleles.push_back(values2alleles(mem_hierarchy.L1I.block_size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L1I.size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L1I.associativity.get_values()));
  
  alleles.push_back(values2alleles(mem_hierarchy.L2U.block_size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L2U.size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L2U.associativity.get_values()));

  // processor parameters
  alleles.push_back(values2alleles(processor.integer_units.get_values()));
  alleles.push_back(values2alleles(processor.float_units.get_values()));
  alleles.push_back(values2alleles(processor.memory_units.get_values()));
  alleles.push_back(values2alleles(processor.branch_units.get_values()));
  alleles.push_back(values2alleles(processor.gpr_static_size.get_values()));
  alleles.push_back(values2alleles(processor.fpr_static_size.get_values()));
  alleles.push_back(values2alleles(processor.cr_static_size.get_values()));
  alleles.push_back(values2alleles(processor.pr_static_size.get_values()));
  alleles.push_back(values2alleles(processor.btr_static_size.get_values()));

  IND individual(alleles, GA_Evaluator, eud);
  individual.metric(DefaultObjectiveDistance);

  ga.initialize(individual);

  ga.maxGenerations(ga_parameters->max_generations);
  ga.pCrossover(ga_parameters->pcrossover);
  ga.pMutate(ga_parameters->pmutation);
  ga.pop1Size(ga_parameters->population_size);
  ga.pop2Size(ga_parameters->population_size);
}
//********************************************************************

void Explorer::init_GA_Fuzzy(SPEA& ga, ExportUserData* eud,GA_parameters* ga_parameters)
{

  ga.objectiveDimensions(n_obj); 

  ga.minimize(); /* minimization of the objectives */
  vector<vector<AlleleString::Allele> > alleles;
  // memory hierarchy parameters
  alleles.push_back(values2alleles(mem_hierarchy.L1D.block_size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L1D.size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L1D.associativity.get_values()));

  alleles.push_back(values2alleles(mem_hierarchy.L1I.block_size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L1I.size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L1I.associativity.get_values()));
  
  alleles.push_back(values2alleles(mem_hierarchy.L2U.block_size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L2U.size.get_values()));
  alleles.push_back(values2alleles(mem_hierarchy.L2U.associativity.get_values()));

  // processor parameters
  alleles.push_back(values2alleles(processor.integer_units.get_values()));
  alleles.push_back(values2alleles(processor.float_units.get_values()));
  alleles.push_back(values2alleles(processor.memory_units.get_values()));
  alleles.push_back(values2alleles(processor.branch_units.get_values()));
  alleles.push_back(values2alleles(processor.gpr_static_size.get_values()));
  alleles.push_back(values2alleles(processor.fpr_static_size.get_values()));
  alleles.push_back(values2alleles(processor.cr_static_size.get_values()));
  alleles.push_back(values2alleles(processor.pr_static_size.get_values()));
  alleles.push_back(values2alleles(processor.btr_static_size.get_values()));

  IND individual(alleles, GA_Fuzzy_Evaluator, eud);
  individual.metric(DefaultObjectiveDistance);

  ga.initialize(individual);

  ga.maxGenerations(ga_parameters->max_generations);
  ga.pCrossover(ga_parameters->pcrossover);
  ga.pMutate(ga_parameters->pmutation);
  ga.pop1Size(ga_parameters->population_size);
  ga.pop2Size(ga_parameters->population_size);
}


/*************************************************************************/

vector<AlleleString::Allele> Explorer::values2alleles(vector<int> values)
{
  vector<AlleleString::Allele> avec;
  
  for (int i=0; i<values.size(); i++)
    avec.push_back((AlleleString::Allele)values[i]);
  
  return avec;
}


/*************************************************************************/

void Explorer::ga_show_info(SPEA& ga, ExportUserData& eud,
			    string fname)
{
  ofstream fmsg(fname.c_str(), ios::ate);
  assert(fmsg.good());

  ga.parameters(fmsg);
  fmsg << endl << "simulations: " << eud.history.size() << endl;
}


/*************************************************************************/

void GA_Evaluator(IND& ind, ObjectiveVector& scores, void *user_data)
{
  double exec_time, energy, area;
  bool   feasible;

  feasible = GA_Evaluation(ind, user_data, exec_time, energy, area);

  if (((ExportUserData*)user_data)->explorer->get_obj_number()==2) 
  {
      if (feasible)
	{
	  scores[IDX_CYCLES] = exec_time;
	  scores[IDX_ENERGY] = energy;
	}
      else
	{
	  scores[IDX_CYCLES] = BIG_CYCLES;
	  scores[IDX_ENERGY] = BIG_ENERGY;
	}
  }
  else // 3 objectives
  {

      if (feasible)
	{
	  scores[IDX_CYCLES] = exec_time;
	  scores[IDX_ENERGY] = energy;
	  scores[IDX_AREA]   = area;
	}
      else
	{
	  scores[IDX_CYCLES] = BIG_CYCLES;
	  scores[IDX_ENERGY] = BIG_ENERGY;
	  scores[IDX_AREA]   = BIG_AREA;   
	}
  }

}


/*************************************************************************/

void GA_Fuzzy_Evaluator(IND& ind, ObjectiveVector& scores, void *user_data)
{
  double exec_time, energy, area;
  bool   feasible;

  feasible = GA_Fuzzy_Evaluation(ind, user_data, exec_time, energy, area);

  if (((ExportUserData*)user_data)->explorer->get_obj_number()==2) 
  {
      if (feasible)
	{
	  scores[IDX_CYCLES] = exec_time;
	  scores[IDX_ENERGY] = energy;
	}
      else
	{
	  scores[IDX_CYCLES] = BIG_CYCLES;
	  scores[IDX_ENERGY] = BIG_ENERGY;
	}
  }
  else // 3 objectives
  {

      if (feasible)
	{
	  scores[IDX_CYCLES] = exec_time;
	  scores[IDX_ENERGY] = energy;
	  scores[IDX_AREA]   = area;
	}
      else
	{
	  scores[IDX_CYCLES] = BIG_CYCLES;
	  scores[IDX_ENERGY] = BIG_ENERGY;
	  scores[IDX_AREA]   = BIG_AREA;   
	}
  }

}

/*************************************************************************/

bool GA_Evaluation(IND& ind, void *user_data, double& exec_time, double& energy,double& area)
{
  ExportUserData *eud      = (ExportUserData *)user_data;
  Explorer       *explorer = eud->explorer;
  HashGA         *ht_ga    = eud->ht_ga;

  Configuration conf;

  conf.L1D_block = ind[0];
  conf.L1D_size  = ind[1];
  conf.L1D_assoc = ind[2];

  conf.L1I_block = ind[3];
  conf.L1I_size  = ind[4];
  conf.L1I_assoc = ind[5];
  
  conf.L2U_block = ind[6];
  conf.L2U_size  = ind[7];
  conf.L2U_assoc = ind[8];

  conf.integer_units = ind[9];
  conf.float_units   = ind[10];
  conf.memory_units  = ind[11];
  conf.branch_units  = ind[12];

  conf.gpr_static_size = ind[13];
  conf.fpr_static_size = ind[14];
  conf.cr_static_size  = ind[15];
  conf.pr_static_size  = ind[16];
  conf.btr_static_size = ind[17];

  if ( !conf.is_feasible() ) return false;

  Simulation sim;
  sim.config = conf;
  Simulation *psim = ht_ga->searchT(sim);
  if (psim == NULL)
    {
      vector<Configuration> vconf;
      vconf.push_back(conf);

      vector<Simulation> vsim = explorer->simulate_space(vconf);
      assert(vsim.size() == 1);

      sim.energy = vsim[0].energy;
      sim.area   = vsim[0].area;
      sim.exec_time = vsim[0].exec_time;
      sim.clock_freq = vsim[0].clock_freq;
      ht_ga->addT(sim);
      
      eud->history.push_back(sim);
    }
  else
    {
      sim.energy = psim->energy;
      sim.area   = psim->area;
      sim.exec_time = psim->exec_time;      
      sim.clock_freq = psim->clock_freq;      
    }
  
  exec_time = sim.exec_time;
  energy  = sim.energy;
  area   = sim.area;
  
  return true;
}

/*************************************************************************/

bool GA_Fuzzy_Evaluation(IND& ind, void *user_data, double& exec_time, double& energy,double& area)
{
    ExportUserData *eud      = (ExportUserData *)user_data;
    Explorer       *explorer = eud->explorer;
    HashGA         *ht_ga    = eud->ht_ga;

    Configuration conf;

    conf.L1D_block = ind[0];
    conf.L1D_size  = ind[1];
    conf.L1D_assoc = ind[2];

    conf.L1I_block = ind[3];
    conf.L1I_size  = ind[4];
    conf.L1I_assoc = ind[5];

    conf.L2U_block = ind[6];
    conf.L2U_size  = ind[7];
    conf.L2U_assoc = ind[8];

    conf.integer_units = ind[9];
    conf.float_units   = ind[10];
    conf.memory_units  = ind[11];
    conf.branch_units  = ind[12];

    conf.gpr_static_size = ind[13];
    conf.fpr_static_size = ind[14];
    conf.cr_static_size  = ind[15];
    conf.pr_static_size  = ind[16];
    conf.btr_static_size = ind[17];

    if ( !conf.is_feasible() ) return false;

    Simulation sim;
    sim.config = conf;
    Simulation *psim = ht_ga->searchT(sim);

    if (psim == NULL) // not present in cache
    {
	vector<Configuration> vconf;
	vconf.push_back(conf);

	vector<Simulation> vsim;

	float max_avg_err;

	// check history
	psim = eud->ht_hy->searchT(sim);
	if (psim != NULL) 
	    vsim.push_back(*psim);
	    else 
		vsim = explorer->simulate_space(vconf);

	bool cacheable = vsim[0].simulated;
	int pos = 0;

	sim.energy = vsim[0].energy;
	sim.area   = vsim[0].area;
	sim.exec_time = vsim[0].exec_time;
	sim.clock_freq = vsim[0].clock_freq;

	eud->history.push_back(sim);
	eud->pareto.push_back(sim);
	eud->pareto = explorer->get_pareto(eud->pareto);

	if (cacheable) 
	    ht_ga->addT(sim);
	else 
	{
	    if (pos = explorer->simulation_present(sim,eud->pareto)) {
		psim = eud->ht_hy->searchT(sim);
		if (psim != NULL) sim = *psim;
		else 
		{
		    explorer->set_force_simulation(true);
		    sim = explorer->simulate_space(vconf)[0];
		    explorer->set_force_simulation(false);
		}
		eud->history[eud->history.size()-1] = sim;
		eud->pareto[pos] = sim;
		ht_ga->addT(sim);
	    }
	}

    }
    else
    {
	sim.energy = psim->energy;
	sim.area   = psim->area;
	sim.exec_time = psim->exec_time;      
	sim.clock_freq = psim->clock_freq;      
    }

    exec_time = sim.exec_time;
    energy  = sim.energy;
    area   = sim.area;

    return true;
}


/*************************************************************************/

void Explorer::SimulateBestWorst(ExportUserData& eud)
{
  vector<pair<int,int> > minmax = getParameterRanges();

  Configuration cnf_best;
  cnf_best.integer_units = minmax[0].second;
  cnf_best.float_units = minmax[1].second;
  cnf_best.branch_units = minmax[2].second;
  cnf_best.memory_units = minmax[3].second;
  cnf_best.gpr_static_size = minmax[4].second;
  cnf_best.fpr_static_size = minmax[5].second;
  cnf_best.pr_static_size = minmax[6].second;
  cnf_best.cr_static_size = minmax[7].second;
  cnf_best.btr_static_size = minmax[8].second;
  cnf_best.L1D_size = minmax[9].second;
  cnf_best.L1D_block = minmax[10].second;
  cnf_best.L1D_assoc = minmax[11].second;
  cnf_best.L1I_size = minmax[12].second;
  cnf_best.L1I_block = minmax[13].second;
  cnf_best.L1I_assoc = minmax[14].second;
  cnf_best.L2U_size = minmax[15].second;
  cnf_best.L2U_block = minmax[16].second;
  cnf_best.L2U_assoc = minmax[17].second;

  Configuration cnf_worst;
  cnf_worst.integer_units = minmax[0].first;
  cnf_worst.float_units = minmax[1].first;
  cnf_worst.branch_units = minmax[2].first;
  cnf_worst.memory_units = minmax[3].first;
  cnf_worst.gpr_static_size = minmax[4].first;
  cnf_worst.fpr_static_size = minmax[5].first;
  cnf_worst.pr_static_size = minmax[6].first;
  cnf_worst.cr_static_size = minmax[7].first;
  cnf_worst.btr_static_size = minmax[8].first;
  cnf_worst.L1D_size = minmax[9].first;
  cnf_worst.L1D_block = minmax[10].first;
  cnf_worst.L1D_assoc = minmax[11].first;
  cnf_worst.L1I_size = minmax[12].first;
  cnf_worst.L1I_block = minmax[13].first;
  cnf_worst.L1I_assoc = minmax[14].first;
  cnf_worst.L2U_size = minmax[15].first;
  cnf_worst.L2U_block = minmax[16].first;
  cnf_worst.L2U_assoc = minmax[17].first;

  vector<Configuration> cnf_best_worst;
  cnf_best_worst.push_back(cnf_best);
  cnf_best_worst.push_back(cnf_worst);

  vector<Simulation> sim_best_worst = simulate_space(cnf_best_worst);
  eud.history.push_back(sim_best_worst[0]);
  eud.history.push_back(sim_best_worst[1]);
  eud.ht_ga->addT(sim_best_worst[0]);
  eud.ht_ga->addT(sim_best_worst[1]);

}

////////////////////////////////////////////////////////////////////////////
// The following  are utility functions used by exploring functions
////////////////////////////////////////////////////////////////////////////
//********************************************************************
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
	cout << "\n DEBUG temp(ordinato) : " << temp[i].exec_time;
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
	cout << "\n DEBUG (orderer E): " << temp[i].energy;
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
	cout << "\n DEBUG (orderer Area): " << temp[i].area;
#endif

    return temp;
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
	      simulations[j].energy !=simulations[i].energy ||
	      simulations[j].area != simulations[i].area ||
	      simulations[j].exec_time != simulations[i].exec_time) 
	   )
	    dominated = true;

      // avoid repeated pareto configs
      if ( (!dominated) && (!simulation_present(simulations[i],pareto_set)) )
	pareto_set.push_back(simulations[i]);
  }

  return pareto_set;
}
// power/area pareto set

////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::load_simulations(const string& filename)
{
    vector<Simulation> sims;

    std::ifstream input_file(filename.c_str());

    if (!input_file) {
	cout << "\nError opening file " << filename;
	//wait_key();
    }
    else
    {
	go_until("L2U(CBA)",input_file);

	Simulation sim;

	string word;

	while (input_file.good())
	{
	    input_file >> sim.area;
	    input_file >> sim.energy;
	    input_file >> sim.exec_time;
	    
	    input_file >> word; // - separator
	    input_file >> sim.clock_freq;
	    input_file >> word;
	    input_file >> sim.config.integer_units;
	    input_file >> sim.config.float_units;
	    input_file >> sim.config.branch_units;
	    input_file >> sim.config.memory_units;
	    input_file >> word; // / separator
	    input_file >> sim.config.gpr_static_size;
	    input_file >> sim.config.fpr_static_size;
	    input_file >> sim.config.pr_static_size;
	    input_file >> sim.config.cr_static_size;
	    input_file >> sim.config.btr_static_size;
	    input_file >> word;// / separator
	    input_file >> sim.config.L1D_size;
	    input_file >> sim.config.L1D_block;
	    input_file >> sim.config.L1D_assoc;
	    input_file >> word;// / separator
	    input_file >> sim.config.L1I_size;
	    input_file >> sim.config.L1I_block;
	    input_file >> sim.config.L1I_assoc;
	    input_file >> word;// / separator
	    input_file >> sim.config.L2U_size;
	    input_file >> sim.config.L2U_block;
	    input_file >> sim.config.L2U_assoc;

	    if (input_file.good())
	    {
		sims.push_back(sim);
///		cout << "\n ho aggiunto ";
///		print_simulation(sim);
///		cin >> word;
	    }
	}
    }

    return sims;
}

////////////////////////////////////////////////////////////////////////////
Configuration Explorer::create_configuration(const Processor& p,const Mem_hierarchy& mem)
{
    Configuration conf;

    conf.gpr_static_size = processor.gpr_static_size.get_val();
    conf.fpr_static_size = processor.fpr_static_size.get_val();
    conf.pr_static_size = processor.pr_static_size.get_val();
    conf.cr_static_size = processor.cr_static_size.get_val();
    conf.btr_static_size = processor.btr_static_size.get_val();

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
    if (mask.integer_units) config.integer_units=base.integer_units;
    if (mask.float_units) config.float_units=base.float_units;
    if (mask.memory_units) config.memory_units=base.memory_units;
    if (mask.branch_units) config.branch_units=base.branch_units;
    if (mask.gpr_static_size) config.gpr_static_size=base.gpr_static_size;
    if (mask.fpr_static_size) config.fpr_static_size=base.fpr_static_size;
    if (mask.pr_static_size) config.pr_static_size=base.pr_static_size;
    if (mask.cr_static_size) config.cr_static_size=base.cr_static_size;
    if (mask.btr_static_size) config.btr_static_size=base.btr_static_size;

    return config;
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

    save_simulations(res,filename+".exp",SHOW_L1D);
    vector<Simulation> pareto_set = get_pareto(res);

    save_simulations(pareto_set,filename+".pareto.exp",SHOW_L1D);
    */


    /* Example 2 ******************************************
     * exploring a parameter indentified by a string
     * ****************************************************

    string parameter_name = ....
    Space_mask mask= get_space_mask(UNSET_ALL);

    if (parameter_name=="gpr_static_size") mask.gpr_static_size = true;
    else
    if (parameter_name=="fpr_static_size") mask.fpr_static_size = true;
    else
    if (parameter_name=="pr_static_size") mask.pr_static_size = true;
    else
    if (parameter_name=="cr_static_size") mask.cr_static_size = true;
    else
    if (parameter_name=="btr_static_size") mask.btr_static_size = true;

    // add further code for detecting other parameter names ...
    
    vector<Configuration> space = build_space(mask);
    vector<Simulation> simulations = simulate_space(space);
    vector<Simulation> pareto_set = get_pareto(simulations);

    string filename = Options.benchmark+"_EXHA_"+parameter_name;

    save_simulations(simulations,filename+".exp",SHOW_ALL);
    save_simulations(pareto_set,filename+".pareto.exp",SHOW_ALL);
     */
}


//*************************************************************************

int Explorer::count_needed_recompilations(const vector<Configuration>& space)
{
    bool processor_changed;
    int recompilations = 0;

    int last_integer_units = -1;
    int last_float_units = -1;
    int last_branch_units = -1;
    int last_memory_units = -1;

    int last_gpr_static_size = -1;
    int last_fpr_static_size = -1;
    int last_pr_static_size = -1;
    int last_cr_static_size = -1;
    int last_btr_static_size = -1;

    for (unsigned int i=0;i<space.size();i++)
    {
	processor_changed = (  (space[i].integer_units!= last_integer_units)
			    || (space[i].float_units!= last_float_units)
			    || (space[i].branch_units!=last_branch_units)
			    || (space[i].memory_units!=last_memory_units)
			    || (space[i].gpr_static_size!=last_gpr_static_size)
			    || (space[i].fpr_static_size!=last_fpr_static_size)
			    || (space[i].pr_static_size!= last_pr_static_size)
			    || (space[i].cr_static_size!= last_cr_static_size)
			    || (space[i].btr_static_size!=last_btr_static_size));

	if (processor_changed) recompilations++;

	last_integer_units = space[i].integer_units;
	last_float_units = space[i].float_units;
	last_memory_units = space[i].memory_units;
	last_branch_units = space[i].branch_units;
	last_gpr_static_size = space[i].gpr_static_size;
	last_fpr_static_size = space[i].fpr_static_size;
	last_pr_static_size = space[i].pr_static_size;
	last_cr_static_size = space[i].cr_static_size;
	last_btr_static_size = space[i].btr_static_size;
    }

    return recompilations;
}

////////////////////////////////////////////////////////////////////////////
void Explorer::save_simulations(const vector<Simulation>& simulations, string filename,int format)
{

    int L1D_C,L1D_B,L1D_A;
    int L1I_C,L1I_B,L1I_A;
    int L2U_C,L2U_B,L2U_A;

    int gpr,fpr,pr,cr,btr; // register file sizes
    int IU,FPU,MU,BU;      // number of instancies of each unit
    double area,energy;
    double energydelay;
    double exec_time;
    
    FILE * fp;

    string path = get_base_dir()+"/trimaran-workspace/epic-explorer/";
    if (Options.hyperblock) filename = "H_"+filename;
    string file_path = path+filename;

    time_t now = time(NULL);
    string uora = string(asctime(localtime(&now)));
    string pretitle = "\n%% Epic Explorer simulation file - created on " + uora;

    string pretitle2 ="\n%% Enabled minimization objectives : ";
    string pretitle3 ="\n%% Benchmark : "+Options.benchmark;

    if (Options.objective_area) pretitle2+="Area, ";
    if (Options.objective_energy) pretitle2+="Energy, ";
    if (Options.objective_power) pretitle2+= "Average Power, ";
    if (Options.objective_exec_time) pretitle2+="Execution Time";
    if (Options.hyperblock) pretitle2+="\n%% Hyperblock formation : ENABLED";
    else
	pretitle2+="\n%% Hyperblock formation : DISABLED";

    string title = "\n%% ";

    // currently, energy and power are mutually exclusive objectives
    if (Options.objective_energy) title+="Area(cm^2)           Energy(J)  Exec time(ms) - ";
    else
	title+="Area(cm^2)     Average Power(W)  Exec time(ms) - ";

    if ((format==SHOW_L1D)||(format==SHOW_L1I)||(format==SHOW_L2U))
	title+= " Size Bsize Assoc ";
    if (format==SHOW_REGISTER_FILES) title+= " gpr fpr pr cr btr ";
    if (format==SHOW_UNITS) title+=" IU FPU BU MU ";
    if (format==SHOW_ALL) 
	title+="/ clock(mhz) / I F B M / gpr fpr pr cr btr / L1D(CBA) / L1I(CBA) / L2U(CBA) ";
    if (format==SHOW_ENERGYDELAY)
    {
	if (Options.objective_energy)
	title=" Energy*Exec time - I F B M / gpr fpr pr cr btr / L1D(CBA) / L1I(CBA) / L2U(CBA) ";
	else
	title=" Power*Exec time - I F B M / gpr fpr pr cr btr / L1D(CBA) / L1I(CBA) / L2U(CBA) ";
    }


    fp=fopen(file_path.c_str(),"w");
    
    fprintf(fp,"\n% ----------------------------------------------");
    fprintf(fp,pretitle.c_str());
    fprintf(fp,pretitle2.c_str());
    fprintf(fp,pretitle3.c_str());
    fprintf(fp,title.c_str());
    fprintf(fp,"\n% ----------------------------------------------");

    for (unsigned int i =0;i<simulations.size();i++)
    {
	area = simulations[i].area;
	energy = simulations[i].energy;
	exec_time = simulations[i].exec_time*1000;

	if (format==SHOW_L1D)
	{
	    L1D_C = simulations[i].config.L1D_size;
	    L1D_B = simulations[i].config.L1D_block;
	    L1D_A = simulations[i].config.L1D_assoc;
	    fprintf(fp,"\n%.14f  %.14f  %.14f  %/  %d %d %d",area,energy,exec_time,L1D_C,L1D_B,L1D_A);
	}
	else if (format==SHOW_L1I)
	{
	    L1I_C = simulations[i].config.L1I_size;
	    L1I_B = simulations[i].config.L1I_block;
	    L1I_A = simulations[i].config.L1I_assoc;
	    fprintf(fp,"\n%.14f  %.14f  %.14f %/  %d %d %d",area,energy,exec_time,L1I_C,L1I_B,L1I_A);
	}
	else if (format==SHOW_L2U)
	{
	    L2U_C = simulations[i].config.L2U_size;
	    L2U_B = simulations[i].config.L2U_block;
	    L2U_A = simulations[i].config.L2U_assoc;
	    fprintf(fp,"\n%.14f  %.14f  %.14f %/  %d %d %d",area,energy,exec_time,L2U_C,L2U_B,L2U_A);
	}
	else if (format==SHOW_REGISTER_FILES)
	{
	    gpr=simulations[i].config.gpr_static_size;
	    fpr=simulations[i].config.fpr_static_size;
	    pr= simulations[i].config.pr_static_size;
	    cr= simulations[i].config.cr_static_size;
	    btr=simulations[i].config.btr_static_size;
	    fprintf(fp,"\n%.14f  %.14f  %.14f %/  %d %d %d %d %d",area,energy,exec_time,gpr,fpr,pr,cr,btr);
	}
	else if (format==SHOW_UNITS)
	{
	    IU = simulations[i].config.integer_units;
	    FPU =simulations[i].config.float_units;
	    BU = simulations[i].config.branch_units;
	    MU = simulations[i].config.memory_units;
	    fprintf(fp,"\n%.14f  %.14f  %.14f %/  %d %d %d %d ",area,energy,exec_time,IU,FPU,BU,MU);
	}
	else if (format==SHOW_ALL)
	{
	    gpr=simulations[i].config.gpr_static_size;
	    fpr=simulations[i].config.fpr_static_size;
	    pr= simulations[i].config.pr_static_size;
	    cr= simulations[i].config.cr_static_size;
	    btr=simulations[i].config.btr_static_size;

	    IU = simulations[i].config.integer_units;
	    FPU =simulations[i].config.float_units;
	    BU = simulations[i].config.branch_units;
	    MU = simulations[i].config.memory_units;

	    L1D_C = simulations[i].config.L1D_size;
	    L1D_B = simulations[i].config.L1D_block;
	    L1D_A = simulations[i].config.L1D_assoc;

	    L1I_C = simulations[i].config.L1I_size;
	    L1I_B = simulations[i].config.L1I_block;
	    L1I_A = simulations[i].config.L1I_assoc;
	    
	    L2U_C = simulations[i].config.L2U_size;
	    L2U_B = simulations[i].config.L2U_block;
	    L2U_A = simulations[i].config.L2U_assoc;

	    int mhz = int(simulations[i].clock_freq/1e6);

	   
	    char ch = ' ';
	    if (!simulations[i].simulated) ch = '*';

	    fprintf(fp,"\n%.14f  %.14f  %.14f %/ %d / %d %d %d %d / %d %d %d %d %d / %d %d %d / %d %d %d / %d %d %d %c",area,energy,exec_time,mhz,IU,FPU,BU,MU,gpr,fpr,pr,cr,btr,L1D_C,L1D_B,L1D_A,L1I_C,L1I_B,L1I_A,L2U_C,L2U_B,L2U_A,&ch);
	}
	else if (format==SHOW_ENERGYDELAY)
	{
	    gpr=simulations[i].config.gpr_static_size;
	    fpr=simulations[i].config.fpr_static_size;
	    pr= simulations[i].config.pr_static_size;
	    cr= simulations[i].config.cr_static_size;
	    btr=simulations[i].config.btr_static_size;

	    IU = simulations[i].config.integer_units;
	    FPU =simulations[i].config.float_units;
	    BU = simulations[i].config.branch_units;
	    MU = simulations[i].config.memory_units;

	    L1D_C = simulations[i].config.L1D_size;
	    L1D_B = simulations[i].config.L1D_block;
	    L1D_A = simulations[i].config.L1D_assoc;

	    L1I_C = simulations[i].config.L1I_size;
	    L1I_B = simulations[i].config.L1I_block;
	    L1I_A = simulations[i].config.L1I_assoc;
	    
	    L2U_C = simulations[i].config.L2U_size;
	    L2U_B = simulations[i].config.L2U_block;
	    L2U_A = simulations[i].config.L2U_assoc;

	    energydelay = simulations[i].energy * simulations[i].exec_time;


	    fprintf(fp,"\n%.14f % - %d %d %d %d / %d %d %d %d %d / %d %d %d / %d %d %d / %d %d %d ",energydelay,IU,FPU,BU,MU,gpr,fpr,pr,cr,btr,L1D_C,L1D_B,L1D_A,L1I_C,L1I_B,L1I_A,L2U_C,L2U_B,L2U_A);
	}
    }
    fclose(fp);
}

void Explorer::save_objectives_details(const Dynamic_stats& dyn,const Configuration& config, const string filename) const
{
    int L1D_C,L1D_B,L1D_A;
    int L1I_C,L1I_B,L1I_A;
    int L2U_C,L2U_B,L2U_A;

    int gpr,fpr,pr,cr,btr; // register file sizes
    int IU,FPU,MU,BU;      // number of instancies of each unit

    FILE * fp;
    fp=fopen(filename.c_str(),"a");

    gpr=config.gpr_static_size;
    fpr=config.fpr_static_size;
    pr= config.pr_static_size;
    cr= config.cr_static_size;
    btr=config.btr_static_size;

    IU = config.integer_units;
    FPU =config.float_units;
    BU = config.branch_units;
    MU = config.memory_units;

    L1D_C = config.L1D_size;
    L1D_B = config.L1D_block;
    L1D_A = config.L1D_assoc;

    L1I_C = config.L1I_size;
    L1I_B = config.L1I_block;
    L1I_A = config.L1I_assoc;
    
    L2U_C = config.L2U_size;
    L2U_B = config.L2U_block;
    L2U_A = config.L2U_assoc;

    fprintf(fp,"\n %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu ",
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
							    dyn.L1D_capacity_misses,
							    dyn.L1D_conflict_misses,
							    dyn.L1I_hit,
							    dyn.L1I_miss,
							    dyn.L1I_capacity_misses,
							    dyn.L1I_conflict_misses,
							    dyn.L2U_r_hit,
							    dyn.L2U_w_hit,
							    dyn.L2U_r_miss,
							    dyn.L2U_w_miss,
							    dyn.L2U_capacity_misses,
							    dyn.L2U_conflict_misses);

    fprintf(fp,"%% %d %d %d %d / %d %d %d %d %d / %d %d %d / %d %d %d / %d %d %d ",IU,FPU,BU,MU,gpr,fpr,pr,cr,btr,L1D_C,L1D_B,L1D_A,L1I_C,L1I_B,L1I_A,L2U_C,L2U_B,L2U_A);
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

    fprintf(fp,"\n Space size : %lf ",stats.space_size);
    fprintf(fp,"\n Feasible configurations : %lf ",stats.feasible_size);
    fprintf(fp,"\n n. simulations: %d ",stats.n_sim);
    fprintf(fp,"\n total exploration time : %d minutes ",elapsed_time);
    fprintf(fp,"\n simulation start : %s ",asctime(localtime(&stats.start_time)));
    fprintf(fp,"\n simulation end : %s ",asctime(localtime(&stats.end_time)));
    //fprintf(" needed recompilations : %d ( %d% )",stats.recompilations,stats.recompilations/stats.total_explorations);

    fclose(fp);
}

////////////////////////////////////////////////////////////////////////////

// DA CONTROLLARE, causa problemi !
int Explorer::DEP_phase2_time(float P,int * n_ott)
{
    double est_time = 0;
    int prod;

    for (int i=1;i<9;i++)
    {
	prod = 1;

	for (int j=1;j<i+2;j++) prod = prod * n_ott[j-1];

	est_time += pow(P,i-1) * prod;
    }
    // assuming recompilation in 50% of cases 
    est_time = est_time * (2*average_exec_time+average_compilation_time)/2;

    // in minutes
    est_time = est_time/60;
    return (int)est_time;
}

////////////////////////////////////////////////////////////////////////////
vector<Simulation> Explorer::simulate_space(const vector<Configuration>& space)
{

    static int n_simulate_space_call = 0;
    static int n_exploration_executed = 0; 

    //----------------------------------------------------------------
    if (Options.save_spaces)
    {
	n_simulate_space_call++;

	if (get_sim_counter()==0)
	{
	    // if sim_counter has be resetted , you probably started a
	    // new exploration alg, so n_simulate_space_call must be
	    // resetted and n_exploration_executed increased
	    //
	    // example : epic_space_EXP_2.12
	    // 12th explorated space of 2nd exploration algorithm
	    //
	    n_simulate_space_call=1;
	    n_exploration_executed++;
	}

	char name[40];
	sprintf(name,"_simulatedspace_%d",n_simulate_space_call);

	string file_name = Options.benchmark+"_"+current_algo+"_"+current_space+string(name);

	// a pseudo vector<Simulation> must be created , because it's only
	// available a save_simultation() function 

	vector<Simulation> pseudo_sims;
	Simulation pseudo_sim;

	for (unsigned int i = 0;i< space.size();i++)
	{
	    pseudo_sim.config = space[i];
	    pseudo_sim.area = 0.0;
	    pseudo_sim.exec_time = 0.0;
	    pseudo_sim.energy = 0.0;
	    pseudo_sim.simulated = fuzzy_enabled;
	    pseudo_sims.push_back(pseudo_sim);
	}
	save_simulations(pseudo_sims,file_name,SHOW_ALL);

    } // save simulated spaces option ------------------------------------



    // -------------------------------------------------------------------
    // This is an optimization to reduce exploration time .
    // If the space that has to be explored is the same of the
    // previous call of simulate space, we can avoid to repeat all
    // the simulations. This happens for example when a space is
    // combined with all possible values of a parameter that has only
    // an single admissible value.
    // NOTE: benchmark and hyperblock formation must be the same too.
    // NOTE bis: within the exploration of single space an
    // algorithm can make multiple call of simulate_space(...)
    // function. 

    vector<Configuration> previous_space = extract_space(previous_simulations);

    if ((Options.benchmark==previous_benchmark) && 
	(Options.hyperblock==previous_hyperblock) &&
	 equivalent_spaces(previous_space,space)) 
	    return previous_simulations;// return here, without simulating 
    // -------------------------------------------------------------------



    vector<Simulation> simulations;
    Simulation temp_sim;

    bool processor_changed;
    bool compilation_changed;
    bool do_simulation = (force_simulation || (!(fuzzy_enabled && fuzzy_approx.Reliable())));

    int last_integer_units;
    int last_float_units;
    int last_branch_units;
    int last_memory_units;
    int last_gpr_static_size;
    int last_fpr_static_size;
    int last_pr_static_size;
    int last_cr_static_size;
    int last_btr_static_size;

    // -------------------------------------------------------------------
    if (do_simulation)
    {
	// here are stored the values of the last simulation
	Configuration last_config;

	if (previous_simulations.size()>0)
	    last_config = previous_simulations[previous_simulations.size()-1].config;
	else 
	    // this is the first call of simulate_space(...)
	    // current config should always be considered a new one
	{
	    last_config.integer_units = -1;
	    last_config.float_units = -1;
	    last_config.branch_units = -1;
	    last_config.memory_units = -1;
	    last_config.gpr_static_size = -1;
	    last_config.fpr_static_size = -1;
	    last_config.pr_static_size = -1;
	    last_config.cr_static_size = -1;
	    last_config.btr_static_size = -1;
	}

	// Even if the simulate_space(...) call uses the same space of the previous call,
	// we should recompile on the first cycle of the for loop below 
	// if the benchmark and/or the compilation option are different

	compilation_changed = (Options.benchmark!=previous_benchmark)||(Options.hyperblock!=previous_hyperblock);

    }
    // -------------------------------------------------------------------
    //  main exploration cycles
    // -------------------------------------------------------------------

    for (unsigned int i = 0;i< space.size();i++)
    {
	cout << "\n -------> E P I C  E X P L O R E R >>>> simulation n." << i+1 << " / " << space.size();

	temp_sim.config = space[i];

	processor.set_config(space[i]);
	mem_hierarchy.set_config(space[i]);





	if (do_simulation)
	{
	            processor_changed = 
		       (space[i].integer_units!= last_integer_units)
		    || (space[i].float_units!= last_float_units)
		    || (space[i].branch_units!=last_branch_units)
		    || (space[i].memory_units!=last_memory_units)
		    || (space[i].gpr_static_size!=last_gpr_static_size)
		    || (space[i].fpr_static_size!=last_fpr_static_size)
		    || (space[i].pr_static_size!= last_pr_static_size)
		    || (space[i].cr_static_size!= last_cr_static_size)
		    || (space[i].btr_static_size!=last_btr_static_size);


	    // if a processor parameter changed, we need to update current
	    // processor config , save hmdes file, and recompile benchmark
	    // NOTE: for the first loop cycle we must check whether
	    // previous benchmark or compilation options to be simulated
	    // changed.

	    if ( (processor_changed) || (i==0 && compilation_changed) )
	    {
		last_integer_units = space[i].integer_units;
		last_float_units =  space[i].float_units;
		last_branch_units = space[i].branch_units;
		last_memory_units = space[i].memory_units;
		last_gpr_static_size = space[i].gpr_static_size;
		last_fpr_static_size = space[i].fpr_static_size;
		last_pr_static_size = space[i].pr_static_size;
		last_cr_static_size = space[i].cr_static_size;
		last_btr_static_size = space[i].btr_static_size;

		time_t begin = time(NULL);

		trimaran_interface->save_processor_config(processor);
		trimaran_interface->compile_hmdes_file();
		trimaran_interface->compile_benchmark();

		time_t end = time(NULL);

		average_compilation_time = (int)(difftime(end,begin));
	    }

	    trimaran_interface->save_mem_config(mem_hierarchy);

	    time_t t1 = time(NULL);
	    trimaran_interface->execute_benchmark();
	    time_t t2 = time(NULL);
	    average_exec_time = (int)(difftime(t2,t1));

	    dyn_stats = trimaran_interface->get_dynamic_stats();
	    estimate = estimator.get_estimate(dyn_stats,mem_hierarchy,processor);

	    if (fuzzy_enabled) fuzzy_approx.Learn(space[i],dyn_stats);
	}
	else 
	{
	    dyn_stats = fuzzy_approx.EstimateG(space[i]);
	    estimate = estimator.get_estimate(dyn_stats,mem_hierarchy,processor);
	}
	// -------------------------------------------------------------------

	if (Options.objective_energy) temp_sim.energy = estimate.total_system_energy;
	else if (Options.objective_power) temp_sim.energy = estimate.total_average_power;
	else 
	{
	    cout << "\n **** FATAL ERROR: No energy/power objective enabled ! ";
	    cout << "\n Quitting...";
	    exit(1);
	};
	temp_sim.area = estimate.total_area;
	temp_sim.exec_time = estimate.execution_time;
	temp_sim.clock_freq = estimate.clock_freq;
	temp_sim.simulated = do_simulation;

	simulations.push_back(temp_sim);

	// -------------------------------------------------------------------
	if (Options.save_PD_STATS && do_simulation)
	{
	    string command;
	    string epic_path = get_base_dir()+"/trimaran-workspace/epic-explorer";
	    char temp[10];
	    sprintf(temp,"%d",i);

	    if (Options.hyperblock)
		command = "cp "+epic_path+"/simu_intermediate/PD_STATS.HS "+epic_path+"/PD_STATS.HS_"+string(temp);
	    else
		command = "cp "+epic_path+"/simu_intermediate/PD_STATS.O "+epic_path+"/PD_STATS.O_"+string(temp);

	    system(command.c_str());
	}

	// -------------------------------------------------------------------
	if (Options.save_estimation && do_simulation)
	{
	    char temp[10];
	    sprintf(temp,"%d",i);
	    string filename= Options.benchmark+"_"+current_algo+"_"+current_space+"."+string(temp)+".est";
	    save_estimation_file(dyn_stats,estimate,processor, mem_hierarchy,filename);
	}

	//if (Options.save_objectives_details && do_simulation)
	{
	    string filename= Options.benchmark+"_"+current_algo+"_"+current_space+".details";
	    save_objectives_details(dyn_stats,temp_sim.config,filename);
	}

    } // end for

    // when simultation is finished , all default values must be
    // restored

    processor.set_to_default();
    trimaran_interface->save_processor_config(processor);

    mem_hierarchy.set_to_default();
    trimaran_interface->save_mem_config(mem_hierarchy);

    sim_counter+=simulations.size();

    // update current simulated space and benchmark
    previous_simulations.clear();
    append_simulations(previous_simulations,simulations);
    previous_benchmark = Options.benchmark;
    previous_hyperblock = Options.hyperblock;

    return simulations;
}

////////////////////////////////////////////////////////////////////////////
void Explorer::save_estimation_file( const Dynamic_stats& dynamic_stats , 
				         const Estimate& estimate, 
					 Processor& processor, 
					 Mem_hierarchy& mem,
					 string& filename) const
{
    string file_path;
    file_path = get_base_dir()+"/trimaran-workspace/epic-explorer/";
    if (Options.hyperblock) filename = "H_"+filename;
    file_path += filename;

    std::ofstream output_file(file_path.c_str());

    if (!output_file)
    {
	cout << "\n Error while saving " << file_path ;
	wait_key();
    }
    else
    {
	output_file << "\n >>>>>>>> EPIC Explorer estimation file : " << filename;
	output_file << "\n";
	output_file << "\n **************************************************";
	output_file << "\n GPR,FPR,PR,CR,BTR = " <<processor.gpr_static_size.get_val();
	output_file << "," <<  processor.fpr_static_size.get_val();
	output_file << "," <<  processor.pr_static_size.get_val();
	output_file << "," <<  processor.cr_static_size.get_val();
	output_file << "," <<  processor.btr_static_size.get_val();

	output_file << "\n IU,FPR,MU,BU =  " << processor.integer_units.get_val();
	output_file << "," <<   processor.float_units.get_val();
	output_file << "," <<  processor.memory_units.get_val();
	output_file << "," <<  processor.branch_units.get_val();

	output_file <<"\n";
	output_file <<"\n L2U Size,BSize,Assoc = " << mem.L2U.size.get_val() << "," << mem.L2U.block_size.get_val() << "," << mem.L2U.associativity.get_val();
	output_file <<"\n L1I Size,BSize,Assoc = " << mem.L1I.size.get_val() << "," << mem.L1I.block_size.get_val() << "," << mem.L1I.associativity.get_val();
	output_file <<"\n L1D Size,BSize,Assoc = " << mem.L1D.size.get_val() << "," << mem.L1D.block_size.get_val() << "," << mem.L1D.associativity.get_val();
	output_file << "\n ****************************************************";
	output_file << "\n";
	output_file << "\n  P e r f o r m a n c e ";
	output_file << "\n ----------------------------------------------------";
	output_file << "\n     Total cycles: " << estimate.execution_cycles;
	output_file << "\n   Compute cycles: " << estimate.compute_cycles;
	output_file << "\n     Stall cycles: " << estimate.stall_cycles;
	output_file << "\n   Execution time (ms): " << estimate.execution_time*1000 << " (clock : " << estimate.clock_freq/1e6 << "MHz)";
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
	output_file << "\n   L1D cache energy (mJ) : " << estimate.L1D_energy*1000;
	output_file << "\n   L1I cache energy (mJ) : " << estimate.L1I_energy*1000;
	output_file << "\n   L2U cache energy (mJ) : " << estimate.L2U_energy*1000;
	output_file << "\n-->  Total Cache energy (mJ) " << estimate.total_cache_energy*1000;
	output_file << "\n   Main Memory energy (mJ) : " << estimate.main_memory_energy*1000;
	output_file << "\n   Processor energy (mJ) : " << estimate.total_processor_energy*1000;
	output_file << "\n\n--> Total System Energy (mJ) : "<< estimate.total_system_energy*1000;
	output_file << "\n--> Average System Power (W) :"<< estimate.total_average_power;
	output_file << "\n\n   Total System Energy (no stalls) (mJ) : "<< estimate.NO_MEM_system_energy*1000;
	output_file << "\n   Average System Power (no stalls) (W) :"<< estimate.NO_MEM_system_energy/(estimate.clock_T*dynamic_stats.total_cycles);

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
	output_file << "\n L2U (read hit/miss): ("<< dynamic_stats.L2U_r_hit<<"/" << dynamic_stats.L2U_r_miss<<")";
	output_file << "\n L2U (write hit/miss): ("<< dynamic_stats.L2U_w_hit<<"/" << dynamic_stats.L2U_w_miss<<")";
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


    switch (mask_type)
    {
	case SET_ALL:
	    mask.gpr_static_size = true;
	    mask.fpr_static_size = true;
	    mask.pr_static_size = true;
	    mask.cr_static_size = true;
	    mask.btr_static_size = true;

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

	    break;

	case UNSET_ALL:
	    // do nothing , mask is already set!
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
	    mask.integer_units = true;
	    mask.float_units = true;
	    mask.memory_units = true;
	    mask.branch_units = true;
	    break;

	default :
	    break;
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

    if ((mem_hierarchy.test_valid_cache(base_conf.L1D_size,base_conf.L1D_block,base_conf.L1D_assoc)) &&
	(mem_hierarchy.test_valid_cache(base_conf.L1I_size,base_conf.L1I_block,base_conf.L1I_assoc)) &&
	(mem_hierarchy.test_valid_cache(base_conf.L2U_size,base_conf.L2U_block,base_conf.L2U_assoc)) )
	{
	    // Admit or not all cache size combinations

	    if (opt==NO_L2_SIZE_CHECK) space.push_back(base_conf);
	    else
	    if (base_conf.L2U_size >= base_conf.L1D_size + base_conf.L1I_size) space.push_back(base_conf);
	}
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
				    }while ( (mask.btr_static_size) && (processor.btr_static_size.increase() ) );
				}while ( (mask.cr_static_size) && (processor.cr_static_size.increase() ) );
			    }while ( (mask.pr_static_size) && (processor.pr_static_size.increase() ) );
			} while ( (mask.fpr_static_size) && (processor.fpr_static_size.increase() ) );
		    } while ( (mask.gpr_static_size) && (processor.gpr_static_size.increase() ) );
		} while ( (mask.branch_units) && (processor.branch_units.increase()) );
	    } while ( (mask.memory_units) && (processor.memory_units.increase()) ) ;

	} while( (mask.float_units) && (processor.float_units.increase()) );

    } while ( (mask.integer_units) && (processor.integer_units.increase() ));

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
// Example :
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

	for(unsigned int n2=0;n2<s2.size();n2++)
	{
	    if (mask2.gpr_static_size) base_conf.gpr_static_size=s2[n2].gpr_static_size;
	    if (mask2.fpr_static_size) base_conf.fpr_static_size=s2[n2].fpr_static_size;
	    if (mask2.pr_static_size) base_conf.pr_static_size=s2[n2].pr_static_size;
	    if (mask2.cr_static_size) base_conf.cr_static_size=s2[n2].cr_static_size;
	    if (mask2.btr_static_size) base_conf.btr_static_size=s2[n2].btr_static_size;

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

	    // resulting combination of two configurations may result
	    // in not feasible config

    if ((mem_hierarchy.test_valid_cache(base_conf.L1D_size,base_conf.L1D_block,base_conf.L1D_assoc)) &&
	(mem_hierarchy.test_valid_cache(base_conf.L1I_size,base_conf.L1I_block,base_conf.L1I_assoc)) &&
	(mem_hierarchy.test_valid_cache(base_conf.L2U_size,base_conf.L2U_block,base_conf.L2U_assoc)) && 
	(base_conf.L2U_size>=base_conf.L1D_size+base_conf.L1I_size) &&
	(!(configuration_present(base_conf,merged_space))) ) merged_space.push_back(base_conf);

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
	    &&(conf.integer_units == space[i].integer_units        )
	    &&(conf.float_units == space[i].float_units            )
	    &&(conf.memory_units == space[i].memory_units          )
	    &&(conf.branch_units == space[i].branch_units          )
	    &&(conf.L1D_size == space[i].L1D_size                  )
	    &&(conf.L1D_block == space[i].L1D_block      )
	    &&(conf.L1D_assoc == space[i].L1D_assoc)
	    &&(conf.L1I_size == space[i].L1I_size                  )
	    &&(conf.L1I_block == space[i].L1I_block      )
	    &&(conf.L1I_assoc == space[i].L1I_assoc)
	    &&(conf.L2U_size == space[i].L2U_size                  )
	    &&(conf.L2U_block == space[i].L2U_block      )
	    &&(conf.L2U_assoc == space[i].L2U_assoc))

	    return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////
int Explorer::simulation_present(const Simulation& sim,const vector<Simulation>& simulations) const
{
    if (simulations.size()==0) return (0);

    for (int i=0;i<simulations.size();i++)
    {
	if ( 
		(simulations[i].area == sim.area) &&
		(simulations[i].energy == sim.energy) &&
		(simulations[i].exec_time == sim.exec_time) 
	   )
	    return (i);
    }
    return (0);
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

    return mask;

}

////////////////////////////////////////////////////////////////////////////
// compute the total number of configurations considering all
// possible values of paremeters whose mask boolean is set true

double Explorer::get_space_size(const Space_mask& mask) const
{
    double size = 1;


    if (mask.gpr_static_size) size = size*(processor.gpr_static_size.get_size());
    if (mask.fpr_static_size) size = size*(processor.fpr_static_size.get_size());
    if (mask.pr_static_size) size = size*(processor.pr_static_size.get_size());
    if (mask.cr_static_size) size = size*(processor.cr_static_size.get_size());
    if (mask.btr_static_size) size = size*(processor.btr_static_size.get_size());

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

    return size;
}

////////////////////////////////////////////////////////////////////////////
// if no mask is specified, total configuration space size is returned
double Explorer::get_space_size() const
{
    Space_mask all_parm = get_space_mask(SET_ALL);
    return get_space_size(all_parm);

}

////////////////////////////////////////////////////////////////////////////
// WARNING : disabled ( returns 0)
double Explorer::get_feasible_size() const
{
    Space_mask mask = get_space_mask(SET_ALL);

    //vector<Configuration> space = build_space(mask);

    return 0;
    //return space.size();
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
	if (!simulation_present(new_sims[i],dest)) dest.push_back(new_sims[i]);
    }
}

////////////////////////////////////////////////////////////////////////////
void Explorer::write_log(string mess)
{
    time_t t = time(NULL);
    char * data;
    FILE * fp;

    string filename = get_base_dir()+"/trimaran-workspace/epic-explorer/";

    filename +=current_algo+"_exploration.log";

    fp=fopen(filename.c_str(),"a");

    data = asctime(localtime(&t));

    fprintf(fp,"\n %s ---> %s \n",data,mess.c_str());

    fclose(fp);
}

////////////////////////////////////////////////////////////////////////////
int Explorer::DEP_phase1_time(int n_ott_c1,int n_ott_c2,int n_ott_c4)
{
    double l1d_s = get_space_size(get_space_mask(SET_L1D));
    double l1i_s = get_space_size(get_space_mask(SET_L1I));
    double l2u_s = get_space_size(get_space_mask(SET_L2U));

    double units_size = get_space_size(get_space_mask(SET_PROCESSOR_UNITS));

    double sum_of_reg_sizes = 0;
    // sum of configuration space sizes of register files

    Space_mask mask;
    
    mask = get_space_mask(UNSET_ALL);
    mask.gpr_static_size = true; 
    sum_of_reg_sizes += sum_of_reg_sizes + get_space_size(mask);

    mask = get_space_mask(UNSET_ALL);
    mask.fpr_static_size = true;
    sum_of_reg_sizes += sum_of_reg_sizes + get_space_size(mask);

    mask = get_space_mask(UNSET_ALL);
    mask.pr_static_size = true;
    sum_of_reg_sizes += sum_of_reg_sizes + get_space_size(mask);

    mask = get_space_mask(UNSET_ALL);
    mask.cr_static_size = true;
    sum_of_reg_sizes += sum_of_reg_sizes + get_space_size(mask);

    mask = get_space_mask(UNSET_ALL);
    mask.btr_static_size = true;
    sum_of_reg_sizes += sum_of_reg_sizes + get_space_size(mask);

    double n_sim_only = l1d_s + l1i_s + l2u_s*n_ott_c1*n_ott_c2;

    double n_comp_sim = units_size*n_ott_c1*n_ott_c2 + n_ott_c4*sum_of_reg_sizes;

    double t_sec = average_exec_time * n_sim_only + (average_compilation_time +average_exec_time)*n_comp_sim;

    return int(t_sec/60);
}



////////////////////////////////////////////////////////////////////////////
void Explorer::print_simulation(const Simulation& simulation) const
{
    cout << "\n---------------------------------------";
    cout << "\n " << simulation.area;
    cout << "\n " << simulation.energy;
    cout << "\n " << simulation.exec_time;

    cout << "\n " << (simulation.config.L1D_size);
    cout << "\n " << (simulation.config.L1D_block);
    cout << "\n " << (simulation.config.L1D_assoc);

    cout << "\n " << (simulation.config.L1I_size);
    cout << "\n " << (simulation.config.L1I_block);
    cout << "\n " << (simulation.config.L1I_assoc);

    cout << "\n " << (simulation.config.L2U_size);
    cout << "\n " << (simulation.config.L2U_block);
    cout << "\n " << (simulation.config.L2U_assoc);

    cout << "\n " << (simulation.config.integer_units);
    cout << "\n " << (simulation.config.float_units);
    cout << "\n " << (simulation.config.memory_units);
    cout << "\n " << (simulation.config.branch_units);

    cout << "\n " << (simulation.config.gpr_static_size);
    cout << "\n " << (simulation.config.fpr_static_size);
    cout << "\n " << (simulation.config.pr_static_size);
    cout << "\n " << (simulation.config.cr_static_size);
    cout << "\n " << (simulation.config.btr_static_size);
    cout << "\n---------------------------------------";
}

////////////////////////////////////////////////////////////////////////////
void Explorer::print_configuration(const Configuration& config) const
{
    cout << "\n---------------------------------------";
    cout << "\n " << (config.L1D_size);
    cout << "\n " << (config.L1D_block);
    cout << "\n " << (config.L1D_assoc);

    cout << "\n " << (config.L1I_size);
    cout << "\n " << (config.L1I_block);
    cout << "\n " << (config.L1I_assoc);

    cout << "\n " << (config.L2U_size);
    cout << "\n " << (config.L2U_block);
    cout << "\n " << (config.L2U_assoc);

    cout << "\n " << (config.integer_units);
    cout << "\n " << (config.float_units);
    cout << "\n " << (config.memory_units);
    cout << "\n " << (config.branch_units);

    cout << "\n " << (config.gpr_static_size);
    cout << "\n " << (config.fpr_static_size);
    cout << "\n " << (config.pr_static_size);
    cout << "\n " << (config.cr_static_size);
    cout << "\n " << (config.btr_static_size);
    cout << "\n---------------------------------------";
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
vector<pair<int,int> > Explorer::getParameterRanges()
{
  vector<pair<int,int> > v;

  v.push_back(pair<int,int>(processor.integer_units.get_first(),
			    processor.integer_units.get_last()));

  v.push_back(pair<int,int>(processor.float_units.get_first(),
			    processor.float_units.get_last()));

  v.push_back(pair<int,int>(processor.branch_units.get_first(),
			    processor.branch_units.get_last()));

  v.push_back(pair<int,int>(processor.memory_units.get_first(),
			    processor.memory_units.get_last()));

  v.push_back(pair<int,int>(processor.gpr_static_size.get_first(),
			    processor.gpr_static_size.get_last()));

  v.push_back(pair<int,int>(processor.fpr_static_size.get_first(),
			    processor.fpr_static_size.get_last()));

  v.push_back(pair<int,int>(processor.pr_static_size.get_first(),
			    processor.pr_static_size.get_last()));

  v.push_back(pair<int,int>(processor.cr_static_size.get_first(),
			    processor.cr_static_size.get_last()));

  v.push_back(pair<int,int>(processor.btr_static_size.get_first(),
			    processor.btr_static_size.get_last()));

  v.push_back(pair<int,int>(mem_hierarchy.L1D.size.get_first(),
			    mem_hierarchy.L1D.size.get_last()));

  v.push_back(pair<int,int>(mem_hierarchy.L1D.block_size.get_first(),
			    mem_hierarchy.L1D.block_size.get_last()));

  v.push_back(pair<int,int>(mem_hierarchy.L1D.associativity.get_first(),
			    mem_hierarchy.L1D.associativity.get_last()));

  v.push_back(pair<int,int>(mem_hierarchy.L1I.size.get_first(),
			    mem_hierarchy.L1I.size.get_last()));

  v.push_back(pair<int,int>(mem_hierarchy.L1I.block_size.get_first(),
			    mem_hierarchy.L1I.block_size.get_last()));

  v.push_back(pair<int,int>(mem_hierarchy.L1I.associativity.get_first(),
			    mem_hierarchy.L1I.associativity.get_last()));

  v.push_back(pair<int,int>(mem_hierarchy.L2U.size.get_first(),
			    mem_hierarchy.L2U.size.get_last()));

  v.push_back(pair<int,int>(mem_hierarchy.L2U.block_size.get_first(),
			    mem_hierarchy.L2U.block_size.get_last()));

  v.push_back(pair<int,int>(mem_hierarchy.L2U.associativity.get_first(),
			    mem_hierarchy.L2U.associativity.get_last()));


  return v;
}

////////////////////////////////////////////////////////////////////////////
int Explorer::get_obj_number() const
{
    return n_obj;
}

void Explorer::set_force_simulation(bool s)
{
    force_simulation = s;
}

void Explorer::set_space_name(const string& spacename)
{
    current_space = spacename;
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

	processor.set_to_default();
	trimaran_interface->save_processor_config(processor);
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

	mem_hierarchy.L1D.size.set_to_first();
	mem_hierarchy.L1D.block_size.set_to_first();
	mem_hierarchy.L1D.associativity.set_to_first();

	mem_hierarchy.L1I.size.set_to_first();
	mem_hierarchy.L1I.block_size.set_to_first();
	mem_hierarchy.L1I.associativity.set_to_first();

	mem_hierarchy.L2U.size.set_to_first();
	mem_hierarchy.L2U.block_size.set_to_first();
	mem_hierarchy.L2U.associativity.set_to_first();

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

	output_file << "\n\n [END_SPACE]";

    }

}

void Explorer::set_base_dir(const string& dir)
{
    base_dir = dir;
    trimaran_interface->set_base_dir(dir);
    estimator.set_base_dir(dir);
}

string Explorer::get_base_dir() const
{
    return base_dir;
}
