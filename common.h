
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
// DAV #define BASE_DIR "PWD"
#define BASE_DIR "HOME"

#define DEFAULT_BENCH "fir_int"
#define MAIN_HMDES2 "hpl_pd_elcor_std.hmdes2"
#define EXPLORER_HMDES2 "explorer.hmdes2"
#define COMPILER_PARAM "compiler_param" //db
#define EE_TAG "\n    >> EE: "
#define N_PARAMS 27

// ---------------------------------------------------------------------------

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
int atoi(const string& s);
string get_base_dir();
double max(const double& a,const double& b);
string noyes(int x);
bool file_exists(const string& filename);

typedef unsigned long long uint64;

template<typename T> std::string to_string(const T& t){
     std::stringstream s;
     s << t;
     return s.str();
}

struct User_Settings 
{
    string benchmark;
    bool objective_area;
    bool objective_exec_time;
    bool objective_power;
    bool objective_energy;
    bool save_spaces;
    bool save_PD_STATS;
    bool save_PD_TRACE;
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
    bool multidir;
    //G
    bool multibench; //G enable multiple benchmarks
    vector<string> bench_v; //G additional benchmarks
    bool save_tcclog;
};

struct Space_mask
{
  bool L1D_block; bool L1D_size; bool L1D_assoc;
  bool L1I_block; bool L1I_size; bool L1I_assoc;
  bool L2U_block; bool L2U_size; bool L2U_assoc;

  bool num_clusters;
  bool integer_units; bool float_units; bool memory_units; bool branch_units;

  bool gpr_static_size;
  bool fpr_static_size;
  bool cr_static_size;
  bool pr_static_size;
  bool btr_static_size;

  bool tcc_region;	//db
  bool max_unroll_allowed;	//db
  bool regroup_only;	//db
  bool do_classic_opti;	//db
  bool do_prepass_scalar_scheduling;	//db
  bool do_postpass_scalar_scheduling;	//db
  bool do_modulo_scheduling;	//db
  bool memvr_profiled;		//db	
};

  enum Space_opt { STANDARD,NO_L2_CHECK };

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
enum Mask_type { SET_ALL, UNSET_ALL, SET_L1D, SET_L1I, SET_L2U, SET_PROCESSOR_UNITS, SET_PROCESSOR,SET_COMPILER }; //db

struct Rule {
	int* antecedents;
	double* consequents;
	double* degrees;
};

struct Configuration
{
  int L1D_block, L1D_size, L1D_assoc;
  int L1I_block, L1I_size, L1I_assoc;
  int L2U_block, L2U_size, L2U_assoc;
  
  int num_clusters;
  int integer_units; int float_units; int memory_units; int branch_units;
  
  int gpr_static_size;
  int fpr_static_size;
  int cr_static_size;
  int pr_static_size;
  int btr_static_size;
  

  int tcc_region;	//db
  int max_unroll_allowed;	//db
  int regroup_only;	//db
  int do_classic_opti;	//db
  int do_prepass_scalar_scheduling;	//db
  int do_postpass_scalar_scheduling;	//db
  int do_modulo_scheduling;	//db
  int memvr_profiled;		//db		

  bool is_feasible(); // mau
  void invalidate();
  bool check_difference(const Configuration&,Space_mask);
  string get_header() const;
  string get_executable_dir() const;
  string get_mem_dir() const;
};

struct Simulation 
{
  double energy,area,exec_time;
  double clock_freq;
  Configuration config;
  bool simulated;
  //G
  void add_simulation(const Simulation&);
  private:
  vector<double> energy_v, area_v, exec_time_v;
};

struct Dynamic_stats
{
    uint64 total_cycles, stall_cycles,compute_cycles,total_dynamic_operations;
    uint64 ialu,falu,load,store,branch,cmp,pbr,icm;
    uint64 spills_restores;

    double average_issued_ops_compute_cycles; 
    double average_issued_ops_total_cycles;

    // memory stats
    uint64 L1D_w_fetches, L1D_r_fetches, L1D_miss, L1D_hit;
    uint64 L1D_r_hit, L1D_r_miss, L1D_w_hit, L1D_w_miss; 
    uint64 L1D_writebacks;

    uint64 L1I_fetches,L1I_hit, L1I_miss;
    uint64 L1I_writebacks;

    uint64 L2U_demand, L2U_miss, L2U_hit;
    uint64 L2U_writebacks;

    double L1D_transition_p, L1I_transition_p,L2U_transition_p;

    uint64 SDRAM_accesses;

};
// ---------------------------------------------------------------------------
typedef struct
{
    // performance 
    double L1D_access_time, L1I_access_time;
    double clock_freq, clock_T, power_density_scale;

    uint64 execution_cycles, compute_cycles, stall_cycles;

    double execution_time;
    double IPC;
    double L1D_transition_p, L1I_transition_p;

    // power/energy

    double L1I_energy, L1D_energy, L2U_energy, main_memory_energy;
    double total_cache_energy, total_processor_energy;
    double NO_MEM_system_energy, total_system_energy, total_average_power; 
    // area
    double L1D_area, L1I_area, L2U_area, processor_area;
    double total_area;

} Estimate;
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
	    t1.config.num_clusters == t2.config.num_clusters &&
	    t1.config.float_units == t2.config.float_units && 
	    t1.config.memory_units == t2.config.memory_units &&
	    t1.config.branch_units == t2.config.branch_units &&
	    t1.config.gpr_static_size == t2.config.gpr_static_size &&
	    t1.config.fpr_static_size == t2.config.fpr_static_size &&
	    t1.config.cr_static_size == t2.config.cr_static_size &&
	    t1.config.pr_static_size == t2.config.pr_static_size &&
	    t1.config.btr_static_size == t2.config.btr_static_size &&
	    t1.config.tcc_region == t2.config.tcc_region &&	//db
	    t1.config.max_unroll_allowed == t2.config.max_unroll_allowed &&	//db
	    t1.config.regroup_only == t2.config.regroup_only &&		//db
	    t1.config.do_classic_opti == t2.config.do_classic_opti &&	//db
	    t1.config.do_prepass_scalar_scheduling == t2.config.do_prepass_scalar_scheduling && 	//db
	    t1.config.do_postpass_scalar_scheduling == t2.config.do_postpass_scalar_scheduling &&		//db
	    t1.config.do_modulo_scheduling == t2.config.do_modulo_scheduling &&		//db
	    t1.config.memvr_profiled == t2.config.memvr_profiled);	//db
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
	      t.config.num_clusters +
	      t.config.float_units +
	      t.config.memory_units +
	      t.config.branch_units +
	      t.config.gpr_static_size +
	      t.config.fpr_static_size +
	      t.config.cr_static_size +
	      t.config.pr_static_size +
	      t.config.btr_static_size +
	      t.config.tcc_region +	//db
	      t.config.max_unroll_allowed +	//db
	      t.config.regroup_only +		//db
	      t.config.do_classic_opti +	//db
	      t.config.do_prepass_scalar_scheduling + 	//db
	      t.config.do_postpass_scalar_scheduling +		//db
	      t.config.do_modulo_scheduling +		//db
	      t.config.memvr_profiled ) % vhash.size() ); //db
  }
};

// ---------------------------------------------------------------------------
// wrapper for the communication between Explorer and ga evaluation function
struct ExportUserData //G
{
//G  class Explorer     *explorer;
  HashGA             *ht_ga;
//G  HashGA	     *ht_hy;
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
