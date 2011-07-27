/***************************************************************************
                          user_interface.cpp  -  description
                             -------------------
    begin                : Sat Aug 24 2002
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

#include "user_interface.h"

#ifdef EPIC_MPI
#include "mpi.h"
#endif

// uncomment this line if you want interface to be verbose
//#define VERBOSE_INTERFACE

User_interface::User_interface(const string& path){

    base_path = path;
    string epic_path = base_path + "/trimaran-workspace/epic-explorer/";

    trimaran_interface = new Trimaran_interface(base_path);
    my_explorer = new Explorer(trimaran_interface);

    user_settings.default_settings_file = epic_path + "epic_default.conf";
    load_settings(user_settings.default_settings_file);
}

User_interface::~User_interface(){
}

void User_interface::interact(){
#ifdef EPIC_MPI
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    MPI_Comm_size(MPI_COMM_WORLD,&mysize);
    MPI_Request req;
    int chiudi = 1;
    
    if (myrank == 0) {
    	while (show_menu()!=113) ;
//G	show_menu();
	chiudi = 0;
	
	cout<<"\nProcessor "<<myrank<<" is exiting"<<endl;
	for (int p=1; p<mysize; p++) {
    		MPI_Isend(&chiudi,1,MPI_INT, p, 10, MPI_COMM_WORLD, &req); 
		MPI_Send(&chiudi,1,MPI_INT, p, 98, MPI_COMM_WORLD); 
	}		
    }
    else {
    	MPI_Irecv(&chiudi, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &req);
    	while (chiudi) my_explorer->simulate_space();
	cout<<"\nProcessor "<<myrank<<" is exiting"<<endl;
    }

#else
	myrank = 0;
	mysize = 1;
	while (show_menu()!=113);
#endif
		
}

void User_interface::start_exploration_message()
{
    cout << "\n Ok, you are ready to start exploration.";
    cout << "\n Results will be saved in the directory:\n";
    cout << base_path + "/trimaran-workspace/epic-explorer";
}


int User_interface::show_menu()
{

    char ch;
    char c;
    long double space_size;
    struct GA_parameters ga_parameters;

    if (myrank == 0) {
	system("clear");

	cout << "\n =====================================================";
	cout << "\n   e p i c   e x p l o r e r   [release "<< VERSION<<"]";
	cout << "\n _____________________________________________________";
	cout << "\n ______m a i n________________________________________" << endl;

	cout << "\n [b] - General Settings ";
	cout << "\n [c] - Design Space ";
	cout << "\n [h] - About ";
	cout << "\n [q] - Quit ";
	cout << "\n _____________________________________________________";
	cout << "\n ______s p a c e___e x p l o r a t i o n______________" << endl;

	cout << "\n [g] - Genetic based (GA)";
	cout << "\n [d] - Dependency-graph based (DEP)";
	cout << "\n [s] - Sensivity analysis (SAP)";
	cout << "\n [p] - Pareto-based sensitivity (PBSA)";
	cout << "\n [e] - Exhaustive (EXHA)";
	cout << "\n [r] - Random (RANDOM)";
	cout << "\n [x] - Schedule multiple explorations";
	cout << "\n [v] - Re-evaluate pareto";
	cout << "\n [t] - run Explorer::test() code";

	cout << "\n _____________________________________________________";
	cout << "\n _______m a n u a l___t e s t ________________________" << endl;
	cout << "\n [1] - Compile hmdes file";
	cout << "\n [2] - Compile benchmark";
	cout << "\n [3] - Execute benchmark";
	cout << "\n [4] - View execution statistics";
	cout << "\n [5] - Estimate objectives";
	cout << "\n [6] - Show system config";
	cout << "\n Make your choice >";

	//G    ch = 'r';
	cin >> ch;
    }    
    string start;

    if ( ch=='g' || ch=='d' || ch=='s' || ch=='w' || ch=='p' || ch=='e' || ch =='r' || ch=='v')
	    my_explorer->init_approximation();

    switch (ch)
    {
	case 'b':
	    if (myrank == 0) edit_user_settings();
	    break;

	case 'h':
	    if (myrank == 0) info();
	    if (myrank == 0) wait_key();
	    break;

	case 'c':
	    if (myrank == 0) edit_exploration_space();
	    break;

	case 'g': // mau

	    if (myrank == 0) {
		cout << "\n\n GA based approach ";
		cout << "\n-------------------------------";
		cout << "\n\n Enter random seed (0 = auto): ";
		cin >> ga_parameters.random_seed;
		if (ga_parameters.random_seed == 0)
		    ga_parameters.random_seed = rand()*rand(); //G this is not random
		else
		    srand(ga_parameters.random_seed);
		do 
		{
		    cout << "\n Please enter a value (divisible by 2) for Population size: ";
		    cin >> ga_parameters.population_size;
		}
		while (ga_parameters.population_size%2!=0);

		cout << " Crossover prob: ";
		cin >> ga_parameters.pcrossover;
		cout << " Mutation prob: ";
		cin >> ga_parameters.pmutation;
		cout << " Max Generations: ";
		cin >> ga_parameters.max_generations;
		cout << " Report pareto step: ";
		cin >> ga_parameters.report_pareto_step;

		start_exploration_message();
		cout << "\n\n Start exploration (y/n) ? ";
		cin >> ch;
		if (ch=='y')
		    my_explorer->start_GA(ga_parameters);
	    }
	    break;

	case 'd':
	    if (myrank == 0) {
		int graph;
		cout << "\n Dependency-Graph based approach ";
		cout << "\n ---------------------------------------------";
		cout << "\n (1) DEP\t(full graph) ";
		cout << "\n (2) DEP2\t(alternative graph)";
		cout << "\n (3) DEPMOD\t(simplified graph)";
		cout << "\n ---------------------------------------------";
		cout << "\n choose a graph: ";
		cin >> graph;

		if (graph==1) 
		    my_explorer->start_DEP();
		else if (graph==2) 
		    my_explorer->start_DEP2();
		else if (graph==3)
		    my_explorer->start_DEPMOD();
		else
		    cout << "\n Choice '"<< graph << "' not valid!";
	    }
	    break;

	case 's':
	    if (myrank == 0) {
		start_exploration_message();
		cout << "\n\n Start exploration (y/n) ? ";
		cin >> ch;
		if (ch=='y') my_explorer->start_SAP();
	    }
	    break;

	case 'w':
	    if (myrank == 0) {
		start_exploration_message();
		cout << "\n\n Start exploration (y/n) ? ";
		cin >> ch;
		if (ch=='y') my_explorer->start_SAPMOD();
	    }
	    break;
	case 'p':
	    if (myrank == 0) {
		start_exploration_message();
		cout << "\n\n Start exploration (y/n) ? ";
		cin >> ch;
		if (ch=='y') my_explorer->start_PBSA();
	    }
	    break;
	case 'x':
	    if (myrank == 0) {
		schedule_explorations();
	    }
	    break;

	case 'e':

	    if (myrank == 0) {
		start_exploration_message();

		cout << "\n\n WARNING: You are going to start an exhaustive exploration of all parameters .";
		cout << "\n Depending on range variation of space parameters simulation might take";
		cout << "\n too much time to be completed. ";
		space_size = my_explorer->get_space_size();
		cout << "\n Number of simulations to be performed: " << space_size;

		cout << "\n\n Proceed ? (y/n) ";

		cin >> c;

		if (c=='y') my_explorer->start_EXHA();
		wait_key();
	    }
	    break;

	case 'r':
	    if (myrank == 0) {
		start_exploration_message();
		int n;
		unsigned int seed;
		cout << "\n Number of  random simulations:";
		//G	    n = 10000;
		cin >> n;
		cout << "\n Enter random seed (0 = auto):";
		//G	    seed = 1962;
		cin >> seed;
		if (seed==0)
		    srand((unsigned int)time((time_t*)NULL));
		else
		    srand(seed);
		my_explorer->start_RAND(n);
		wait_key();
	    }
	    break;

	case 'v':
	    if (myrank == 0) {
		string command = "ls ";
		string pareto_path = base_path + "/trimaran-workspace/epic-explorer/";
		command+= pareto_path;
		command+= " | grep -e pareto -e history";
		bool done = false;
		string fpath;
		while(!done){
		    system(command.c_str());
		    cout << "\n\n Choose a pareto or history file: ";
		    string b;
		    cin >> b;
		    fpath = pareto_path + b;
		    ifstream infile(fpath.c_str());
		    done = infile.good();
		    if(!done)
			cout << "\n Could non open file " << fpath;
		    else
			cout << "\n Using file " << fpath;
		    infile.close();
		}
		start_exploration_message();
		cout << "\n\n Start exploration (y/n) ? ";
		cin >> ch;
		if (ch=='y')
		    my_explorer->start_REP(fpath);
		wait_key();
	    }
	    break;

	case 't':
	    if (myrank == 0) my_explorer->test();
	    if (myrank == 0) wait_key();
	    break;

	case '1':
	    if (myrank == 0) compile_hmdes_file();
	    if (myrank == 0) wait_key();
	    break;

	case '2':
	    if (myrank == 0) compile_benchmark();
	    if (myrank == 0) wait_key();
	    break;

	case '3':
	    if (myrank == 0) execute_benchmark();
	    if (myrank == 0) wait_key();
	    break;


	case '4':
	    if (myrank == 0) view_statistics();
	    if (myrank == 0) wait_key();
	    break;

	case '5':
	    if (myrank == 0) compute_cost();
	    if (myrank == 0) wait_key();
	    break;

	case '6':

	    if (myrank == 0) show_system_config();
	    if (myrank == 0) wait_key();
	    break;

	case 'q':
	    // will exit...
	    break;
	default:
	    cout << endl << "Not valid choice";
	    wait_key();

    }

    return ch;
}

void User_interface::edit_user_settings()
{
    string ch;
    do {
	system("clear");

#ifdef EPIC_MPI
        cout << "\n WARNING!";
	cout << "\n ----------------------------------------------------------";
        cout << "\n You are running an MPI enabled binary of epic explorer.";
        cout << "\n Current version does not support settings update to slaves precesses.";
        cout << "\n This means that every change you make here will NOT be used by the other";
        cout << "\n slave instancies of epic explorer.";
        cout << "\n In order to set properly your preferences for each process you should ";
        cout << "\n modify directly the file epic_default.conf and then relaunch the mpirun command";
        wait_key();

    if (!user_settings.save_restore)
    {
	cout << "\n WARNING !!!!";
	cout << "\n****************";
	cout << "\n EPIC_MPI compilation detected, save/restore simulations NOT enabled!";
	cout << "\n Currently, MPI works ONLY if save/restore simulations (multidir) otion is enabled.";
	cout << "\n SEE the VERY IMPORTANT NOTE on file MPI.README (section III).";
	wait_key();
    }
#endif


	cout << "\n  S e t t i n g s  ";
	cout << "\n ----------------------------------------------------------";
	cout << "\n  (1) - Objective Area               --> " << status_string(user_settings.objective_area);
	cout << "\n  (2) - Objective Execution Time     --> " << status_string(user_settings.objective_exec_time);
	cout << "\n  (3) - Objective Energy             --> " << status_string(user_settings.objective_energy);
	cout << "\n  (4) - Objective Average Power      --> " << status_string(user_settings.objective_power);
	cout << "\n  (5) - save simulated spaces        --> " << status_string(user_settings.save_spaces);
	cout << "\n  (7) - save Trimaran PD_TRACE files --> " << status_string(user_settings.save_PD_TRACE);
	cout << "\n  (8) - Save Trimaran tcc logs       --> " << status_string(user_settings.save_tcclog);
	cout << "\n  (9) - save estimation detail files --> " << status_string(user_settings.save_estimation);
	cout << "\n (10) - Benchmark                    --> " << trimaran_interface->get_benchmark_name();
	cout << "\n (11) - Automatic clock freq         --> " << status_string(user_settings.auto_clock);
	cout << "\n (12) - Approximation                --> " << user_settings.approx_settings.enabled;
	if (user_settings.approx_settings.enabled>0)
	{
	    cout << " ( " << user_settings.approx_settings.threshold << 
            " , " << user_settings.approx_settings.min << 
            " - " << user_settings.approx_settings.max << " )";
	}

	cout << "\n (13) - Save/Restore simulations     --> " << status_string(user_settings.save_restore);
	cout << "\n (14) - Multi benchmark simulation   --> " << status_string(user_settings.multibench);
	cout << "\n (15) - Continue on Failure          --> " << status_string(user_settings.continue_on_failure);
	if(user_settings.multibench){
	    cout << "\n Additional benchmarks:" << endl;
	    for(vector<string>::iterator it = user_settings.bench_v.begin(); it != user_settings.bench_v.end(); it++)
	        cout << " " << *it;
	    cout << endl;
	}
	cout << "\n (16) - Genetic Proportional Operators   --> " << status_string(user_settings.properators);
	cout << "\n" ;
	cout << "\n ----------------------------------------------------------";
	cout << "\n (s) - Save current settings";
	cout << "\n (q) - Return to main menu";
	cout << "\n ----------------------------------------------------------";
	cout << "\n Make your choice:";

	cin >> ch;

	if (ch=="1") user_settings.objective_area =   !user_settings.objective_area;
	if (ch=="2") user_settings.objective_exec_time = !user_settings.objective_exec_time;
	if (ch=="3") user_settings.objective_energy = !user_settings.objective_energy;
	if (ch=="4") user_settings.objective_power =  !user_settings.objective_power;
	if (ch=="5") user_settings.save_spaces = !user_settings.save_spaces;
	if (ch=="7") user_settings.save_PD_TRACE = !user_settings.save_PD_TRACE;
	if (ch=="8") 
	{
	    user_settings.save_tcclog = !user_settings.save_tcclog;
	    trimaran_interface->set_save_tcclog(user_settings.save_tcclog);
	}
	if (ch=="9") user_settings.save_estimation = !user_settings.save_estimation;
	if (ch=="10") choose_benchmark();
	if (ch=="11") user_settings.auto_clock = !user_settings.auto_clock;
	if (ch=="12") 
	{
	    cout << "\n Select Approximation method:";
	    cout << "\n (0) None";
	    cout << "\n (1) Fuzzy System";
	    cout << "\n (2) Artificial Neural Network";
	    cout << "\n select: ";
	    cin >> user_settings.approx_settings.enabled;

	    if (user_settings.approx_settings.enabled>0)
	    {
		cout << "\n Enter % error threshold : ";
		cin >> user_settings.approx_settings.threshold;
		cout << "\n Min number of simulations: ";
		cin >> user_settings.approx_settings.min;
		cout << "\n Max number of simulations: ";
		cin >> user_settings.approx_settings.max;   
	    }

	}
	if (ch=="13")
	{
	    cout << "\n--------------------------------------------------------------------";
	    cout << "\n IMPORTAT NOTE:";
	    cout << "\n Enabling save/restore simulations support will save the binary";
	    cout << "\n and statistics of each simulation in a different folder, under";
	    cout << "\n the trimaran-workspace/<benchmark name> directory." << endl;
	    cout << "\n This allows the avoidance of unnecessary compilations/simulations";
	    cout << "\n potentially increasing the speed of the exploration." << endl;
	    cout << "\n Althought useful, this could be very disk-space consuming, so";
	    cout << "\n be aware of this before performing your exploration." << endl;
	    cout << "\n A common way to reduce disk-space usage is to disable the saving of";
	    cout << "\n PD_TRACE files generated during simulation.";
	    cout << "\n--------------------------------------------------------------------";

	    wait_key();

	    user_settings.save_restore = !user_settings.save_restore;
	}

	if (ch=="14")
	{
            cout << "\n IMPORTANT: multi-benchmark support is EXPERIMENTAL ";
            cout << "\n this setting is not saved in the config file! \n\n";
	    int mbenable;
            cout << "\n (0) Disable multi-benchmark simulation";
            cout << "\n (1) Enable multi-benchmark simulation";
	    cout << "\n make your choice: ";
	    cin >> mbenable;
	    user_settings.multibench = (mbenable > 0)? true : false;
	    if(user_settings.multibench){
	        cout << "\n How many additional benchmarks do you want to execute?\n ";
	        int nbench;
	        cin >> nbench;

	        cout << "\n You must choose one of the benchmark available in the " << endl;
	        cout << " '" << base_path << "/trimaran/benchmarks/simple' directory " << endl << endl;
	        cout << " \tCurrently available benchmarks:" << endl;
	        cout << "-----------------------------------------------------------" << endl;
	        string command = "ls ";
	        command+= base_path + "/trimaran/benchmarks/simple";
	        system(command.c_str());

	        user_settings.bench_v.clear();
	        for(int i=0; i<nbench; i++){
	            cout << "\n Select benchmark " << i+1 << " of " << nbench << ":\n ";
	            string s;
                    cin >> s;
		    cout << " selected: " << s << endl;
                    user_settings.bench_v.push_back(s);
	        }
	    }
	}

	if (ch=="15")
	{
	    cout << "\n When Enabled, continue on failure allows epic explorer to continue ";
	    cout << "\n the design space exploration discarding configurations that caused ";
	    cout << "\n problems in trimaran (e.g. segfaults, corrupted statistics).";
	    cout << "\n When NOT enable, epic explorer will exit and exploration terminated.";
	    cout << "\n In both cases, the incidents will be reported in epic.log";
	    wait_key();
	    user_settings.continue_on_failure = !user_settings.continue_on_failure;
	    trimaran_interface->set_continue_on_failure(user_settings.continue_on_failure);
	}
	if (ch=="16") user_settings.properators = !user_settings.properators;

	if (ch=="s") save_settings_wrapper();

    } while(ch!="q");

    my_explorer->set_options(user_settings);
}

void User_interface::choose_benchmark() {

    cout << endl;
    cout << "IMPORTANT: if you want to add a benchmark package put it in the " << endl;
    cout << "'" << base_path << "/trimaran/benchmarks/simple' directory " << endl;
    cout << "\tCurrently available benchmarks:" << endl;
    cout << "-----------------------------------------------------------" << endl;

    string command = "ls ";
    command+= base_path + "/trimaran/benchmarks/simple";

    //sleep(5); // to avoid that 'system' execute before last cout has completed
    system(command.c_str());

    cout << "\n\n Choose a benchmark: ";

    string b;
    cin >> b;

    trimaran_interface->set_benchmark(b);
    user_settings.benchmark = b;

    my_explorer->set_options(user_settings);

    cout << "\n\n Ok, benchmark set to: " << trimaran_interface->get_benchmark_name();
    cout << "\n IMPORTANT: If you want to make permanent this change you must save ";
    cout << "\n user settings from main menu...";

}

inline string User_interface::status_string(bool b)
{
    if (b) return "ENABLED";
    return " --- ";
}


void User_interface::edit_exploration_space()
{
    char ch;
    do
    {
	system("clear");
	my_explorer->processor.num_clusters.set_to_first();
	my_explorer->processor.integer_units.set_to_first();
	my_explorer->processor.float_units.set_to_first();
	my_explorer->processor.branch_units.set_to_first();
	my_explorer->processor.memory_units.set_to_first();
	my_explorer->processor.gpr_static_size.set_to_first();
	my_explorer->processor.fpr_static_size.set_to_first();
	my_explorer->processor.pr_static_size.set_to_first();
	my_explorer->processor.cr_static_size.set_to_first();
	my_explorer->processor.btr_static_size.set_to_first();

	my_explorer->mem_hierarchy.L1D.size.set_to_first();
	my_explorer->mem_hierarchy.L1D.block_size.set_to_first();
	my_explorer->mem_hierarchy.L1D.associativity.set_to_first();

	my_explorer->mem_hierarchy.L1I.size.set_to_first();
	my_explorer->mem_hierarchy.L1I.block_size.set_to_first();
	my_explorer->mem_hierarchy.L1I.associativity.set_to_first();

	my_explorer->mem_hierarchy.L2U.size.set_to_first();
	my_explorer->mem_hierarchy.L2U.block_size.set_to_first();
	my_explorer->mem_hierarchy.L2U.associativity.set_to_first();

	my_explorer->compiler.tcc_region.set_to_first();	//db
	my_explorer->compiler.max_unroll_allowed.set_to_first();	//db
	my_explorer->compiler.regroup_only.set_to_first();	//db
	my_explorer->compiler.do_classic_opti.set_to_first();	//db
	my_explorer->compiler.do_prepass_scalar_scheduling.set_to_first();	//db
	my_explorer->compiler.do_postpass_scalar_scheduling.set_to_first();	//db
	my_explorer->compiler.do_modulo_scheduling.set_to_first();	//db
	my_explorer->compiler.memvr_profiled.set_to_first();	//db
	cout << "\n [1]           L1D size: " ;
	do { cout << my_explorer->mem_hierarchy.L1D.size.get_val() << ","; } while (my_explorer->mem_hierarchy.L1D.size.increase()); 
	cout << "\n [2]     L1D block_size: " ;
	do { cout << my_explorer->mem_hierarchy.L1D.block_size.get_val() << ","; } while (my_explorer->mem_hierarchy.L1D.block_size.increase());
	cout << "\n [3]  L1D associativity: " ;
	do { cout << my_explorer->mem_hierarchy.L1D.associativity.get_val() << ","; } while (my_explorer->mem_hierarchy.L1D.associativity.increase());
	cout << "\n [4]           L1I size: " ;
	do { cout << my_explorer->mem_hierarchy.L1I.size.get_val() << ","; } while (my_explorer->mem_hierarchy.L1I.size.increase());
	cout << "\n [5]     L1I block_size: " ;
	do { cout << my_explorer->mem_hierarchy.L1I.block_size.get_val() << ","; } while (my_explorer->mem_hierarchy.L1I.block_size.increase());
	cout << "\n [6]  L1I associativity: " ;
	do { cout << my_explorer->mem_hierarchy.L1I.associativity.get_val() << ","; } while (my_explorer->mem_hierarchy.L1I.associativity.increase());
	cout << "\n [7]           L2U size: " ;
	do { cout << my_explorer->mem_hierarchy.L2U.size.get_val() << ","; } while (my_explorer->mem_hierarchy.L2U.size.increase());
	cout << "\n [8]     L2U block_size: " ;
	do { cout << my_explorer->mem_hierarchy.L2U.block_size.get_val() << ","; } while (my_explorer->mem_hierarchy.L2U.block_size.increase());
	cout << "\n [9]  L2U associativity: " ;
	do { cout << my_explorer->mem_hierarchy.L2U.associativity.get_val() << ","; } while (my_explorer->mem_hierarchy.L2U.associativity.increase());
	cout << "\n [10]     integer_units: ";
	do { cout << my_explorer->processor.integer_units.get_val() << ","; } while (my_explorer->processor.integer_units.increase());
	cout << "\n [11]       float_units: ";
	do { cout << my_explorer->processor.float_units.get_val() << ","; } while (my_explorer->processor.float_units.increase());
	cout << "\n [12]      memory_units: ";
	do { cout << my_explorer->processor.memory_units.get_val() << ","; } while (my_explorer->processor.memory_units.increase());
	cout << "\n [13]      branch_units: ";
	do { cout << my_explorer->processor.branch_units.get_val() << ","; } while (my_explorer->processor.branch_units.increase());
	cout << "\n [14]   gpr_static_size: ";
	do { cout << my_explorer->processor.gpr_static_size.get_val() << ","; } while (my_explorer->processor.gpr_static_size.increase());
	cout << "\n [15]   fpr_static_size: ";
	do { cout << my_explorer->processor.fpr_static_size.get_val() << ","; } while (my_explorer->processor.fpr_static_size.increase());
	cout << "\n [16]    pr_static_size: ";
	do { cout << my_explorer->processor.pr_static_size.get_val() << ","; } while (my_explorer->processor.pr_static_size.increase());
	cout << "\n [17]    cr_static_size: ";
	do { cout << my_explorer->processor.cr_static_size.get_val() << ","; } while (my_explorer->processor.cr_static_size.increase());
	cout << "\n [18]   btr_static_size: ";
	do { cout << my_explorer->processor.btr_static_size.get_val() << ","; } while (my_explorer->processor.btr_static_size.increase());
	cout << "\n [19]   num_clusters: ";
	do { cout << my_explorer->processor.num_clusters.get_val() << ","; } while (my_explorer->processor.num_clusters.increase());
	cout << "\n [20]  tcc_region (1=b 2=h 3=s): ";	//db
	do { cout << my_explorer->compiler.tcc_region.get_val() << ","; } while (my_explorer->compiler.tcc_region.increase());	//db
	cout << "\n [21]  max_unroll_allowed: ";	//db
	do { cout << my_explorer->compiler.max_unroll_allowed.get_val() << ","; } while (my_explorer->compiler.max_unroll_allowed.increase());	//db
	cout << "\n [22]  regroup_only: ";	//db
	do { cout << noyes(my_explorer->compiler.regroup_only.get_val()) << ","; } while (my_explorer->compiler.regroup_only.increase());	//db
	cout << "\n [23]  do_classic_opti: ";	//db
	do { cout << noyes(my_explorer->compiler.do_classic_opti.get_val()) << ","; } while (my_explorer->compiler.do_classic_opti.increase());	//db
	cout << "\n [24]  do_prepass_scalar_scheduling: ";	//db
	do { cout << noyes(my_explorer->compiler.do_prepass_scalar_scheduling.get_val()) << ","; } //db
		while (my_explorer->compiler.do_prepass_scalar_scheduling.increase());	//db
	cout << "\n [25]  do_postpass_scalar_scheduling: ";	//db
	do { cout << noyes(my_explorer->compiler.do_postpass_scalar_scheduling.get_val()) << ","; } //db
		while (my_explorer->compiler.do_postpass_scalar_scheduling.increase());	//db
	cout << "\n [26]  do_modulo_scheduling: ";	//db
	do { cout << noyes(my_explorer->compiler.do_modulo_scheduling.get_val()) << ","; } //db
		while (my_explorer->compiler.do_modulo_scheduling.increase());	//db
	cout << "\n [27]  memvr_profiled: ";	//db
	do { cout << noyes(my_explorer->compiler.memvr_profiled.get_val()) << ","; } while (my_explorer->compiler.memvr_profiled.increase());	//db
	cout << "\n---------------------------------------------";
	cout << "\n Current space: " << my_explorer->get_space_name() << " [ total Space size: " << my_explorer->get_space_size()<<"]";
	cout << "\n (s) - Set space from file ";
	cout << "\n (x) - Exit to main menu";
	cout << "\n---------------------------------------------";
	cout << "\n choice: ";
	cin >> ch;

	if (ch=='s') set_subspace_wrapper();

    }
    while (ch!='x');

}

void User_interface::info()
{
    system("clear");

    cout << "\n     E p i c   E x p l o r e r  - release " << VERSION ;
    cout << "\n ******************************************************** ";
    cout << "\n Open Platform for Design Space Exploration of EPIC/VLIW architectures";
    cout << "\n";
    cout << "\n -------------------------------------------------------- ";
    cout << "\n Written by Davide Patti <dpatti@diit.unict.it>" << endl;
    cout << "\n Additional coding by Maurizio Palesi <mpalesi@diit.unict.it>";
    cout << "\n Fuzzy code by Alessandro Di Nuovo <adinuovo@diit.unict.it>";
    cout << "\n MPI GA Multi-Bench code by Gianmarco De Francisci Morales <gmorales@diit.unict.it>";
    cout << "\n -------------------------------------------------------- ";
    cout << "\n";
    cout << "\n For usage instructions see README file.";
    cout << "\n";
    cout << "\n For more informations on area, power, energy estimation models and";
    cout << "\n DSE algorithms implemented in Epic Explorer please refer to documentation";
    cout << "\n section in: ";
    cout << "\n             http://epic-explorer.sourceforge.net/ ";
    cout << "\n" ;
    cout << "\n Enjoy your exploration ;) !";
}

void User_interface::show_system_config() {

    system("clear");
    reload_system_config();
    cout << endl << " -----------------------------------------" << endl;
    cout << my_explorer->mem_hierarchy.L1I.label<< ": ";
    cout << my_explorer->mem_hierarchy.L1I.block_size.get_val() << "/";
    cout << my_explorer->mem_hierarchy.L1I.size.get_val() << "/";
    cout << my_explorer->mem_hierarchy.L1I.associativity.get_val();
    cout << "  (block/size/assoc)" << endl;
    
    cout << my_explorer->mem_hierarchy.L1D.label<< ": ";
    cout << my_explorer->mem_hierarchy.L1D.block_size.get_val() << "/";
    cout << my_explorer->mem_hierarchy.L1D.size.get_val() << "/";
    cout << my_explorer->mem_hierarchy.L1D.associativity.get_val();
    cout << "  (block/size/assoc)" << endl;

    cout << my_explorer->mem_hierarchy.L2U.label<< ": ";
    cout << my_explorer->mem_hierarchy.L2U.block_size.get_val() << "/";
    cout << my_explorer->mem_hierarchy.L2U.size.get_val() << "/";
    cout << my_explorer->mem_hierarchy.L2U.associativity.get_val();
    cout << "  (block/size/assoc)";

    cout <<"\n -----------------------------------------";
   cout<<"\n num_clusters:\t"<< my_explorer->processor.num_clusters.get_val();
   cout<<"\n integer_units:\t"<< my_explorer->processor.integer_units.get_val();
   cout<<"\n float_units:\t"<< my_explorer->processor.float_units.get_val();
   cout<<"\n memory_units:\t"<< my_explorer->processor.memory_units.get_val();
   cout<<"\n branch_units:\t"<< my_explorer->processor.branch_units.get_val();

   //cout<<"\n local_memory_units "<< my_explorer->processor.memory_units.get_val();

    cout <<"\n -----------------------------------------";
   cout<<"\n gpr_static_size:\t"<< my_explorer->processor.gpr_static_size.get_val();
   cout<<"\n fpr_static_size:\t"<< my_explorer->processor.fpr_static_size.get_val();
   cout<<"\n pr_static_size:\t"<< my_explorer->processor.pr_static_size.get_val();
   cout<<"\n cr_static_size:\t"<< my_explorer->processor.cr_static_size.get_val();
   cout<<"\n btr_static_size:\t"<< my_explorer->processor.btr_static_size.get_val();

    cout <<"\n -----------------------------------------";
    if (my_explorer->compiler.tcc_region.get_val()== 1) cout << "\n tcc_region: basic block" << endl; 	//db
    if (my_explorer->compiler.tcc_region.get_val()== 2) cout << "\n tcc_region: hyperblock" << endl; 	//db
    if (my_explorer->compiler.tcc_region.get_val()== 3) cout << "\n tcc_region: superblock" << endl;	//db
    // impact
    cout << "\n max_unroll_allowed: " << my_explorer->compiler.max_unroll_allowed.get_val();	//db
    cout << "\n regroup_only: " << noyes(my_explorer->compiler.regroup_only.get_val());
    // Elcor
    cout << "\n do_classic_opti: " << noyes(my_explorer->compiler.do_classic_opti.get_val());	//db 
    cout << "\n do_prepass_scalar_scheduling: " << noyes(my_explorer->compiler.do_prepass_scalar_scheduling.get_val());
    cout << "\n do_postpass_scalar_scheduling: " << noyes(my_explorer->compiler.do_postpass_scalar_scheduling.get_val());
    cout << "\n do_modulo_scheduling: " << noyes(my_explorer->compiler.do_modulo_scheduling.get_val());
    cout << "\n memvr_profiled: " << noyes(my_explorer->compiler.memvr_profiled.get_val());

}

void User_interface::set_subspace_wrapper()
{
   string subspaces_dir = base_path + "/trimaran-workspace/epic-explorer/SUBSPACES/";
   string filename;

   cout << "\n\n List of available subspace files: \n";
   string command = "ls "+subspaces_dir;
   system(command.c_str());
   cout << "\n Enter file name without path (e.g. 'my_space.sub' or  use 'x' to cancel): ";
   cin >> filename;
   if (filename!="x") 
   {
       cout << "\n Setting space " << filename << " as current subspace..." << endl;
       my_explorer->load_space_file(subspaces_dir+filename);
       my_explorer->set_space_name(filename);
       // A small workaround is required since 
       // link current.sub must be created without full path
       char old_path[200];
       getcwd(old_path,200);
       chdir(subspaces_dir.c_str());
       command = "ln -sf "+filename+" current.sub";
       system(command.c_str());
       chdir(old_path);
   }
}


void User_interface::load_settings(string settings_file)
{

   FILE * fp;
   fp = fopen(settings_file.c_str(),"r");

   // if there is no config file a default one is created
   if (fp==NULL)
   {
       cout << "\n WARNING:configuration file not found... creating it ";
       user_settings.benchmark = DEFAULT_BENCH;
       user_settings.objective_area = false;
       user_settings.objective_exec_time = true;
       user_settings.objective_energy = false;
       user_settings.objective_power = true;
       user_settings.save_spaces = false;
       user_settings.save_PD_TRACE = false;
       user_settings.save_estimation = false;
       user_settings.auto_clock = false;
       user_settings.approx_settings.enabled = 0;
       user_settings.approx_settings.threshold = 0;
       user_settings.approx_settings.min = 0;
       user_settings.approx_settings.max = 0;
       user_settings.save_restore = false;
       user_settings.save_tcclog = false;
       user_settings.continue_on_failure = false;
       user_settings.multibench = false;
       user_settings.properators = false;

       fp= fopen(settings_file.c_str(),"w");
       fclose(fp);
       save_settings(settings_file);
   }

   else fclose(fp);

   std::ifstream input_file (settings_file.c_str());

   if (!input_file)
   {
       cout << "\n Error while loading " << settings_file;
       wait_key();
   }
   else
   {
	user_settings.objective_area = false;
	user_settings.objective_exec_time = false;
	user_settings.objective_energy = false;
	user_settings.objective_power = false;
	user_settings.save_spaces = false;
	user_settings.save_estimation = false;
	user_settings.auto_clock = false;
	user_settings.save_tcclog = false;
	user_settings.save_PD_TRACE = false;
	user_settings.continue_on_failure = false;
	user_settings.save_restore = false;
	user_settings.properators = false;

	go_until("benchmark",input_file);
	input_file >> word;
	user_settings.benchmark = word;

	go_until("area",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.objective_area = true;

	go_until("cycles",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.objective_exec_time = true;

	go_until("energy",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.objective_energy = true;

	go_until("power",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.objective_power = true;

	go_until("save_spaces",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.save_spaces = true;

	go_until("save_PD_TRACE",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.save_PD_TRACE = true;

	go_until("save_estimation",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.save_estimation = true;

	go_until("AUTO_CLOCK",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.auto_clock = true;

	go_until("fuzzy_enabled",input_file);
	input_file >> user_settings.approx_settings.enabled;

	go_until("fuzzy_threshold",input_file);
	input_file >> user_settings.approx_settings.threshold;

	go_until("fuzzy_min",input_file);
	input_file >> user_settings.approx_settings.min;

	go_until("fuzzy_max",input_file);
	input_file >> user_settings.approx_settings.max;

	go_until("save_restore",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.save_restore = true;

	go_until("save_tcclog",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.save_tcclog= true;

	go_until("continue_on_failure",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.continue_on_failure= true;

	go_until("properators",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.properators= true;

	// not saved - must be enabled at runtime
        user_settings.multibench = false;
        
	trimaran_interface->set_benchmark(user_settings.benchmark);
	trimaran_interface->set_save_tcclog(user_settings.save_tcclog);
	trimaran_interface->set_continue_on_failure(user_settings.continue_on_failure);

	my_explorer->set_options(user_settings);
   }
}

void User_interface::save_settings_wrapper()
{
   string base_dir = base_path + "/trimaran-workspace/epic-explorer/";

   cout << "\n User settings file will be saved in: ";
   cout << "\n " << base_dir ; 

   save_settings(base_dir+"epic_default.conf");
}


void User_interface::save_settings(string settings_file)
{
    std::ofstream output_file(settings_file.c_str());

    if (!output_file)
    {
	cout << "\n Error while saving " << settings_file ;
	wait_key();
    }
    else
    {
	output_file << "\n\n# Epic Explorer settings";

	output_file << "\nbenchmark "  << user_settings.benchmark ;
	output_file << "\narea " << status_string(user_settings.objective_area);
	output_file << "\ncycles " << status_string(user_settings.objective_exec_time);
	output_file << "\nenergy " << status_string(user_settings.objective_energy);
	output_file << "\npower " << status_string(user_settings.objective_power);
	output_file << "\nsave_spaces " << status_string(user_settings.save_spaces);
	output_file << "\nsave_PD_TRACE " << status_string(user_settings.save_PD_TRACE);
	output_file << "\nsave_estimation " << status_string(user_settings.save_estimation);
	output_file << "\nAUTO_CLOCK " << status_string(user_settings.auto_clock);
	output_file << "\nfuzzy_enabled " << user_settings.approx_settings.enabled;
	output_file << "\nfuzzy_threshold " << user_settings.approx_settings.threshold;
	output_file << "\nfuzzy_min " << user_settings.approx_settings.min;
	output_file << "\nfuzzy_max " << user_settings.approx_settings.max;
	output_file << "\nsave_restore " << status_string(user_settings.save_restore);
	output_file << "\nsave_tcclog " << status_string(user_settings.save_tcclog);
	output_file << "\ncontinue_on_failure " << status_string(user_settings.continue_on_failure);
	output_file << "\nproperators " << status_string(user_settings.properators);

	cout << "\n Ok, saved current settings in " << settings_file;
    }
}

void User_interface::schedule_explorations()
{
    
    cout << "\n Ok, you must enter a string containing a character for each exploration";
    cout << "\n to be scheduled.";
    cout << "\n Legend:";
    cout << "\n g -> GA\n s -> SAP\n p -> PBSA\n r -> Random\n d -> DEP\n z -> DEP2\n m -> DEPMOD";
    cout << "\n\n Enter a schedule string:";
    string schedule_string;
    cin >> schedule_string;

    string::size_type ga_pos = schedule_string.find("g");
    string::size_type random_pos = schedule_string.find("r");

    struct GA_parameters ga_parameters;
    int n_random;

    if (ga_pos!=string::npos)
    {
	cout << "\n Schedule sequence contains GA . You must enter informations ";
	cout << "\n needed for this kind of exploration . ";

	cout << "\n\n GA based approach ";
	cout << "\n-------------------------------";
	cout << "\n\n Population size: ";
	cin >> ga_parameters.population_size;
	cout << " Crossover prob: ";
	cin >> ga_parameters.pcrossover;
	cout << " Mutation prob: ";
	cin >> ga_parameters.pmutation;
	cout << " Max Generations: ";
	cin >> ga_parameters.max_generations;
	cout << "\n Report pareto step: ";
	cin >> ga_parameters.report_pareto_step;
    }

    if (random_pos!=string::npos)
    {
	unsigned int seed;

	cout << "\n Schedule sequence contains RANDOM exploration.  ";
	cout << "\n How many random configuration must be explored ?";
	cin >> n_random;
	cout << "\n Enter random seed (0 = auto):";
	cin >> seed;
	if (seed==0)
	    srand((unsigned int)time((time_t*)NULL));
	else
	    srand(seed);
    }

    int i=0;

// NOT SUPPORTED forking code
// 
//    if (fork()==0) // child process must perform exploration
//    {
//	signal(SIGHUP,SIG_IGN); // must ignore parent termination
//	                        // useful to avoid abnormal
//				// exploration stop when executing
//				// from remoting shell
//
	while (i<schedule_string.size())
	{
	    char ch = schedule_string[i];

	    switch (ch)
	    {
		case 'd':
		    cout << "\n Executing DEP ";
		    my_explorer->start_DEP();
		    break;
		case 'z':
		    cout << "\n Executing DEP2 ";
		    my_explorer->start_DEP2();
		    break;
		case 'm':
		    cout << "\n Executing DEMOD ";
		    my_explorer->start_DEPMOD();
		    break;
		case 's':
		    cout << "\n Executing SAP ";
		    my_explorer->start_SAP();
		    break;
		case 'p':
		    cout << "\n Executing PBSA ";
		    my_explorer->start_PBSA();
		    break;
		case 'e':
		    cout << "\n Executing EXHAUSTIVE ";
		    my_explorer->start_EXHA();
		    break;
		case 'g':
		    cout << "\n Executing GA ";
		    my_explorer->start_GA(ga_parameters);
		    break;
		case 'r':
		    cout << "\n Executing RAND ";
		    my_explorer->start_RAND(n_random);
		    break;
		default:
		    cout << "\n Error in schedule string ";
		    wait_key();
		    break;
	    }

	    i++;
	}

//	exit(0);
//   }
//   cout << "\n Il padre continua come se nulla fosse ";
}


void User_interface::reload_system_config() 
{
    string filename = base_path+"/trimaran-workspace/epic-explorer/step_by_step/machines/"+EXPLORER_HMDES2;
    cout << "\n\n Loading processor configuration: " << filename;
    trimaran_interface->load_processor_config(&(my_explorer->processor),filename);

    string cache_config_file = base_path + "/trimaran-workspace/epic-explorer/step_by_step/m5elements/default.py";

    cout << "\n\n Loading mem configuration: " << cache_config_file;
    trimaran_interface->load_mem_config(&(my_explorer->mem_hierarchy),cache_config_file);

    string comp_filename = base_path+"/trimaran-workspace/epic-explorer/step_by_step/machines/compiler_param";	//db
    cout << "\n\n Loading compiler configuration: " << comp_filename;	//db
    trimaran_interface->load_compiler_parameter(&(my_explorer->compiler),comp_filename);	//db
}


void User_interface::compile_hmdes_file() {

    reload_system_config();
    string hmdes_dir = base_path + "/trimaran-workspace/epic-explorer/step_by_step/machines";
    trimaran_interface->compile_hmdes_file(hmdes_dir);
}

void User_interface::compile_benchmark() {
    reload_system_config();
    string base_dir = base_path + "/trimaran-workspace/epic-explorer/step_by_step";
    trimaran_interface->compile_benchmark(&(my_explorer->compiler),base_dir);	//db
}

void User_interface::execute_benchmark() {

    reload_system_config();
    string base_dir = base_path + "/trimaran-workspace/epic-explorer/step_by_step";
    trimaran_interface->execute_benchmark(&(my_explorer->compiler),base_dir,"simu_intermediate");
}


void User_interface::view_statistics() {

    system("clear");
    string simu_path = base_path + "/trimaran-workspace/epic-explorer/step_by_step/simu_intermediate/";
    Dynamic_stats dynamic_stats = trimaran_interface->get_dynamic_stats(simu_path);

    cout << "\n D y n a m i c  S t a t s ";
    cout << "\n------------------------------------------------";
    cout << "\n            Total cycles: " << dynamic_stats.total_cycles;
    cout << "\n          Compute cycles: " << dynamic_stats.compute_cycles;
    cout << "\n            Stall cycles: " << dynamic_stats.stall_cycles;
    cout << "\n       Total dynamic ops: " << dynamic_stats.total_dynamic_operations;
    cout << "\n       Average ops/cycle: " << dynamic_stats.average_issued_ops_total_cycles;
    cout << "\n------------------------------------------------";
    cout << "\n             Integer alu: " << dynamic_stats.ialu;
    cout << "\n               Float alu: " << dynamic_stats.falu;
    cout << "\n                    Load: " << dynamic_stats.load;
    cout << "\n                   Store: " << dynamic_stats.store;
    cout << "\n    Compare to predicate: " << dynamic_stats.cmp;
    cout << "\n       Prepare to branch: " << dynamic_stats.pbr;
    cout << "\n                Branches: " << dynamic_stats.branch;
    cout << "\n                     icm: " << dynamic_stats.icm;
    cout << "\n------------------------------------------------";
    cout << "\n     L1D (read hit/miss): "<< dynamic_stats.L1D_r_hit<<"/"<<dynamic_stats.L1D_r_miss;
    cout << "\n    L1D (write hit/miss): "<< dynamic_stats.L1D_w_hit<<"/"<<dynamic_stats.L1D_w_miss;
    cout << "\n     L1I (total fetches): " << dynamic_stats.L1I_fetches;
    cout << "\n          L1I (hit/miss): "<< dynamic_stats.L1I_hit <<"/"<< dynamic_stats.L1I_miss;
    cout << "\n          L2U (hit/miss): "<< dynamic_stats.L2U_hit<<"/" << dynamic_stats.L2U_miss;
}

void User_interface::compute_cost() {

    system("clear");

    string simu_path = base_path + "/trimaran-workspace/epic-explorer/step_by_step/simu_intermediate/";

    Dynamic_stats dynamic_stats = trimaran_interface->get_dynamic_stats(simu_path);
    Estimate estimate = my_explorer->estimator.get_estimate(dynamic_stats,my_explorer->mem_hierarchy,my_explorer->processor);

    cout << "\n  P e r f o r m a n c e ";
    cout << "\n ----------------------------------------------------";
    cout << "\n   Total cycles executed: " << estimate.execution_cycles;
    cout << "\n   Total execution time (msec): " << estimate.execution_time*1000;
    cout << "\n   L1D access time (ns): " << estimate.L1D_access_time*1e9;
    cout << "\n   L1I access time (ns): " << estimate.L1I_access_time*1e9;
    cout << "\n   clock freq (Mhz): " << estimate.clock_freq/1e6;
    cout << "\n   Average Ops/Cycle: "  << dynamic_stats.average_issued_ops_total_cycles;
    cout << "\n   L1D bus transition prob: " << estimate.L1D_transition_p;
    cout << "\n   L1I bus transition prob: " << estimate.L1I_transition_p << endl;

    cout << "\n  E n e r g y  &  P o w e r ";
    cout << "\n ---------------------------------------------------";
    cout << "\n   L1D Cache energy (mJ): " << estimate.L1D_energy*1000;
    cout << "\n   L1I Cache energy (mJ): " << estimate.L1I_energy*1000;
    cout << "\n   L2U Cache energy (mJ): " << estimate.L2U_energy*1000;
    cout << "\n   Total Cache energy (mJ) " << estimate.total_cache_energy*1000;
    cout << "\n   Main Memory energy (mJ): " << estimate.main_memory_energy*1000;
    cout << "\n   Processor energy (mJ): " << estimate.total_processor_energy*1000;
    cout << "\n   Total System Energy (mJ): "<< estimate.total_system_energy*1000;
    cout << "\n   Average System Power (W):"<< estimate.total_average_power << endl;

    cout << "\n  A r e a ";
    cout << "\n ---------------------------------------------------";
    cout << "\n   L1 Instruction cache area (cm^2): " << estimate.L1I_area;
    cout << "\n   L1 Data cache area (cm^2): " << estimate.L1D_area;
    cout << "\n   L2 Unified cache area (cm^2): " << estimate.L2U_area;
    cout << "\n   Processor core area (cm^2): " << estimate.processor_area;
    cout << "\n   Total System Area (cm^2 ): " << estimate.total_area;
}

