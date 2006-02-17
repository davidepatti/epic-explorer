/*---------------------------------------------------------------------------------------------*
 * This is the class for Strength Pareto Evolutionary Algorithms, designed in:                 *
 *                                                                                             *
 *     E. Zitzler, L. Thiele, "Multiobjective Evolutionary Algorithms: A Comparative Case Study*
 *     and the Strenghth Pareto Approch", IEEE Transactions on Evolutionary Computation.       *
 *                                                                                             *
 *                                                               Xianming Chen, Aug, 2000      *
 *                                                               Modified on Jan 17, 2001      *
 *---------------------------------------------------------------------------------------------*/

  
#ifndef _SPEA_H_
#define _SPEA_H_

#include "MOEA.h"

using namespace std;


class SPEA : public MOEA {
 public:
  MOEADefineIdentity("SPEA", "SPEA", 301);

  SPEA(){pop1Size(DEFAULT_POP1_initSIZE); pop2_initSz=0;pop2_maxSz=DEFAULT_POP2_maxSIZE;}
  virtual ~SPEA() { }
  SPEA (const SPEA&)            {cerr<<"SPEA copy constructor not available\n";exit(-1);}
  SPEA& operator= (const SPEA&) { cerr<<"SPEA assignment not available\n";exit(-1);}

  virtual void  initialize(const IND& ind) {
    MOEA::initialize(ind);

    delete ppop1; delete ppop2; ppop1  = new POP; ppop2 = new POP;
    assert(pop1_initSz==pop1_maxSz); assert(!pop2_initSz);
    ppop1->initialize(ind,pop1_initSz, this); 
    ppop2->initialize(ind,(pop2_initSz=0), this); 

    delete slctor; delete asiner; delete razor;
    slctor = new BinarySelector;
    asiner = new StrengthAssigner();
    razor = new ClusteringReducer();
  } 

  virtual void  step() {
    assignFitnesses();
    POP* pNewPop = ppop1->clone();
    generate(*pNewPop);

    ppop1->cull();
    ppop2->append(*ppop1);
    ppop2->cull();    // keep pop2 nondominated.

    POP* tmp = ppop1;  ppop1 = pNewPop; delete tmp;

    reduce();      
    nextGeneration();
  }
};
#endif



