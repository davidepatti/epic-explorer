/***************************************************************************
                          memory_level.h  -  description
                             -------------------
    begin                : Sat Aug 17 2002
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

#ifndef MEMORY_LEVEL_H
#define MEMORY_LEVEL_H

#include <string>
#include <iostream>
#include <fstream>

#include "parameter.h"
#include "common.h"

using namespace std;

enum cache_type { L1D_CACHE,L1I_CACHE,L2U_CACHE };

class Mem_hierarchy {
    public: 
	Mem_hierarchy();

	struct Cache  
	{
	    string label;
	    cache_type type;
	    Parameter block_size;
	    Parameter size;
	    Parameter associativity;
	    int latency;
	    int bus_width;
	} L1D,L1I,L2U;

	void set_to_default();
	void set_config(const Configuration&);
};

#endif
