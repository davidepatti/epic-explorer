#include "PTSGA.h"
#include <string>
#include <iomanip>
#include <sstream>

PTSGA::PTSGA(): pTree(NULL), max_depth(DEFAULT_MAX_DEPTH), 
  init_depth(DEFAULT_INIT_DEPTH), init_step(DEFAULT_INIT_STEP), 
  _multiplier(DEFAULT_MULTIPLIER) {

  pop1_initSize(0); pop1_maxSize(DEFAULT_POP1_maxSIZE);
  paraList.add("multiplier for step size", "multiplier", "m", Float, &_multiplier);
  paraList.add("initial step size", "initStepSize", "initS", Int, &init_step);
  paraList.add("current step size", NULL, NULL, Int, &cur_step);
  paraList.add("initial depth of tree", "initDepth", "initD", Int, &init_depth);
  paraList.add("current depth of tree", NULL, NULL, Int, &cur_depth);
  paraList.add("maximum depth of tree", "maxDepth", "maxD", Int, &max_depth);
}


void PTSGA::initialize(IND& ind) {
  assert(pop1_initSize()==0);
  MOEA::initialize(ind); 
  cur_depth = init_depth;
  cur_step = init_step;
  delete pTree;  pTree = new PTree(this); 
  delete slctor; slctor = new BinarySelector; 
}



void PTSGA::step() { 
  assert(indTemplate->livingINDs() == 2+population1().currentSize());
  assert(currentTreeSize() == livingNODEs());

  POP tmpPop;
  generate(tmpPop);                        // generate offspring from cur pop in PTree.
  while(tmpPop.currentSize() < pop1_maxSz) // reinitialize the remainder.
    tmpPop.append(indTemplate->randomClone());

  updateTree(tmpPop);              // update population (in PTree) from tmpPop.
  assignFitnesses();               // niching&reducing, and assigning fitness.

  nextGeneration();
}


void PTSGA::increaseDepth() {
  static int nextStage = init_step;
  if( curGen == nextStage && cur_depth != max_depth ) {
    cur_depth++; 
    cur_step = (int) (_multiplier*cur_step); nextStage += cur_step;  
#ifdef DEBUG
    cout << "current gen = " << curGen << "\tcurrent depth = " << cur_depth << endl;
    cout << "cur_step = " << cur_step << "  " << "next_stage = " << nextStage << endl << endl;
#endif
  }
}
