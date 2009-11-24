#include "explorer.h"
#include "common.h"

#ifdef EPIC_MPI
#include "mpi.h"
#endif

#include <ctime>
#include <values.h>
/*
{
if (myrank == 0) {
strcpy(message,"Hello, there");
MPI_Send(message, strlen(message), MPI_CHAR, 1, 99, MPI_COMM_WORLD);
}
else
{
MPI_Recv(message, 20, MPI_CHAR, 0, 99, MPI_COMM_WORLD, &status);
printf("received :%s:\n", message);
}
MPI_Finalize();
}
*/
//extern int global_argc;
//extern char** global_argv;
 
////////////////////////////////////////////////////////////////////////////


vector<Simulation> Explorer::simulate_loop(const vector<Configuration>& space)
{
    Simulation current_sim;
    vector<Simulation> simulations;

    static int n_simulate_space_call = 0;
    static int n_exploration_executed = 0; 

    // don't do simulation if a reliable fuzzy approximation is
    // enabled
    bool do_simulation = (force_simulation || 
	                 !(Options.approx_settings.enabled && function_approx->Reliable()));

    for (unsigned int i = 0;i< space.size();i++)
    {
	cout << endl;
	cout << "\n -------> E p i c  E x p l o r e r   [ simulation n." << i+1 << " / " << space.size() << " ]";
	cout << endl;

	processor.set_config(space[i]);
	mem_hierarchy.set_config(space[i]);
	compiler.set_config(space[i]);	//db
	current_sim.config = space[i];

	prepare_explorer(Options.benchmark,space[i]); //DDD

	if (do_simulation)
	{
	    int explorer_status = get_explorer_status(); //DDD

	    if (explorer_status != EXPLORER_ALL_DONE)
	    {
		if (explorer_status != EXPLORER_BINARY_DONE)
		{
		    trimaran_interface->save_processor_config(processor,hmdes_filename);
		    trimaran_interface->save_compiler_parameter(compiler,comp_filename);  //db
		    trimaran_interface->compile_hmdes_file(machine_dir);
		    trimaran_interface->compile_benchmark(&compiler,processor_dir);	//db
		}
		trimaran_interface->save_mem_config(mem_hierarchy,mem_hierarchy_filename);
		trimaran_interface->execute_benchmark(&compiler,processor_dir,cache_dir_name); //db
	    }

	    dyn_stats = trimaran_interface->get_dynamic_stats(mem_hierarchy_dir);

	    estimate = estimator.get_estimate(dyn_stats,mem_hierarchy,processor);
	    current_sim.area = estimate.total_area;
	    current_sim.exec_time = estimate.execution_time;
	    current_sim.clock_freq = estimate.clock_freq;
	    current_sim.simulated = true;//do_simulation;

	    if (Options.objective_energy) current_sim.energy = estimate.total_system_energy;
	    else if (Options.objective_power) current_sim.energy = estimate.total_average_power;
	    
	    if (Options.approx_settings.enabled>0) 
		    function_approx->Learn(space[i],current_sim,processor,mem_hierarchy);

	    if (!Options.multidir)
	    {
		string cmd = "rm -rf ";
		cmd += processor_dir;
		cout << EE_TAG << "Cleaning " << processor_dir;
		system(cmd.c_str());
	    }
	    else if (!Options.save_PD_TRACE)
	    {
		string cmd = "rm -f "+processor_dir+"/"+cache_dir_name+"/PD_TRACE";
		cout << EE_TAG << "Cleaning PD_TRACE in " << processor_dir << "/" << cache_dir_name;
		system(cmd.c_str());
	    }



	}

	/* TODO: re-enable
	else   // NOT do simulation...
	{   // using pproximation instead of simulation
	    assert(Options.approx_settings.enabled);

	    current_sim = function_approx->Estimate1(space[i],processor,mem_hierarchy);
	    current_sim.simulated = false;
	    current_sim.area = estimator.get_processor_area(processor);
	    
	}
	*/
	
	simulations.push_back(current_sim);

	// -------------------------------------------------------------------
	//  when doing simulation some interesting info can be optionally saved 
	if (do_simulation)
	{
	    if (Options.save_spaces)
	    {
		n_simulate_space_call++;
		if (get_sim_counter()==0)
		{
		    n_simulate_space_call=1;
		    n_exploration_executed++;
		}
		// epic_space_EXP_2.12, 12th explorated space of 2nd exploration algorithm
		char name[40];
		sprintf(name,"_simulatedspace_%d",n_simulate_space_call);
		string filename = Options.benchmark+"_"+current_algo+"_"+current_space+string(name);
		save_configurations(space,filename);
	    } 
	    if (Options.save_PD_STATS)  // trimaran PD_STATS file report
	    {
		// TODO: fix this 
		assert(false);
		string command;
		char temp[10];
		sprintf(temp,"%d",i);

		command = "cp "+mem_hierarchy_dir+"/PD_STATS* "+mem_hierarchy_dir+"/PD_STATS_"+string(temp);

		system(command.c_str());
	    }

	    if (Options.save_estimation) // detailed and verbose estimator report
	    {
		char temp[10];
		sprintf(temp,"%d",i);
		string filename= Options.benchmark+"_"+current_algo+"_"+current_space+"."+string(temp)+".est";
		save_estimation_file(dyn_stats,estimate,processor, mem_hierarchy,compiler,filename);	//db
	    }

	    if (Options.save_objectives_details) // 
	    {
		string filename= Options.benchmark+"_"+current_algo+"_"+current_space+".details";
		save_objectives_details(dyn_stats,current_sim.config,filename);
	    }
	    // -------------------------------------------------------------------
	}

    } // end for loop

    return simulations;
}


vector<Simulation> Explorer::simulate_space(const vector<Configuration>& space)
{

    int myrank;
    int mysize;

    vector<Simulation> simulations;
    vector<Configuration> space2;

#ifdef EPIC_MPI
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    MPI_Comm_size(MPI_COMM_WORLD,&mysize);
#else
    myrank = 0;
    mysize = 1;
#endif


    //  main exploration loop
    // ********************************************************

    int communicator[N_PARAMS]; //db
    int counter = 0;
    int counter2 = 0;

    int committed = 0;
    int rest = 0;

    committed = space.size()/mysize;
    rest = space.size() - (committed*mysize);
    vector<int> to_sim;

    for (int p = 0; p < mysize; p++) {

	if (p < rest) 
	    to_sim.push_back(committed+1);
	else
	    to_sim.push_back(committed);
    }	

#ifdef EPIC_MPI
    counter2 = to_sim[0];

    for (int p = 1; p < mysize; p++) {

	counter = 0;
	for (int s = counter2; s < counter2+to_sim[p]; s++) {
	    space2.push_back(space[s]);
	}
	counter = space2.size();
	MPI_Send(&counter, 1, MPI_INT, p, 98, MPI_COMM_WORLD);

	int bench_len = Options.benchmark.length() + 1; // null character terminated string
	MPI_Send(&bench_len, 1, MPI_INT, p, 95, MPI_COMM_WORLD);
	char* bench_cstr = new char[bench_len];
	strcpy(bench_cstr, Options.benchmark.c_str());
	MPI_Send(bench_cstr, bench_len, MPI_CHAR, p, 96, MPI_COMM_WORLD);

	for (int s = 0; s < counter; s++) {
	    communicator[0] = space2[s].L1D_block;
	    communicator[1] = space2[s].L1D_size;
	    communicator[2] = space2[s].L1D_assoc;
	    communicator[3] = space2[s].L1I_block;
	    communicator[4] = space2[s].L1I_size;
	    communicator[5] = space2[s].L1I_assoc;
	    communicator[6] = space2[s].L2U_block;
	    communicator[7] = space2[s].L2U_size;
	    communicator[8] = space2[s].L2U_assoc;
	    communicator[9] = space2[s].integer_units; 
	    communicator[10] = space2[s].float_units; 
	    communicator[11] = space2[s].memory_units; 
	    communicator[12] = space2[s].branch_units;
	    communicator[13] = space2[s].gpr_static_size;
	    communicator[14] = space2[s].fpr_static_size;
	    communicator[15] = space2[s].cr_static_size;
	    communicator[16] = space2[s].pr_static_size;
	    communicator[17] = space2[s].btr_static_size;
	    communicator[18] = space2[s].num_clusters;
	    communicator[19] = space2[s].tcc_region;	//db
	    communicator[20] = space2[s].max_unroll_allowed; //db
	    communicator[21] = space2[s].regroup_only;	//db
	    communicator[22] = space2[s].do_classic_opti;	//db
	    communicator[23] = space2[s].do_prepass_scalar_scheduling;	//db
	    communicator[24] = space2[s].do_postpass_scalar_scheduling;	//db
	    communicator[25] = space2[s].do_modulo_scheduling;	//db
	    communicator[26] = space2[s].memvr_profiled;	//db
	    MPI_Send(communicator, N_PARAMS, MPI_INT, p, 99, MPI_COMM_WORLD);
	}
	space2.clear();
	counter2 += counter;
    }
#endif

    for (int s = 0; s < to_sim[0]; s++) {
	space2.push_back(space[s]);
    }
    counter = space2.size();

    // Main simulation loop
    simulations = simulate_loop(space2);

    space2.clear();

#ifdef EPIC_MPI
    cout<<"\nParallel Execution of "<<counter<<" simulations was completed by processor "<<myrank<<endl;

    double comms[4];
    counter2 = to_sim[0];
    Simulation current_sim;

    for(int p = 1; p<mysize; p++) 
    {
	for (int s = counter2; s < counter2+to_sim[p]; s++) 
	    space2.push_back(space[s]); 

	if (space2.size() > 0) 
	    MPI_Recv(&counter, 1, MPI_INT, p, 97, MPI_COMM_WORLD, &status);
	else counter = 0;

	for(int s = 0; s < counter; s++) {
	    MPI_Recv(comms, 4, MPI_DOUBLE, p, 99, MPI_COMM_WORLD, &status); 
	    current_sim.config = space2[s];
	    current_sim.area = comms[0];
	    current_sim.exec_time = comms[1];
	    current_sim.energy = comms[2];
	    current_sim.clock_freq = comms[3]; //dovrebbe essere sempre la stessa...
	    current_sim.simulated = true;//do_simulation;

	    simulations.push_back(current_sim);

	    /* TODO: re-enable
	    if (Options.approx_settings.enabled>0) 
	    {
		processor.set_config(space2[s]);
		mem_hierarchy.set_config(space2[s]); 
		compiler.set_config(space2[s]);	//db
		function_approx->Learn(space2[s],current_sim,processor,mem_hierarchy);
	    }
	    */
	}
	space2.clear();
	counter2 += counter;
    }
#else
    cout<<"\nExecution of "<<counter<<" simulations was completed by processor "<<myrank<<endl;
#endif

    sim_counter+=simulations.size();

    // update current simulated space and benchmark
    previous_simulations.clear();
    append_simulations(previous_simulations,simulations);
    previous_benchmark = Options.benchmark;

    return simulations;
}


#ifdef EPIC_MPI
int Explorer::simulate_space()
{
    // questo medoto viene lanciato sugli altri processori e si interfaccia con la simulate space classica 

    vector<Simulation> simulations;
    vector<Configuration> space;
    
    int communicator[N_PARAMS];  //db
    Configuration tmp;
    MPI_Status status;
    int counter = 0;
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

    MPI_Recv(&counter, 1, MPI_INT, 0, 98, MPI_COMM_WORLD, &status);
    
    if (counter == 0) 
    	return (0);

    int bench_len;
    MPI_Recv(&bench_len,1,MPI_INT,0,95,MPI_COMM_WORLD,&status);
    char bench_cstr[bench_len];
    MPI_Recv(bench_cstr,bench_len,MPI_CHAR,0,96,MPI_COMM_WORLD,&status);
    string bench(bench_cstr);
    Options.benchmark = bench;
    trimaran_interface->set_benchmark(Options.benchmark);

    for(int i = 0; i<counter; i++) {
    	MPI_Recv(communicator,N_PARAMS,MPI_INT,0,99,MPI_COMM_WORLD,&status); //db
        tmp.L1D_block = communicator[0];
	tmp.L1D_size = communicator[1];
	tmp.L1D_assoc = communicator[2];
	tmp.L1I_block = communicator[3];
	tmp.L1I_size = communicator[4];
	tmp.L1I_assoc = communicator[5];
	tmp.L2U_block = communicator[6];
	tmp.L2U_size = communicator[7];
	tmp.L2U_assoc = communicator[8];
	tmp.integer_units = communicator[9]; 
	tmp.float_units = communicator[10]; 
	tmp.memory_units = communicator[11]; 
	tmp.branch_units = communicator[12];
  	tmp.gpr_static_size = communicator[13];
        tmp.fpr_static_size = communicator[14];
        tmp.cr_static_size = communicator[15];
        tmp.pr_static_size = communicator[16];
        tmp.btr_static_size = communicator[17];
        tmp.num_clusters = communicator[18];
	tmp.tcc_region = communicator[19];	//db
        tmp.max_unroll_allowed = communicator[20]; //db
	tmp.regroup_only = communicator[21];	//db
	tmp.do_classic_opti = communicator[22];	//db
	tmp.do_prepass_scalar_scheduling = communicator[23];	//db
	tmp.do_postpass_scalar_scheduling = communicator[24];	//db
	tmp.do_modulo_scheduling = communicator[25];	//db
	tmp.memvr_profiled = communicator[26];	//db
 	space.push_back(tmp);
    }

    //  main exploration loop
    // *********************************************************

    simulations = simulate_loop(space);

    cout<<"\nParallel Execution of "<<counter<<" simulations was completed by processor "<<myrank<<endl;
	
    //MPI_Finalize();
    double comms[4];
    MPI_Send(&counter,1,MPI_INT,0,97, MPI_COMM_WORLD);
    for (int i=0; i<counter; i++) {
    	comms[0] = simulations[i].area;
	comms[1] = simulations[i].exec_time;
	comms[2] = simulations[i].energy;
    	comms[3] = simulations[i].clock_freq;
	MPI_Send(comms,4,MPI_DOUBLE,0,99, MPI_COMM_WORLD);
    }
    
    return (1);
}
#endif
