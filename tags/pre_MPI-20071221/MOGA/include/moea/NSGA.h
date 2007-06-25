/*---------------------------------------------------------------------------------------------*
 * This is the class for Nondominated Sorting Genetic Algorithm, by:                           *
 *                                                                                             *
 * Srinivas, N. and K. Deb (1994). Multiobjective optimization using non-dominated sorting in  *
 * genetic algorithms. Evolutionary Computation 2(3),221-248.                                  *
 *                                                               Xianming Chen, Feb 20, 2001   *
 *---------------------------------------------------------------------------------------------*/

  
#ifndef _NSGA_H_
#define _NSGA_H_
#include "MOEA.h"

using namespace std;


class NSGA : public MOEA {
 public:
  MOEADefineIdentity("NSGA", "NSGA", 304);

  NSGA() { nicheRadius(DEFAULT_NICHE_RADIUS); pop1Size(DEFAULT_POP1_initSIZE); }
  virtual ~NSGA() { }
  NSGA (const NSGA&)            {cerr<<"NSGA copy constructor not available\n";exit(-1);}
  NSGA& operator= (const NSGA&) { cerr<<"NSGA assignment not available\n";exit(-1);}

  virtual void  initialize(const IND& ind) {
    MOEA::initialize(ind);
    delete ppop1;     assert(pop1_initSz == pop1_maxSz);
    ppop1  = new POP; ppop1->initialize(ind,pop1_initSz, this);
    delete slctor;   slctor = new BinarySelector;
    delete asiner;   asiner = new NSGAassigner;
  } 

  virtual void  step() {
    assignFitnesses();
    POP* pNewPop = ppop1->clone();
    generate(*pNewPop);
    POP* tmp  = ppop1; ppop1 = pNewPop; delete tmp;
    nextGeneration();
  }
};
#endif
