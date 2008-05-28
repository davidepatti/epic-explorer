/***************************************************************************
                          explorer.h  -  description
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

#ifndef EXPLORER_H
#define EXPLORER_H

#include "version.h"
#include "stdio.h"
#include "stdlib.h"
#include <vector>
#include <string>
#include "processor.h"
#include "trimaran_interface.h"
#include "mem_hierarchy.h"
#include "estimator.h"
#include "parameter.h"
#include "hash.h" // mau
#include "FuzzyApprox.h" // ale
//#include "FannApprox.h" // ale
#include "containers.h" //G
#include "compiler.h"	//db

// ---------------------------------------------------------------------------
#define EXPLORER_NOTHING_DONE 0
#define EXPLORER_BINARY_DONE 1
#define EXPLORER_ALL_DONE 2
#define EXPLORER_RETRY_TIME 1
// ---------------------------------------------------------------------------

class Explorer {
public: 
  Processor processor;
  Mem_hierarchy mem_hierarchy;
  Compiler compiler;	//db
  Estimator estimator;

  Explorer(Trimaran_interface * ti);
  ~Explorer();

  // Functions to modify explorer options
  void set_options(const struct User_Settings& user_settings);
  void set_base_dir(const string& dir);
  string get_base_dir() const;

  // Exploration algorithms
	
  void start_DEP();
  void start_DEP2();
  void start_DEPMOD();
  void start_SAP();
  void start_SAPMOD();
  void start_PBSA();
  void start_RAND(int n);
  void start_GA(const GA_parameters& parameters); 
  void start_EXHA();
  void start_REP(const string&); //G
  void test(); // for testing only

  // Main function for simulating a parameter space 
  vector<Simulation> simulate_space(const vector<Configuration>& space);
  int simulate_space();

  // Functions for building configuration spaces needed by
  // simultate_space(vector<Configuration>& space)
	

  vector<Configuration> extract_space(const vector<Simulation>& sims) const;

  vector<Configuration> build_space(const Space_mask& mask);
  vector<Configuration> build_space(const Space_mask& mask,enum Space_opt opt);
  vector<Configuration> build_space(const Space_mask& mask,Configuration base_config);
  vector<Configuration> build_space(const Space_mask& mask,Configuration base_config, Space_opt opt);
  vector<Configuration> build_space_cross_merge(const vector<Configuration>& s1,
						const vector<Configuration>& s2,
						const Space_mask& mask1,
						const Space_mask& mask2) const;
  bool equivalent_spaces(const vector<Configuration>& s1,const vector<Configuration>& s2) const;

  long double get_space_size() const;
  long double get_space_size(const Space_mask& mask) const;
  int count_needed_recompilations(const vector<Configuration>& space);


  // get one of the commonly used masks used by
  // build_space_exhaustive(Space_mask mask)
  // In alternative any user-defined mask can be used by setting
  // single mask boolean values
	
  Space_mask get_space_mask(Mask_type type) const;
  Space_mask mask_union(Space_mask& m1,Space_mask& m2) const;
  Space_mask create_space_mask(const vector<bool>& boolean_mask);
  Space_mask negate_mask(Space_mask mask);

  vector<bool> get_boolean_mask(const Space_mask& mask);

  Configuration create_configuration() const; // default values
  Configuration create_configuration(const Space_mask& mask,const Configuration& base) const;
  Configuration create_configuration(const Processor& p,const Mem_hierarchy& mem, const Compiler& comp);

  bool configuration_present(const Configuration& conf,const vector<Configuration>& space) const;
  int simulation_present(const Simulation& sim,const vector<Simulation>& simulations) const;

  //*********************************************************//
  // function to manipulate Simulations

  vector<Simulation> get_pareto(const vector<Simulation>& simulations);
  vector<Simulation> get_pareto3d(const vector<Simulation>& simulations);
  vector<Simulation> get_pareto_CyclesPower(const vector<Simulation>& simulations);
  vector<Simulation> get_pareto_AreaCycles(const vector<Simulation>& simulations);
  vector<Simulation> get_pareto_AreaPower(const vector<Simulation>& simulations);

  void remove_dominated_simulations(vector<Simulation>& sims);
  vector<Simulation> normalize(const vector<Simulation>& sims);

  vector<Simulation> sort_by_exec_time(vector<Simulation> sims);
  vector<Simulation> sort_by_energy(vector<Simulation> sims);
  vector<Simulation> sort_by_area(vector<Simulation> sims);
  vector<Simulation> sort_by_energydelay_product(vector<Simulation> sims);

  double get_sensivity_energydelay(const vector<Simulation>& sim);
  double get_sensivity_PBSA(const vector<Simulation>& sim,const vector<Simulation>& all_sims);
  double distance(const Simulation& s1,const Simulation& s2);

  void append_simulations(vector<Simulation>& dest,const vector<Simulation>& new_sims);
  void save_simulations(const vector<Simulation>& simulations,const string& filename);
  void save_configurations(const vector<Configuration>& space,const string& filename);
  void save_stats(const Exploration_stats& stats,const string& filename);
  void save_estimation_file(const Dynamic_stats& ,const Estimate& ,Processor& ,Mem_hierarchy&, Compiler& comp, string& filename) const;
  void save_objectives_details(const Dynamic_stats& dyn,const Configuration& conf, const string filename ) const;

  int get_sim_counter() const;
  void reset_sim_counter();
    
  void write_log(string mess);

  CFunctionApproximation *function_approx;

  void set_fuzzy(bool);
  void init_approximation();
  void set_force_simulation(bool);
  void set_space_name(const string& space_name);
  void load_space_file(const string& space_name);
  void save_space_file(const string& space_name);

  vector<pair<int,int> > getParameterRanges();
  vector<pair<int,int> > getParametersNumber();
  int n_objectives() {return n_obj;}

private:

  int get_explorer_status() const;
  void prepare_explorer(const string& application,const Configuration& config);
  vector<Simulation> simulate_loop(const vector<Configuration>& space);


  // private methods 
  void init_GA(); //G
  //---------FuzzyApprox
  void SimulateBestWorst();
  //--------------------
  void GA_evaluate(population* pop); //G
  Configuration ind2conf(const individual& ind); //G
  // private class variables
  Trimaran_interface  * trimaran_interface;

  Estimate estimate;
  Dynamic_stats dyn_stats;
  Configuration current_config;

  bool force_simulation;

  string base_dir;
  string epic_dir;
  string application_dir;
  string processor_dir;
  string mem_hierarchy_dir;
  string cache_dir_name;
  string machine_dir;
  string m5_dir;
  string mem_hierarchy_filename;
  string hmdes_filename;
  string comp_filename;	//db
  string bench_executable;
  string current_algo;
  string current_space;
  string pd_stats_file;

  string REP_source_file;
  int n_obj;
  vector<string> benchmarks; //G
  struct ExportUserData eud; //G
  struct User_Settings Options;

  int average_compilation_time, average_exec_time;
  int sim_counter;
  vector<Simulation> previous_simulations;
  string previous_benchmark;

  int DEP_phase1_time(int n_ott_c1,int n_ott_c2,int n_ott_c4);
  int DEP_phase2_time(float P,int * n_ott);

};

// Functions for GA-fuzzy
bool isDominated(Simulation sim, const vector<Simulation>& simulations);
// Evaluation functions for GA-based exploration
// void GA_Evaluator(IND& ind, ObjectiveVector& scores, void *user_data); 
// bool GA_Evaluation(IND& ind, void *user_data, double& exec_time, double& power, double& area); 
#endif
