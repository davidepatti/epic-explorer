/***************************************************************************
                          memory_level.cpp  -  description
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

#include "mem_hierarchy.h"

Mem_hierarchy::Mem_hierarchy(){


    // L1 Data cache 
    L1D.label = "L1_data";
    L1D.type = L1D_CACHE;
    // L1 Instruction Cache
    L1I.label = "L1_inst";
    L1I.type = L1I_CACHE;
    // L2 Unified Cache 
    L2U.label = "L2_unified";
    L2U.type = L2U_CACHE;

    // set labels
    L1D.size.set_label("L1D_size");
    L1D.block_size.set_label("L1D_block") ;
    L1D.associativity.set_label("L1D_assoc");

    L1I.size.set_label("L1I_size");
    L1I.block_size.set_label("L1I_block") ;
    L1I.associativity.set_label("L1I_assoc");

    L2U.size.set_label("L2U_size");
    L2U.block_size.set_label("L2U_block") ;
    L2U.associativity.set_label("L2U_assoc");
}

void Mem_hierarchy::set_to_default()
{
    // default values are specified as a constructor Parameter

    L1D.size.set_to_default();
    L1D.block_size.set_to_default() ;
    L1D.associativity.set_to_default();

    L1I.size.set_to_default();
    L1I.block_size.set_to_default();
    L1I.associativity.set_to_default();

    L2U.size.set_to_default();
    L2U.block_size.set_to_default();
    L2U.associativity.set_to_default();

}

void Mem_hierarchy::set_config(const Configuration& conf)
{
    L1D.size.set_val(conf.L1D_size);
    L1D.block_size.set_val(conf.L1D_block);
    L1D.associativity.set_val(conf.L1D_assoc);
    L1I.size.set_val(conf.L1I_size);
    L1I.block_size.set_val(conf.L1I_block);
    L1I.associativity.set_val(conf.L1I_assoc);
    L2U.size.set_val(conf.L2U_size);
    L2U.block_size.set_val(conf.L2U_block);
    L2U.associativity.set_val(conf.L2U_assoc);
}


