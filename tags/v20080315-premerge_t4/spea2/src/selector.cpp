/*
 *  selector.cpp
 *  spea2
 *
 *  Created by Gianmarco De Francisci Morales [gmorales(at)diit.unict.it] on 04/12/07.
 *
 */

#include <assert.h>
#include <math.h>
#include <iostream>
#include "utility.h"
#include "containers.h"
#include "selector.h"

using namespace std;

/*-----------------------| initialization |------------------------------*/

selector::selector(int t, common* c): tournament(t), alpha(c->alpha), mu(c->mu), lambda(c->lambda), dim(c->dim),
		ini_pop(c->ini_pop), var_pop(c->var_pop),	sel_indexes(c->sel_indexes), arc_indexes(c->arc_indexes){
	pp_all = create_pop(alpha + lambda, dim);
	pp_sel = create_pop(mu, dim);
}

selector::~selector(){
	free_memory();
}


/*-------------------| memory allocation functions |---------------------*/

void* selector::chk_malloc(size_t size)
/* Wrapper function for malloc(). Checks for failed allocations. */
{
	void *return_value = malloc(size);
	if(return_value == NULL)
		PISA_ERROR("Selector: Out of memory.");
	return (return_value);
}


pop* selector::create_pop(int maxsize, int dim)
/* Allocates memory for a population. */
{
	int i;
	pop *pp;
	
	assert(dim >= 0);
	assert(maxsize >= 0);
	
	pp = (pop*) chk_malloc(sizeof(pop));
	pp->size = 0;
	pp->maxsize = maxsize;
	pp->ind_array = (ind**) chk_malloc(maxsize * sizeof(ind*));
	
	for (i = 0; i < maxsize; i++)
  	pp->ind_array[i] = NULL;
	
	return (pp);
}


ind* selector::create_ind(int dim)
/* Allocates memory for one individual. */
{
	ind *p_ind;
	
	assert(dim >= 0);
	
	p_ind = (ind*) chk_malloc(sizeof(ind));
	
	p_ind->index = -1;
	p_ind->fitness = -1;
	p_ind->f = (double*) chk_malloc(dim * sizeof(double));
	return (p_ind);
}


void selector::free_memory()
/* Frees all memory. */
{
	free_pop(pp_sel);
	complete_free_pop(pp_all);
	free_pop(pp_new);
	pp_sel = NULL;
	pp_all = NULL;
	pp_new = NULL;
}

void selector::free_pop(pop *pp)
/* Frees memory for given population. */
{
	if (pp != NULL)
	{
		free(pp->ind_array);
		free(pp);
	}
}

void selector::complete_free_pop(pop *pp)
/* Frees memory for given population and for all individuals in the
population. */
{
	int i = 0;
	if (pp != NULL)
	{
		if(pp->ind_array != NULL)
		{
			for (i = 0; i < pp->size; i++)
			{
				if (pp->ind_array[i] != NULL)
				{
					free_ind(pp->ind_array[i]);
					pp->ind_array[i] = NULL;
				}
			}
			
			free(pp->ind_array);
		}
		
		free(pp);
	}
}

void selector::free_ind(ind *p_ind)
/* Frees memory for given individual. */
{
	assert(p_ind != NULL);
	
	free(p_ind->f);
	free(p_ind);
}



/*-----------------------| selection functions|--------------------------*/

void selector::selection()
{
	int i;
	int size;
	
	/* Join offspring individuals from variator to population */
	mergeOffspring();
	
	size = pp_all->size;
	
	/* Create internal data structures for selection process */
	/* Vectors */
	fitness_bucket = (int*) chk_malloc(size * size * sizeof(int));
	fitness_bucket_mod = (int*) chk_malloc(size * sizeof(int));
	copies = (int*) chk_malloc(size * sizeof(int));
	old_index = (int*) chk_malloc(size * sizeof(int));
	
	/* Matrices */
	dist = (double**) chk_malloc(size * sizeof(double*));
	NN = (int**) chk_malloc(size * sizeof(int*));
	for (i = 0; i < size; i++)
	{
		dist[i] = (double*) chk_malloc(size * sizeof(double));	  
		NN[i] = (int*) chk_malloc(size * sizeof(int));
	}
	
	/* Calculates SPEA2 fitness values for all individuals */
	calcFitnesses();
	
	/* Calculates distance matrix dist[][] */
	calcDistances();
	
	/* Performs environmental selection
		(truncates 'pp_all' to size 'alpha') */
	environmentalSelection();
	
	/* Performs mating selection
		(fills mating pool / offspring population pp_sel */
	matingSelection();
	
	/* Frees memory of internal data structures */    
	free(fitness_bucket);
	fitness_bucket = NULL;
	free(fitness_bucket_mod);
	fitness_bucket_mod = NULL;
	free(copies);
	copies = NULL;
	free(old_index);
	old_index = NULL;
	for (i = 0; i < size; i++)
	{
		if (NULL != dist)
			free(dist[i]);
		if (NULL != NN)
			free(NN[i]);
	}
	free(dist);
	dist = NULL;
	free(NN);
	NN = NULL;
	
	return;
}


void selector::mergeOffspring()
{
	int i;
	
	assert(pp_all->size + pp_new->size <= pp_all->maxsize);
	
	for (i = 0; i < pp_new->size; i++)
	{
		pp_all->ind_array[pp_all->size + i] = pp_new->ind_array[i];
	}
	
	pp_all->size += pp_new->size;
	
	free_pop(pp_new);
	pp_new = NULL;
}


void selector::calcFitnesses()
{
	int i, j;
	int size;
	int *strength;
	
	size = pp_all->size;
	strength = (int*) chk_malloc(size * sizeof(int));
	
	/* initialize fitness and strength values */
	for (i = 0; i < size; i++)
	{
		pp_all->ind_array[i]->fitness = 0;
		strength[i] = 0;
		fitness_bucket[i] = 0;
		fitness_bucket_mod[i] = 0;	
		for (j = 0; j < size; j++)
		{
	    fitness_bucket[i * size + j] = 0;
		}
	}
	
	/* calculate strength values */
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			if (dominates(pp_all->ind_array[i], pp_all->ind_array[j]))
	    {
				strength[i]++;
	    }
		}
	}
	
	/* Fitness values =  sum of strength values of dominators */
	for (i = 0; i < size; i++)
	{
		int sum = 0;
		for (j = 0; j < size; j++)
		{
			if (dominates(pp_all->ind_array[j], pp_all->ind_array[i]))
	    {
				sum += strength[j];
	    }
		}
		pp_all->ind_array[i]->fitness = sum;
		fitness_bucket[sum]++;
		fitness_bucket_mod[(sum / size)]++;
	}
	
	free(strength);
	strength = NULL;
	
	return;
}


void selector::calcDistances()
{
	int i, j;
	int size = pp_all->size;
	
	/* initialize copies[] vector and NN[][] matrix */
	for (i = 0; i < size; i++)
	{
		copies[i] = 1;
		for (j = 0; j < size; j++)
		{
	    NN[i][j] = -1;
		}
		
	}
	
	/* calculate distances */
	for (i = 0; i < size; i++)
	{
		NN[i][0] = i;
		for (j = i + 1; j < size; j++)
		{
	    dist[i][j] = calcDistance(pp_all->ind_array[i], pp_all->ind_array[j]);
//G	    cout << "distance is: " << dist[i][j] << endl;
//G	    assert(dist[i][j] < PISA_MAXDOUBLE);
	    dist[j][i] = dist[i][j];
	    if (dist[i][j] == 0)
	    {
				NN[i][copies[i]] = j;
				NN[j][copies[j]] = i;
				copies[i]++;
				copies[j]++;
	    }
		}
		dist[i][i] = 0;
	}
}


int selector::getNN(int index, int k)
/* lazy evaluation of the k-th nearest neighbor
pre-condition: (k-1)-th nearest neigbor is known already */
{
	assert(index >= 0);
	assert(k >= 0);
	assert(copies[index] > 0);
	
	if (NN[index][k] < 0)
	{
		int i;
		double min_dist = PISA_MAXDOUBLE;
		int min_index = -1;
		int prev_min_index = NN[index][k-1];
		double prev_min_dist = dist[index][prev_min_index];
		assert(prev_min_dist >= 0);
		
		for (i = 0; i < pp_all->size; i++)
		{
	    double my_dist = dist[index][i];
	    
	    if (my_dist < min_dist && index != i)
	    {
				if (my_dist > prev_min_dist ||
						(my_dist == prev_min_dist && i > prev_min_index))
				{
					min_dist = my_dist;
					min_index = i;
				}
	    }
		}
		
		NN[index][k] = min_index;
	}
	
	return (NN[index][k]);
}


double selector::getNNd(int index, int k)
/* Returns the distance to the k-th nearest neigbor
if this individual is still in the population.
For for already deleted individuals, returns -1 */
{
	int neighbor_index = getNN(index, k);
	
	if (copies[neighbor_index] == 0)
		return (-1);
	else
		return (dist[index][neighbor_index]);
}


void selector::environmentalSelection()
{
	int i;
	int new_size = 0;
	
	if (fitness_bucket[0] > alpha)
	{
		truncate_nondominated();
	}
	else if (pp_all->size > alpha)
	{
		truncate_dominated();
	}
	
	/* Move remaining individuals to top of array in 'pp_all' */
	for (i = 0; i < pp_all->size; i++)
	{
		ind* temp_ind = pp_all->ind_array[i];
		if (temp_ind != NULL)
		{
	    assert(copies[i] > 0);	    
	    pp_all->ind_array[i] = NULL;
	    pp_all->ind_array[new_size] = temp_ind;
	    old_index[new_size] = i;
	    new_size++;    
		}
	}
	assert(new_size <= alpha);
	pp_all->size = new_size;
	
	return;
}


void selector::truncate_nondominated()
/* truncate from nondominated individuals (if too many) */
{
	int i;	
	
	/* delete all dominated individuals */
	for (i = 0; i < pp_all->size; i++)
	{
		if (pp_all->ind_array[i]->fitness > 0)
		{
	    free_ind(pp_all->ind_array[i]);
	    pp_all->ind_array[i] = NULL;
	    copies[i] = 0;
		}
	}
	
	/* truncate from non-dominated individuals */
	while (fitness_bucket[0] > alpha)
	{
		int *marked;
		int max_copies = 0;
		int count = 0;
		int delete_index;
		
		marked = (int*) chk_malloc(pp_all->size * sizeof(int));
		
		/* compute inds with maximal copies */
		for (i = 0; i < pp_all->size; i++)
		{
	    if (copies[i] > max_copies)
	    {
				count = 0;
				max_copies = copies[i];
	    }
	    if (copies[i] == max_copies)
	    {
				marked[count] = i;
				count++;
	    }
		}
		
		assert(count >= max_copies);
		
		if (count > max_copies)
		{    
	    int *neighbor;
	    neighbor = (int*) chk_malloc(count * sizeof(int));
	    for (i = 0; i < count; i++)
	    {
				neighbor[i] = 1;  /* pointers to next neighbor */
	    }
	    
	    while (count > max_copies)
	    {
				double min_dist = PISA_MAXDOUBLE;
				int count2 = 0;
				
				for (i = 0; i < count; i++)
				{
					double my_dist = -1;
					while (my_dist == -1 && neighbor[i] < pp_all->size)
					{
						my_dist = getNNd(marked[i],neighbor[i]);
						neighbor[i]++;
					}
					
					if (my_dist < min_dist)
					{
						count2 = 0;
						min_dist = my_dist;
					}
					if (my_dist == min_dist)
					{
						marked[count2] = marked[i];
						neighbor[count2] = neighbor[i];
						count2++;
					}
				}
				count = count2;
				if (min_dist == -1) /* all have equal distances */
				{
					break;
				}
	    }   
	    free(neighbor);
			neighbor = NULL;
		}
		
		/* remove individual from population */
		delete_index = marked[irand(count)];
		free_ind(pp_all->ind_array[delete_index]);
		pp_all->ind_array[delete_index] = NULL;
		for (i = 0; i < count; i++)
		{
	    if (dist[delete_index][marked[i]] == 0)
	    {
				copies[marked[i]]--;
	    }
		}
		copies[delete_index] = 0; /* Indicates that this index is empty */
		fitness_bucket[0]--;
		fitness_bucket_mod[0]--;
		free(marked);
		marked = NULL;
	}
	
	return;
}


void selector::truncate_dominated()
/* truncate from dominated individuals */
{
	int i, j;
	int size;
	int num = 0;
	
	size = pp_all->size;
	
	i = -1;
	while (num < alpha)
	{
		i++;
		num += fitness_bucket_mod[i];
	}
	
	j = i * size;
	num = num - fitness_bucket_mod[i] + fitness_bucket[j];
	while (num < alpha)
	{
		j++;
		num += fitness_bucket[j];
	}
	
	if (num == alpha)
	{
		for (i = 0; i < size; i++)
		{
	    if (pp_all->ind_array[i]->fitness > j)
	    {
				free_ind(pp_all->ind_array[i]);
				pp_all->ind_array[i] = NULL;
	    }
		}
	}
	else /* if not all fit into the next generation */
	{
		int k;
		int free_spaces;
		int fill_level = 0;
		int *best = NULL;
		
		free_spaces = alpha - (num - fitness_bucket[j]);
		best = (int*) chk_malloc(free_spaces * sizeof(int));
		for (i = 0; i < size; i++)
		{
	    if (pp_all->ind_array[i]->fitness > j)
	    {
				free_ind(pp_all->ind_array[i]);
				pp_all->ind_array[i] = NULL;
	    }
	    else if (pp_all->ind_array[i]->fitness == j)
	    {
				if (fill_level < free_spaces)
				{
					best[fill_level] = i;
					fill_level++;
					for (k = fill_level - 1; k > 0; k--)
					{
						int temp;
						if (getNNd(best[k], 1) <= getNNd(best[k - 1], 1))
						{
							break;
						}
						temp = best[k];
						best[k] = best[k-1];
						best[k-1] = temp;
					}
				}
				else
				{
					if (getNNd(i, 1) <= getNNd(best[free_spaces - 1], 1))
					{
						free_ind(pp_all->ind_array[i]);
						pp_all->ind_array[i] = NULL;
					}
					else
					{
						free_ind(pp_all->ind_array[best[free_spaces - 1]]);
						pp_all->ind_array[best[free_spaces - 1]] = NULL;
						best[free_spaces - 1] = i;
						for (k = fill_level - 1; k > 0; k--)
						{
							int temp;
							if (getNNd(best[k], 1) <= getNNd(best[k - 1], 1))
							{
								break;
							}
							temp = best[k];
							best[k] = best[k-1];
							best[k-1] = temp;
						}
					}
				}
	    }
		}        
		free(best);
		best = NULL;
		return;
	}
}


void selector::matingSelection()
/* Fills mating pool 'pp_sel' */
{
	int i, j;
	
	for (i = 0; i < mu; i++)
	{
		int winner = irand(pp_all->size);
		
		for (j = 1; j < tournament; j++)
		{
	    int opponent = irand(pp_all->size);
	    if (pp_all->ind_array[opponent]->fitness
					< pp_all->ind_array[winner]->fitness || winner == opponent)
	    {
				winner = opponent;
	    }
	    else if (pp_all->ind_array[opponent]->fitness
							 == pp_all->ind_array[winner]->fitness)
	    {
				if (dist[old_index[opponent]][getNN(old_index[opponent], 1)] >
						dist[old_index[winner]][getNN(old_index[winner], 1)])
				{
					winner = opponent;
				}
	    }
		}  
		pp_sel->ind_array[i] = pp_all->ind_array[winner];
	}
	pp_sel->size = mu;
}


void selector::select_initial()
/* Performs initial selection. */
{
	selection();
}


void selector::select_normal()
/* Performs normal selection.*/
{
	selection();
}


int selector::dominates(ind *p_ind_a, ind *p_ind_b)
/* Determines if one individual dominates another.
Minimizing fitness values. */
{
	int i;
	int a_is_worse = 0;
	int equal = 1;
	
	for (i = 0; i < dim && !a_is_worse; i++)
	{
		a_is_worse = p_ind_a->f[i] > p_ind_b->f[i];
		equal = (p_ind_a->f[i] == p_ind_b->f[i]) && equal;
	}
	
	return (!equal && !a_is_worse);
}


int selector::is_equal(ind *p_ind_a, ind *p_ind_b)
/* Determines if two individuals are equal in all objective values.*/
{
	int i;
	int equal = 1;
	
	for (i = 0; i < dim; i++)
		equal = (p_ind_a->f[i] == p_ind_b->f[i]) && equal;
	
	return (equal);
}


double selector::calcDistance(ind *p_ind_a, ind *p_ind_b)
{
	int i;
	double distance = 0;
	
	if (is_equal(p_ind_a, p_ind_b) == 1)
	{
		return (0);
	}
	
	for (i = 0; i < dim; i++)
		distance += pow(p_ind_a->f[i]-p_ind_b->f[i],2);
	
	if (0.0 == distance)
		distance = PISA_MINDOUBLE;
	
	return (sqrt(distance));
}

/*--------------------| data exchange functions |------------------------*/

int selector::read_ini(){
	int i;
	pp_new = create_pop(alpha, dim);
	
	for (i = 0; i < alpha; i++)
	  pp_new->ind_array[i] = create_ind(dim);
	pp_new->size = alpha;
		
	return (read_pop(ini_pop, pp_new, alpha, dim));
}


int selector::read_var(){
	int i;
	pp_new = create_pop(lambda, dim);
	
	for (i = 0; i < lambda; i++)
		pp_new->ind_array[i] = create_ind(dim);
	
	pp_new->size = lambda;
	
	return (read_pop(var_pop, pp_new, lambda, dim));
}


void selector::write_sel(){		
	write_pop(sel_indexes, pp_sel, mu);
}


void selector::write_arc(){
	write_pop(arc_indexes, pp_all, pp_all->size);
}


/*---------- I/O functions ----------*/

int selector::read_pop(population *origin, pop *pp, int size, int dim){
	assert(dim >= 0);
	assert(pp != NULL);
	
	for (int j = 0; j < size; j++){
		pp->ind_array[j]->index = (*origin)[j].index();
		assert(dim == (*origin)[j].objectives_dim());
		for (int i = 0; i < dim; i++){
			pp->ind_array[j]->f[i] = (*origin)[j].objectives[i];
			assert(pp->ind_array[j]->f[i] >= 0);
		}
	}      
	return (0);
}

void selector::write_pop(int *destination, pop* pp, int size){
	assert(size >= 0);
	assert(size <= pp->size);
	for (int i = 0; i < size; i++)
	{
		destination[i] = pp->ind_array[i]->index;
//		cout << destination[i] << " ";
	}
//	cout << endl;
}
