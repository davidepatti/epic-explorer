
// ********************************************************************
//   GA-based exploration
// ********************************************************************

//******************************************************************************************//
#include "explorer.h"
#include "common.h"
#include "containers.h"
#include "variator.h"
#include "selector.h"
#include <values.h>
#define CHROMOSOME_DIM N_PARAMS // TO_CHECK
#define DEF_TOURNAMENT 2
#define DEF_HASH_TABLE_SIZE   128

void Explorer::start_GA(const GA_parameters& parameters)
{
    current_algo="GA";
    string logfile = get_base_dir()+string(EE_LOG_PATH);
    int myrank = get_mpi_rank();

    // TODO map it to an option in epic
    bool adjustedOperators = Options.properators;

    if (Options.approx_settings.enabled == 1)
	current_algo+="_fuzzy";
    if (Options.approx_settings.enabled == 2)
	current_algo+="_ANN";

    eud.ht_ga = new HashGA(DEF_HASH_TABLE_SIZE);
    eud.history.clear();
    eud.pareto.clear();

    Exploration_stats stats;
    stats.space_size = get_space_size();
    stats.start_time = time(NULL);
    reset_sim_counter();

    benchmarks.push_back(Options.benchmark);
    benchmarks.insert(benchmarks.end(), Options.bench_v.begin(), Options.bench_v.end());
    string file_name;
    if(Options.multibench) {
	for(vector<string>::iterator it=benchmarks.begin(); it!=benchmarks.end(); it++)
	    file_name += *it + "_";
	file_name += current_algo + "_" + current_space;
    } else
	file_name = Options.benchmark+"_"+current_algo+"_"+current_space;

    write_to_log(myrank,logfile,"Starting "+file_name);

    SimulateBestWorst();

    // GA init
    init_GA(); // call it before creating anything GA related

    int pop_size = parameters.population_size;
    common* comm = new common(pop_size, pop_size, pop_size, n_obj); // (alpha, mu, lambda, dim)
    variator* var = new variator(parameters.pcrossover, parameters.pmutation, CHROMOSOME_DIM, comm); // (xover_p, mutation_p, chromo_dim)
    selector* sel = new selector(DEF_TOURNAMENT, comm); // (tournament)
    int generation = 0;
    const int MAX_GENERATIONS = parameters.max_generations;

    // GA start
    GA_evaluate(var->get_offspring()); // evaluate initial population

    var->write_ini();		// write ini population
    write_to_log(myrank,logfile,"Initial population"+ to_string(var->get_ini()));

    sel->read_ini();		// read ini population
    sel->select_initial();		// do selection
    sel->write_arc();		// write arc population (all individuals that could ever be used again)
    sel->write_sel();		// write sel population

    while ( generation++ < MAX_GENERATIONS )
    {
	write_to_log(myrank,logfile,"Iteration " + generation);

	var->read_arc();
	write_to_log(myrank,logfile, "Archive population");
	write_to_log(myrank,logfile, to_string(var->get_arc()));

	var->read_sel();
	write_to_log(myrank,logfile,"Selected population");
	write_to_log(myrank,logfile,to_string(var->get_sel()));

	var->variate(adjustedOperators);	// create offspring

	GA_evaluate(var->get_offspring()); // evaluate offspring population

	var->write_var();		// write var population
	write_to_log(myrank,logfile,"Variated population");
	write_to_log(myrank,logfile,to_string(var->get_var()));

	sel->read_var();		// read var population
	sel->select_normal();		// do selection
	sel->write_arc();		// write arc population (all individuals that could ever be used again)
	sel->write_sel();		// write sel population

	// save pareto-set every report_pareto_step generations
	if ( generation % parameters.report_pareto_step == 0)
	{
	    char temp[30];
	    sprintf(temp, "_%d", generation);
	    stats.end_time = time(NULL);
	    stats.n_sim = get_sim_counter();
	    save_stats(stats,file_name+string(temp)+".stat");
	    save_simulations(eud.pareto, file_name+string(temp)+".pareto.exp");
	    save_simulations(eud.history, file_name+".history");
	}
    }
    var->read_arc();
    write_to_log(myrank,logfile,"Final archive population");
    write_to_log(myrank,logfile,to_string(var->get_arc()));

    var->write_output();
    // save history
    save_simulations(eud.history, file_name+".history");

    // save statistics
    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats, file_name+".stat");
}

//********************************************************************

void Explorer::init_GA()
{
  vector<alleleset> alleles;

  // processor parameters
  alleles.push_back((processor.integer_units.get_values()));
  alleles.push_back((processor.float_units.get_values()));
  alleles.push_back((processor.branch_units.get_values()));
  alleles.push_back((processor.memory_units.get_values()));

  alleles.push_back((processor.gpr_static_size.get_values()));
  alleles.push_back((processor.fpr_static_size.get_values()));
  alleles.push_back((processor.pr_static_size.get_values()));
  alleles.push_back((processor.cr_static_size.get_values()));
  alleles.push_back((processor.btr_static_size.get_values()));
  // memory hierarchy parameters
  alleles.push_back((mem_hierarchy.L1D.size.get_values()));
  alleles.push_back((mem_hierarchy.L1D.block_size.get_values()));
  alleles.push_back((mem_hierarchy.L1D.associativity.get_values()));

  alleles.push_back((mem_hierarchy.L1I.size.get_values()));
  alleles.push_back((mem_hierarchy.L1I.block_size.get_values()));
  alleles.push_back((mem_hierarchy.L1I.associativity.get_values()));
  
  alleles.push_back((mem_hierarchy.L2U.size.get_values()));
  alleles.push_back((mem_hierarchy.L2U.block_size.get_values()));
  alleles.push_back((mem_hierarchy.L2U.associativity.get_values()));

  alleles.push_back(processor.num_clusters.get_values());

  alleles.push_back(compiler.tcc_region.get_values());
  alleles.push_back(compiler.max_unroll_allowed.get_values());
  alleles.push_back(compiler.regroup_only.get_values());
  alleles.push_back(compiler.do_classic_opti.get_values());
  alleles.push_back(compiler.do_prepass_scalar_scheduling.get_values());
  alleles.push_back(compiler.do_postpass_scalar_scheduling.get_values());
  alleles.push_back(compiler.do_modulo_scheduling.get_values());
  alleles.push_back(compiler.memvr_profiled.get_values());


  individual::setAllelesets(alleles); // set static allele sets genome for individuals
}

/*************************************************************************/

Configuration Explorer::ind2conf(const individual& ind){
    Configuration conf;

    conf.integer_units = ind.phenotype(0);
    conf.float_units   = ind.phenotype(1);
    conf.branch_units  = ind.phenotype(2);
    conf.memory_units  = ind.phenotype(3);

    conf.gpr_static_size = ind.phenotype(4);
    conf.fpr_static_size = ind.phenotype(5);
    conf.pr_static_size  = ind.phenotype(6);
    conf.cr_static_size  = ind.phenotype(7);
    conf.btr_static_size = ind.phenotype(8);

    conf.L1D_size  = ind.phenotype(9);
    conf.L1D_block = ind.phenotype(10);
    conf.L1D_assoc = ind.phenotype(11);

    conf.L1I_size  = ind.phenotype(12);
    conf.L1I_block = ind.phenotype(13);
    conf.L1I_assoc = ind.phenotype(14);

    conf.L2U_size  = ind.phenotype(15);
    conf.L2U_block = ind.phenotype(16);
    conf.L2U_assoc = ind.phenotype(17);

    conf.num_clusters = ind.phenotype(18);

    conf.tcc_region = ind.phenotype(19);
    conf.max_unroll_allowed = ind.phenotype(20);
    conf.regroup_only = ind.phenotype(21);
    conf.do_classic_opti = ind.phenotype(22);
    conf.do_prepass_scalar_scheduling = ind.phenotype(23);
    conf.do_postpass_scalar_scheduling = ind.phenotype(24);
    conf.do_modulo_scheduling = ind.phenotype(25);
    conf.memvr_profiled = ind.phenotype(26);
    return conf;
}

/*************************************************************************/

void Explorer::GA_evaluate(population* pop)
{
    vector<Configuration> vconf;
    vector<int> indexes;
    vector<Simulation> vsim(pop->size());
    vconf.reserve(pop->size());
    indexes.reserve(pop->size());

    string logfile = get_base_dir()+string(EE_LOG_PATH);
    int myrank = get_mpi_rank();

    for(int index=0; index < pop->size(); index++)
    {
	Configuration conf = ind2conf(pop->at(index));	
	Simulation sim;
	sim.config = conf;    

	if(!conf.is_feasible()){
	    write_to_log(myrank,logfile,"WARNING: GA configuration " + to_string(index) + " not feasible");
	    sim.exec_time = BIG_CYCLES;
	    sim.energy = BIG_ENERGY;
	    sim.area = BIG_AREA;
	    vsim[index] = sim;
	} 
	else {
	    Simulation *psim = eud.ht_ga->searchT(sim);
	    if(psim != NULL) { // present in cache
		vsim[index] = *psim;
	    } else { // not present in cache
		indexes.push_back(index); // save index for later use
		vconf.push_back(conf); // schedule configuration for simulation
	    }
	}
    } // for pop


    // simulate the configurations for every benchmark
    int bench = 0;
    vector<Simulation> results;

    for(int bench=0; bench<benchmarks.size(); bench++ ){
	Options.benchmark = benchmarks.at(bench);
	this->trimaran_interface->set_benchmark(Options.benchmark);

	write_to_log(myrank,logfile,"Simulating " + to_string(vconf.size()) + " configurations");
	if(bench == 0) { // first benchmark
	    results = simulate_space(vconf);
	} else { // other benchmarks, merge simulations into older results
	    vector<Simulation> tmp = simulate_space(vconf);
	    for(vector<Simulation>::iterator it1=tmp.begin(),it2=results.begin(); 
		    it1!=tmp.end() && it2!=results.end(); it1++,it2++)
		it2->add_simulation(*it1);
	}
    } // for bench


    // cache results
    for(int i=0; i<results.size(); i++){
	Simulation sim = results[i];
	eud.history.push_back(sim);
	bool cacheable = sim.simulated;
	if(cacheable){
	    eud.ht_ga->addT(sim);
	    eud.pareto.push_back(sim);
	    eud.pareto = get_pareto(eud.pareto); //FIXME TODO perche lo fa ogni volta invece di farlo solo alla fine?
	} else if (!isDominated(sim, eud.pareto)){ //if it could be a pareto solution, the configuration is simulated
	    //FIXME
	    //		explorer->set_force_simulation(true);
	    //		sim = explorer->simulate_space(vconf)[0];
	    //		explorer->set_force_simulation(false);
	    eud.history[eud.history.size() - 1] = sim; // updates history with new simulated values
	    eud.ht_ga->addT(sim);
	    eud.pareto.push_back(sim);
	    eud.pareto = get_pareto(eud.pareto); // funziona solo con multibench con la media
	}

	//G
	vsim[indexes[i]] = sim; // update simulation vector with new results
    } // for results


    // reinsert simulation values into GA

        //const int SCALE = 1; // seconds
    const int SCALE = 1000; // milliseconds
    assert(vsim.size() == pop->size());
    //    int disp = bench * n_obj;
    for(int i=0; i<pop->size(); i++)
    {
	(*pop)[i].objectives[0] = vsim[i].exec_time * SCALE;

	if( (*pop)[i].objectives_dim() > 1)
	    (*pop)[i].objectives[1] = vsim[i].energy;
	if( (*pop)[i].objectives_dim() > 2)
	    (*pop)[i].objectives[2] = vsim[i].area;
    }

}

/*************************************************************************/

void Explorer::SimulateBestWorst()
{
  vector<pair<int,int> > minmax = getParameterRanges();

  Configuration cnf_best;
  cnf_best.integer_units = minmax[0].second;
  cnf_best.float_units = minmax[1].second;
  cnf_best.branch_units = minmax[2].second;
  cnf_best.memory_units = minmax[3].second;
  cnf_best.gpr_static_size = minmax[4].second;
  cnf_best.fpr_static_size = minmax[5].second;
  cnf_best.pr_static_size = minmax[6].second;
  cnf_best.cr_static_size = minmax[7].second;
  cnf_best.btr_static_size = minmax[8].second;
  cnf_best.L1D_size = minmax[9].second;
  cnf_best.L1D_block = minmax[10].second;
  cnf_best.L1D_assoc = minmax[11].second;
  cnf_best.L1I_size = minmax[12].second;
  cnf_best.L1I_block = minmax[13].second;
  cnf_best.L1I_assoc = minmax[14].second;
  cnf_best.L2U_size = minmax[15].second;
  cnf_best.L2U_block = minmax[16].second;
  cnf_best.L2U_assoc = minmax[17].second;
  cnf_best.num_clusters = minmax[18].second;
  cnf_best.tcc_region = minmax[19].second;
  cnf_best.max_unroll_allowed = minmax[20].second;
  cnf_best.regroup_only = minmax[21].second;
  cnf_best.do_classic_opti = minmax[22].second;
  cnf_best.do_prepass_scalar_scheduling = minmax[23].second;
  cnf_best.do_postpass_scalar_scheduling = minmax[24].second;
  cnf_best.do_modulo_scheduling = minmax[25].second;
  cnf_best.memvr_profiled = minmax[26].second;

  Configuration cnf_worst;
  cnf_worst.integer_units = minmax[0].first;
  cnf_worst.float_units = minmax[1].first;
  cnf_worst.branch_units = minmax[2].first;
  cnf_worst.memory_units = minmax[3].first;
  cnf_worst.gpr_static_size = minmax[4].first;
  cnf_worst.fpr_static_size = minmax[5].first;
  cnf_worst.pr_static_size = minmax[6].first;
  cnf_worst.cr_static_size = minmax[7].first;
  cnf_worst.btr_static_size = minmax[8].first;
  cnf_worst.L1D_size = minmax[9].first;
  cnf_worst.L1D_block = minmax[10].first;
  cnf_worst.L1D_assoc = minmax[11].first;
  cnf_worst.L1I_size = minmax[12].first;
  cnf_worst.L1I_block = minmax[13].first;
  cnf_worst.L1I_assoc = minmax[14].first;
  cnf_worst.L2U_size = minmax[15].first;
  cnf_worst.L2U_block = minmax[16].first;
  cnf_worst.L2U_assoc = minmax[17].first;
  cnf_worst.num_clusters = minmax[18].first;

  cnf_best.tcc_region = minmax[19].first;
  cnf_best.max_unroll_allowed = minmax[20].first;
  cnf_best.regroup_only = minmax[21].first;
  cnf_best.do_classic_opti = minmax[22].first;
  cnf_best.do_prepass_scalar_scheduling = minmax[23].first;
  cnf_best.do_postpass_scalar_scheduling = minmax[24].first;
  cnf_best.do_modulo_scheduling = minmax[25].first;
  cnf_best.memvr_profiled = minmax[26].first;

  vector<Configuration> cnf_best_worst;
  cnf_best_worst.push_back(cnf_best);
  cnf_best_worst.push_back(cnf_worst);
//G FIXME re-enable
/*
  vector<Simulation> sim_best_worst = simulate_space(cnf_best_worst);
  eud.history.push_back(sim_best_worst[0]);
  eud.history.push_back(sim_best_worst[1]);
  eud.ht_ga->addT(sim_best_worst[0]);
  eud.ht_ga->addT(sim_best_worst[1]);
  eud.pareto.push_back(sim_best_worst[0]);
  eud.pareto.push_back(sim_best_worst[1]);
*/
}

