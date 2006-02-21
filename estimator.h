/***************************************************************************
                          estimator.h  -  description
                             -------------------
    begin                : sab nov 9 2002
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


#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <math.h>
#include "stdio.h"
#include "assert.h"

#include "power_densities.h"
#include "cacti_area_interface.h"
#include "processor.h"
#include "mem_hierarchy.h"
#include "trimaran_interface.h"


#define	IDEAL_MEM 0
#define REAL_MEM 1

typedef struct
{
    // performance 
    double L1D_access_time, L1I_access_time, max_access_time;
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


using namespace std;

class Estimator {
public: 
	Estimator();
	~Estimator();

	Estimate get_estimate(const Dynamic_stats& dynamic_stats, const Mem_hierarchy&, const Processor& processor );
	void set_autoclock(bool enabled);

	void set_base_dir(const string& dir);
	
private:
	double CLOCK_FREQ;
	double CLOCK_T;
	double POWER_DENSITY_SCALE;

	bool autoclock;
	string base_dir;

	typedef struct 
	{
	  double static_area;
	  double dynamic_area;
	  double clock_area;
	  double memory_area;
	  double pla_area;
	} Area;


	string cache_config_file;
	string tmp_transitions_file;

	int max_reg_size(const Processor& p);

	// area estimation models /////////////////////////////////////
	double get_processor_area(const Processor& processor);
	double get_mem_hierarchy_area(const Mem_hierarchy& mem_hierarchy);
	double get_cache_area(const Mem_hierarchy::Cache& cache);
	double get_total_area(Processor& processor, Mem_hierarchy& mem_hierarchy);
	void compute_units_areas();

	// performance estimation models /////////////////////////////

	void update_clock_frequency(const double & min_clock_period);
	double get_current_clock() const;

	uint64 get_execution_cycles(const Dynamic_stats& dynamic_stats,const Mem_hierarchy& m);
	int get_miss_penality(const Mem_hierarchy::Cache& cache);
	double get_cycles_per_hit(const Mem_hierarchy::Cache& cache);
	double get_access_time(const Mem_hierarchy::Cache& cache);

	// energy/power estimation models ///////////////////////////
	double get_functional_unit_energy(uint64 total_cycles,
		                          uint64 total_unit_operations,
					  const Area& area,
					  int unit_instancies);


	double get_processor_energy(const Dynamic_stats& dynamic_stats, 
		                    const Processor& processor,
				    const Mem_hierarchy& mem,
				    const int& flag);

	double get_cache_energy(uint64& r_hit,
		                uint64& r_miss,
			        uint64& w_hit,
			        uint64& w_miss,
			        uint64& N_wb_req,
				    double& addr_trans_prob,
			        int& L_higher_block,
			        const Mem_hierarchy::Cache& cache);

	double get_main_memory_energy(uint64& n_block_request,int& block_size);

	double get_registerfile_standbypower(int words,int size);
	double get_registerfile_dynamic_power(int words,int size);
	double get_registerfile_active_power(int words,int size);
};

#endif
