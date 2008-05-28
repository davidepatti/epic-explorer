/***************************************************************************
                          Processor.cpp  -  description
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

#include "processor.h"

Processor::Processor(){
    num_clusters.set_label("num_clusters");
    integer_units.set_label("integer_units");
    float_units.set_label("float_units");
    branch_units.set_label("branch_units");
    memory_units.set_label("memory_units");

    gpr_static_size.set_label("gpr_static_size");
    fpr_static_size.set_label("fpr_static_size");
    pr_static_size.set_label("pr_static_size");
    cr_static_size.set_label("cr_static_size");
    btr_static_size.set_label("btr_static_size");
}

Processor::~Processor(){
}

void Processor::set_to_default()
{
    num_clusters.set_to_default();
    integer_units.set_to_default();
    float_units.set_to_default();
    branch_units.set_to_default();
    memory_units.set_to_default();

    gpr_static_size.set_to_default();
    fpr_static_size.set_to_default();
    pr_static_size.set_to_default();
    cr_static_size.set_to_default();
    btr_static_size.set_to_default();

}

void Processor::set_config(const Configuration& conf)
{
    num_clusters.set_val(conf.num_clusters);
    integer_units.set_val(conf.integer_units);
    float_units.set_val(conf.float_units);
    branch_units.set_val(conf.branch_units);
    memory_units.set_val(conf.memory_units);

    gpr_static_size.set_val(conf.gpr_static_size);
    fpr_static_size.set_val(conf.fpr_static_size);
    pr_static_size.set_val(conf.pr_static_size);
    cr_static_size.set_val(conf.cr_static_size);
    btr_static_size.set_val(conf.btr_static_size);
}

