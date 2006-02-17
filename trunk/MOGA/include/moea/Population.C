#include "Population.h"
#include <string>
#include <iomanip>

POP::POP(int chunkSize, class MOEA* pA) : 
  curSz(0), ppind(NULL), curBd(0), chunkSz(chunkSize), pAlgo(pA) { }

void POP :: initialize(const IND& ind, int no, class MOEA* pA) {
  for(int i=0; i<curSz; i++) 
    delete ppind[i];
  if(no>curBd) {
    while(no>curBd) curBd += chunkSz;
    delete ppind;
    ppind = new IND*[curBd];
  }
  memset(ppind, 0, sizeof(IND*)*curBd);  
  for(int i=0; i<no; i++)
    ppind[i] = ind.randomClone();

  curSz = no;
  pAlgo = pA;
}


void POP :: copy( const POP & other) {
  if(this == &other) return;
  for(int i=0; i<curSz; i++) delete ppind[i];
  if(curBd != other.curBd) {
    delete ppind;
    ppind = new IND* [curBd = other.curBd];
  }
  memset(ppind, 0, sizeof(IND*)*curBd);  
  curSz = other.curSz; 
  for(int i=0; i<curSz; i++) 
    ppind[i] = other.ppind[i]->clone();

  pAlgo = other.pAlgo;
}



void POP :: replace(int index, IND* ind) {
  assert(curSz && index >= 0 && index < curSz);
  delete ppind[index];
  ppind[index] = ind;
}


void POP :: append(IND* ind) {
  if(curSz == curBd) {
    IND** new_ppind = new IND* [ curBd += chunkSz ];
    memset(new_ppind, 0, sizeof(IND*)*curBd);  
    for(int i=0; i<curSz; i++)
      new_ppind[i] = ppind[i];
    IND** tmp = ppind;
    ppind = new_ppind; delete tmp;
  }
  ppind[curSz++] = ind;
}

void POP :: del(int index) {
  assert(curSz && index >= 0 && index < curSz);
  delete ppind[index]; curSz--;
  if(index != curSz)
    ppind[index] = ppind[curSz];
}

IND* POP :: remove(int index) {
  assert(curSz && index >= 0 && index < curSz);
  IND* rt = ppind[index]; curSz--;
  if(index != curSz)
    ppind[index] = ppind[curSz];
  return rt;
}

void POP::cull()  // cull dominated individuals from population.
{
  for(int i=0; i<curSz-1; i++) 
    for(int j=i+1; j<curSz; j++) 
      switch(ppind[i]->compare(*ppind[j])) {
      case SAME       : del(j--); break;
      case DOMINATING:	del(j--); break;
      case DOMINATED : del(i--); j = curSz; break;
      }
}  
    


//output phenotypic individuals in the population.
ostream& operator<< (ostream& os, const POP& pop) {
  cout << "currently total " << pop.curSz << " individuals in this popualtion.\n";
  for(int i=0; i<pop.curSz; i++) 
    os << *pop.ppind[i] << endl;
  return os;
}


void POP :: printGenomes(ostream& os) const {
  os << "currently total " << curSz << " individuals in this popualtion.\n";
  for(int i=0; i<curSz; i++) {
    ppind[i]->printGenome(os);
    os << endl;
  }
}


void POP :: printFitnesses(ostream& os) const {
  for(int i=0; i<curSz; i++) {
    if(i%4 == 0) os << endl << setw(2) << i << ":";
    os << "\t" << ppind[i]->fitness(); 
  }
  os << endl;
}

void POP :: printScores(ostream& os) const {
  for(int i=0; i<curSz; i++) {
    if(&os != &cout) {
      for(int d=0; d<dimens; d++) 
	os << ppind[i]->objectiveVector()[d] << '\t';
      os << endl;
    } 
    else {
      if(i%4 == 0) cout << endl << setw(2) << i << ":";
      cout << "\t" << ppind[i]->objectiveVector(); 
    }
  }
  os << endl;
}

void POP :: printAll(ostream& os) const {
  os << "currently total " << curSz << " individuals in this popualtion.\n";
  for(int i=0; i<curSz; i++) {
    ppind[i]->printGenome(os); os << endl;
    os << *ppind[i] << '\n' << ppind[i]->objectiveVector() << '\n' << ppind[i]->fitness() << endl;
  }
}
