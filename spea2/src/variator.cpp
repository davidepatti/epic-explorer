/*
 *  variator.cpp
 *  spea2
 *
 *  Created by Gianmarco De Francisci Morales [gmorales(at)diit.unict.it] on 25/11/07.
 *
 */

#include <iostream>
#include "utility.h"
#include "containers.h"
#include "variator.h"

using namespace std;

/* VARIATOR */

variator::variator(double cp, double mp, int chromo_dim, common* c): global_pop(new population()), offspring(new population()), crossover_p(cp), mutation_p(mp), alpha(c->alpha), mu(c->mu), lambda(c->lambda), dim(c->dim), ini_pop(c->ini_pop), var_pop(c->var_pop), sel_indexes(c->sel_indexes), arc_indexes(c->arc_indexes){
	individual adam(chromo_dim, dim);
	adam.randomize();
	offspring->push_back(adam);
	for(int i=0; i<alpha-1; i++){
		individual tmp = adam.child();
		tmp.randomize();
		offspring->push_back(tmp);
	}
}

variator::~variator(){
	delete global_pop;
	delete offspring;
}

void variator::read_sel(){
	// reads selected elements into mating pool
}

void variator::read_arc(){
	// strips down population to archive
}

void variator::write_ini(){
	// saves global population
	global_pop->assign(offspring->begin(), offspring->end());
	ini_pop->assign(offspring->begin(), offspring->end());
}

void variator::write_var(){
	global_pop->insert(global_pop->end(), offspring->begin(), offspring->end());
	var_pop->assign(offspring->begin(), offspring->end());
}

void variator::variate(){
	// clears current generation
	//var_pop->clear();
	if(!offspring->empty())
		offspring->clear();
	//population offspring = new population(global_pop);
	for(int i=0; i+1<mu; i+=2){
		// choses elements for mating
		int id1 = sel_indexes[i];
		int id2 = sel_indexes[i+1];
		individual tmp1 = global_pop->at(id1).child();
		individual tmp2 = global_pop->at(id2).child();
		// father and mother crossover
		if(drand(1) < crossover_p)
			//tmp1.crossover(tmp2);
			tmp1.adjusted_crossover(tmp2);
		// brother mutation
		if(drand(1) < mutation_p)
			//tmp1.mutation(mutation_p);
			tmp1.adjusted_mutation(mutation_p);
		// sister mutation
		if(drand(1) < mutation_p)
			//tmp2.mutation(mutation_p);
			tmp2.adjusted_mutation(mutation_p);
		//global_pop->push_back(tmp1);
		//global_pop->push_back(tmp2);
		offspring->push_back(tmp1);
		offspring->push_back(tmp2);
		//var_pop->push_back(tmp1);
		//var_pop->push_back(tmp2);
	}
}

population* variator::get_offspring(){
	return offspring;
}

population variator::get_ini(){
	return *ini_pop;
}

population variator::get_var(){
	return *var_pop;
}

population variator::get_sel(){
	// get individuals from sel_indexes
	population tmp;
	for(int i=0; i<lambda; i++)
		tmp.push_back(global_pop->at(sel_indexes[i]));
	return tmp;
}

population variator::get_arc(){
	// get individuals from arc_indexes
	population tmp;
	for(int i=0; i<alpha; i++)
		tmp.push_back(global_pop->at(arc_indexes[i]));
	return tmp;
}

void variator::write_output(){}
