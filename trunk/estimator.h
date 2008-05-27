/***************************************************************************
                          estimator.h  -  description
                             -------------------
    begin                : sab nov 9 2002
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
#include "common.h"

using namespace std;

class Estimator {
public: 
	Estimator();
	~Estimator();

	Estimate get_estimate(const Dynamic_stats& dynamic_stats, const Mem_hierarchy&, const Processor& processor);

	void set_autoclock(bool enabled);

	// TO_BE_FIXED: needs to be public 
	double get_processor_area(const Processor& processor);
private:
	double current_clock_freq;
	double current_clock_T;
	double current_power_density_scale;

	bool autoclock;

	typedef struct 
	{
	  double static_area;
	  double dynamic_area;
	  double clock_area;
	  double memory_area;
	  double pla_area;
	} Area;

	int max_reg_size(const Processor& p);

	// area estimation models /////////////////////////////////////
	//double get_processor_area(const Processor& processor);
	double get_mem_hierarchy_area(const Mem_hierarchy& mem_hierarchy);
	double get_cache_area(const Mem_hierarchy::Cache& cache);
	double get_total_area(Processor& processor, Mem_hierarchy& mem_hierarchy);

	// energy/power estimation models ///////////////////////////
	double get_functional_unit_energy(uint64 total_cycles,
		                          uint64 total_unit_operations,
					  const Area& area,
					  int unit_instancies);


	double get_processor_energy(const Dynamic_stats& dynamic_stats, 
		                    const Processor& processor,
				    const Mem_hierarchy& mem);

	double get_cache_energy(const uint64& r_hit,
		                const uint64& r_miss,
			        const uint64& w_hit,
			        const uint64& w_miss,
			        const uint64& N_wb_req,
			        const double& addr_trans_prob,
			        const int& L_higher_block,
			        const Mem_hierarchy::Cache& cache);

	double get_main_memory_energy(const uint64& n_request);

	double get_registerfile_standbypower(int words,int size);
	double get_registerfile_dynamic_power(int words,int size);
	double get_registerfile_active_power(int words,int size);

	double get_cache_access_time(const Mem_hierarchy::Cache& cache);
	double get_registerfile_access_time(int words,int size,int read_ports,int write_ports);

#ifdef OLD_STYLE_STALLS
	// performance estimation models /////////////////////////////
	uint64 get_execution_cycles(const Dynamic_stats& dynamic_stats,const Mem_hierarchy& m);
	int get_miss_penality(const Mem_hierarchy::Cache& cache);
	double get_cycles_per_hit(const Mem_hierarchy::Cache& cache);
#endif
};

#endif
