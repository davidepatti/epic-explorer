#include "Assign.h"
#include "MOEA.h"

void Assigner::initialize(const POP* p1, const POP* p2) { 
  ppop1=p1; ppop2=p2; 
  if(ppop1) sz1 = ppop1->currentSize();
  if(ppop2) sz2 = ppop2->currentSize();
} 


// assigner for SPEA.
void StrengthAssigner::_calcStrength() { 
  IND*  nondomInd;
  int   popsz = ppop1->currentSize();
  int   domCount;
  
  for (int i = ppop2->currentSize() - 1; i >= 0; i--) {
    nondomInd = (*ppop2)[i];
    domCount = 0;
    for (int j = popsz - 1; j >= 0; j--)
      if ( *nondomInd >= *(*ppop1)[j] )
	domCount++;
    nondomInd->fitness( domCount / double(popsz + 1) );
  }
}

void StrengthAssigner::_calcFitness() { 
  IND     *ind, *xind;  //ind in pop and pop2.
  double  fitness;
  
  for (int i = ppop1->currentSize() - 1; i >= 0; i--) {
    ind = (*ppop1)[i];  fitness = 0.0;
    for (int j = ppop2->currentSize() - 1; j >= 0; j--) 
      if( *(xind = (*ppop2)[j]) >= *ind )
	fitness += xind->fitness();
    ind->fitness(1.0 + fitness);
  }
}




// "Peeling off" assigner used in NSGA.
void NSGAassigner::initialize(const POP* p1, const POP* p2) { 
  assert(!p2);
  Assigner::initialize(p1);
  niche_radius = ppop1->algorithm().nicheRadius();
  delete not_cur_nondominated; not_cur_nondominated = new bool [sz1];
  delete niche_count;          niche_count = new double[sz1];
} 


void NSGAassigner::assignFitnesses() {
  remainedSz = sz1;
  for(int i=0; i<sz1; i++) niche_count[i] = 0;

  double dummyFitness = (double) sz1;
  while(remainedSz) {
    for(int i=0; i<sz1; i++) { // mark currently dominated.
      if(niche_count[i]) // a positive value indicating fitness assigned in previous iteration.
	continue;        // while a value of '-1' indicating dominated in this loop.
      for(int j=i+1; j<sz1; j++) {
	if(niche_count[j]) 
	  continue;        
	switch((*ppop1)[i]->compare(*(*ppop1)[j])) {
	case DOMINATING: niche_count[j]--; break;
	case DOMINATED : niche_count[i]-- ; j = sz1; break;
	}
      }
    }

    for(int i=0; i<sz1; i++) 
      not_cur_nondominated[i] = niche_count[i]? true : false;

    for(int i=0; i<sz1; i++) { // calculate niche count.
      if(not_cur_nondominated[i]) continue;
      for(int j=0; j<sz1; j++) {
	if(not_cur_nondominated[j]) continue;
	double share = (*ppop1)[i]->distance(*(*ppop1)[j]);
	if(share >= niche_radius) 
	  share = 0;
	else 	  
	  share = 1 - (share/niche_radius) * (share/niche_radius);
	niche_count[i] += share; niche_count[j] += share;
      }
    }

    double max_count = 0, cur_count;
    for(int i=0; i<sz1; i++) { // dummy fitness reduction by niche count.
      if(not_cur_nondominated[i]) continue;
      if((cur_count=niche_count[i]) > max_count) max_count = cur_count;
      (*ppop1)[i]->fitness(cur_count/dummyFitness);
    }
    dummyFitness = dummyFitness/max_count * 0.99;

    remainedSz = 0; // mark remained individual for next peeling off.
    for(int i=0; i<sz1; i++) {
      if(niche_count[i] == -1) {
	niche_count[i]++;
	remainedSz++;
      }
    }
  }
}

