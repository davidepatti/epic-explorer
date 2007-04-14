/*---------------------------------------------------------------------------------------------*
 * This is the class for Niched Pareto Genetic Algorithm, by:                                  *
 *                                                                                             *
 * Jeffrey Horn and Nicholas Nafpliotis. Multiobjective Optimization using the Niched Pareto   *
 * Genetic Algorithm. Technical Report IlliGAL Report 93005, University of Illinois at         *
 * Urbana, Illinois, USA, 1993                                                                 *
 *                                                               Xianming Chen, Feb 16, 2001   *
 *---------------------------------------------------------------------------------------------*/

  
#ifndef _NPGA_H_
#define _NPGA_H_
#include "MOEA.h"
#include <strstream>

using namespace std;

#define DEFAULT_COMP_SIZE    10

class NPGA : public MOEA {
 public:
  MOEADefineIdentity("NPGA", "NPGA", 303);

  NPGA() : compSz(DEFAULT_COMP_SIZE) { 
    nicheRadius(DEFAULT_NICHE_RADIUS);
    pop1Size(DEFAULT_POP1_initSIZE); 
    paraList.add("size of comparison set", "comparisonSetSize", "cmpSz", Int, &compSz);
  }
  virtual ~NPGA() { }
  NPGA (const NPGA&)            {cerr<<"NPGA copy constructor not available\n";exit(-1);}
  NPGA& operator= (const NPGA&) { cerr<<"NPGA assignment not available\n";exit(-1);}

  int comparisonSetSize() const { return compSz; }
  int comparisonSetSize(int s)  { return compSz = s; }

  virtual void  initialize(const IND& ind) {
    MOEA::initialize(ind);
    delete ppop1; assert(pop1_initSz == pop1_maxSz);
    ppop1  = new POP;
    ppop1->initialize(ind,pop1_initSz, this); //randomly generate individuals.
    delete slctor; 
    slctor = new ParetoTournamentSelector(compSz, niche_radius);
  } 

  virtual void  step() {
    POP* pNewPop = ppop1->clone();
    generate(*pNewPop);
    POP* tmp  = ppop1; 
    ppop1 = pNewPop;
    delete tmp;
    nextGeneration();
  }

 protected:
  int compSz;  // size of comparison set used in domination tournament selection.
};
#endif
