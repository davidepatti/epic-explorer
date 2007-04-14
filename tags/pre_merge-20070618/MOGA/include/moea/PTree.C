#include "PTSGA.h"
#include <string>
#include <iomanip>


PTree::PTree(PTSGA* pA) : pPTSGA(pA), root(NULL), cur_depth(pA->curDepth()),
  ppop(new POP), ppop_valid(false), 
  list(new PT_NODE* [pA->maxDepth()+1]), list_valid(false),
  curOffset(NULL), curLargest(NULL), curRange(NULL) { 
  Grid_Loc loc; loc.update(cur_depth);
}

PTree::~PTree() { 
  delete root; delete[] list;
  delete[] curOffset; delete[] curLargest; delete[] curRange;
  for(int i = ppop->currentSize()-1; i>=0; i--)
    ppop->remove(i);   // individuals already deleted by "delete root".
  delete ppop;            
}


void PTree::invalidate() const {
  PTree* This = (PTree*) this;
  This->ppop_valid = This->list_valid = false; 
}

void PTree::validate_list() const {
  if(list_valid) return;
  assert(root);
  PTree* This = (PTree*) this;
  This->list[0] = root;
  for(int i=1; i<=cur_depth; i++) {
    This->list[i] = list[i-1]? list[i-1]->leftest_child() : 0;
  }
  This->list_valid = true;
}


void PTree::validate_population() const { 
  if(ppop_valid) return;

  for(int i = ppop->currentSize() - 1; i>=0;  i--) 
    ppop->remove(i);

  if(root) {
    PT_NODE* leaf = leftest_leaf();
    while(leaf) {
      leaf->init_iterate();
      assert(leaf->curInd());
      ppop->append(leaf->curInd());
      while(IND* ind=leaf->nextInd()) 
	ppop->append(ind);
      leaf = leaf->right_node();
    }
  }
  PTree* This = (PTree*) this;
  This->ppop_valid = true; 
}




void PTree::init_rangeFrom(const POP& pop) {

  assert(!curOffset && !curLargest && !curRange);
  curOffset=new double[dimens];curLargest=new double[dimens];curRange=new double[dimens];

  curOffset[0] = curOffset[1] = 0;
  curLargest[0] = curLargest[1] = pow((double)2, 15);
  curRange[0] = curRange[1] = pow((double)2, 15);
  return;

  for(int j=0; j<dimens; j++)
    curOffset[j] = LARGE, curLargest[j] = -LARGE;

  double tmp;
  for(int i=0; i<pop.currentSize(); i++) 
    for(int j=0; j<dimens; j++) {
      if((tmp=pop[i]->objectiveVector()[j]) < curOffset[j])
	curOffset[j] = tmp;
      if((tmp=pop[i]->objectiveVector()[j]) > curLargest[j])
	curLargest[j] = tmp;
    }

  for(int j=0; j<dimens; j++) {
    curOffset[j] *= 0.8;
    curLargest[j] *= 1.2;
    curRange[j] = curLargest[j] - curOffset[j];
  }
  
}


void PTree::update(const POP& pop) {
  if(! root) {
    root = new PT_NODE(0, 0, 0);
    invalidate();
    if(!curOffset) init_rangeFrom(pop);
  }

  int sz = pop.currentSize(); 
  PT_NODE** dad = new PT_NODE*[sz];
  for(int i=0; i<sz; i++)
    dad[i] = root;    
  Grid_Loc* loc = new Grid_Loc[sz];

  double* range = new double[dimens]; 
  double* half_range = new double[dimens]; 
  double** offset = new double* [sz]; 

  for(int i=0; i<sz; i++)
    offset[i] = new double [dimens];
  for(int j=0; j<dimens; j++) {
    range[j] = curRange[j];
    half_range[j] = range[j]/2;
  }
  for(int i=0; i<sz; i++)
    for(int j=0; j<dimens; j++) 
      offset[i][j] = curOffset[j];
 

  list[0] = root;
  for(int lvl=1; lvl<=cur_depth; lvl++) { 
    double middle;  
    for(int i=0; i<sz; i++) {
      if(!dad[i]) continue; //this individual discarded in uppler level insertion.
      IND * ind = pop[i];
      for(int j=0; j<dimens; j++) {
	middle = offset[i][j] + half_range[j];
	if( ind->objectiveVector()[j] > middle ) {
	  loc[i].set(j, lvl);
	  offset[i][j] = middle;
	}
      }
      dad[i] = insert(lvl, loc[i], i, dad, lvl==cur_depth?ind:0);
    }

    for(int j=0; j<dimens; j++) {
      range[j] = half_range[j];
      half_range[j] /= 2;
    }
    list[lvl] = list[lvl-1]->leftest_child();
  }

  delete [] dad;
  delete [] loc;
  delete [] range;
  delete [] half_range;
  for(int i=0; i<sz; i++)
    delete [] offset[i];
  delete [] offset;
  invalidate();
}




/*-------------------------------------   insert()   -------------------------------------------*
 *                                                                                              *
 * Compare the idx'th individual(grid location 'loc')to those in nodes of level 'lvl'.          *
 *   if dorminated, do nothing and return 0.                                                    *
 *   if equal to some node, insert individual and return that node.                             *
 *   if dorminating, delete dorminated subtree.                                                 *
 *   if (dominating or indifferent) create new node and return this new node.                   *
 * NOTE:                                                                                        *
 *     the pointer array is also used as flags.  When the inserted one dominates other nodes    *
 *     already in the same level of the tree, all dad[i<idx] whose value equal to any of the    *
 *     dominated (and deleted) nodes should be cleared to 0, and consequently such individual   *
 *     will not be considered for comparison in deeper level.                                   *
 *----------------------------------------------------------------------------------------------*/

PT_NODE* PTree::insert(int lvl, Grid_Loc& loc, int idx, PT_NODE** dad, IND* ind) {
  assert(list[lvl-1]);
  assert(lvl==cur_depth || ind == 0); // actual insertion of individual only occurs in leaf level.

  PT_NODE* cur = list[lvl-1]->leftest_child();  
  while(cur) {
    PT_NODE* next = cur->right_node();
    switch( cur->compare(loc) ) {
    case DOMINATING: return 0;
    case SAME      : if(ind) cur->insert(ind->clone()); return cur;
    case DOMINATED : for(int i=0; i<idx; i++) 
		       if(dad[i]==cur) dad[i] = 0;
		     cur->del_tree(); break;
    }
    cur = next;
  }
  return new PT_NODE(loc, dad[idx], ind? ind->clone():0);
}
  

void PTree::assignFitnesses() {
#ifdef DEBUG
  static int call = 0;  call++;
#endif
  /* 
   * iterate upward to count resident of each subtree 
   */
  root->reset();   //clear count in all nodes.
  bool deleted = false; 
  assert((call%50? 0: (void*)(cout<<"\n\nindividuals in each leaf node\n"), 1));
  PT_NODE* leaf = leftest_leaf(); 
  while(leaf) {  
    int leaf_count = leaf->differentInds(deleted);
    leaf->count() += leaf_count;
    PT_NODE* dad = leaf->parent();
    PT_NODE* child  = leaf;
    while(dad) {
      dad->count() += leaf_count;
      child = dad; dad = dad->parent();
    }
    assert((call%50? 0:(void*)(cout<<leaf_count<<" "), 1));
    leaf = leaf->right_node();
  }
  assert((call%50? 0:(void*)(cout<<endl), 1));
  if(deleted) invalidate();

  /* 
   * now recursively iterate downward to accumulate parent 'count' to 
   * that of child node, and reduce subtree size if too large.
   */
  int curSz = population().currentSize();
  assert(curSz == root->count());
  int maxInds = pPTSGA->pop1_maxSize();
  int del_Inds = curSz>maxInds? curSz-maxInds : 0;
  root->niching_reducing(del_Inds);
  if(del_Inds) invalidate();

  /*
   * finally, assign each individual the following fitness:
   * 'count' of its (leaf) node - curSz.
   */
  curSz = population().currentSize(); 
  leaf = leftest_leaf();
  while(leaf) { 
    int fit = leaf->count() - curSz;
    leaf->init_iterate();
    assert(leaf->curInd());
    leaf->curInd()->fitness(fit);
    while(IND* ind=leaf->nextInd()) 
      ind->fitness(fit);
    leaf = leaf->right_node();
  }

  return;
}

bool PTree::depthChanged() {
  int old = cur_depth;
  cur_depth = pPTSGA->curDepth();
  if(old != cur_depth) {
    Grid_Loc loc; loc.update(cur_depth);
    return true;
  }
  return false;
}


bool PTree::rangeChanged() { 

  double* offset = new double [dimens];
  double* largest = new double [dimens];
  for(int j=0; j<dimens; j++)
    offset[j] = largest[j] = population()[0]->objectiveVector()[j];

  POP& pop = population();
  int sz = pop.currentSize();
  double current;
  for(int i=1; i<sz; i++) {
    for(int j=0; j<dimens; j++) {
      current = pop[i]->objectiveVector()[j];
      if( offset[j] > current) offset[j]  = current;
      if(largest[j] < current) largest[j] = current;
    }
  }
  bool changed = false;
  for(int j=0; j<dimens; j++) {
    if(fabs(curOffset[j] - offset[j]) > 0.1*curOffset[j])
      { changed = true;  break; }
    if(fabs(curLargest[j] - largest[j]) > 0.1*curLargest[j])
      { changed = true;  break; }
  }
  if(changed) {
    for(int j=0; j<dimens; j++) {
      curOffset[j]  = (1-0.1) * offset[j];
      curLargest[j] = (1+0.1) * largest[j];
      curRange[j] = curLargest[j] - curOffset[j];
    }
  }
  delete [] offset; delete [] largest;
  return changed;
}


void PTree::rebuild() {
  bool changed = false;
  if(rangeChanged()) changed = true;
  if(depthChanged()) changed = true;
  if(changed) {
    PT_NODE* old_root = root;
    POP& pop = population();
    root = 0; update(pop);
    delete old_root;
  }
  return;
}


