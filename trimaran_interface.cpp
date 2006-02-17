/***************************************************************************
                          Trimaran_interface.cpp  -  description
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

#include "trimaran_interface.h"
#include "environment.h"
#include "common.h"

Trimaran_interface::Trimaran_interface(){

    string default_benchmark = DEFAULT_BENCH;
    set_benchmark(default_benchmark);
    set_environment();
    set_hyperblock(false);
    updated_dynamic_stats = false;
}

Trimaran_interface::~Trimaran_interface()
{
    cout << "\n Destroying Trimaran_interface object...";
}

void Trimaran_interface::set_hyperblock(bool hyperblock)
{
    settings.hyperblock = hyperblock;
}

void Trimaran_interface::set_benchmark(string& new_benchmark)
{
    settings.benchmark = new_benchmark;
}

string Trimaran_interface::get_benchmark_name() const
{
    return settings.benchmark;
}


void Trimaran_interface::compile_hmdes_file() const{

    string main_hmdes2 = MAIN_HMDES2;
    string path = get_base_dir() +"/trimaran-workspace/machines";
    chdir(path.c_str());

    string command ="$IMPACT_REL_PATH/scripts/hc "+main_hmdes2;
    system(command.c_str());

    path = get_base_dir()+"/trimaran-workspace/epic-explorer";

    chdir(path.c_str());
}


void Trimaran_interface::compile_benchmark() {
    // TODO: this and similar functions should acquire hmdes paths
    // from get_base_dir()
    string command="";

    if (settings.hyperblock)
    {
    command =command +"tcc -host "+TRIMARAN_HOST_PLATFORM+" -bench "+settings.benchmark+" -region h -i2s -E\"\" -M$HOME/trimaran-workspace/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -clean -gui | tee -a compilation.log";
    //command =command +"tcc -host "+TRIMARAN_HOST_PLATFORM+" -bench "+settings.benchmark+" -region h -i2s -E\" -Fdo_triceps=no \" -M$HOME/trimaran-workspace/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -clean -gui | tee -a compilation.log";
    }
    else
    {
    command =command +"tcc -host "+TRIMARAN_HOST_PLATFORM+" -bench "+settings.benchmark+" -region b -i2s -E\"\" -M$HOME/trimaran-workspace/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -clean -gui | tee -a compilation.log";
    //command =command +"tcc -host "+TRIMARAN_HOST_PLATFORM+" -bench "+settings.benchmark+" -region b -i2s -E\" -Fdo_triceps=no \" -M$HOME/trimaran-workspace/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -clean -gui | tee -a compilation.log";
    };

    system(command.c_str());
    
    updated_dynamic_stats = false;
}

void Trimaran_interface::execute_benchmark() 
{
    // TODO: this and similar functions should acquire hmdes paths
    // from get_base_dir()
    // TODO: check proper tcc command options
    string command;

    if (settings.hyperblock)
    {
    command =command +"tcc -host "+TRIMARAN_HOST_PLATFORM+" -bench "+settings.benchmark+" -region h -m2m -E\"\" -M$HOME/trimaran-workspace/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -gui | tee -a compilation.log";
    }
    else
    {
    command =command +"tcc -host "+TRIMARAN_HOST_PLATFORM+" -bench "+settings.benchmark+" -region b -m2m -E\"\" -M$HOME/trimaran-workspace/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -gui | tee -a compilation.log";
    };

    system(command.c_str());
    updated_dynamic_stats = false;
}


// extract dynamic statistics from Trimaran PD_STATS file

void Trimaran_interface::update_dynamic_stats()
{
    string path;

    if (settings.hyperblock) path = "./simu_intermediate/PD_STATS.HS";
    else
	path = "./simu_intermediate/PD_STATS.O";

    std::ifstream input_file(path.c_str());

    string pippo;

    if (!input_file) 
    {
	cout << "\nError opening file :" << path;
	wait_key();
    }
    else
    {
	std::ifstream input_file(path.c_str());
	go_until("Totals",input_file);
	pippo = skip_to(input_file,2);
	dynamic_stats.total_cycles = atoll(pippo.erase(0,63));

	input_file >> pippo;
	dynamic_stats.compute_cycles = atoll(pippo.erase(0,63));

	pippo = skip_to(input_file,"stall_cycles");
	dynamic_stats.stall_cycles = atoll(pippo.erase(0,63));

	pippo = skip_to(input_file,"total_dynamic_operations");
	dynamic_stats.total_dynamic_operations = atoll(pippo.erase(0,63));

	pippo = skip_to(input_file,"average_issued_ops/total_cycles");
	dynamic_stats.average_issued_ops_total_cycles = atof(pippo.erase(0,63));

	pippo = skip_to(input_file,"average_issued_ops/compute_cycles");
	dynamic_stats.average_issued_ops_compute_cycles = atof(pippo.erase(0,63));

	go_until("branch",input_file);
	input_file >> dynamic_stats.branch;

	go_until("load",input_file);
	input_file >> dynamic_stats.load;

	go_until("store",input_file);
	input_file >> dynamic_stats.store;

	go_until("ialu",input_file);
	input_file >> dynamic_stats.ialu;

	go_until("falu",input_file);
	input_file >> dynamic_stats.falu;

	go_until("cmp",input_file);
	input_file >> dynamic_stats.cmp;

	go_until("pbr",input_file);
	input_file >> dynamic_stats.pbr;

	go_until("spills_restores",input_file);
	input_file >> dynamic_stats.spills_restores;

	string word;

	// TODO: considerare prefetch misses ?

	go_until("l1-icache",input_file);
	go_until("Demand",input_file);

	input_file>>word;
	input_file>>dynamic_stats.L1I_fetches;

	go_until("Demand",input_file);

	input_file>>word;
	input_file>>dynamic_stats.L1I_miss;

	go_until("Capacity",input_file);
	input_file >> word;
	input_file >> dynamic_stats.L1I_capacity_misses;

	go_until("Conflict",input_file);
	input_file >> word;
	input_file >> dynamic_stats.L1I_conflict_misses;

	dynamic_stats.L1I_hit = dynamic_stats.L1I_fetches-dynamic_stats.L1I_miss;

	go_until("l1-dcache",input_file);

	go_until("Fetches",input_file);

	input_file>>word;
	input_file>>word;
	input_file>>word;

	input_file>>dynamic_stats.L1D_r_fetches;
	input_file>>dynamic_stats.L1D_w_fetches;

	go_until("Misses",input_file);

	input_file>>word;
	input_file>>word;
	input_file>>word;

	input_file>>dynamic_stats.L1D_r_miss;
	input_file>>dynamic_stats.L1D_w_miss;

	go_until("Capacity",input_file);
	input_file >> word;
	input_file >> dynamic_stats.L1D_capacity_misses;

	go_until("Conflict",input_file);
	input_file >> word;
	input_file >> dynamic_stats.L1D_conflict_misses;

	dynamic_stats.L1D_miss = dynamic_stats.L1D_r_miss+dynamic_stats.L1D_w_miss;
	dynamic_stats.L1D_r_hit = dynamic_stats.L1D_r_fetches-dynamic_stats.L1D_r_miss;
	dynamic_stats.L1D_w_hit = dynamic_stats.L1D_w_fetches-dynamic_stats.L1D_w_miss;
	dynamic_stats.L1D_hit = dynamic_stats.L1D_r_hit+dynamic_stats.L1D_w_hit;

	go_until("l2-ucache",input_file);
	go_until("Fetches",input_file);

	input_file>>dynamic_stats.L2U_demand;
	input_file>>dynamic_stats.L2U_i_demand;
	input_file>>dynamic_stats.L2U_d_demand;
	input_file>>dynamic_stats.L2U_dr_demand;
	input_file>>dynamic_stats.L2U_dw_demand;

	go_until("Misses",input_file);

	input_file>>dynamic_stats.L2U_miss;
	input_file>>dynamic_stats.L2U_i_miss;
	input_file>>dynamic_stats.L2U_d_miss;
	input_file>>dynamic_stats.L2U_dr_miss;
	input_file>>dynamic_stats.L2U_dw_miss;

	go_until("Capacity",input_file);
	input_file >> word;
	input_file >> dynamic_stats.L2U_capacity_misses;

	go_until("Conflict",input_file);
	input_file >> word;
	input_file >> dynamic_stats.L2U_conflict_misses;

	dynamic_stats.L2U_r_miss = dynamic_stats.L2U_i_miss+dynamic_stats.L2U_dr_miss;
	dynamic_stats.L2U_i_hit = dynamic_stats.L2U_i_demand-dynamic_stats.L2U_i_miss;
	dynamic_stats.L2U_dr_hit = dynamic_stats.L2U_dr_demand-dynamic_stats.L2U_dr_miss;
	dynamic_stats.L2U_dw_hit = dynamic_stats.L2U_dw_demand-dynamic_stats.L2U_dw_miss;
	dynamic_stats.L2U_r_hit = dynamic_stats.L2U_i_hit+dynamic_stats.L2U_dr_hit;
	dynamic_stats.L2U_w_hit = dynamic_stats.L2U_dw_hit;
	dynamic_stats.L2U_w_miss = dynamic_stats.L2U_dw_miss;
	dynamic_stats.L2U_hit = dynamic_stats.L2U_w_hit+dynamic_stats.L2U_r_hit;
    }
}

Dynamic_stats Trimaran_interface::get_dynamic_stats() {

    if (!updated_dynamic_stats) 
    {
	update_dynamic_stats();
	updated_dynamic_stats = true;
    }
    return dynamic_stats;
}

void Trimaran_interface::set_environment() {

    string base_dir = get_base_dir();

    setenv("TRIMARAN_ROOT",(base_dir+"/trimaran/").c_str(),1);

    setenv("CC",CC,1);
    setenv("CXX",CXX,1);
    setenv("TRIMARAN_HOST_PLATFORM",TRIMARAN_HOST_PLATFORM,1);
    setenv("TRIMARAN_HOST_COMPILER",TRIMARAN_HOST_COMPILER,1);
    setenv("USER_BENCH_PATH1",(base_dir+USER_BENCH_PATH1).c_str(),1);


    setenv("IMPACT_ROOT",(base_dir+IMPACT_ROOT).c_str(),1);
    setenv("IMPACT_REL_PATH",(base_dir+IMPACT_REL_PATH).c_str(),1);
    setenv("IMPACT_BIN_PATH",(base_dir+IMPACT_BIN_PATH).c_str(),1);
    setenv("IMPACT_LIB_PATH",(base_dir+IMPACT_LIB_PATH).c_str(),1);
    setenv("STD_PARMS_FILE",(base_dir+STD_PARMS_FILE).c_str(),1);
    setenv("IMPACT_BUILD_TYPE",IMPACT_BUILD_TYPE,1);

    setenv("ELCOR_HOME",(base_dir+ELCOR_HOME).c_str(),1);
    setenv("ELCOR_REL_PATH",(base_dir+ELCOR_REL_PATH).c_str(),1);
    setenv("ELCOR_BIN_PATH",(base_dir+ELCOR_BIN_PATH).c_str(),1);
    setenv("ELCOR_PARMS_FILE",(base_dir+ELCOR_PARMS_FILE).c_str(),1);
    setenv("ELCOR_USER",ELCOR_USER,1);

    setenv("SIMU_HOME",(base_dir+SIMU_HOME).c_str(),1);
    setenv("SIMU_REL_PATH",(base_dir+SIMU_REL_PATH).c_str(),1);
    setenv("SIMU_BIN_PATH",(base_dir+SIMU_BIN_PATH).c_str(),1);
    setenv("SIMU_PARMS_FILE",(base_dir+SIMU_PARMS_FILE).c_str(),1);

    setenv("DINERO_HOME",(base_dir+DINERO_HOME).c_str(),1);
    setenv("DINERO_REL_PATH",(base_dir+DINERO_REL_PATH).c_str(),1);


    string pp;
    pp = "/usr/local/bin:/bin:/usr/bin:/usr/X11R6/bin:"+base_dir+"/bin:"+base_dir+"/trimaran/./bin:"+base_dir+"/trimaran/./impact/bin/x86lin_c:"+base_dir+"/trimaran/./impact/scripts:"+base_dir+"/trimaran/./impact/config:"+base_dir+"/trimaran/./elcor/bin:"+base_dir+"/trimaran/./simu/bin:"+base_dir+"/trimaran/./gui";
    setenv("PATH",pp.c_str(),1);
    setenv("DEFAULT_PROJECT",DEFAULT_PROJECT ,1);

    string path;
    string command;

    path = base_dir+"/trimaran-workspace/epic-explorer";
    command = "mkdir "+path;

    system(command.c_str());

    chdir(path.c_str());
}



