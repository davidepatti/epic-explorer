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

	void set_environment(const string& path);

	string get_benchmark_name() const;

	void compile_hmdes_file(const string& path) const;
	void compile_benchmark(const string& path);
	void execute_benchmark(const string& path,const string& cache_dir);

	void save_processor_config(const Processor&,const string& path) const;
	void load_processor_config(Processor*,const string& filename) const;

	void save_mem_config(const Mem_hierarchy&,const string& path) const;
	void load_mem_config(Mem_hierarchy*,const string& filename) const;

	Dynamic_stats get_dynamic_stats(const string& filename);

private:
	string current_benchmark; 
	string base_path;
	bool do_hyperblock;
};

#endif
