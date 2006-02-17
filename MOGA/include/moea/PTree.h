/*---------------------------------------------------------------------------------*
 * Pareto Tree class:                                                              *
 *   the population of Pareto tree searching GA is maintained here.                *
 *------------------------------------------------ Jan 25, 2001, xianming chen-----*/

#ifndef _PTree_H
#define _PTree_H

#include "PTreeNode.h"

using namespace std;

class PTSGA;

class PTree : public Pareto { 
 public:
  PTree(PTSGA*);
  ~PTree();
  PTree(const PTree&)      { cerr << "copy constructor for PTree n/a.\n", exit(-1); }
  PTree& operator=(const PTree&) { cerr << "assignment for PTree n/a.\n", exit(-1); }

  POP& population() const  { if(!ppop_valid) validate_population(); return *ppop; }

  void update(const POP& pop);     // update the tree from population.
  void rebuild();                  // rebuild the tree if range or depth changed.
  void assignFitnesses();  


 private:
  PT_NODE* root; 
  PTSGA*   pPTSGA;         // the PTSGA algorithm this tree is related.

  int      cur_depth;
  bool     depthChanged(); // Is current Pareto tree depth changed?

  double* curRange;        // These are attributes of current hyper rectangular
  double* curOffset;       // in objective space, used for node location
  double* curLargest;      // calculation when given ind's objective values.
  bool    rangeChanged();  // Is current hyper rectangular changed due to update().
  void    init_rangeFrom(const POP&);

  PT_NODE** list;          // head of list(consisting of nodes in the level) of each level. 
  POP* ppop;               // shallow copy of the individuals in the tree. do NOT delete.
  void invalidate() const; // invalidate lists and population.
  bool list_valid;
  void validate_list() const;
  bool ppop_valid;
  void validate_population() const;

  PT_NODE* insert(int lvl, Grid_Loc& loc, int i, PT_NODE** dad, IND*);
  PT_NODE* leftest_leaf() const { if(!list_valid) validate_list(); return list[cur_depth]; }


#ifdef DEBUG
 public:  //for debugging.
  int treeSize() const      { return root? root->subtreeSize():0; }
  int livingNODEs()         { return root? root->livingNODEs():0; }
  void printRanges() const  { if(!curOffset) return;
    for(int i=0; i<dimens;i++) cout <<i<<":\t"<<curOffset[i] <<"  "<<curLargest[i]<<endl;}
#endif
};

#endif
