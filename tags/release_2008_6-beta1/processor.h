/***************************************************************************
                          hpl_pd.h  -  description
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

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "environment.h"
#include "parameter.h"
#include "common.h"

#include <fstream>
#include <cstdlib>
#include <string>

class Processor {

    public: 

	Processor();

	~Processor();
	
	void set_to_default();
	void set_config(const Configuration&);

	Parameter num_clusters;

	Parameter integer_units;
	Parameter float_units;
	Parameter branch_units;
	Parameter memory_units;

	Parameter gpr_static_size;
	Parameter fpr_static_size;
	Parameter pr_static_size;
	Parameter cr_static_size;
	Parameter btr_static_size;
};

#endif
