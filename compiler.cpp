/***************************************************************************
                          compiler.cpp  -  description
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

#include "compiler.h"
using namespace std;

Compiler::Compiler(){
	tcc_region.set_label("tcc_region");
	max_unroll_allowed.set_label("max_unroll_allowed");
	regroup_only.set_label("regroup_only");
	
	do_classic_opti.set_label("do_classic_opti");
	do_prepass_scalar_scheduling.set_label("do_prepass_scheduling");
	do_postpass_scalar_scheduling.set_label("do_postpass_scheduling");
	do_modulo_scheduling.set_label("do_modulo_scheduling");
	memvr_profiled.set_label("memvr_profiled");
}

Compiler::~Compiler(){
	cout << EE_TAG<<"destroying compiler...";
}

void Compiler::set_to_default()
{
	tcc_region.set_to_default();
	max_unroll_allowed.set_to_default();
	regroup_only.set_to_default();
	
	do_classic_opti.set_to_default();
	do_prepass_scalar_scheduling.set_to_default();
	do_postpass_scalar_scheduling.set_to_default();
	do_modulo_scheduling.set_to_default();
	memvr_profiled.set_to_default();
}

void Compiler::set_config(const Configuration& conf)
{
	tcc_region.set_val(conf.tcc_region);
	max_unroll_allowed.set_val(conf.max_unroll_allowed);
	regroup_only.set_val(conf.regroup_only);

	do_classic_opti.set_val(conf.do_classic_opti);
	do_prepass_scalar_scheduling.set_val(conf.do_prepass_scalar_scheduling);
	do_postpass_scalar_scheduling.set_val(conf.do_postpass_scalar_scheduling);
	do_modulo_scheduling.set_val(conf.do_modulo_scheduling);
	memvr_profiled.set_val(conf.memvr_profiled);
}
