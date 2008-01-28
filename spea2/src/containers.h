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

/*---ALLELE---*/
typedef int allele;


/*---ALLELESET---*/
typedef std::vector<allele> alleleset;


/*---INDIVIDUAL---*/
class individual {
private:
	static int uid;
	static std::vector<alleleset> als;

	int _index;
	int _chromosome_dim;
	int _objectives_dim;
	allele* chromosome;

public:
	static void setAllelesets(std::vector<alleleset> a) {individual::als = a;}
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
	void crossover(individual&);

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
