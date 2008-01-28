/*
 *  spea2.c
 *  spea2
 *
 *  Created by Gianmarco De Francisci Morales [gmorales(at)diit.unict.it] on 23/11/07.
 *
 */

#define MAX_GENERATIONS 50

#include <stdio.h>

#include <iostream>
#include "variator.h"
#include "selector.h"
// for the uid
#include "containers.h"

#define CHROMO_DIM 10

using namespace std;

int individual::uid = 0;
vector<alleleset> individual::als;

void evaluate(population *pop){
        for(population::iterator it=pop->begin(); it!=pop->end(); it++){
	        if(it->objectives_dim() == 2){
	                double even=0,by3=0;
	                for(int i=0; i<it->chromosome_dim(); i++){
	                        if(it->phenotype(i) % 2 == 0)
	                                even++;
	                        if(it->phenotype(i) % 3 == 0)
	                                by3++;
	                }
			it->objectives[0] = even;
			it->objectives[1] = by3;
        	}
	}
}



int main(int argc, char* argv[]){
	bool interactive = (argc > 2);
	int seed=1492;
	if(argc > 1)
		seed = atoi(argv[1]);
	else
		cerr << "WARNING: Using default seed" << endl;
	srand(seed); /* seeding random number generator */
	
	// Allele sets building
	vector<alleleset> sets;
	alleleset tmp;
//	for(int i=10; i<20; i++)
//		tmp.push_back(i);
	tmp.push_back(0);
	tmp.push_back(1);
	tmp.push_back(2);
	tmp.push_back(3);
	tmp.push_back(4);
	for(int i=0; i<CHROMO_DIM; i++)
		sets.push_back(tmp);
	individual::setAllelesets(sets);
	
	common* comm = new common(20,10,10,2); // (alpha, mu, lambda, dim)
	int generation = 0;
	variator* var = new variator(0.8, 0.1, CHROMO_DIM, comm); // (xover_p, mutation_p, chromo_dim)
	selector* sel = new selector(2, comm); // (tournament)
	
	// GA start
	evaluate(var->get_offspring()); // evaluate initial population
	var->write_ini();			/* write ini population */
	
	cout << "Initial population" << endl;
	cout << var->get_ini() << endl;
	
	sel->read_ini();			/* read ini population */
	sel->select_initial();			/* do selection */

	sel->write_arc();			/* write arc population (all individuals that could ever be used again) */
	cout << "Archive population" << endl;
	cout << var->get_arc() << endl;
	
	sel->write_sel();			/* write sel population */
	cout << "Selected population" << endl;
	cout << var->get_sel() << endl;
	// main exploration loop
	while(generation++ < MAX_GENERATIONS){
		var->read_sel();
		var->read_arc();
		var->variate();
		evaluate(var->get_offspring()); // evaluate offspring
		var->write_var();
		
		cout << "Variated population" << endl;
		cout << var->get_var() << endl;
		
		sel->read_var();		/* read var population */
		sel->select_normal();		/* do selection */
		
		sel->write_arc();		/* write arc population (all individuals that could ever be used again) */
		cout << "Archive population" << endl;
		cout << var->get_arc() << endl;
		
		sel->write_sel();		/* write sel population */
		cout << "Selected population" << endl;
		cout << var->get_sel() << endl;
		
		if(interactive)
			getchar();
	}
	var->read_arc();
	var->write_output();
}
