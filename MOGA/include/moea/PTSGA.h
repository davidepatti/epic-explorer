/*---------------------------------------------------------------------------------------------*
 * This is the Pareto Tree Searching Genetic Algorithm.                                        *
 * We override ptr2pop1() so that any access to population in the algorithm will be redirected *
 * to the population in the tree, while 'ppop1' member in base MOEA keeps NULL.                *
 *-----------------------------------------------------------------Jan 22, 2001-xianming Chen--*/


#ifndef _PTSGA_H
#define _PTSGA_H

#include "MOEA.h"
#include "PTree.h"


#define DEFAULT_MAX_DEPTH         15
#define DEFAULT_INIT_DEPTH        2
#define DEFAULT_INIT_STEP         50
#define DEFAULT_MULTIPLIER        1.2

class PTSGA : public MOEA {
 public:
  MOEADefineIdentity("PTSGA", "PTSGA", 302);

  PTSGA(); 
  ~PTSGA() { delete pTree; }

  void initialize(IND&);
  void step();  
  void nextGeneration()                { increaseDepth(); MOEA::nextGeneration(); }
  void updateTree(const POP& pop)      { pTree->update(pop); }
  void assignFitnesses()               { pTree->rebuild(); pTree->assignFitnesses();}

  int initDepth()                      { return init_depth; }
  int initDepth(int id)                { return init_depth = id; }
  int maxDepth()                       { return max_depth; }
  int maxDepth(int md)                 { return max_depth = md; }
  int curDepth()                       { return cur_depth; }
  int initStepSize()                   { return init_step; }
  int initStepSize(int ss)             { return init_step = ss; }
  float multiplier()                   { return _multiplier; }
  float multiplier(float mp)           { return _multiplier = mp; }

 private:
  PTree * pTree; 
  POP*    ptr2pop1() const             { return &(pTree->population()); }

  int init_depth;    // initial depth of binary division in PTreeSearchingGA.
  int max_depth;     // max depth of binary division in PTreeSearchingGA.
  int cur_depth;     // current depth of binary division in PTreeSearchingGA.
  int init_step;     // init step size(in generations) for increase current depth to next depth.
  int cur_step;      // cur  step size(in generations) for increase current depth to next depth.
  float _multiplier;        //multiplier for step size.

  void increaseDepth();

#ifdef DEBUG
 public: 
  unsigned long currentTreeSize() const   { return pTree? pTree->treeSize() : 0; }
  int livingNODEs()                       { return pTree->livingNODEs(); }
  void printRanges()                      { pTree->printRanges(); }
#endif     
};

#endif
