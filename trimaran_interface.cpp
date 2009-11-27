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
    set_benchmark(string("BENCH_NOT_SET"));
    set_environment(base_dir);
    set_save_tcclog(false);
    set_continue_on_failure(false);
}

Trimaran_interface::~Trimaran_interface()
{
    cout << "\n Destroying Trimaran_interface object...";
}


void Trimaran_interface::set_benchmark(string new_benchmark)
{
   current_benchmark = new_benchmark;
}

void Trimaran_interface::set_save_tcclog(bool save_log)
{
    do_save_log = save_log;
}

void Trimaran_interface::set_continue_on_failure(bool value)
{
    continue_on_failure = value;
}

string Trimaran_interface::get_benchmark_name() const
{
    return current_benchmark;
}


// TODO: pass hmdes file path, not machine dir
void Trimaran_interface::compile_hmdes_file(const string& machine_dir) const
{
    string logfile = base_path+string(EE_LOG_PATH);
    int myid = get_mpi_rank();

    char old_path[50];
    getcwd(old_path,50);
    chdir(machine_dir.c_str());

    string main_hmdes2 = MAIN_HMDES2;

    if (file_exists(main_hmdes2))
    {
	string command ="$IMPACT_REL_PATH/scripts/hc "+main_hmdes2;
	system(command.c_str());
    }
    else
    {
	write_to_log(myid,logfile,"FATAL ERROR: cannot find " + main_hmdes2 +  " in " + machine_dir);
	exit(EXIT_FAILURE);
    }

    chdir(old_path);
}

// Paramters of interested when invoking tcc
// IMPACT: -p to pass parameters file
//


void Trimaran_interface::compile_benchmark(Compiler* c,const string& path) 
{

    char old_path[50];
    getcwd(old_path,50);
    chdir(path.c_str());
    int mua = c->max_unroll_allowed.get_val();	//db
    int ro = c->regroup_only.get_val();	//db
    ostringstream ossmua;	//db
    ossmua << mua;	//db
    string muas = ossmua.str();	//db

    string command;
    string redirection;

    string tcc_args = " -bench "+current_benchmark;

    if (c->tcc_region.get_val() == 1) tcc_args +=" -region b";  //db	
    if (c->tcc_region.get_val() == 2) tcc_args +=" -region h";  //db
    if (c->tcc_region.get_val() == 3) tcc_args +=" -region s";  //db

    string tcc_impact_args, tcc_elcor_args="";	//db
    tcc_impact_args =" -I\"-max_unroll "+muas+"\" ";	//db
    if (c->regroup_only.get_val() == 1) tcc_impact_args +=" -I\"-no_inlining\""; 	//db
    if (c->do_classic_opti.get_val() == 2) tcc_elcor_args += " -E\"-Fdo_classic_opti=yes\"";	//db
    if (c->do_prepass_scalar_scheduling.get_val() == 2) tcc_elcor_args += " -E\"-Fdo_prepass_scalar_scheduling=yes\"";	//db
    if (c->do_postpass_scalar_scheduling.get_val() == 2) tcc_elcor_args += " -E\"-Fdo_postpass_scalar_scheduling=yes\"";	//db
    if (c->do_modulo_scheduling.get_val() == 1) tcc_elcor_args += " -E\"-Fdo_do_modulo_scheduling=no\"";	//db
    if (c->memvr_profiled.get_val() == 2) tcc_elcor_args += " -E\"-Fmemvr_profiled=yes\"";	//db


    if (do_save_log) redirection += " | tee -a "+path+"/compilation.log";
    else redirection += " | tee -a /dev/null";

    command = "tcc "+tcc_args+tcc_impact_args+tcc_elcor_args+" -i2s -M"+path+"/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -clean -gui "+redirection; //db

#ifdef DEBUG
    cout << EE_TAG << "executing: " << command;
    //int c; cin>> c;
#endif
    system(command.c_str());

    chdir(old_path);
}

void Trimaran_interface::execute_benchmark(Compiler* c,const string& path, const string& cache_dir) //db
{
    char old_path[50];
    getcwd(old_path,50);
    chdir(path.c_str());
    
    int mua = c->max_unroll_allowed.get_val();	//db
    int ro = c->regroup_only.get_val();	//db
    ostringstream ossmua;	//db
    ossmua << mua;	//db
    string muas = ossmua.str();	//db
     
    string command;
    string tcc_args = " -bench "+current_benchmark;
    string redirection;   
    
    if (c->tcc_region.get_val() == 1) tcc_args +=" -region b";  //db	
    if (c->tcc_region.get_val() == 2) tcc_args +=" -region h";  //db
    if (c->tcc_region.get_val() == 3) tcc_args +=" -region s";  //db
	
    string tcc_impact_args, tcc_elcor_args="";	//db
	   tcc_impact_args =" -I\"-max_unroll "+muas+"\" ";	//db
   if (c->regroup_only.get_val() == 1) tcc_impact_args +=" -I\"-no_inlining\""; 	//db
   if (c->do_classic_opti.get_val() == 2) tcc_elcor_args += " -E\"-Fdo_classic_opti=yes\"";	//db
   if (c->do_prepass_scalar_scheduling.get_val() == 2) tcc_elcor_args += " -E\"-Fdo_prepass_scalar_scheduling=yes\"";	//db
   if (c->do_postpass_scalar_scheduling.get_val() == 2) tcc_elcor_args += " -E\"-Fdo_postpass_scalar_scheduling=yes\"";	//db
   if (c->do_modulo_scheduling.get_val() == 1) tcc_elcor_args += " -E\"-Fdo_do_modulo_scheduling=no\"";	//db
   if (c->memvr_profiled.get_val() == 2) tcc_elcor_args += " -E\"-Fmemvr_profiled=yes\"";	//db

    
    

    if (do_save_log) redirection += " | tee -a "+path+"/"+cache_dir+"/execution.log";
    else redirection += " | tee -a /dev/null";

   // if (do_hyperblock) tcc_args += " -region h";
    //else tcc_args += " -region b";

    command = "tcc "+tcc_args+tcc_impact_args+tcc_elcor_args+" -m2m -M"+path+"/machines/hpl_pd_elcor_std.lmdes2 -S\"-Fcontrol_flow_trace=yes -Faddress_trace=yes\" -project \"full\" -gui -cache_dir "+cache_dir+redirection;

#ifdef DEBUG
    cout << EE_TAG << "executing: " << command;
    //int c; cin>> c;
#endif
    system(command.c_str());
    chdir(old_path);
}

// read statistics from PD_STATS and m5stats.txt files
Dynamic_stats Trimaran_interface::get_dynamic_stats(const string& path)
{
    int x;
    string pd_filename = path + "PD_STATS";
    string m5_filename = path + "m5stats.txt";

    std::ifstream pd_file(pd_filename.c_str());
    std::ifstream m5_file(m5_filename.c_str());


    string pippo;
    Dynamic_stats dynamic_stats;
    dynamic_stats.valid = true; // set on false if continue_on_failure occurs

    string logfile = base_path+string(EE_LOG_PATH);
    int myid = get_mpi_rank();

    if (!pd_file) 
    {
	if (continue_on_failure)
	{
	    write_to_log(myid,logfile,"ERROR: get_dynamic_stats() cannot open file:" + pd_filename + ", continuing...");
	    dynamic_stats.valid = false;
	}
	else
	{
	    write_to_log(myid,logfile,"FATAL ERROR: get_dynamic_stats() cannot open file:" + pd_filename+". Terminating epic, to change this beaviour see continue_on_failure setting");
	    exit(EXIT_FAILURE);
	}
    }
    else if (!m5_file)
    {
	if (continue_on_failure)
	{
	    write_to_log(myid,logfile,"ERROR: get_dynamic_stats() cannot open file:" + m5_filename + ", continuing...");
	    dynamic_stats.valid = false;
	}
	else
	{
	    write_to_log(myid,logfile,"FATAL ERROR: get_dynamic_stats() cannot open file:" + m5_filename+". Terminating epic, to change this beaviour see continue_on_failure setting");
	    exit(EXIT_FAILURE);
	}
    }
    else
    {
	go_until("Totals",pd_file);
	pippo = skip_to(pd_file,2);
	dynamic_stats.total_cycles = atoll(pippo.erase(0,63));

	pd_file >> pippo;
	dynamic_stats.compute_cycles = atoll(pippo.erase(0,63));

	pippo = skip_to(pd_file,"stall_cycles");
	dynamic_stats.stall_cycles = atoll(pippo.erase(0,63));

	pippo = skip_to(pd_file,"total_dynamic_operations");
	dynamic_stats.total_dynamic_operations = atoll(pippo.erase(0,63));

	pippo = skip_to(pd_file,"average_issued_ops/total_cycles");
	dynamic_stats.average_issued_ops_total_cycles = atof(pippo.erase(0,63));

	pippo = skip_to(pd_file,"average_issued_ops/compute_cycles");
	dynamic_stats.average_issued_ops_compute_cycles = atof(pippo.erase(0,63));

	go_until("ialu",pd_file);
	pd_file >> dynamic_stats.ialu;
	go_until("falu",pd_file);
	pd_file >> dynamic_stats.falu;
	go_until("load",pd_file);
	pd_file >> dynamic_stats.load;
	go_until("store",pd_file);
	pd_file >> dynamic_stats.store;
	go_until("cmp",pd_file);
	pd_file >> dynamic_stats.cmp;
	go_until("pbr",pd_file);
	pd_file >> dynamic_stats.pbr;
	go_until("branch",pd_file);
	pd_file >> dynamic_stats.branch;
	go_until("icm",pd_file);
	pd_file >> dynamic_stats.icm;

	go_until("spills_restores",pd_file);
	pd_file >> dynamic_stats.spills_restores;

	pd_file.close();

	// TODO: considerare prefetch misses ?
	// TODO: demand vs overall ?
	// account mshr ?
	// L2U ///////////////////////////////////////////

	go_until("L2.demand_accesses",m5_file);
	m5_file>>dynamic_stats.L2U_demand;

	go_until("L2.demand_misses",m5_file);
	m5_file>>dynamic_stats.L2U_miss;

	go_until("L2.writebacks",m5_file);
	m5_file>>dynamic_stats.L2U_writebacks;

	go_until("SDRAM.accesses",m5_file);
	m5_file >> dynamic_stats.SDRAM_accesses;

	dynamic_stats.L2U_hit = dynamic_stats.L2U_demand-dynamic_stats.L2U_miss;

	// L1D ///////////////////////////////////////////
	go_until("cluster0.dcache.read_accesses",m5_file);
	m5_file>>dynamic_stats.L1D_r_fetches;
	go_until("cluster0.dcache.read_misses",m5_file);
	m5_file>>dynamic_stats.L1D_r_miss;

	go_until("cluster0.dcache.write_accesses",m5_file);
	m5_file>>dynamic_stats.L1D_w_fetches;
	go_until("cluster0.dcache.write_misses",m5_file);
	m5_file>>dynamic_stats.L1D_w_miss;

	go_until("cluster0.dcache.writebacks",m5_file);
	m5_file>>dynamic_stats.L1D_writebacks;

	dynamic_stats.L1D_r_hit = dynamic_stats.L1D_r_fetches-dynamic_stats.L1D_r_miss;
	dynamic_stats.L1D_w_hit = dynamic_stats.L1D_w_fetches-dynamic_stats.L1D_w_miss;
	dynamic_stats.L1D_hit = dynamic_stats.L1D_r_hit+dynamic_stats.L1D_w_hit;
	dynamic_stats.L1D_miss = dynamic_stats.L1D_r_miss+dynamic_stats.L1D_w_miss;

	// L1I ///////////////////////////////////////////
	go_until("cluster0.icache.demand_accesses",m5_file);
	m5_file>>dynamic_stats.L1I_fetches;
	go_until("cluster0.icache.demand_misses",m5_file);
	m5_file>>dynamic_stats.L1I_miss;
	go_until("cluster0.icache.writebacks",m5_file);
	m5_file>>dynamic_stats.L1I_writebacks;

	dynamic_stats.L1I_hit = dynamic_stats.L1I_fetches - dynamic_stats.L1I_miss;
	m5_file.close();
    }

    dynamic_stats.L1D_transition_p = 0.20;
    dynamic_stats.L1I_transition_p = 0.20;
    // unlike L1 caches, L2 is considered to have little locality 
    dynamic_stats.L2U_transition_p = 0.5;

    return dynamic_stats;
}
// extract dynamic statistics from Trimaran PD_STATS file

void Trimaran_interface::set_environment(const string& base_dir) {

    this->base_path = base_dir;

    setenv("TRIMARAN_ROOT",(base_dir+"/trimaran/").c_str(),1);
    setenv("TRIMARAN_HOST_TYPE",TRIMARAN_HOST_TYPE,1);
    setenv("TRIMARAN_HOST_COMPILER",TRIMARAN_HOST_COMPILER,1);
    setenv("IMPACT_REL_PATH",(base_dir+IMPACT_REL_PATH).c_str(),1);
    setenv("IMPACT_ROOT",(base_dir+IMPACT_ROOT).c_str(),1);
    setenv("IMPACT_HOST_PLATFORM",IMPACT_HOST_PLATFORM,1);
    setenv("IMPACT_HOST_COMPILER",IMPACT_HOST_COMPILER,1);
    setenv("IMPACT_BUILD_TYPE",IMPACT_BUILD_TYPE,1);
    setenv("STD_PARMS_FILE",(base_dir+STD_PARMS_FILE).c_str(),1);
    setenv("DEFAULT_PROJECT",DEFAULT_PROJECT ,1);
    setenv("CC",CC,1);
    setenv("CXX",CXX,1);

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


    setenv("USER_BENCH_PATH1",(base_dir+USER_BENCH_PATH1).c_str(),1);

    setenv("DAVINCIHOME",DAVINCIHOME,1);
    setenv("DAVINCIEXE",DAVINCIEXE,1);
    setenv("M5_ROOT",(base_dir+M5_ROOT).c_str(),1);
    setenv("M5_OUTPUT_DIR",M5_OUTPUT_DIR,1);
    // M5_CONFIG_FILE should be set depending on the config simulated
    //setenv("M5_CONFIG_FILE",(base_dir+M5_CONFIG_FILE).c_str(),1);
    setenv("GUI_BIN",(base_dir+GUI_BIN).c_str(),1);
    setenv("PGPPATH",PGPPATH,1);
    setenv("SUIFHOME",(base_dir+SUIFHOME).c_str(),1);
    //setenv("",().c_str(),1);
    setenv("MACHINE",MACHINE,1);
    setenv("COMPILER_NAME",COMPILER_NAME,1);

    string cmd = "$SUIFHOME/setup_suif -sh 2> /dev/null";
    system(cmd.c_str());
    //setenv("",().c_str(),1);
    setenv("SUIFPATH","$SUIFPATH:/usr/bin",1);

    string path = getenv("PATH");
    path = path + base_dir + ":"
	+base_dir+IMPACT_REL_PATH+"/bin:"
	+base_dir+IMPACT_REL_PATH+"/scripts:"
	+base_dir+IMPACT_REL_PATH+"/driver:"
	+base_dir+ELCOR_BIN_PATH+":"
	+base_dir+SIMU_BIN_PATH+":"
	+base_dir+"/trimaran/scripts:"
        +base_dir+GUI_BIN+":"
        +base_dir+TAS_BIN;

// when using mpi multiple processes, some addition paths can be added (see environment.h)
#ifdef EPIC_MPI
    path = MPI_EXPORT_PATH+path;
#endif
    setenv("PATH",path.c_str(),1);

    // setting library path for m5
    string ld_library_path; 
    if (getenv("LD_LIBRARY_PATH")!=NULL)
	ld_library_path += getenv("LD_LIBRARY_PATH");

    ld_library_path += +":"+base_dir+M5_ROOT+"/m5/build/ALPHA";
    setenv("LD_LIBRARY_PATH",ld_library_path.c_str(),1);
    //cout << "\n setting LD_LIBRARY_PATH to " << ld_library_path;

    //system("/usr/bin/env > STICAZZI.txt");

    string epic_dir = base_dir + "/trimaran-workspace/epic-explorer";

    if (chdir(epic_dir.c_str())==-1)
    {
	string logfile = base_path + string(EE_LOG_PATH);
	int myid = get_mpi_rank();
	write_to_log(myid,logfile,"FATAL ERROR: cannot find "+epic_dir);
	cout << "Cannot find " << epic_dir << endl;
	cout << "Please run ./post_install.sh script from epic explorer dir";
	exit(EXIT_FAILURE);
    }
}

//db
void Trimaran_interface::save_compiler_parameter(const Compiler& cmp, const string& path) const
{

  string filename = path;
  std::ofstream output_file(filename.c_str());

    string logfile = base_path + string(EE_LOG_PATH);
    int myid = get_mpi_rank();
  
   if (!output_file)
    {
	write_to_log(myid,logfile,"FATAL ERROR: cannot save compiler file " + filename);
	exit(EXIT_FAILURE);
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
	output_file << "\n// Compiler parameters ";
	output_file << "\n ----------------------------------";
	output_file << "\n$def !tcc_region:\t " << cmp.tcc_region.get_val();
	output_file << "\n\n";
	output_file << "\n// Impact Parameters ";	
	output_file << "\n ----------------------------------";	
	output_file << "\n$def !max_unroll_allowed:\t" << cmp.max_unroll_allowed.get_val();
	output_file << "\n$def !regroup_only:\t\t"<< cmp.regroup_only.get_val();
	output_file << "\n\n";
	output_file << "\n// Elcor Parameters ";		
	output_file << "\n ----------------------------------";	
	output_file << "\n$def !do_classic_opti:\t\t\t"<< cmp.do_classic_opti.get_val();
	output_file << "\n$def !do_prepass_scalar_scheduling:\t"<< cmp.do_prepass_scalar_scheduling.get_val();
	output_file << "\n$def !do_postpass_scalar_scheduling:\t"<< cmp.do_postpass_scalar_scheduling.get_val();
	output_file << "\n$def !do_modulo_scheduling:\t\t"<< cmp.do_modulo_scheduling.get_val();
	output_file << "\n$def !memvr_profiled:\t\t\t"<< cmp.memvr_profiled.get_val();
	}

}

void Trimaran_interface::load_compiler_parameter(Compiler* c, const string& filename) const
{
	std::ifstream input_file(filename.c_str());

    if (!input_file) 
    {
	string logfile = base_path + string(EE_LOG_PATH);
	int myid = get_mpi_rank();
	write_to_log(myid,logfile,"FATAL ERROR: cannot load compiler file " + filename);
	exit(EXIT_FAILURE);
    }
    else
    {
	int val;
	go_until("!tcc_region:",input_file);
	input_file >> val;
	c->tcc_region.set_val(val);

	go_until("!max_unroll_allowed:",input_file);
	input_file >> val;
	c->max_unroll_allowed.set_val(val);

	go_until("!regroup_only:",input_file);
	input_file >> val;
	c->regroup_only.set_val(val);

	go_until("!do_classic_opti:",input_file);
	input_file >> val;
	c->do_classic_opti.set_val(val);
	
	go_until("!do_prepass_scalar_scheduling:",input_file);
	input_file >> val;
	c->do_prepass_scalar_scheduling.set_val(val);
	
	go_until("!do_postpass_scalar_scheduling:",input_file);
	input_file >> val;
	c->do_postpass_scalar_scheduling.set_val(val);

	go_until("!do_modulo_scheduling:",input_file);
	input_file >> val;
	c->do_modulo_scheduling.set_val(val);

	go_until("!memvr_profiled:",input_file);
	input_file >> val;
	c->memvr_profiled.set_val(val);
    }
} //db

void Trimaran_interface::save_processor_config(const Processor& p, const string& path) const
{
    string filename = path;
    std::ofstream output_file(filename.c_str());

    if (!output_file) 
    {
	string logfile = base_path + string(EE_LOG_PATH);
	int myid = get_mpi_rank();
	write_to_log(myid,logfile,"FATAL ERROR: cannot save hmdes file " + filename);
	exit(EXIT_FAILURE);
    }
    else
    {
	// NOTE: 
	// - we assume that rotating and static portion of registers
	// have equal sizes.
	// - All the other parameters are fixed to their default values as specified
	// in the standard trimaran hmdes2 files. 

	output_file << "\n// DO NOT EDIT: this file is generated by epic explorer ";
	output_file << "\n// and should be included into the main hmdes2 machine ";
	output_file << "\n// description file using the $include directive";

	output_file << "\n\n";
	output_file << "\n$def !num_clusters\t" << p.num_clusters.get_val();
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

	output_file << "\n\n// Per-cluster functional units \n";

	output_file << "\n$def !integer_units\t" << p.integer_units.get_val();
	output_file << "\n$def !float_units\t" <<   p.float_units.get_val();
	output_file << "\n$def !memory_units\t" <<  p.memory_units.get_val();
	output_file << "\n$def !branch_units\t" <<  p.branch_units.get_val();
    }
}

//NOTE : This function only read from hmdes configuration file
//the parameter values that are subject of exploration.
void Trimaran_interface::load_processor_config(Processor* p,const string& filename) const
{
    std::ifstream input_file(filename.c_str());

    if (!input_file) {
	string logfile = base_path + string(EE_LOG_PATH);
	int myid = get_mpi_rank();
	write_to_log(myid,logfile,"FATAL ERROR: cannot open hmdes file " + filename);
	exit(EXIT_FAILURE);
    }
    else
    {
	int val;
	go_until("!num_clusters",input_file);
	input_file>>val;
	p->num_clusters.set_val(val);

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

	go_until("!branch_units",input_file);
	input_file>>val;
	p->branch_units.set_val(val);
    }
}

void Trimaran_interface::save_mem_config(const Mem_hierarchy& mem, const string& filename) const
{
    std::ofstream output_file(filename.c_str());

    if (!output_file) 
    {
	string logfile = base_path + string(EE_LOG_PATH);
	int myid = get_mpi_rank();
	write_to_log(myid,logfile,"FATAL ERROR: cannot save mem config " + filename);
	exit(EXIT_FAILURE);
    }
    else
    {
	setenv("M5_CONFIG_FILE",filename.c_str(),1);

	output_file << "\nfrom M5E_Cluster import *";
	output_file << "\n";
	output_file << "\n";
	output_file << "\nclass IL1Cache(BaseCache):";
	output_file << "\n	size = '"+to_string(mem.L1I.size.get_val())+"B'";
	output_file << "\n	assoc = "+to_string(mem.L1I.associativity.get_val());
	output_file << "\n	block_size = "+to_string(mem.L1I.block_size.get_val());
	output_file << "\n	mshrs = 32";
	output_file << "\n	tgts_per_mshr = 16";
	output_file << "\n	latency = 1 * Parent.clock.period";
	output_file << "\n	protocol = Parent.busProtocol";
	output_file << "\n";
	output_file << "\nclass DL1Cache(BaseCache):";
	output_file << "\n	size = '"+to_string(mem.L1D.size.get_val())+"B'";
	output_file << "\n	assoc = "+to_string(mem.L1D.associativity.get_val());
	output_file << "\n	block_size = "+to_string(mem.L1D.block_size.get_val());
	output_file << "\n	mshrs = 32";
	output_file << "\n	tgts_per_mshr = 16";
	output_file << "\n	latency = 1 * Parent.clock.period";
	output_file << "\n	protocol = Parent.busProtocol";
	output_file << "\n";
	output_file << "\nclass L2Cache(BaseCache):";
	output_file << "\n	size = '"+to_string(mem.L2U.size.get_val())+"B'";
	output_file << "\n	assoc = "+to_string(mem.L2U.associativity.get_val());
	output_file << "\n	block_size = "+to_string(mem.L2U.block_size.get_val());
	output_file << "\n	mshrs = 32";
	output_file << "\n	tgts_per_mshr = 16";
	output_file << "\n	latency = 10 * Parent.clock.period";
	output_file << "\n";
	output_file << "\nclass MyBaseMemory(BaseMemory):";
	output_file << "\n	latency = 100 * Parent.clock.period";
	output_file << "\n	snarf_updates = False";
	output_file << "\n	do_writes = True";
	output_file << "\n";
	output_file << "\nclass MyCluster(M5E_Cluster):";
	output_file << "\n	icache = IL1Cache(out_bus=Parent.toL2Bus)";
	output_file << "\n	dcache = DL1Cache(out_bus=Parent.toL2Bus)";
	output_file << "\n	main_mem = Parent.mainMem";
	output_file << "\n	max_loads = 5000000";
	output_file << "\n";
	output_file << "\nclass MyRoot(Root):";
	output_file << "\n    hier = HierParams(do_data=True, do_events=True)";
	output_file << "\n    toMemBus = Bus(width=128, clock=1 * Parent.clock.period)";
	output_file << "\n    toL2Bus = Bus(width=64, clock=1 * Parent.clock.period)";
	output_file << "\n    L2 = L2Cache(in_bus=Parent.toL2Bus, out_bus=Parent.toMemBus)";
	output_file << "\n    SDRAM = MyBaseMemory(in_bus=Parent.toMemBus)";
	output_file << "\n    mainMem = MainMemory()";
	output_file << "\n    cluster = MyCluster()";
	output_file << "\n    busProtocol = CoherenceProtocol(protocol='moesi')";
	output_file << "\n";
	output_file << "\nroot = MyRoot()";
	output_file << "\n#root.trace.flags = \"Bus Cache Event\"";
	output_file << "\nroot.cluster = [ MyCluster(cluster_id=i) for i in xrange(4) ] ";
    }
}

void Trimaran_interface::load_mem_config(Mem_hierarchy* mem,const string& filename) const
{
    std::ifstream input_file(filename.c_str());

    int value ;
    string word;
    int pos;

    if (!input_file) 
    {
	string logfile = base_path + string(EE_LOG_PATH);
	int myid = get_mpi_rank();
	write_to_log(myid,logfile,"FATAL ERROR: cannot load mem config " + filename);
	exit(EXIT_FAILURE);
    }
    else
    {   // TODO: handling even the 'kB' notation would be nicer
	
	// L1I

	setenv("M5_CONFIG_FILE",filename.c_str(),1);

	go_until("IL1Cache",input_file);
	go_until("size",input_file);
	input_file >> word;
	input_file >> word;

	pos = word.find("B'")-1;
	if (word.find("kB'")!=-1) 
	{
	    string logfile = base_path+string(EE_LOG_PATH);
	    int myid = get_mpi_rank();
	    write_to_log(myid,logfile,"FATAL ERROR: 'kB' postfix not supported in "+ filename+". Use only 'B'.");
	    cout << "\n Error: the 'kB' postfix in " << filename;
	    cout << "\n is currently not supported by epic explorer";
	    cout << "\n You must express cache size in bytes using 'B' postfix";
	    exit(EXIT_FAILURE);
	}
	value = atoi(word.substr(1,pos));
	mem->L1I.size.set_val(value);

	go_until("assoc",input_file);
	input_file >> word;
	input_file >> value;
	mem->L1I.associativity.set_val(value);

	go_until("block_size",input_file);
	input_file >> word;
	input_file >> value;
	mem->L1I.block_size.set_val(value);

	// L1D

	go_until("DL1Cache",input_file);
	go_until("size",input_file);
	input_file >> word;
	input_file >> word;

	pos = word.find("B'")-1;
	if (word.find("kB'")!=-1) 
	{
	    string logfile = base_path+string(EE_LOG_PATH);
	    int myid = get_mpi_rank();
	    write_to_log(myid,logfile,"FATAL ERROR: 'kB' postfix not supported in "+ filename+". Use only 'B'.");
	    cout << "\n Error: the 'kB' postfix in " << filename;
	    cout << "\n is currently not supported by epic explorer";
	    cout << "\n You must express cache size in bytes using 'B' postfix";
	    exit(EXIT_FAILURE);
	}
	value = atoi(word.substr(1,pos));
	mem->L1D.size.set_val(value);

	go_until("assoc",input_file);
	input_file >> word;
	input_file >> value;
	mem->L1D.associativity.set_val(value);

	go_until("block_size",input_file);
	input_file >> word;
	input_file >> value;
	mem->L1D.block_size.set_val(value);

	// L2U

	go_until("L2Cache",input_file);
	go_until("size",input_file);
	input_file >> word;
	input_file >> word;

	pos = word.find("B'")-1;
	if (word.find("kB'")!=-1) 
	{
	    string logfile = base_path+string(EE_LOG_PATH);
	    int myid = get_mpi_rank();
	    write_to_log(myid,logfile,"FATAL ERROR: 'kB' postfix not supported in "+ filename+". Use only 'B'.");
	    cout << "\n Error: the 'kB' postfix in " << filename;
	    cout << "\n is currently not supported by epic explorer";
	    cout << "\n You must express cache size in bytes using 'B' postfix";
	    exit(EXIT_FAILURE);
	}
	value = atoi(word.substr(1,pos));
	mem->L2U.size.set_val(value);

	go_until("assoc",input_file);
	input_file >> word;
	input_file >> value;
	mem->L2U.associativity.set_val(value);

	go_until("block_size",input_file);
	input_file >> word;
	input_file >> value;
	mem->L2U.block_size.set_val(value);
    }
}


