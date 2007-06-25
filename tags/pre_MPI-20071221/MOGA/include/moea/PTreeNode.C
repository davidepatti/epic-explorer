#include "PTreeNode.h"
#include <string>
#include <iomanip>


// we always insert the new node as the first child of its parent
PT_NODE::PT_NODE(const Grid_Loc& lc, PT_NODE* parnt, IND* ind) :
  loc(lc), _count(0), _parent(parnt), first_child(NULL),
  firstInd(ind? new IND_NODE(ind) : NULL) {
  if(_parent) {
    next_sibling = _parent->first_child;
    _parent->first_child = this;
  }
  else 
    next_sibling = 0;
  assert(++seq);
}


void PT_NODE::del_tree() { 
  if(_parent) {
    PT_NODE* prev = _parent->first_child;
    if(prev == this)  // 'this' is the first child of its parent.
      _parent->first_child = next_sibling; 
    else { // linkage updating.
      PT_NODE* cur;
      while( (cur=prev->next_sibling) != this )
	prev = cur;
      prev->next_sibling = next_sibling;
    }
  }
  next_sibling = 0; // delete this subtree, not this forest.
  delete this;
}


PT_NODE* PT_NODE::leftest_child() {
  if(first_child) return first_child;
  else {
    PT_NODE* next_root = right_node();
    if(next_root) return next_root->leftest_child();
    else          return 0;
  }
}

//regarding nodes in each level as a list, return next node in this list.
PT_NODE* PT_NODE::right_node() {
  if(!_parent) { assert(!next_sibling); return 0; }
  if(next_sibling) return next_sibling;
  PT_NODE* uncle = _parent->right_node();
  if(!uncle) return 0;
  else      return uncle->leftest_child();
}

int PT_NODE::differentInds(bool& deleted) { // will del redundant individuals.

  int rt = 0;  
  IND_NODE *indNode, *prev, *cur;

  for(indNode=firstInd; indNode; rt++, indNode=indNode->next) {
    prev = indNode; // del extra inds which are genotypically the same as indNode.
    while(cur=prev->next) { 
      if( *(AlleleString*)indNode->ind == *(AlleleString*)cur->ind ) {
	prev->next = cur->next;
	cur->next = 0; delete cur; 
	deleted = true;
      }
      else
	prev = cur;
    }
  }
  assert(rt?level()==curDepth():level()<curDepth());  //only leaves have inds.
  return rt;  
}


void PT_NODE::reset() {
  _count = 0; 
  if(next_sibling) next_sibling->reset();
  if(first_child) first_child->reset();
}


//downward to accumulate resident in parent node  to that of child, and 
//reduce (recursively) subtree size if its resident too large.

void PT_NODE::niching_reducing(int del_Inds) {
  assert(_count);      // this method(message) will not be passed to pending nodes.
  _count -= del_Inds;  // reducing resident size.
  assert(_count>0);    // otherwise, this node will be deleted in upper recursive level.
  int total_resident = _count;
  if(_parent)_count += _parent->_count;
  
  if(!del_Inds) {        // no reducing needed.
    if(!firstInd) {      // if not leaf node, accumulate recursively, else return.
      assert(level()!=curDepth());
      for(PT_NODE *child = first_child; child; child = child->next_sibling) 
	if(child->_count != 0) // otherwise skip pending node.
	  child->niching_reducing(0);
    }
    return;           
  }

  if(firstInd) {        // delete individuals from leaf node.
    assert(level()==curDepth());
    IND_NODE *prev = firstInd, *cur = firstInd->next;
    for(int i=1; i<del_Inds; i++)  // del the left part of the list.
      { prev = cur; cur = cur->next; }
    prev->next = 0; delete firstInd;
    firstInd = cur; assert(cur); 
  }
  else { // reduce and accumulate recursively. We skip pending child node(count=0).
    int max = (int) pow((double)2, dimens);     // maximal children.
    int *resident = new int [max+1];    // # of inds in each non-pending subtree.
    resident[0] = -LARGE;               // the [0] ele is sentry.
    int *del= new int [max+1]; 

    PT_NODE *child, *next; int i;    
    for(child = first_child, i=1; child; child = child->next_sibling) {
      int tmp = child->_count;    
      if(tmp) resident[i++] = tmp;   // otherwise skip pending node.
    } 
    int children = i-1;              // total children excluding pending ones.
    reduce(children, resident, del, total_resident);

    next = first_child, i=1; 
    while(child = next) {
      next = child->next_sibling;
      if(!child->_count) continue;  // skip pending node.
      if(resident[i]>del[i])        
	child->niching_reducing(del[i]);
      else 
	child->del_tree(); 
      i++;
    }
    assert(i==children+1);

    delete [] resident; delete [] del;
  }

  return;
}


void PT_NODE::reduce(int children, int const * count, int* del, int total) {

  int *order = new int[children+1]; // keep sorting result here.
  for(int i=0; i<=children; i++)  order[i] = i;

  for(int i=1; i<=children; i++) {  // insert sort count[] in ascending order.
    int j=i, od = order[i], key = count[od];
    while( key < count[order[j-1]] ) 
      { order[j] = order[j-1]; j--; }
    order[j] = od;
  }

  // c is # of children whose residents are actually reduced.
  int i = 1, c = children;
  for(; float(total)/c >= count[order[i]] && i<=children; i++) { 
    del[order[i]] = 0;         // this child node will not be reduced.
    total -= count[order[i]];  // so subtract its count from total count.
    c--;                       // and decrease # of child nodes to be reduced.
  }

  // reduce the remained child nodes. they are order[i] to order[children].
  int integer = total/c, remainder = total%c;
  for(int j=0; j<remainder; j++, i++) {
    del[order[i]] = count[order[i]] - (integer+1);
    assert(del[order[i]]>=0);
  }
  for(; i<=children; i++)
    del[order[i]] = count[order[i]] - integer;

  delete [] order;
  return;
}



#ifdef DEBUG

int PT_NODE::seq = 0;

int PT_NODE::level() const {
  int lvl = 0;
  const PT_NODE* dad = this;
  while(dad = dad->_parent) lvl++;
  return lvl;
}

PT_NODE* PT_NODE::tree_root() const {
  const PT_NODE *cur = this;
  while(PT_NODE* dad = cur->_parent) cur = dad;
  return (PT_NODE*)cur;
}


int PT_NODE::subtreeDepth() const {

  PT_NODE* child = first_child;
  if(!first_child) return 0;

  int depth = child->subtreeDepth();
  while(child = child->next_sibling) {
    int d = child->subtreeDepth();
    if(d > depth) depth = d;
  }
  return depth+1;
}

  

int PT_NODE::subtreeSize() const {
  PT_NODE*child = first_child;
  int sz = 1;
  while(child) {
    sz += child->subtreeSize();
    child = child->next_sibling;
  }
  return sz;
}

#endif
