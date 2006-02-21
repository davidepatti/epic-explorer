/***************************************************************************
                          Trimaran_interface.h  -  description
                             -------------------
    begin                : Sat Aug 17 2002
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

#ifndef TRIMARAN_INTERFACE_H
#define TRIMARAN_INTERFACE_H
#include<string>
#include <cstdlib>
#include <unistd.h>
#include <fstream>

#include "processor.h"
#include "mem_hierarchy.h"
#include "common.h"


using namespace std;

class Trimaran_interface {

public: 
    
    
    Trimaran_interface(const string& base_dir); 
    
    ~Trimaran_interface();

	void set_benchmark(string new_benchmark);
	void set_hyperblock(bool hyperblock);

	void set_base_dir(const string& dir);
	string get_base_dir() const;

	string get_benchmark_name() const;

	void compile_hmdes_file() const;
	void compile_benchmark();
	void execute_benchmark();

	void save_processor_config(const Processor&) const;
	void load_processor_config(Processor*) const;

	void save_mem_config(const Mem_hierarchy&) const;
	void load_mem_config(Mem_hierarchy*) const;

	Dynamic_stats get_dynamic_stats();

private:

	string current_benchmark; 
	string base_dir;
	bool do_hyperblock;

	bool updated_dynamic_stats;

	Dynamic_stats dynamic_stats;

	void set_environment();
	void update_dynamic_stats();
};

#endif
