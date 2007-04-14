/*---------------------------------------------------------------------------------------------*
 * This is the class for Vector Evaluated Genetic Algorithm, by:                               *
 *                                                                                             *
 * Schaffer, J.D.(1985). Multi-objective optimization with vector evaluated genetic algorithms.*
 * In J.J.Grefenstette (Ed.), "Proceedings of an International Conference on Genetic Algorihtms*
 * and Their Applications", Pittsburgh, PA, pp.93-100. sponsored by Texas Instruments and U.S. *
 * Navy Center for Appliced Research in Artificial Intelligence(NCARAI).                       *
 *                                                                                             *
 *                                                               Xianming Chen, Feb 20, 2001   *
 *---------------------------------------------------------------------------------------------*/

  
#ifndef _VEGA_H_
#define _VEGA_H_
#include "MOEA.h"

#define DEFAULT_COMP_SIZE    10

using namespace std;

class VEGA : public MOEA {
 public:
  MOEADefineIdentity("VEGA", "VEGA", 305);

  VEGA() { pop1Size(DEFAULT_POP1_initSIZE); }
  virtual ~VEGA() { }
  VEGA (const VEGA&)            {cerr<<"VEGA copy constructor not available\n";exit(-1);}
  VEGA& operator= (const VEGA&) { cerr<<"VEGA assignment not available\n";exit(-1);}

  virtual void  initialize(const IND& ind) {
    MOEA::initialize(ind);
    delete ppop1; assert(pop1_initSz == pop1_maxSz);
    ppop1  = new POP;
    ppop1->initialize(ind,pop1_initSz, this); //randomly generate individuals.
    delete slctor; 
    slctor = new VEGATournamentSelector;
  } 

  virtual void  step() {
    POP* pNewPop = ppop1->clone();
    generate(*pNewPop);
    POP* tmp  = ppop1; 
    ppop1 = pNewPop;
    delete tmp;
    nextGeneration();
  }
};
#endif
