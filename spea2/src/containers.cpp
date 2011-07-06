/*
 *  containers.cpp
 *  spea2
 *
 *  Created by Gianmarco De Francisci Morales [gmorales(at)diit.unict.it] on 24/11/07.
 *
 */

#include <assert.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <list>
#include "utility.h"
#include "containers.h"

using namespace std;

/*---INDIVIDUAL---*/
unsigned int individual::uid = 0;
vector<alleleset> individual::als;
unsigned int individual::als_sum = 0;

individual::individual(int chromo_dim, int obj_dim)
: _chromosome_dim(chromo_dim),
  _objectives_dim(obj_dim) {
	assert(_chromosome_dim > 0);
	assert(_objectives_dim > 0);
	assert(_chromosome_dim == individual::als.size());
	chromosome = new allele[_chromosome_dim];
	objectives = new double[_objectives_dim];
	_index = individual::uid++;
}

individual::individual(const individual& clone)
: _index(clone._index),
  _chromosome_dim(clone._chromosome_dim),
  _objectives_dim(clone._objectives_dim),
  chromosome(new allele[clone._chromosome_dim]),
  objectives(new double[clone._objectives_dim]) {
	//FIXME use std::copy
	memcpy(this->chromosome, clone.chromosome, _chromosome_dim*sizeof(allele));
	memcpy(this->objectives, clone.objectives, _objectives_dim*sizeof(double));
}

individual& individual::operator=(const individual& other){
	_index = other._index;
	_chromosome_dim = other._chromosome_dim;
	_objectives_dim = other._objectives_dim;
	delete[] chromosome;
	delete[] objectives;
	chromosome = new allele[_chromosome_dim];
	objectives = new double[_objectives_dim];
	//FIXME use std::copy
	memcpy(this->chromosome, other.chromosome, _chromosome_dim*sizeof(allele));
	memcpy(this->objectives, other.objectives, _objectives_dim*sizeof(double));
	return *this;
}

individual::~individual(){
	delete[] chromosome;
	delete[] objectives;
}

individual individual::child() const {
	// create a NEW individual with the same chromosome as this
	individual tmp(*this);
	tmp._index = individual::uid++;
	return tmp;
}

void individual::randomize(){
	for (unsigned int i=0; i<_chromosome_dim; i++){
		this->chromosome[i]=irand(individual::als[i].size());
	}
}

void individual::crossover(individual& other){
	unsigned int section_point = 1+irand(_chromosome_dim-1);
	//cout << "section_point: " << section_point << endl;
	assert(0 < section_point && section_point < _chromosome_dim);
	unsigned int rest = _chromosome_dim - section_point;
	allele* tmp = new allele[rest];
	// WARNING assuming "shallow copy"-able chromosome
	// implements 1 point crossover
	//FIXME use std::swap
	memcpy(tmp, other.chromosome+section_point, rest*sizeof(allele));
	memcpy(other.chromosome+section_point, this->chromosome+section_point, rest*sizeof(allele));	
	memcpy(this->chromosome+section_point, tmp, rest*sizeof(allele));
}

void individual::uniform_crossover(individual& other){
	allele tmp;
	for(unsigned int i=0; i < _chromosome_dim; i++){
		if(drand(1) < 0.5)
			tmp = this->chromosome[i];
			this->chromosome[i] = other.chromosome[i];
			other.chromosome[i] = tmp;
	}
}

void individual::half_uniform_crossover(individual& other){
	int distance=0, half, j, k;
	allele tmp;
	list<int> positions;
	list<int>::iterator it;

	// find hamming distance and bookkeep differing genes' position	
	for(unsigned int i=0; i < _chromosome_dim; i++){
		if(this->chromosome[i] != other.chromosome[i]){
			distance++;
			positions.push_back(i);
		}
	}
	half = distance / 2;
	// draw half of the genes for crossover
	for(int i=0; i < half; i++){
		// draw a random gene
		j = irand(positions.size());
		it = positions.begin();
		advance(it, j);
		k = *it;
		// swap the selected gene
		tmp = this->chromosome[k];
		this->chromosome[k] = other.chromosome[k];
		other.chromosome[k] = tmp;
		// remove gene from the pool
		positions.erase(it);
	}
	//delete positions;
}

void individual::adjusted_crossover(individual& other){
	unsigned int section_point = adjusted_rand_allele();
	//cout << "section_point: " << section_point << endl;
	assert(0 <= section_point && section_point < _chromosome_dim);
	unsigned int rest = _chromosome_dim - section_point;
	allele* tmp = new allele[rest];
	// WARNING assuming "shallow copy"-able chromosome
	// implements 1 point crossover
	//FIXME use std::swap
	memcpy(tmp, other.chromosome+section_point, rest*sizeof(allele));
	memcpy(other.chromosome+section_point, this->chromosome+section_point, rest*sizeof(allele));	
	memcpy(this->chromosome+section_point, tmp, rest*sizeof(allele));

}

void individual::bit_mutation(){
	// implements random 1 gene mutation
	int mutation_point = irand(_chromosome_dim);
	// WARNING assuming int alleles
 	this->chromosome[mutation_point] = irand(als[mutation_point].size());
}

void individual::mutation(double pmut){
	int assuredmutated = (int) (_chromosome_dim * pmut);
	double randommutated = _chromosome_dim * pmut - assuredmutated;
	int pos;
	for(int j=0; j < assuredmutated; j++){
		pos = irand(_chromosome_dim);
		this->chromosome[pos] = irand(als[pos].size());
	}
	if(drand(1) < randommutated){  //mutatation occurs.
		pos = irand(_chromosome_dim);
		this->chromosome[pos] = irand(als[pos].size());
	}
}

void individual::adjusted_mutation(double pmut) {
	int assuredmutated = (int) (_chromosome_dim * pmut);
	double randommutated = _chromosome_dim * pmut - assuredmutated;
	int pos;
	for(int j=0; j < assuredmutated; j++){
		pos = adjusted_rand_allele();
		this->chromosome[pos] = irand(als[pos].size());
	}
	if(drand(1) < randommutated){  //mutatation occurs.
		pos = adjusted_rand_allele();
		this->chromosome[pos] = irand(als[pos].size());
	}
}

unsigned int individual::adjusted_rand_allele() {
	unsigned int rand_allele;
	unsigned int allele_selector = irand(individual::als_sum);
	//cout << "allele_selector: " << allele_selector << endl;
	for (rand_allele = 0; rand_allele < individual::als.size()
		&& allele_selector > mylog2(individual::als.at(rand_allele).size()); rand_allele++)
		allele_selector -= mylog2(individual::als.at(rand_allele).size());
	//cout << "rand_allele: " << rand_allele << endl;
	return rand_allele;
}

std::ostream& operator<<(std::ostream& os, const individual& ind){
	os << ind.index() << "\t-> ";
	for(int i=0; i<ind.chromosome_dim(); i++){
		os << ind.phenotype(i) << '-';
	}
	os << "\b \tf: ";
	os << setiosflags(ios::fixed) << setprecision(14);
	for(int i=0; i<ind.objectives_dim(); i++){
		os << ind.objectives[i] << ' ';
	}
	os << endl;
	return os;
}

/*---POPULATION---*/
// population ctors only to call default vector ctors
/*
population::population(): std::vector<individual>(){}
population::population(const population& clone): std::vector<individual>(clone){}
population::~population(){};
*/

std::ostream& operator<<(std::ostream& os, const population& pop){
	//for (population::iterator it=(population::iterator)pop.begin(); it!=pop.end(); it++){
	 for (population::const_iterator it=pop.begin(); it!=pop.end(); it++){
		os << "IND " << *it;
	}
	return os;
}


/*---COMMON---*/
common::common(int a, int m, int l, int d)
: alpha(a),
  mu(m),
  lambda(l),
  dim(d),
  ini_pop(new population()),
  var_pop(new population()),
  sel_indexes(new int[m]),
  arc_indexes(new int[a+l]) {
	assert(alpha > 0);
	assert(mu > 0);
	assert(lambda > 0);
	assert(dim > 0);
}

common::~common(){
	delete ini_pop;
	delete var_pop;
	delete[] sel_indexes;
	delete[] arc_indexes;
}
