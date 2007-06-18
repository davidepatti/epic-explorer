
/***********************************************************************
 * common.h 
 ***********************************************************************
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "hash.h"

// IMPORTANT:
// you should set this to "DIR" if you install trimaran and
// trimaran-workspace directories to the content of environment
// variable $DIR
// Default is $HOME directory
#define BASE_DIR "HOME"

#define DEFAULT_BENCH "wave"
#define MAIN_HMDES2 "hpl_pd_elcor_std.hmdes2"
#define EXPLORER_HMDES2 "explorer.hmdes2"

// ---------------------------------------------------------------------------
#define SHOW_L1D 1
#define SHOW_L1I 2
#define SHOW_L2U 3
#define SHOW_REGISTER_FILES 4
#define SHOW_UNITS 5
#define SHOW_ALL 6
#define SHOW_NONE 7
#define SHOW_ENERGYDELAY 8

// ---------------------------------------------------------------------------

#define REAL double
#ifndef NULL
#define NULL 0
#endif
#define TRIANGLE_SET 1
#define GAUSSIAN_SET 2

using namespace std;

void wait_key();
void go_until(const string& dest,ifstream& ifs);
string skip_to(ifstream& ifs,int n);
string skip_to(ifstream& ifs,const string& target);
int count_word(const string& w,ifstream& ifs);
long long atoll(const string& s);
double atof(const string& s);
string get_base_dir();
double max(const double& a,const double& b);
bool file_exists(const string& filename);

typedef unsigned long long uint64;

struct User_Settings 
{
    string benchmark;
    bool hyperblock;
    bool objective_area;
    bool objective_exec_time;
    bool objective_power;
    bool objective_energy;
    bool save_spaces;
    bool save_PD_STATS;
    bool save_estimation;
    bool auto_clock;
    bool save_objectives_details;
    string default_settings_file;
    struct 
    {
	int enabled;
	float threshold;
	int min,max;
    } approx_settings;
};

struct Space_mask
{
  bool L1D_block; bool L1D_size; bool L1D_assoc;
  bool L1I_block; bool L1I_size; bool L1I_assoc;
  bool L2U_block; bool L2U_size; bool L2U_assoc;

  bool integer_units; bool float_units; bool memory_units; bool branch_units;

  bool gpr_static_size;
  bool fpr_static_size;
  bool cr_static_size;
  bool pr_static_size;
  bool btr_static_size;
};

  enum Space_opt { STANDARD,NO_L2_SIZE_CHECK };

// ---------------------------------------------------------------------------
//  GA parameters used in exploration
struct GA_parameters
{
    int population_size;
    float pcrossover;
    float pmutation;
    int max_generations;
    int report_pareto_step;
    unsigned long random_seed;
};

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// predefined mask types 
enum Mask_type { SET_ALL, UNSET_ALL, SET_L1D, SET_L1I, SET_L2U, SET_PROCESSOR_UNITS, SET_PROCESSOR };

struct Rule {
	int* antecedents;
	REAL* consequents;
	REAL* degrees;
};

struct Configuration
{
  int L1D_block, L1D_size, L1D_assoc;
  int L1I_block, L1I_size, L1I_assoc;
  int L2U_block, L2U_size, L2U_assoc;
  
  int integer_units; int float_units; int memory_units; int branch_units;
  
  int gpr_static_size;
  int fpr_static_size;
  int cr_static_size;
  int pr_static_size;
  int btr_static_size;
  
  bool is_feasible(); // mau
  void invalidate();
  bool check_difference(const Configuration&,Space_mask);
  string to_string() const;
  string get_processor_string() const;
  string get_mem_hierarchy_string() const;
};

struct Simulation 
{
  double energy,area,exec_time;
  double clock_freq;
  Configuration config;
  bool simulated;
};

struct Dynamic_stats
{
    uint64 total_cycles, stall_cycles,compute_cycles,total_dynamic_operations;
    uint64 ialu,falu,load,store,branch,cmp,pbr;
    uint64 spills_restores;

    double average_issued_ops_compute_cycles; 
    double average_issued_ops_total_cycles;

    // cache stats
    uint64 L1D_w_fetches, L1D_r_fetches, L1D_miss, L1D_hit;
    uint64 L1D_r_hit, L1D_r_miss, L1D_w_hit, L1D_w_miss; 
    uint64 L1D_capacity_misses, L1D_conflict_misses;

    uint64 L1I_fetches,L1I_hit, L1I_miss;
    uint64 L1I_capacity_misses, L1I_conflict_misses;

    uint64 L2U_miss, L2U_hit;
    uint64 L2U_demand, L2U_i_demand, L2U_d_demand,L2U_dw_demand, L2U_dr_demand;
    uint64 L2U_i_miss, L2U_i_hit;
    uint64 L2U_dr_hit, L2U_dw_hit, L2U_d_miss, L2U_dr_miss, L2U_dw_miss;
    uint64 L2U_r_hit, L2U_r_miss, L2U_w_hit, L2U_w_miss;
    uint64 L2U_capacity_misses, L2U_conflict_misses;

};
// ---------------------------------------------------------------------------
struct Exploration_stats 
{
  double space_size;
  double feasible_size;
  int n_sim;
  int recompilations;
  time_t start_time;
  time_t end_time;
};

// ---------------------------------------------------------------------------
// simple cache to store already simulated configurations
class HashGA : public Hash<Simulation> // mau
{
public:
  HashGA(int _size) : Hash<Simulation>(_size) {}

  virtual bool equal(Simulation &t1, Simulation &t2) {
    return (t1.config.L1D_block == t2.config.L1D_block &&
	    t1.config.L1D_size == t2.config.L1D_size &&
	    t1.config.L1D_assoc == t2.config.L1D_assoc &&
	    t1.config.L1I_block == t2.config.L1I_block &&
	    t1.config.L1I_size == t2.config.L1I_size &&
	    t1.config.L1I_assoc == t2.config.L1I_assoc &&
	    t1.config.L2U_block == t2.config.L2U_block &&
	    t1.config.L2U_size == t2.config.L2U_size &&
	    t1.config.L2U_assoc == t2.config.L2U_assoc &&
	    t1.config.integer_units == t2.config.integer_units &&
	    t1.config.float_units == t2.config.float_units && 
	    t1.config.memory_units == t2.config.memory_units &&
	    t1.config.branch_units == t2.config.branch_units &&
	    t1.config.gpr_static_size == t2.config.gpr_static_size &&
	    t1.config.fpr_static_size == t2.config.fpr_static_size &&
	    t1.config.cr_static_size == t2.config.cr_static_size &&
	    t1.config.pr_static_size == t2.config.pr_static_size &&
	    t1.config.btr_static_size == t2.config.btr_static_size);
  }

  virtual unsigned int T2Index(Simulation& t) {
    return ( (t.config.L1D_block +
	      t.config.L1D_size +
	      t.config.L1D_assoc +
	      t.config.L1I_block +
	      t.config.L1I_size +
	      t.config.L1I_assoc +
	      t.config.L2U_block +
	      t.config.L2U_size +
	      t.config.L2U_assoc +
	      t.config.integer_units +
	      t.config.float_units +
	      t.config.memory_units +
	      t.config.branch_units +
	      t.config.gpr_static_size +
	      t.config.fpr_static_size +
	      t.config.cr_static_size +
	      t.config.pr_static_size +
	      t.config.btr_static_size) % vhash.size() );
  }
};

// ---------------------------------------------------------------------------
// wrapper for the communication between Explorer and ga evaluation function
struct ExportUserData // mau
{
  class Explorer     *explorer;
  HashGA             *ht_ga;
  HashGA	     *ht_hy;
  vector<Simulation> history;
  vector<Simulation> pareto;
};
/***********************************************************************
 * debug routines
 * Maurizio Palesi [mpalesi@diit.unict.it]
 ***********************************************************************
 */
#define message(msg) cout << "MSG: " << msg << endl;
#define fatal(msg) { cerr << "FATAL: " << msg << endl; exit(-1); }
#define warning(msg) cerr << "WARNING: " << msg << endl;

#ifdef DEBUG
#define debug(msg) cout << "DEBUG: " << msg << endl;
#define tracemark cout << "---> " << __FILE__ << ":" << __LINE__ << endl;
#else
#define debug(msg)
#define tracemark 
#endif


#endif
