#include "Genome.h"
#include "Bin2DecGenome.h"

#ifdef DEBUG
int IND::seq = 0;
#endif

IND::Distance  IND::distFp = 0;
IND::Evaluator IND::evalFp = 0;
void * IND::user_data = 0;

IND :: IND(int ln, Evaluator eFp, int alleles) 
: AlleleString(ln, alleles), pscores(new ObjectiveVector), 
  scores_valid(false) 
{ 
  assert(++seq); 
  if(evalFp && evalFp != eFp) 
    cout << "evaluator changed!\n";
  evalFp = eFp;
}

IND :: IND(vector<vector<Allele> > alss, Evaluator eFp, void *_user_data) 
: AlleleString(alss), pscores(new ObjectiveVector), 
  scores_valid(false) 
{ 
  assert(++seq); 
  if(evalFp && evalFp != eFp) 
    cout << "evaluator changed!\n";
  evalFp = eFp;
  user_data = _user_data;
}


void IND :: copy(const IND& other) {
  if(this == &other) return;
  AlleleString :: copy(other);
  scores_valid = other.scores_valid;   fit = other.fit;
  if(pscores) pscores->copy(*other.pscores);
  else        pscores = other.pscores->clone();
}

void IND :: crossover(float pcross, const IND& dad, IND*& sis, IND*& bro)const {

  sis = clone(); 
  bro = dad.clone(); 

  int pos;
  if( Rand() < pcross ) { // cross over occurs.
    pos = Randint(1, len - 1); //crossover position.
    for(int j=pos; j<len; j++)
      (*sis)[j] = dad[j];
    for(int j=pos; j<len; j++)
      (*bro)[j] = (*this)[j];
    sis->invalidateCaches();
    bro->invalidateCaches();
  }
}  

void IND :: mutate(float pmut) {
  int   assuredMutated = (int) (len * pmut); 
  float randomMutated = len * pmut - assuredMutated;
  int pos;  Allele gene;

  for(int j=0; j<assuredMutated; j++) {
    //    gene = Randint(0,_alleles-1) + (_alleles>10 ? 'a':'0');
    pos = Randint(0, len - 1);
    gene = alleles_set[pos][Randint(0, alleles_set[pos].size()-1)];
    if((*this)[pos] != gene) {
      invalidateCaches();
      (*this)[pos] = gene;
    }
  }
  if(Rand() < randomMutated) {  //mutatation occurs.
    //    gene = Randint(0,_alleles-1) + (_alleles>10 ? 'a':'0');
    pos = Randint(0, len - 1);
    gene = alleles_set[pos][Randint(0, alleles_set[pos].size()-1)];
    if((*this)[pos] != gene) {
      invalidateCaches();
      (*this)[pos] = gene;
    }
  }
}


const ObjectiveVector& IND::objectiveVector() const { 
  if(!scores_valid) {
    IND* This = (IND*)this;
    (*evalFp)(*This, *pscores, user_data);
    This->scores_valid = true;
  }
  return *pscores; 
}

// The overloaded << operator always output individual in its phenotypic form, 
// This is same as genotypic form for some kind of ind, while different for others.
#include "Bin2DecGenome.h"
ostream& operator<< (ostream & os,const IND& ind) {
  if(ind.classID() != 101) // not Bin2DecIND.
    return operator<< (os, (const AlleleString&) ind);

  Bin2DecIND& Ind = (Bin2DecIND&) ind;
  os << "(";  
  int i = 0;
  for(; i<Ind.phenotypicVector().dimensions()-1; i++)
    os << Ind.phenotype(i) << ", ";
  os << Ind.phenotype(i) << ")";
  return os;
}

