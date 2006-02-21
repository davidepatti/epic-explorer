/***************************************************************************
                          Processor.cpp  -  description
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

#include "processor.h"

Processor::Processor(){

}

Processor::~Processor(){
}

void Processor::set_to_default()
{
    config.integer_units.set_to_default();
    config.float_units.set_to_default();
    config.branch_units.set_to_default();
    config.memory_units.set_to_default();

    config.gpr_static_size.set_to_default();
    config.fpr_static_size.set_to_default();
    config.pr_static_size.set_to_default();
    config.cr_static_size.set_to_default();
    config.btr_static_size.set_to_default();

}

