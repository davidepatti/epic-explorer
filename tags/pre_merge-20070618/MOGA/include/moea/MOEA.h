/*---------------------------------------------------------------------------------------------*
 *++++++++++ This is the base class for Multi-Objective Evolutionary Algorithms +++++++++++++++*
 *                                                                                             *
 * It has two populations; though typically a generational(primary)one, pop1, and an external  *
 * (secondary) one, pop2, we do not assume any characteristics of each population. As a matter *
 * of fact, the two populations can generally be regarded as two cooperative populations, with *
 * the specific properties of cooperation to be defined by the specific algorithm derived      *
 * from this base class.                                                                       *
 *                                                                                             *
 * The genetic operations on these two populations are mainly done by three embeded objects:   *
 *   razor  : reduce the external popualtion size to predefined value, typically by niching.   *
 *   slector: select one individual froem the two ulations for each call.                   *
 *   assiner: assign fitness to each individual in both populations, typically by niching      *
 *            and pareto ranking.                                                              *
 *                                                                                             *
 * Different MOEA algorithms can be designed using this class, with different embeded objects  *
 * mentioned above.                                                                            *
 *                                                                                             *
 * It is very likely for some overlap of the functions of razor and assiner, because niching,  *
 * pareto ranking, and fitness assignment are usually interdependent.                          *
 * I implement it in this way, mainly for the convenience of testing different algorithms      *
 * components.                                                                                 *
 *                                                                                             *
 * The whole genetic operations are packed in one function step(). This is a pure virtual      *
 * function in MOEA class.                                                                     *
 *                                                                                             *
 * Derived classes can implement different GA (e.x., overlapping GA, incremental GA(only one   *
 * offspring each iteration), or even (landa + 1) ES, etc) with its specific step() and        * 
 * initialize(), the former determines the general structure of GA, the latter determines      *
 * specific feature of GA, such as selection, niching, reducing population size, etc.          *
 *                                                                                             *
 *                                                               Xianming Chen, Aug, 2000      *
 *                                                               Modified on Jan 17, 2001      *
 *---------------------------------------------------------------------------------------------*/



  
#ifndef _MOEA_H_
#define _MOEA_H_

#include <stdlib.h>
#include "Population.h"
#include "Select.h"
#include "Assign.h"
#include "Reduction.h"
#include "ParameterList.h"
 
#define DEFAULT_POP1_initSIZE      50
#define DEFAULT_POP2_initSIZE      0
#define DEFAULT_POP1_maxSIZE       100
#define DEFAULT_POP2_maxSIZE       50

#define DEFAULT_MAX_GENERATIONS    500
#define DEFAULT_PCROSS             0.8
#define DEFAULT_PMUT               0.01

#define DEFAULT_NICHE_RADIUS       0.5

using namespace std;

class MOEA : public Pareto{
 public:
  MOEADefineIdentity("MOEA", "MOEA", 300);

  MOEA();
  virtual ~MOEA();
  MOEA (const MOEA&)            {cerr<<"MOEA copy constructor not available\n";exit(-1);}
  MOEA& operator= (const MOEA&) {cerr<<"MOEA assignment not available\n";exit(-1);}


  /*============================== genetic operators ========================================*/

  virtual void  step() = 0;
  virtual void  nextGeneration() { curGen++; }
  virtual void  initialize(const IND& ind);

  void evolve(const IND& i) { initialize(i); while(!done()) step(); } 
  void generate(POP& tmpPop);  //generate from current population(s) and append to tmpPop.

  virtual const IND* select()    { return slctor->select(); }
  virtual void init_select()     { slctor->initialize(ptr2pop1(), ptr2pop2()); }
  virtual void reduce()
    { razor->initialize(ptr2pop1(),pop1_maxSz,ptr2pop2(),pop2_maxSz); razor->reduce(); } 
  virtual void assignFitnesses() 
    { asiner->initialize(ptr2pop1(), ptr2pop2()); asiner->assignFitnesses();}
  virtual bool done()   { return curGen >= maxGen; }



  /*------------------------ parameters process  ------------------------------------*/

  void addParameter(char* verbose, char* fname, char* sname, PType t, void* pval)
    { paraList.add(verbose, fname, sname, t, pval); }
  void readCommandLine(int argc, char** argv) { paraList.readCommandLine(argc, argv); }


  /*-------------------------  member data access -----------------------------------*/

  Reducer& reducer() const              { return *razor; }
  Reducer& reducer(Reducer* r)          { return *(razor=r); }

  double nicheRadius()const             { return niche_radius; }
  double nicheRadius(double r)          { return niche_radius=r; }

  Selector& selector() const            { return *slctor; }
  Selector& selector(Selector* s)       { return *(slctor = s); }

  Assigner& assigner() const            { return *asiner; }
  Assigner& assigner(Assigner* a)       {delete asiner; return *(asiner = a); }

  const POP& population() const         { return population1(); }
  const POP& population1() const        { return *ptr2pop1(); }
  const POP& population2() const        { return *ptr2pop2(); }
  int genes() const                     { assert(indTemplate); return indTemplate->genes(); }

  int pop1_initSize(int sz)             { return pop1_initSz = sz; }
  int pop1_initSize() const             { return pop1_initSz; }
  int pop2_initSize(int sz)             { return pop2_initSz = sz; }
  int pop2_initSize() const             { return pop2_initSz; }
  int pop1_maxSize(int sz)              { return pop1_maxSz = sz; }
  int pop1_maxSize() const              { return pop1_maxSz; }
  int pop2_maxSize(int sz)              { return pop2_maxSz = sz; }
  int pop2_maxSize() const              { return pop2_maxSz; }

  void popSize(int sz)                  { pop1Size(sz); }
  void pop1Size(int sz)                 { pop1_initSize(sz); pop1_maxSize(sz); }
  void pop2Size(int sz)                 { pop2_initSize(sz); pop2_maxSize(sz); }

  int currentGeneration() const         { return curGen; }
  int maxGenerations() const            { return maxGen; }
  int maxGenerations(int g)             { return maxGen = g; }

  float pCrossover(float pc)            { return pcross = pc; }
  float pCrossover() const              { return pcross; }
  float pMutate(float pm)               { return pmut = pm; }
  float pMutate() const                 { return pmut; }



  /*------------------------------- output ----------------------------------------*/

  void  parameters(ostream& os=cout) const;
  const char* parametersString() const;
  void printFitnesses(ostream& os=cout) const;
  void printScores(ostream& os=cout) const;
  void printSolutions(ostream& os=cout) const;
  void printGenomes(ostream& os=cout) const;
  void printAll(ostream& os=cout) const;
  void printResult(ostream& os=cout) const;

 protected:
  POP* ppop1;             // ptr to generational pop.
  POP* ppop2;             // ptr to external nondominated pop.
  virtual POP* ptr2pop1() const     { return ppop1; }
  virtual POP* ptr2pop2() const     { return ppop2; }

  IND* indTemplate;       // individual temple, used to randomly clone.
  ParameterList paraList; // object dealing with parameters.

  int pop1_initSz;        // initial population 1 size.
  int pop1_maxSz;         // maximum allowed population 1 size.
  int pop2_initSz;        // initial populaton 2 size.
  int pop2_maxSz;         // maximum allowed population 2 size.
  int curGen;             // current generation.
  int maxGen;             // maximum generations.

  float pcross;           // probability of crossover.
  float pmut;             // probability of mutation (bitwise).

  Reducer*     razor;
  Selector*    slctor;
  Assigner*    asiner;
  double       niche_radius;

};



#endif
