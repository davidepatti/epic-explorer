/***************************************************************************
                          user_interface.cpp  -  description
                             -------------------
    begin                : Sat Aug 24 2002
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

#include "user_interface.h"

// uncomment this line if you want interface to be verbose
//#define VERBOSE_INTERFACE

User_interface::User_interface(const string& dir){

    base_dir = dir;
    string epic_path = base_dir+"/trimaran-workspace/epic-explorer/";

    trimaran_interface = new Trimaran_interface(base_dir);
    my_explorer = new Explorer(trimaran_interface);

    user_settings.default_settings_file = epic_path + "epic_default.conf";
    load_settings(user_settings.default_settings_file);
}

User_interface::~User_interface(){
}

void User_interface::interact(){

    while (show_menu()!=113);
}

void User_interface::start_exploration_message()
{
    cout << "\n Ok, you are ready to start exploration.";
    cout << "\n Results will be saved in the directory:\n";
    cout << trimaran_interface->get_base_dir()+"/trimaran-workspace/epic-explorer";
}


int User_interface::show_menu()
{
    system("clear");

    cout << "\n       =======================================================";
    cout << "\n            E p i c   E x p l o r e r   (v. "<< VERSION << ")";
    cout << "\n       =======================================================";
    cout << "\n\n     G e n e r a l  ";
    cout << "\n ------------------------------------------------------------------";
    cout << "\n [b] - Edit user settings ";
    cout << "\n [c] - Edit exploration space ";
    cout << "\n [h] - Info/Help ";
    cout << "\n";
    cout << "\n     D e s i g n   S p a c e   E x p l o r a t i o n ";
    cout << "\n ------------------------------------------------------------------";
    cout << "\n [x] - Schedule Multiple Explorations"; 
    cout << "\n ";
    cout << "\n [g] - GA\t(genetic based)";
    cout << "\n [d] - DEP\t(dependency graph)";
    cout << "\n [z] - DEP2\t(alternative graph)";
    cout << "\n [m] - DEPMOD\t(simplified graph)";
    cout << "\n [s] - SAP\t(mono-objective sensivity analysis)";
    cout << "\n [p] - PBSA\t(pareto-based sensitivity analysis)";
    cout << "\n [e] - Exhaustive exploration";
    cout << "\n [r] - Random exploration ";
    cout << "\n [t] - Explorer::test() code";

    cout << "\n\n     M a n u a l   S t e p s ";
    cout << "\n -------------------------------------------------------------------";
    cout << "\n [1] - Compile hmdes file         [6] - View processor config";
    cout << "\n [2] - Compile benchmark          [7] - View mem config";
    cout << "\n [3] - Execute benchmark          [8] - Reload processor config";
    cout << "\n [4] - View execution statistics  [9] - Reload mem config";
    cout << "\n [5] - Estimate objectives        [0] - Save current machine config"; 
    cout << "\n -------------------------------------------------------------------";
    cout << "\n [q] - Quit ";
    cout << "\n\n Make your choice:";
    char ch;
    char c;
    int space_size;
    struct GA_parameters ga_parameters;

    cin >> ch;

    string start;

    switch (ch)
    {
	case 'b':
	    edit_user_settings();
	    break;

	case 'h':
	    info();
	    wait_key();
	    break;

	case 'c':
	    edit_exploration_space();
	    break;

	case 'x':
	    schedule_explorations();
	    cout << "\n\n End of scheduled explorations . ";
	    wait_key();
	    break;

    case 'g': // mau

	    cout << "\n\n GA based approach ";
	    cout << "\n-------------------------------";
	    cout << "\n\n Enter random seed (0 = auto): ";
	    cin >> ga_parameters.random_seed;
	    if (ga_parameters.random_seed == 0) ga_parameters.random_seed = rand();
	    originalSeed = Seed = ga_parameters.random_seed;
	    cout << "\n\n Population size: ";
	    cin >> ga_parameters.population_size;
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
	    break;

	case 'd':
	    cout << "\n\n Start exploration (y/n) ? ";
	    cin >> ch;
	    if (ch=='y') my_explorer->start_DEP();
	    break;

	case 'z':
	    start_exploration_message();
	    cout << "\n\n Start exploration (y/n) ? ";
	    cin >> ch;
	    if (ch=='y') my_explorer->start_DEP2();
	    break;

	case 'm':
	    start_exploration_message();
	    cout << "\n\n Start exploration (y/n) ? ";
	    cin >> ch;
	    if (ch=='y')
	    my_explorer->start_DEPMOD();
	    break;

	case 's':
	    start_exploration_message();
	    cout << "\n\n Start exploration (y/n) ? ";
	    cin >> ch;
	    if (ch=='y') my_explorer->start_SAP();
	    break;

	case 'w':
	    start_exploration_message();
	    cout << "\n\n Start exploration (y/n) ? ";
	    cin >> ch;
	    if (ch=='y') my_explorer->start_SAPMOD();
	    break;

	case 'p':
	    start_exploration_message();
	    cout << "\n\n Start exploration (y/n) ? ";
	    cin >> ch;
	    if (ch=='y') my_explorer->start_PBSA();
	    break;


	case 'e':

	    start_exploration_message();

	    cout << "\n\n WARNING: You are going to start an exhaustive exploration of all parameters .";
	    cout << "\n Depending on range variation of space parameters simulation might take";
	    cout << "\n too much time to be completed. ";
	    space_size = (int)my_explorer->get_space_size();
	    cout << "\n Number of simulations to be performed: " << space_size;
	    cout << "\n Estimated time: " << (space_size*10)/60 << " minutes ";

	    cout << "\n\n Proceed ? (y/n) ";

	    cin >> c;

	    if (c=='y')
	    {
		my_explorer->start_EXHA();
	    }
	    wait_key();
	    break;

	case 'r':
	    start_exploration_message();
	    int n;
	    unsigned int seed;
	    cout << "\n Number of  random simulations :";
	    cin >> n;
	    cout << "\n Enter random seed (0 = auto):";
	    cin >> seed;
	    if (seed==0)
		srand((unsigned int)time((time_t*)NULL));
	    else
		srand(seed);
	    my_explorer->start_RAND(n);
	    wait_key();
	    break;

	case 't':
	    my_explorer->test();
	    wait_key();
	    break;

	case '1':
	    compile_hmdes_file();
	    wait_key();
	    break;

	case '2':
	    compile_benchmark();
	    wait_key();
	    break;

	case '3':
	    execute_benchmark();
	    wait_key();
	    break;


	case '4':
	    view_statistics();
	    wait_key();
	    break;

	case '5':
	    compute_cost();
	    wait_key();
	    break;

	case '6':

	    view_processor_config();
	    wait_key();
	    break;

	case '7':
	    view_cache_config();
	    wait_key();
	    break;

	case '8':

	    reload_hmdes_file();
	    wait_key();
	    break;
	case '9':

	    reload_memhierarchy_config();
	    wait_key();
	    break;
	case '0':
	    save_processor_config();
	    save_cache_config();
	    wait_key();
	    break;

    }

    return ch;
}

void User_interface::edit_user_settings()
{
    string ch;
    do {
	system("clear");
	cout << "\n  OPTION                                 CURRENT VALUE ";
	cout << "\n ----------------------------------------------------------";
	cout << "\n  (0) - Hyperblock formation         --> " << status_string(user_settings.hyperblock);
	cout << "\n  (1) - Objective Area               --> " << status_string(user_settings.objective_area);
	cout << "\n  (2) - Objective Execution Time     --> " << status_string(user_settings.objective_exec_time);
	cout << "\n  (3) - Objective Energy             --> " << status_string(user_settings.objective_energy);
	cout << "\n  (4) - Objective Average Power      --> " << status_string(user_settings.objective_power);
	cout << "\n" ;
	cout << "\n  (5) - save simulated spaces        --> " << status_string(user_settings.save_spaces);
	cout << "\n  (6) - save Trimaran PD_STATS files --> " << status_string(user_settings.save_PD_STATS);
	cout << "\n  (7) - save estimation detail files --> " << status_string(user_settings.save_estimation);
	cout << "\n  (8) - Benchmark                    --> " << trimaran_interface->get_benchmark_name();
	cout << "\n  (9) - Automatic clock freq         --> " << status_string(user_settings.auto_clock);
	cout << "\n (10) - Fuzzy simulation             --> " << user_settings.fuzzy_settings.enabled;
	if (user_settings.fuzzy_settings.enabled>0)
	{
	    cout << " ( " << user_settings.fuzzy_settings.threshold << " , " << user_settings.fuzzy_settings.min << " - " << user_settings.fuzzy_settings.max << " )";
	}

	cout << "\n ----------------------------------------------------------";
	cout << "\n\n (s) - Save current settings to file";
	cout << "\n (l) - Load settings from file";
	cout << "\n (q) - Quit to main menu";
	cout << "\n ----------------------------------------------------------";

	cout << "\n\n Make a choice : ";
	cin >> ch;

	if (ch=="0") 
	{
	    user_settings.hyperblock = !user_settings.hyperblock;
	    trimaran_interface->set_hyperblock(user_settings.hyperblock);
	}
	if (ch=="1") user_settings.objective_area =   !user_settings.objective_area;
	if (ch=="2") user_settings.objective_exec_time = !user_settings.objective_exec_time;
	if (ch=="3") user_settings.objective_energy = !user_settings.objective_energy;
	if (ch=="4") user_settings.objective_power =  !user_settings.objective_power;
	if (ch=="5") user_settings.save_spaces = !user_settings.save_spaces;
	if (ch=="6") user_settings.save_PD_STATS = !user_settings.save_PD_STATS;
	if (ch=="7") user_settings.save_estimation = !user_settings.save_estimation;
	if (ch=="8") choose_benchmark();
	if (ch=="9") user_settings.auto_clock = !user_settings.auto_clock;
	if (ch=="10") 
	{
	    cout << "\n Select fuzzy approximation:";
	    cout << "\n (0) None";
	    cout << "\n (1) Single Layer";
	    cout << "\n (2) Hierarchical";
	    cout << "\n select: ";
	    cin >> user_settings.fuzzy_settings.enabled;

	    if (user_settings.fuzzy_settings.enabled==2) 
	    {
		cout << "\n Sorry, Hierarchical fuzzy approx not yet supported!";
		cout << "\n Enabling Single Layer approximation";
		wait_key();
		user_settings.fuzzy_settings.enabled=1;
	    }
	    if (user_settings.fuzzy_settings.enabled>0)
	    {
		cout << "\n Enter % error threshold : ";
		cin >> user_settings.fuzzy_settings.threshold;
		cout << "\n Min number of simulations: ";
		cin >> user_settings.fuzzy_settings.min;
		cout << "\n Max number of simulations: ";
		cin >> user_settings.fuzzy_settings.max;
	    }

	}


	if (ch=="s") save_settings_wrapper();
	if (ch=="l") load_settings_wrapper();

    } while(ch!="q");

    my_explorer->set_options(user_settings);
}

void User_interface::choose_benchmark() {

    cout << "\n\n  You must choose one of the benchmark available in the ";
    cout << "\n  $HOME/trimaran/benchmarks directory. ";
    cout << "\n\n Currently available benchmarks :";
    cout << "\n-----------------------------------------------------------";

    string command = "ls ";
    command+=trimaran_interface->get_base_dir()+"/trimaran/benchmarks";

    //sleep(5); // to avoid that 'system' execute before last cout has completed
    system(command.c_str());

    cout << "\n\n  benchmark name : ";

    string b;
    cin >> b;

    trimaran_interface->set_benchmark(b);
    user_settings.benchmark = b;

    my_explorer->set_options(user_settings);

    cout << "\n\n benchmark set to : " << trimaran_interface->get_benchmark_name();
    cout << "\n IMPORTANT : If you want to make permanent this change you must save ";
    cout << "\n user settings from main menu...";

}

inline string User_interface::status_string(bool b)
{
    if (b) return "ENABLED";
    return "DISABLED";
}


void User_interface::edit_exploration_space()
{

    char ch;
    do
    {
	system("clear");
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

	cout << "\n Mem hierarchy parameters ";
	cout << "\n---------------------------------------";

	cout << "\n [1]           L1D size : " ;
	do { cout << my_explorer->mem_hierarchy.L1D.size.get_val() << ","; } while (my_explorer->mem_hierarchy.L1D.size.increase()); 
	cout << "\n [2]     L1D block_size : " ;
	do { cout << my_explorer->mem_hierarchy.L1D.block_size.get_val() << ","; } while (my_explorer->mem_hierarchy.L1D.block_size.increase());
	cout << "\n [3]  L1D associativity : " ;
	do { cout << my_explorer->mem_hierarchy.L1D.associativity.get_val() << ","; } while (my_explorer->mem_hierarchy.L1D.associativity.increase());

	cout << "\n [4]           L1I size : " ;
	do { cout << my_explorer->mem_hierarchy.L1I.size.get_val() << ","; } while (my_explorer->mem_hierarchy.L1I.size.increase());
	cout << "\n [5]     L1I block_size : " ;
	do { cout << my_explorer->mem_hierarchy.L1I.block_size.get_val() << ","; } while (my_explorer->mem_hierarchy.L1I.block_size.increase());
	cout << "\n [6]  L1I associativity : " ;
	do { cout << my_explorer->mem_hierarchy.L1I.associativity.get_val() << ","; } while (my_explorer->mem_hierarchy.L1I.associativity.increase());
	
	cout << "\n [7]           L2U size : " ;
	do { cout << my_explorer->mem_hierarchy.L2U.size.get_val() << ","; } while (my_explorer->mem_hierarchy.L2U.size.increase());
	cout << "\n [8]     L2U block_size : " ;
	do { cout << my_explorer->mem_hierarchy.L2U.block_size.get_val() << ","; } while (my_explorer->mem_hierarchy.L2U.block_size.increase());
	cout << "\n [9]  L2U associativity : " ;
	do { cout << my_explorer->mem_hierarchy.L2U.associativity.get_val() << ","; } while (my_explorer->mem_hierarchy.L2U.associativity.increase());
	cout << "\n\n Processor Parameters ";
	cout << "\n---------------------------------------";
	cout << "\n [10]     integer_units : ";
	do { cout << my_explorer->processor.integer_units.get_val() << ","; } while (my_explorer->processor.integer_units.increase());
	cout << "\n [11]       float_units : ";
	do { cout << my_explorer->processor.float_units.get_val() << ","; } while (my_explorer->processor.float_units.increase());
	cout << "\n [12]      memory_units : ";
	do { cout << my_explorer->processor.memory_units.get_val() << ","; } while (my_explorer->processor.memory_units.increase());
	cout << "\n [13]      branch_units : ";
	do { cout << my_explorer->processor.branch_units.get_val() << ","; } while (my_explorer->processor.branch_units.increase());
	cout << "\n";
	cout << "\n [14]   gpr_static_size : ";
	do { cout << my_explorer->processor.gpr_static_size.get_val() << ","; } while (my_explorer->processor.gpr_static_size.increase());
	cout << "\n [15]   fpr_static_size : ";
	do { cout << my_explorer->processor.fpr_static_size.get_val() << ","; } while (my_explorer->processor.fpr_static_size.increase());
	cout << "\n [16]    pr_static_size : ";
	do { cout << my_explorer->processor.pr_static_size.get_val() << ","; } while (my_explorer->processor.pr_static_size.increase());
	cout << "\n [17]    cr_static_size : ";
	do { cout << my_explorer->processor.cr_static_size.get_val() << ","; } while (my_explorer->processor.cr_static_size.increase());
	cout << "\n [18]   btr_static_size : ";
	do { cout << my_explorer->processor.btr_static_size.get_val() << ","; } while (my_explorer->processor.btr_static_size.increase());

	cout << "\n\n Total space size : " << my_explorer->get_space_size();

	cout << "\n\n ";
	cout << "\n---------------------------------------------";
	cout << "\n (e) - Edit a parameter ";
	cout << "\n (s) - Save current space to file ";
	cout << "\n (l) - Load space from file ";
	cout << "\n (q) - return to main menu";
	cout << "\n---------------------------------------------";
	cout << "\n choice : ";
	cin >> ch;

	if (ch=='e') 
	{
	    int p,val,def_val;
	    vector<int> values;

	    cout << "\n Enter a parameter index (1-18) : ";
	    cin >> p;
	    cout << "\n Enter parameter values, separated by empty spaces ";
	    cout << "\n Use 0 for the last ";
	    cout << "\n values : ";
	    while ( (cin>>val) && (val!=0) ) values.push_back(val);

	    cout << "\n Enter new default value : ";
	    cin >> def_val;

	    switch (p)
	    {
		case 1: 
		    my_explorer->mem_hierarchy.L1D.size.set_values(values,def_val);
		    break;
		case 2: 
		    my_explorer->mem_hierarchy.L1D.block_size.set_values(values,def_val);
		    break;
		case 3: 
		    my_explorer->mem_hierarchy.L1D.associativity.set_values(values,def_val);
		    break;
		case 4: 
		    my_explorer->mem_hierarchy.L1I.size.set_values(values,def_val);
		    break;
		case 5: 
		    my_explorer->mem_hierarchy.L1I.block_size.set_values(values,def_val);
		    break;
		case 6: 
		    my_explorer->mem_hierarchy.L1I.associativity.set_values(values,def_val);
		    break;
		case 7: 
		    my_explorer->mem_hierarchy.L2U.size.set_values(values,def_val);
		    break;
		case 8: 
		    my_explorer->mem_hierarchy.L2U.block_size.set_values(values,def_val);
		    break;
		case 9: 
		    my_explorer->mem_hierarchy.L2U.associativity.set_values(values,def_val);
		    break;
		case 10:
		    my_explorer->processor.integer_units.set_values(values,def_val);
		    break;
		case 11:
		    my_explorer->processor.float_units.set_values(values,def_val);
		    break;
		case 12:
		    my_explorer->processor.memory_units.set_values(values,def_val);
		    break;
		case 13:
		    my_explorer->processor.branch_units.set_values(values,def_val);
		    break;
		case 14:
		    my_explorer->processor.gpr_static_size.set_values(values,def_val);
		    break;
		case 15:
		    my_explorer->processor.fpr_static_size.set_values(values,def_val);
		    break;
		case 16:
		    my_explorer->processor.pr_static_size.set_values(values,def_val);
		    break;
		case 17:
		    my_explorer->processor.cr_static_size.set_values(values,def_val);
		    break;
		case 18:
		    my_explorer->processor.btr_static_size.set_values(values,def_val);
		    break;
		default:
		    cout << "\n ERROR : wrong parameter index ! ";
		    break;
	    }
	}

	if (ch=='s') save_subspace_wrapper();
	if (ch=='l') load_subspace_wrapper();

    }
    while (ch!='q');

}

void User_interface::info()
{
    system("clear");

    cout << "\n     E p i c   E x p l o r e r  - version " << VERSION ;
    cout << "\n ******************************************************** ";
    cout << "\n Open Platform for Design Space Exploration of EPIC/VLIW architectures";
    cout << "\n";
    cout << "\n -------------------------------------------------------- ";
    cout << "\n Written by Davide Patti <dpatti@diit.unict.it>";
    cout << "\n Additional coding by Maurizio Palesi <mpalesi@diit.unict.it>";
    cout << "\n Fuzzy code by Alessandro Di Nuovo <adinuovo@diit.unict.it>";
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

void User_interface::view_processor_config() {

    system("clear");


   cout << "\n Functional units ";
   cout << "\n--------------------------------";

   cout<<"\n integer_units :"<< my_explorer->processor.integer_units.get_val();
   cout<<"\n float_units :"<< my_explorer->processor.float_units.get_val();
   cout<<"\n memory_units :"<< my_explorer->processor.memory_units.get_val();
   cout<<"\n branch_units :"<< my_explorer->processor.branch_units.get_val();

   //cout<<"\n local_memory_units "<< my_explorer->processor.memory_units.get_val();

   cout << "\n\n Register Files ";
   cout << "\n--------------------------------";
   cout<<"\n gpr_static_size :"<< my_explorer->processor.gpr_static_size.get_val();
   cout<<"\n fpr_static_size :"<< my_explorer->processor.fpr_static_size.get_val();
   cout<<"\n pr_static_size :"<< my_explorer->processor.pr_static_size.get_val();
   cout<<"\n cr_static_size :"<< my_explorer->processor.cr_static_size.get_val();
   cout<<"\n btr_static_size :"<< my_explorer->processor.btr_static_size.get_val();

   cout << "\n\n NOTE: rotating portion of each register file is assumed equal";
   cout << "\n to static portion.";

}

void User_interface::view_cache_config() {

    system("clear");

    my_explorer->mem_hierarchy.print_cache_config(my_explorer->mem_hierarchy.L1I);
    my_explorer->mem_hierarchy.print_cache_config(my_explorer->mem_hierarchy.L1D);
    my_explorer->mem_hierarchy.print_cache_config(my_explorer->mem_hierarchy.L2U);

}

void User_interface::load_settings_wrapper()
{
   string base_dir = trimaran_interface->get_base_dir();

   base_dir+="/trimaran-workspace/epic-explorer/";
   string filename;

   cout << "\n\n Available configuration files : ";
   cout << "\n";
   string command = "ls ";
   command+=base_dir+"*.conf";
   system(command.c_str());
   cout << "\n Enter file name without path ( es. 'file.conf' -  Use 'x' to exit) : ";
   cin >> filename;
   if (filename!="x") load_settings(base_dir+filename);

}

void User_interface::load_subspace_wrapper()
{
   string base_dir = trimaran_interface->get_base_dir();

   base_dir+="/trimaran-workspace/epic-explorer/SUBSPACES/";
   string filename;

   cout << "\n\n List of available subspace files : \n";
   string command = "ls "+base_dir;
   system(command.c_str());
   cout << "\n Enter file name without path ( es. 'my_space.sub' -  Use 'x' to exit) : ";
   cin >> filename;
   if (filename!="x") 
   {
       my_explorer->load_space_file(base_dir+filename);
       my_explorer->set_space_name(filename);
   }
}


void User_interface::load_settings(string settings_file)
{

   FILE * fp;

   fp = fopen(settings_file.c_str(),"r");

   // if there is no config file a default one is created
   if (fp==NULL)
   {
       cout << "\n WARNING :configuration file not found... creating it ";
       user_settings.hyperblock = false;
       user_settings.benchmark = "wave";
       user_settings.objective_area = false;
       user_settings.objective_exec_time = true;
       user_settings.objective_energy = false;
       user_settings.objective_power = true;
       user_settings.save_spaces = false;
       user_settings.save_PD_STATS = false;
       user_settings.save_estimation = false;
       user_settings.auto_clock = false;
       user_settings.fuzzy_settings.enabled = 0;
       user_settings.fuzzy_settings.threshold = 0;
       user_settings.fuzzy_settings.min = 0;
       user_settings.fuzzy_settings.max = 0;

       fp= fopen(settings_file.c_str(),"w");
       fclose(fp);
       save_settings(settings_file);
   }

   else fclose(fp);

   std::ifstream input_file (settings_file.c_str());

   if (!input_file)
   {
       cout << "\n Error while reading " << settings_file;
       wait_key();
   }
   else
   {
        user_settings.hyperblock = false;
	user_settings.objective_area = false;
	user_settings.objective_exec_time = false;
	user_settings.objective_energy = false;
	user_settings.objective_power = false;
	user_settings.save_spaces = false;
	user_settings.save_PD_STATS = false;
	user_settings.save_estimation = false;
	user_settings.auto_clock = false;

	go_until("hyperblock",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.hyperblock = true;
	trimaran_interface->set_hyperblock(user_settings.hyperblock);

	go_until("benchmark",input_file);
	input_file >> word;
	trimaran_interface->set_benchmark(word);
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

	go_until("save_PD_STATS",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.save_PD_STATS = true;

	go_until("save_estimation",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.save_estimation = true;

	go_until("AUTO_CLOCK",input_file);
	input_file >> word;
	if (word=="ENABLED") user_settings.auto_clock = true;

	go_until("fuzzy_enabled",input_file);
	input_file >> user_settings.fuzzy_settings.enabled;

	go_until("fuzzy_threshold",input_file);
	input_file >> user_settings.fuzzy_settings.threshold;

	go_until("fuzzy_min",input_file);
	input_file >> user_settings.fuzzy_settings.min;

	go_until("fuzzy_max",input_file);
	input_file >> user_settings.fuzzy_settings.max;

	my_explorer->set_options(user_settings);
   }
}

void User_interface::save_settings_wrapper()
{
   string base_dir = trimaran_interface->get_base_dir();

   base_dir +="/trimaran-workspace/epic-explorer/";

   cout << "\n User settings file will be saved in : ";
   cout << "\n " << base_dir ; 
   cout << "\n\n (1) - Save current settings as default";
   cout << "\n (2) - Save using other filename ";
   cout << "\n (q) - Exit to main menu' ";
   cout << "\n\n Choose  : ";

   char ch;
   cin >> ch;

   if (ch=='1') save_settings(base_dir+"epic_default.conf");
   else
   if (ch=='2')
   {
       cout << "\n Enter filename (USE .conf extension) : ";
       string s;
       cin >> s;
       save_settings(base_dir+s);
   }
}

void User_interface::save_subspace_wrapper()
{
   string base_dir = trimaran_interface->get_base_dir();

   base_dir +="/trimaran-workspace/epic-explorer/SUBSPACES/";

   cout << "\n User settings file will be saved in : ";
   cout << "\n " << base_dir ; 
   cout << "\n\n (1) - Save current subspace as default";
   cout << "\n (2) - Save subspace using an other filename ";
   cout << "\n (q) - Exit to main menu' ";
   cout << "\n\n Choose  : ";

   char ch;
   cin >> ch;

   if (ch=='1') my_explorer->save_space_file(base_dir+"default_space.sub");
   else
   if (ch=='2')
   {
       cout << "\n Enter filename (USE .sub extension) : ";
       string s;
       cin >> s;
       my_explorer->save_space_file(base_dir+s);
       my_explorer->set_space_name(s);
   }
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

	output_file << "\nhyperblock " << status_string(user_settings.hyperblock);
	output_file << "\nbenchmark "  << user_settings.benchmark ;
	output_file << "\narea " << status_string(user_settings.objective_area);
	output_file << "\ncycles " << status_string(user_settings.objective_exec_time);
	output_file << "\nenergy " << status_string(user_settings.objective_energy);
	output_file << "\npower " << status_string(user_settings.objective_power);
	output_file << "\nsave_spaces " << status_string(user_settings.save_spaces);
	output_file << "\nsave_PD_STATS " << status_string(user_settings.save_PD_STATS);
	output_file << "\nsave_estimation " << status_string(user_settings.save_estimation);
	output_file << "\nAUTO_CLOCK " << status_string(user_settings.auto_clock);
	output_file << "\nfuzzy_enabled " << user_settings.fuzzy_settings.enabled;
	output_file << "\nfuzzy_threshold " << user_settings.fuzzy_settings.threshold;
	output_file << "\nfuzzy_min " << user_settings.fuzzy_settings.min;
	output_file << "\nfuzzy_max " << user_settings.fuzzy_settings.max;


	cout << "\n Ok, saved current settings in " << settings_file;
    }
}

void User_interface::schedule_explorations()
{
    
    cout << "\n Ok, enter a string containing the sequence of explorations ";
    cout << "\n to be scheduled (for example : 'psg' ) : ";
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
	cout << "\n\n Population size : ";
	cin >> ga_parameters.population_size;
	cout << " Crossover prob : ";
	cin >> ga_parameters.pcrossover;
	cout << " Mutation prob : ";
	cin >> ga_parameters.pmutation;
	cout << " Max Generations : ";
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


void User_interface::reload_hmdes_file() 
{

    cout << "\n\n Loading processor config from hmdes file ...";
    trimaran_interface->load_processor_config(&(my_explorer->processor));

   cout << "\n\n Updated processor config ";
}

void User_interface::reload_memhierarchy_config()
{
    cout << "\n\n Loading cache config from cache.cgf file ";
    trimaran_interface->load_mem_config(&(my_explorer->mem_hierarchy));
}


void User_interface::compile_hmdes_file() {

// VERBOSE_INTERFACE remains here only for hystorical reasons
// all other similar references have been deleted
#ifdef VERBOSE_INTERFACE
    cout << "\n Now will be invoked the method :";
    cout << "\n\n void Trimaran_interface::compile_hmdes_file() ";
    wait_key();
#endif

    trimaran_interface->compile_hmdes_file();
}

void User_interface::compile_benchmark() {

    trimaran_interface->compile_benchmark();
}

void User_interface::execute_benchmark() {

    trimaran_interface->execute_benchmark();
}

void User_interface::view_statistics() {

    system("clear");
    Dynamic_stats dynamic_stats = trimaran_interface->get_dynamic_stats();

    cout << "\n D y n a m i c  S t a t s ";

    cout << "\n------------------------------------------------";

    cout << "\n            Total cycles: "<<dynamic_stats.total_cycles;
    cout << "\n          Compute cycles: "<<dynamic_stats.compute_cycles;
    cout << "\n            Stall cycles: "<<dynamic_stats.stall_cycles;

    cout << "\n                Branches: "<<dynamic_stats.branch;
    cout << "\n                    Load: " <<dynamic_stats.load;
    cout << "\n                   Store: "<<dynamic_stats.store;
    cout << "\n             Integer alu: " << dynamic_stats.ialu;
    cout << "\n               Float alu: " << dynamic_stats.falu;
    cout << "\n    Compare to predicate: " << dynamic_stats.cmp;
    cout << "\n       Prepare to branch: " << dynamic_stats.pbr;
    cout << "\nAverage issued ops/cycle: ";
    cout << dynamic_stats.average_issued_ops_total_cycles;
    cout << "\n------------------------------------------------";
    cout << "\n\n L1 Data Cache";
    cout << "\n------------------------------------------------";
    cout << "\n read  (hit/miss) : ("<< dynamic_stats.L1D_r_hit<<"/"<<dynamic_stats.L1D_r_miss<<")";
    cout << "\n write (hit/miss) : ("<< dynamic_stats.L1D_w_hit<<"/"<<dynamic_stats.L1D_w_miss<<")";

    cout << "\n\n L1 Instruction Cache ";
    cout << "\n------------------------------------------------";

    cout << "\n total fetches : " << dynamic_stats.L1I_fetches;
    cout << "\n read (hit/miss) : ("<< dynamic_stats.L1I_hit <<"/"<< dynamic_stats.L1I_miss<<")";

    cout << "\n\n Level 2 unified Cache ";
    cout << "\n------------------------------------------------";
    cout << "\n read (hit/miss) : ("<< dynamic_stats.L2U_r_hit<<"/" << dynamic_stats.L2U_r_miss<<")";
    cout << "\n write (hit/miss): " << dynamic_stats.L2U_w_hit<<"/" << dynamic_stats.L2U_w_miss<<")";

}

void User_interface::compute_cost() {

    system("clear");

    Dynamic_stats dynamic_stats = trimaran_interface->get_dynamic_stats();
    Estimate estimate = my_explorer->estimator.get_estimate(dynamic_stats,my_explorer->mem_hierarchy,my_explorer->processor);

    cout << "\n\n";
    cout << "\n  P e r f o r m a n c e ";
    cout << "\n ----------------------------------------------------";
    cout << "\n   Total cycles executed : " << estimate.execution_cycles;
    cout << "\n   Total execution time (sec): " << estimate.execution_time;
    cout << "\n   L1D access time (ns): " << estimate.L1D_access_time*1e9;
    cout << "\n   L1I access time (ns): " << estimate.L1I_access_time*1e9;
    cout << "\n   clock freq (Mhz) : " << estimate.clock_freq/1e6;
    cout << "\n   Average Ops/Cycle : "  << dynamic_stats.average_issued_ops_total_cycles;
    cout << "\n   L1D bus transition prob : " << estimate.L1D_transition_p;
    cout << "\n   L1I bus transition prob : " << estimate.L1I_transition_p;


    cout << "\n\n";

    cout << "\n  E n e r g y  &  P o w e r ";
    cout << "\n ---------------------------------------------------";
    cout << "\n   L1D cache energy (mJ) : " << estimate.L1D_energy*1000;
    cout << "\n   L1I cache energy (mJ) : " << estimate.L1I_energy*1000;
    cout << "\n   L2U cache energy (mJ) : " << estimate.L2U_energy*1000;
    cout << "\n   Total Cache energy (mJ) " << estimate.total_cache_energy*1000;
    cout << "\n";
    cout << "\n   Main Memory energy (mJ) : " << estimate.main_memory_energy*1000;
    cout << "\n";
    cout << "\n   Processor energy (mJ) : " << estimate.total_processor_energy*1000;
    cout << "\n\n--> Total System Energy (mJ) : "<< estimate.total_system_energy*1000;
    cout << "\n--> Average System Power (W) :"<< estimate.total_average_power;

    cout << "\n\n";
    cout << "\n  A r e a ";
    cout << "\n ---------------------------------------------------";
    cout << "\n   L1 Instruction cache area (cm^2) : " << estimate.L1I_area;
    cout << "\n   L1 Data cache area (cm^2) : " << estimate.L1D_area;
    cout << "\n   L2 Unified cache area (cm^2) : " << estimate.L2U_area;
    cout << "\n   Processor core area (cm^2) : " << estimate.processor_area;
    cout << "\n\n--> Total System Area (cm^2 ) : " << estimate.total_area;
}

void User_interface::save_processor_config() {

    cout << "\n Saving current process configuration ...";
    trimaran_interface->save_processor_config(my_explorer->processor);
}

void User_interface::save_cache_config() {
    cout << "\n Saving current cache config ...";

    trimaran_interface->save_mem_config(my_explorer->mem_hierarchy);
}

void User_interface::set_base_dir(const string& dir)
{
    base_dir = dir;
    trimaran_interface->set_base_dir(dir);
    my_explorer->set_base_dir(dir);
}


