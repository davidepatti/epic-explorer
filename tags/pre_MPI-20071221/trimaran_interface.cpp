/***************************************************************************
                          Trimaran_interface.cpp  -  description
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

#include "trimaran_interface.h"
#include "environment.h"
#include "common.h"

Trimaran_interface::Trimaran_interface(const string& base_dir)
{
    set_benchmark(string("DEFAULT_BENCH"));
    set_environment(base_dir);
    set_hyperblock(false);
}

Trimaran_interface::~Trimaran_interface()
{
    cout << "\n Destroying Trimaran_interface object...";
}

void Trimaran_interface::set_hyperblock(bool hyperblock)
{
    do_hyperblock = hyperblock;
}

void Trimaran_interface::set_benchmark(string new_benchmark)
{
   current_benchmark = new_benchmark;
}

string Trimaran_interface::get_benchmark_name() const
{
    return current_benchmark;
}


void Trimaran_interface::compile_hmdes_file(const string& machine_dir) const
{
    char old_path[50];
    getcwd(old_path,50);
    chdir(machine_dir.c_str());

    string main_hmdes2 = MAIN_HMDES2;

    string command ="$IMPACT_REL_PATH/scripts/hc "+main_hmdes2;
    system(command.c_str());
    chdir(old_path);
}


void Trimaran_interface::compile_benchmark(const string& path) {

    char old_path[50];
    getcwd(old_path,50);
    chdir(path.c_str());

    string command="";

    if (do_hyperblock)
    {
    command = command + "tcc -host "+TRIMARAN_HOST_PLATFORM+" -bench "+current_benchmark+" -region h -i2s -E\"\" -M"+path+"/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -clean -gui  | tee -a "+path+"/compilation.log";
    }
    else
    {
    command = command + "tcc -host "+TRIMARAN_HOST_PLATFORM+" -bench "+current_benchmark+" -region b -i2s -E\"\" -M"+path+"/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -clean -gui | tee -a "+path+"/compilation.log";
    };

    system(command.c_str());

    chdir(old_path);
}

void Trimaran_interface::execute_benchmark(const string& path, const string& cache_dir) 
{
    char old_path[50];
    getcwd(old_path,50);
    chdir(path.c_str());
    // TODO: check proper tcc command options
    string command;

    if (do_hyperblock)
    {
    command = command + "tcc -host "+TRIMARAN_HOST_PLATFORM+" -bench "+current_benchmark+" -region h -m2m -E\"\" -M"+path+"/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -gui -cache_cfg "+cache_dir+" | tee -a "+path+"/"+cache_dir+"/execution.log";
    }
    else
    {
    command = command + "tcc -host "+TRIMARAN_HOST_PLATFORM+" -bench "+current_benchmark+" -region b -m2m -E\"\" -M"+path+"/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -gui -cache_cfg "+cache_dir+" | tee -a "+path+"/"+cache_dir+"/execution.log";
    };

    system(command.c_str());
    chdir(old_path);
}


// extract dynamic statistics from Trimaran PD_STATS file

Dynamic_stats Trimaran_interface::get_dynamic_stats(const string& filename)
{
    std::ifstream input_file(filename.c_str());

    string pippo;
    Dynamic_stats dynamic_stats;

    if (!input_file) 
    {
	cout << "\nError opening file:" << filename;
	wait_key();
    }
    else
    {
	std::ifstream input_file(filename.c_str());
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

    return dynamic_stats;
}

void Trimaran_interface::set_environment(const string& base_dir) {

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

    string path = base_dir + "/trimaran-workspace/epic-explorer";

    if (chdir(path.c_str())==-1)
    {
	string command = "mkdir "+path;
	system(command.c_str());
        assert(chdir(path.c_str())!=-1);
    }
}

void Trimaran_interface::save_processor_config(const Processor& p, const string& path) const
{
    string filename = path;
    std::ofstream output_file(filename.c_str());

    if (!output_file) 
    {
	cout << "\nError opening hmdes file :" << filename;
	wait_key();
    }
    else
    {
	// NOTE: 
	// - we assume that rotating and static portion of registers
	// have equal sizes.
	// - All the other parameters are fixed to their default values as specified
	// in the standard trimaran hmdes2 files. 

	output_file << "\n//////////////////////////////////////////////////";
	output_file << "\n// Epic Explorer processor configuration";
	output_file << "\n//////////////////////////////////////////////////";
	output_file << "\n// DO NOT EDIT: this file is generated by epic explorer ";
	output_file << "\n// and should be included into the main hmdes2 machine ";
	output_file << "\n// description file using the $include directive";

	output_file << "\n\n";
	output_file << "\n// Register file sizes ";
	output_file << "\n\n";
	output_file << "\n$def !gpr_static_size\t" << p.gpr_static_size.get_val();
	output_file << "\n$def !gpr_rotating_size\t" << p.gpr_static_size.get_val();
	output_file << "\n$def !fpr_static_size\t" <<   p.fpr_static_size.get_val();
	output_file << "\n$def !fpr_rotating_size\t" << p.fpr_static_size.get_val();
	output_file << "\n$def !pr_static_size\t" <<    p.pr_static_size.get_val();
	output_file << "\n$def !pr_rotating_size\t" <<  p.pr_static_size.get_val();
	output_file << "\n$def !cr_static_size\t" <<    p.cr_static_size.get_val();
	output_file << "\n$def !cr_rotating_size\t" <<  p.cr_static_size.get_val();
	output_file << "\n$def !btr_static_size\t" <<   p.btr_static_size.get_val();

	output_file << "\n\n// Functional Units \n";

	output_file << "\n$def !integer_units\t" << p.integer_units.get_val();
	output_file << "\n$def !float_units\t" <<   p.float_units.get_val();
	output_file << "\n$def !memory_units\t" <<  p.memory_units.get_val();
	output_file << "\n$def !branch_units\t" <<  p.branch_units.get_val();
    }
}

void Trimaran_interface::load_processor_config(Processor* p,const string& filename) const
{
    std::ifstream input_file(filename.c_str());

    if (!input_file) {
	cout << "\nError opening hmdes file :" << filename;
	wait_key();
    }
    else
    {
	int val;

	//NOTE : This function only read from hmdes configuration file
	//the parameter values that are subject of exploration.
	//
	//Rotating size entries are currently ignored, assuming
	//that rotating portion has always the same size of static
	//portion for all register files.

	/// Register files sizes ////////////////////////////////////
	go_until("!gpr_static_size",input_file);
	input_file>>val;
	p->gpr_static_size.set_val(val);

	go_until("!fpr_static_size",input_file);
	input_file>>val;
	p->fpr_static_size.set_val(val);

	go_until("!pr_static_size",input_file);
	input_file>>val;

	p->pr_static_size.set_val(val);

	go_until("!cr_static_size",input_file);
	input_file>>val;
	p->cr_static_size.set_val(val);

	go_until("!btr_static_size",input_file);
	input_file>>val;
	p->btr_static_size.set_val(val);

	//// Functional units ////////////////////////

	go_until("!integer_units",input_file);
	input_file>>val;
	p->integer_units.set_val(val);

	go_until("!float_units",input_file);
	input_file>>val;
	p->float_units.set_val(val);

	go_until("!memory_units",input_file);
	input_file>>val;
	p->memory_units.set_val(val);

	if (val==1) 
	{
	    cout << "\n Fatal error: due a Trimaran bug EPIC-Exlorer currently does not";
	    cout << "\n support spaces with memory_units = 1. ";
	    cout << "\n Please modify your *.sub file in trimaran-workspace/SUBSPACES dir.";
	    cout << "\n For more details : https://lists.csail.mit.edu/pipermail/trimaran-users/2005-September/000083.html";
	    exit(1);
	}

	go_until("!branch_units",input_file);
	input_file>>val;
	p->branch_units.set_val(val);
    }
}

void Trimaran_interface::save_mem_config(const Mem_hierarchy& mem, const string& path) const
{
    string cache_config_file = path;
    std::ofstream output_file(cache_config_file.c_str());

    if (!output_file) 
    {
	cout << "\nerror opening hmdes file :" << cache_config_file;
	wait_key();
    }
    else
    {
	output_file <<"\n# EPIC explorer cache configuration ";
	output_file <<"\n";
	output_file <<"\n# level 2 unified cache";
	output_file <<"\nL2U_blocksize " << mem.L2U.block_size.get_val();
	output_file <<"\nL2U_subblocksize " << mem.L2U.block_size.get_val();
	output_file <<"\nL2U_size " << mem.L2U.size.get_val() ;
	output_file <<"\nL2U_associativity " << mem.L2U.associativity.get_val();
	output_file <<"\n";
	output_file <<"\n# level 1 instruction cache ";
	output_file <<"\n";
	output_file <<"\nL1I_blocksize " << mem.L1I.block_size.get_val();
	output_file <<"\nL1I_subblocksize " << mem.L1I.block_size.get_val();
	output_file <<"\nL1I_size " << mem.L1I.size.get_val();
	output_file <<"\nL1I_associativity " << mem.L1I.associativity.get_val();
	output_file <<"\n";
	output_file <<"\n# level 1 data cache";
	output_file <<"\n";
	output_file <<"\nL1D_blocksize " << mem.L1D.block_size.get_val();
	output_file <<"\nL1D_subblocksize " << mem.L1D.block_size.get_val();
	output_file <<"\nL1D_size " << mem.L1D.size.get_val();
	output_file <<"\nL1D_associativity " << mem.L1D.associativity.get_val();
    }
}

void Trimaran_interface::load_mem_config(Mem_hierarchy* mem,const string& filename) const
{
    std::ifstream input_file(filename.c_str());

    int value ;

    if (!input_file) 
    {
	cout << "\nError opening cache config file :" << filename;
	wait_key();
    }
    else
    {
	// Note that subblock size value in config file is ignored 
	// subblock size is assumed to be equal to block size

	go_until("L2U_blocksize",input_file);
	input_file >> value;
	mem->L2U.block_size.set_val(value);
	mem->L2U.subblock_size = mem->L2U.block_size.get_val();

	go_until("L2U_size",input_file);
	input_file >> value;
	mem->L2U.size.set_val(value);

	go_until("L2U_associativity",input_file);
	input_file >> value;
	mem->L2U.associativity.set_val(value);


	go_until("L1I_blocksize",input_file);
	input_file >> value;
	mem->L1I.block_size.set_val(value);
	mem->L1I.subblock_size = mem->L1I.block_size.get_val();

	go_until("L1I_size",input_file);
	input_file >> value;
	mem->L1I.size.set_val(value);

	go_until("L1I_associativity",input_file);
	input_file >> value;
	mem->L1I.associativity.set_val(value);


	go_until("L1D_blocksize",input_file);
	input_file >> value;
	mem->L1D.block_size.set_val(value) ;
	mem->L1D.subblock_size = mem->L1D.block_size.get_val();

	go_until("L1D_size",input_file);
	input_file >> value;
	mem->L1D.size.set_val(value);

	go_until("L1D_associativity",input_file);
	input_file >> value;
	mem->L1D.associativity.set_val(value);

    }
}


