/*
 *  selector.h
 *  spea2
 *
 *  Created by Gianmarco De Francisci Morales [gmorales(at)diit.unict.it] on 04/12/07.
 *
 */

#ifndef SELECTOR_H
#define SELECTOR_H

// for size_t definition
#include <stdlib.h>

/*----------------------------| macro |----------------------------------*/

#define PISA_ERROR(x) fprintf(stderr, "\nError: " x "\n"), fflush(stderr), exit(EXIT_FAILURE)

/*---------------------------| constants |-------------------------------*/
#define FILE_NAME_LENGTH 128 /* maximal length of filenames */
#define CFG_ENTRY_LENGTH 128 /* maximal length of entries in cfg file */
#define PISA_MAXDOUBLE 1E30  /* Internal maximal value for double */
#define PISA_MINDOUBLE 1E-30  /* Internal minimal value for double */

/*----------------------------| structs |--------------------------------*/

typedef struct ind_st  /* an individual */
{
	int index;
	double *f; /* objective vector */
	double fitness;
} ind;

typedef struct pop_st  /* a population */
{
	int size;
	int maxsize;
	ind **ind_array;
} pop;

#include "containers.h"

class selector{
public:
	selector(int tournament, common* c);
	~selector();
	int read_ini(void);
	int read_var(void);
	void write_sel(void);
	void write_arc(void);

	/*---| functions implementing the selection |---*/
	void select_initial();
	void select_normal();
	
private:
	int tournament;	/* parameter for tournament selection */
	/* variator common variables */
	const int alpha,mu,lambda,dim;
	population* ini_pop;
	population* var_pop;
	int* sel_indexes;
	int* arc_indexes;
		
	/* population containers */
	pop *pp_all;
	pop *pp_new;
	pop *pp_sel;
	
	/* SPEA2 internal global variables */
	int *fitness_bucket;
	int *fitness_bucket_mod;
	int *copies;
	int *old_index;
	double **dist;
	int **NN;
		
	/*---| memory allocation functions |---*/
	
	void* chk_malloc(size_t size);
	pop* create_pop(int size, int dim);
	ind* create_ind(int dim);
	
	void free_memory(void);
	void free_pop(pop *pp);
	void complete_free_pop(pop *pp);
	void free_ind(ind *p_ind);
	
	/*---| functions implementing the selection |---*/
	
	void selection();
	void mergeOffspring();
	void calcFitnesses();
	void calcDistances();
	int getNN(int index, int k);
	double getNNd(int index, int k);
	void environmentalSelection();
	void truncate_nondominated();
	void truncate_dominated();
	void matingSelection();
	
	int dominates(ind *p_ind_a, ind *p_ind_b);
	int is_equal(ind *p_ind_a, ind *p_ind_b);
	double calcDistance(ind *p_ind_a, ind *p_ind_b);
	
	/*---| data exchange functions |---*/
		
	int read_pop(population *origin, pop *pp, int size, int dim);
	void write_pop(int *destination, pop *pp, int size);
};

#endif
