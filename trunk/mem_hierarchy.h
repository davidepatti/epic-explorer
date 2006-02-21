/***************************************************************************
                          memory_level.h  -  description
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

#ifndef MEMORY_LEVEL_H
#define MEMORY_LEVEL_H

#include <string>
#include <iostream>
#include <fstream>

#include "parameter.h"
#include "common.h"

using namespace std;

extern "C" double logtwo_area(double);

enum cache_type { L1D_CACHE,L1I_CACHE,L2U_CACHE };

class Mem_hierarchy {
    public: 
	Mem_hierarchy();
	~Mem_hierarchy();
 

	struct Cache  
	{
	    string label;

	    cache_type type;

	    Parameter block_size;
	    Parameter size;
	    Parameter associativity;

	    int subblock_size;
	    int latency;
	    int bus_width;
	};

	Cache L1D;
	Cache L1I;
	Cache L2U;

	void set_to_default();
	void print_cache_config(const Cache& cache) const;
	bool test_valid_cache(const Cache& cache) const ;
	bool test_valid_cache(int C, int B, int A) const ;
	bool test_valid_config() const;

};

#endif
