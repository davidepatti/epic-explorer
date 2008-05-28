#include "explorer.h"
#include "common.h"
#include <ctime>
//////////////////////////////////////////////////////////////////////
//  Dependency Algorithm - DEP
/// Implementation of system level exploration using clusters .
//
// List of clusters:
//
//  c1: L1 data ( size, block_size, associativity)
//  c2: L1 instruction ( size, block_size, associativity)
//  c3: L2 unified cache ( size, block_size, associativity)
//  c4: Functional units ( IU,FPU,BU,MU )
//  c5: GPR, c6: FPR, c7: PR, c8: CR, c9: BTR

//********************************************************************
void Explorer::start_DEP()
{
    current_algo = "DEP";
    reset_sim_counter();
    Exploration_stats stats;

    stats.space_size = get_space_size();

    stats.start_time = time(NULL);

    char mess[55];
    int t;

    string filename = Options.benchmark+"_DEP_"+current_space;

    // explore c1 and c2, separately .
    // all other parameters are set to arbitrary values because c1 and
    // c2 pareto optimal values are assumed indipendent from them
    //
    //////////////////////////////////////////////////////////////////////


    write_log("  -> Starting DEP simulation ");

    sprintf(mess,"Space size: %lf , feasible %lf",stats.space_size,stats.feasible_size);
    write_log(mess);

    write_log("  -> Starting c1 exploration ");

    Space_mask c1_mask = get_space_mask(SET_L1D);

    vector<Configuration> c1_space = build_space(c1_mask);
    
    t = (c1_space.size() * average_exec_time)/60;
    sprintf(mess," c1: %d explorations, estimated time %d minutes ",c1_space.size(),t);
    write_log(mess);

    vector<Simulation> c1_sims = simulate_space(c1_space);
    vector<Simulation> c1_sims_pareto = get_pareto(c1_sims);

    vector<Configuration> c1_space_pareto = extract_space(c1_sims_pareto);
    int n_ott_c1 = c1_space_pareto.size();

    save_simulations(c1_sims,filename+"_c1.exp");
    save_simulations(c1_sims_pareto,filename+"_c1.pareto.exp");

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

    save_simulations(c2_sims,filename+"_c2.exp");
    save_simulations(c2_sims_pareto,filename+"_c2.pareto.exp");

    ////////////////////////////////////////////////////
    // a first estimation of exploration time can be done here
    // note we are assuming a arbitrary val for n pareto c4
    //
    // Error is proportional to error in n_ott_c4_assumed

    int est_n_ott_c4 = (n_ott_c1 + n_ott_c2);
    int min_estimated_time = DEP_phase1_time(n_ott_c1,n_ott_c2,2);
    int max_estimated_time = DEP_phase1_time(n_ott_c1,n_ott_c2,est_n_ott_c4);

    sprintf(mess," Approx exploration time: (%d - %d) minutes ",min_estimated_time,max_estimated_time);

    write_log(mess);

    // now c3 and c4 must be explored, for each combination of c1 and
    // c2 pareto-optimal configs. All other parameters are set to
    // arbitrary values.
    // Order is not important, but c4 exploration is performed for
    // first in order to obtain n_ott_c4 to estimate phase 1
    // algorithm time as soon as possible
    //////////////////////////////////////////////////////////////////

    write_log("Starting c4 exploration ");

    // first of all, we cross merge c1 and c2

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

    save_simulations(c4_sims,filename+"_c4.exp");
    save_simulations(c4_sims_pareto,filename+"_c4.pareto.exp");

    vector<Configuration> c4_space_pareto = extract_space(c4_sims_pareto);
    int n_ott_c4 = c4_space_pareto.size();

    /////////////////////////////////////////////////////////////////
    // Now it's possible to computer the total time necessary to
    // perform the first phase of algorithm, because we have n_ott_c1
    //, n_ott_c2 and n_ott_c4

    int estimated_time = DEP_phase1_time(n_ott_c1,n_ott_c2,n_ott_c4);
    sprintf(mess," Estimated Phase 1 time: %d minutes ...",estimated_time);

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

    save_simulations(c3_sims,filename+"_c3.exp");
    save_simulations(c3_sims_pareto,filename+"_c3.pareto.exp");

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
    save_simulations(c5_sims,filename+"_c5.exp");
    save_simulations(c5_sims_pareto,filename+"_c5.pareto.exp");

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c6 exploration ");

    t = (c6_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c6_space.size(),t);
    write_log(mess);

    vector<Simulation> c6_sims = simulate_space(c6_space);
    vector<Simulation> c6_sims_pareto = get_pareto(c6_sims);
    save_simulations(c6_sims,filename+"_c6.exp");
    save_simulations(c6_sims_pareto,filename+"_c6.pareto.exp");

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c7 exploration ");

    t = (c7_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c7_space.size(),t);
    write_log(mess);

    vector<Simulation> c7_sims = simulate_space(c7_space);
    vector<Simulation> c7_sims_pareto = get_pareto(c7_sims);
    save_simulations(c7_sims,filename+"_c7.exp");
    save_simulations(c7_sims_pareto,filename+"_c7.pareto.exp");

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c8 exploration ");

    t = (c8_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c8_space.size(),t);
    write_log(mess);

    vector<Simulation> c8_sims = simulate_space(c8_space);
    vector<Simulation> c8_sims_pareto = get_pareto(c8_sims);
    save_simulations(c8_sims,filename+"_c8.exp");
    save_simulations(c8_sims_pareto,filename+"_c8.pareto.exp");

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c9 exploration ");

    t = (c9_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c9_space.size(),t);
    write_log(mess);

    vector<Simulation> c9_sims = simulate_space(c9_space);
    vector<Simulation> c9_sims_pareto = get_pareto(c9_sims);
    save_simulations(c9_sims,filename+"_c9.exp");
    save_simulations(c9_sims_pareto,filename+"_c9.pareto.exp");

    // Phase 1 of alg is finished, we have the pareto optimal
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

    save_simulations(all_sims,filename+"_phase1.pareto.exp");

    write_log("DEP simulation Phase1 ended. ");

    /////////////////////////////////////////////////////////////////////////////
    /// PHASE 2 - merging all clusters, couple by couple . 
    //
    // c1toN_space:
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

    save_simulations(c1to2_sims,filename+"_c1to2.exp");
    save_simulations(pareto_set,filename+"_c1to2.pareto.exp");
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

    save_simulations(c1to3_sims,filename+"_c1to3.exp");
    save_simulations(pareto_set,filename+"_c1to3.pareto.exp");
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

    save_simulations(c1to4_sims,filename+"_c1to4.exp");
    save_simulations(pareto_set,filename+"_c1to4.pareto.exp");

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

    save_simulations(c1to5_sims,filename+"_c1to5.exp");
    save_simulations(pareto_set,filename+"_c1to5.pareto.exp");
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

    save_simulations(c1to6_sims,filename+"_c1to6.exp");
    save_simulations(pareto_set,filename+"_c1to6.pareto.exp");

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

    save_simulations(c1to7_sims,filename+"_c1to7.exp");
    save_simulations(pareto_set,filename+"_c1to7.pareto.exp");

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

    save_simulations(c1to8_sims,filename+"_c1to8.exp");
    save_simulations(pareto_set,filename+"_c1to8.pareto.exp");

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

    save_simulations(c1to9_sims,filename+"_c1to9.exp");
    save_simulations(pareto_set,filename+"_pareto.exp");

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,filename+".stat");

    write_log("End of Phase 2 ");
    write_log("End of DEP simulation ");
}

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

    sprintf(mess,"Space size: %lf , feasible %lf ",stats.space_size,stats.feasible_size);
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

    save_simulations(c3_sims,filename+"_c3.exp");
    save_simulations(c3_sims_pareto,filename+"_c3.pareto.exp");

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

    save_simulations(c1to2_sims,filename+"_c1to2.exp");
    save_simulations(pareto_set,filename+"_c1to2.pareto.exp");

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

    save_simulations(c1to3_sims,filename+"_c1to3.exp");
    save_simulations(pareto_set,filename+"_c1to3.pareto.exp");

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

    save_simulations(c1to5_sims,filename+"_c1to5.exp");
    save_simulations(pareto_set,filename+"_c1to5.pareto.exp");

    save_stats(stats,filename+".stat");
    save_simulations(pareto_set,filename+".pareto.exp");

}



// ********************************************************************
// simplified DEP: do not consider monodirectional dependencies
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

    
    // explore c1 and c2, separately .
    // all other parameters are set to arbitrary values because c1 and
    // c2 pareto optimal values are assumed indipendent from them
    //
    //////////////////////////////////////////////////////////////////////
    write_log("  -> Starting c1 exploration ");

    Space_mask c1_mask = get_space_mask(SET_L1D);

    vector<Configuration> c1_space = build_space(c1_mask);
    
    t = (c1_space.size() * average_exec_time)/60;
    sprintf(mess," c1: %d explorations, estimated time %d minutes ",c1_space.size(),t);
    write_log(mess);

    vector<Simulation> c1_sims = simulate_space(c1_space);
    vector<Simulation> c1_sims_pareto = get_pareto(c1_sims);

    vector<Configuration> c1_space_pareto = extract_space(c1_sims_pareto);
    int n_ott_c1 = c1_space_pareto.size();

    save_simulations(c1_sims,filename+"_c1.exp");
    save_simulations(c1_sims_pareto,filename+"_c1.pareto.exp");

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

    save_simulations(c2_sims,filename+"_c2.exp");
    save_simulations(c2_sims_pareto,filename+"_c2.pareto.exp");

    write_log("Starting c4 exploration ");

    // first of all, we cross merge c1 and c2

    Space_mask c4_mask = get_space_mask(SET_PROCESSOR_UNITS);

    vector<Configuration> c4_space = build_space(c4_mask);

    t = (c4_space.size() * (average_exec_time+average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c4_space.size(),t);
    write_log(mess);

    vector<Simulation> c4_sims = simulate_space(c4_space);
    vector<Simulation> c4_sims_pareto = get_pareto(c4_sims);

    save_simulations(c4_sims,filename+"_c4.exp");
    save_simulations(c4_sims_pareto,filename+"_c4.pareto.exp");

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

    save_simulations(c3_sims,filename+"_c3.exp");
    save_simulations(c3_sims_pareto,filename+"_c3.pareto.exp");

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
    save_simulations(c5_sims,filename+"_c5.exp");
    save_simulations(c5_sims_pareto,filename+"_c5.pareto.exp");

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c6 exploration ");

    t = (c6_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c6_space.size(),t);
    write_log(mess);

    vector<Simulation> c6_sims = simulate_space(c6_space);
    vector<Simulation> c6_sims_pareto = get_pareto(c6_sims);
    save_simulations(c6_sims,filename+"_c6.exp");
    save_simulations(c6_sims_pareto,filename+"_c6.pareto.exp");

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c7 exploration ");

    t = (c7_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c7_space.size(),t);
    write_log(mess);

    vector<Simulation> c7_sims = simulate_space(c7_space);
    vector<Simulation> c7_sims_pareto = get_pareto(c7_sims);
    save_simulations(c7_sims,filename+"_c7.exp");
    save_simulations(c7_sims_pareto,filename+"_c7.pareto.exp");

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c8 exploration ");

    t = (c8_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c8_space.size(),t);
    write_log(mess);

    vector<Simulation> c8_sims = simulate_space(c8_space);
    vector<Simulation> c8_sims_pareto = get_pareto(c8_sims);
    save_simulations(c8_sims,filename+"_c8.exp");
    save_simulations(c8_sims_pareto,filename+"_c8.pareto.exp");

    ///////////////////////////////////////////////////////////////
    write_log("  -> Started c9 exploration ");

    t = (c9_space.size() * (average_exec_time + average_compilation_time))/60;
    sprintf(mess," (%d explorations, estimated time %d minutes) ",c9_space.size(),t);
    write_log(mess);

    vector<Simulation> c9_sims = simulate_space(c9_space);
    vector<Simulation> c9_sims_pareto = get_pareto(c9_sims);
    save_simulations(c9_sims,filename+"_c9.exp");
    save_simulations(c9_sims_pareto,filename+"_c9.pareto.exp");

    // Phase 1 of alg is finished, we have the pareto optimal
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

    save_simulations(all_sims,filename+"_GLOBAL_no_merge.pareto.exp");

    write_log("DEPMOD simulation Phase1 ended. ");

    /////////////////////////////////////////////////////////////////////////////
    /// PHASE 2 - merging all clusters, couple by couple . 
    //
    // c1toN_space:
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

    save_simulations(c1to2_sims,filename+"_c1to2.exp");
    save_simulations(pareto_set,filename+"_c1to2.pareto.exp");
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

    save_simulations(c1to3_sims,filename+"_c1to3.exp");
    save_simulations(pareto_set,filename+"_c1to3.pareto.exp");
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

    save_simulations(c1to4_sims,filename+"_c1to4.exp");
    save_simulations(pareto_set,filename+"_c1to4.pareto.exp");

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

    save_simulations(c1to5_sims,filename+"_c1to5.exp");
    save_simulations(pareto_set,filename+"_c1to5.pareto.exp");
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

    save_simulations(c1to6_sims,filename+"_c1to6.exp");
    save_simulations(pareto_set,filename+"_c1to6.pareto.exp");

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

    save_simulations(c1to7_sims,filename+"_c1to7.exp");
    save_simulations(pareto_set,filename+"_c1to7.pareto.exp");

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

    save_simulations(c1to8_sims,filename+"_c1to8.exp");
    save_simulations(pareto_set,filename+"_c1to8.pareto.exp");

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

    save_simulations(c1to9_sims,filename+"_c1to9.exp");
    save_simulations(pareto_set,filename+"_pareto.exp");

    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats,filename+".stat");

    write_log("End of Phase 2 ");
    write_log("End of DEPMOD simulation ");
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
// TO CHECK
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
