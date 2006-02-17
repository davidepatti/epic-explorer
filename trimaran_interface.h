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
#include "common.h"


using namespace std;

class Trimaran_interface {

public: 
    
    
    Trimaran_interface(); 
    
    ~Trimaran_interface();

	void set_benchmark(string& new_benchmark);
	void set_hyperblock(bool hyperblock);

	string get_benchmark_name() const;

	void compile_hmdes_file() const;
	void compile_benchmark();
	void execute_benchmark();

	Dynamic_stats get_dynamic_stats();

private:

	struct Settings 
	{ 
	    string benchmark; 
	    bool hyperblock;
	} settings ;

	bool updated_dynamic_stats;

	Dynamic_stats dynamic_stats;

	void set_environment();
	void update_dynamic_stats();
};

#endif
