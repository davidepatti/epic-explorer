/*
 *  containers.h
 *  spea2
 *
 *  Created by Gianmarco De Francisci Morales [gmorales(at)diit.unict.it] on 23/11/07.
 *
 */

#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <vector>
#include <iostream>

static unsigned int mylog2 (unsigned int val) {
    const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
    const unsigned int S[] = {1, 2, 4, 8, 16};
    register unsigned int ret = 0; // result of log2(v) will go here
    for (int i = 4; i >= 0; i--) {
	if (val & b[i]) {
		val >>= S[i];
		ret |= S[i];
	} 
    }
    return ret;
}

/*---ALLELE---*/
typedef int allele;


/*---ALLELESET---*/
typedef std::vector<allele> alleleset;


/*---INDIVIDUAL---*/
class individual {
private:
	static unsigned int uid;
	static std::vector<alleleset> als;
	static unsigned int als_sum;

	unsigned int _index;
	unsigned int _chromosome_dim;
	unsigned int _objectives_dim;
	allele* chromosome;

	unsigned int adjusted_rand_allele();

public:
	static void setAllelesets(std::vector<alleleset> a) {
		individual::als = a;
		unsigned int tmp = 0;
		for(unsigned int i=0; i<individual::als.size(); i++)
			tmp += mylog2(individual::als.at(i).size());
		individual::als_sum = tmp;
	}
	static std::vector<alleleset> getAllelesets() {return individual::als;}

	individual(int chromo_dim, int obj_dim);
	individual(const individual&);
	individual& operator=(const individual&);
	~individual();

	int index() const {return _index;}
	int chromosome_dim() const {return _chromosome_dim;}
	int objectives_dim() const {return _objectives_dim;}

	individual child() const;
	allele phenotype(int i) const {return individual::als[i][chromosome[i]];}

	void randomize();
	void bit_mutation();
	void mutation(double pmut);
	void adjusted_mutation(double pmut);
	void crossover(individual&);
	void uniform_crossover(individual&);
	void half_uniform_crossover(individual&);
	void adjusted_crossover(individual&);

	double* objectives;
};


/*---POPULATION---*/
/*
class population: public std::vector<individual> {
public:
	population();
	population(const population& clone);
	population& operator=(const population&);
	~population();
};
*/
typedef std::vector<individual> population;
std::ostream& operator<<(std::ostream&, const population&);


/*---COMMON---*/
class common {
public:
	common(int a, int m, int l, int d);
	~common();
	
	/* common parameters */
	const int alpha;	/* number of individuals in initial population */
	const int mu;			/* number of individuals selected as parents */
	const int lambda;	/* number of offspring individuals */
	const int dim;		/* number of objectives */

	/* shared memory comunication between selector and variator */
	population* ini_pop;
	population* var_pop;
	int* sel_indexes;
	int* arc_indexes;
};

#endif
