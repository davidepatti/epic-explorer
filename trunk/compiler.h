/***************************************************************************
                          compiler.h  -  description
                             -------------------
    begin                : Sat Mar 29 2008
    copyright            : (C) 2008 by Daniele Bologna
    email                : danielb78@alice.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COMPILER_H
#define COMPILER_H

#include "environment.h"
#include "parameter.h"
#include "common.h"

#include <fstream>
#include <cstdlib>
#include <string>

class Compiler {

    public:

	Compiler();
	~Compiler();

	void set_to_default();
	void set_config(const Configuration&);

	//tcc Parameter	
	Parameter tcc_region;	//type of region formation: basic block (b) , hyperblock (h)
	
	//Impact Parameters
	Parameter max_unroll_allowed;	//amount of unrolling allowed (default=32)
	Parameter regroup_only;		//prevent inlining (default=yes)

	//Elcor Parameters
	Parameter do_classic_opti;	//perform classical optimization (default=no)
	Parameter do_prepass_scalar_scheduling;		//perform prepass scheduling on hyperblocks (default=no)
	Parameter do_postpass_scalar_scheduling;	//perform postpass scheduling on hyperblocks (default=no)
	Parameter do_modulo_scheduling;			//perform modulo scheduling (default=yes)
	Parameter memvr_profiled;			//perform memory dependence profiling (default=yes)
};

#endif
