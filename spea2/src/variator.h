/*
 *  variator.h
 *  spea2
 *
 *  Created by Gianmarco De Francisci Morales [gmorales(at)diit.unict.it] on 25/11/07.
 *
 */
#ifndef VARIATOR_H
#define VARIATOR_H

#include "containers.h"

class variator{
public:
	variator(double crossover_prob, double mutation_prob, int chromosome_dim,common* c);
	~variator();
	void read_sel();
	void read_arc();
	void write_ini();
	void write_var();
	void variate(bool adjustedOperators);
	void write_output();

	population* get_offspring();
	population get_ini();
	population get_var();
	population get_sel();
	population get_arc();
	
private:
	variator();
	population* global_pop;
	population* offspring;
	const double crossover_p, mutation_p;
	/* selector common variables */
	const int alpha,mu,lambda,dim;
	population* ini_pop;
	population* var_pop;
	int* sel_indexes;
	int* arc_indexes;
	
	void evaluate(population*);
};

#endif
