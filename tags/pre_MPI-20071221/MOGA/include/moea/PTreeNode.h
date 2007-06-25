/*---------------------------------------------------------------------------------------------*
 * This is the Pareto Tree Node Class for Pareto Tree Searching Genetic Algorithm              *
 * Logically, the tree node is regarded both as a recursive sub tree and a *forest*.           *
 * Any node 'i' represents a forest as below:                                                  *
 *     the forest consists of all the subtrees with root nodes the same level as and to the    *
 *     right of the node 'i'.                                                                  *
 *-----------------------------------------------------------------------------Jan 22, 2001----*/


#ifndef _PTreeNode_H
#define _PTreeNode_H

#include <stdlib.h>
#include "Grid_Loc.h"
#include "Genome.h"

using namespace std;

/*--------------------------------------------------------------------------------------------*
 *IND_NODE class is used to link together all individuals in certain(actually leaf) PTree node*
 *--------------------------------------------------------------------------------------------*/
class IND_NODE {
  friend class PT_NODE;
 private:
  IND_NODE(IND* pind, IND_NODE* nextlink =0) : ind(pind), next(nextlink) { }
  ~IND_NODE()                  { delete ind; delete next; }
  IND_NODE(const IND_NODE&)    { cerr << "copy constructor for IND_NODE n/a.\n", exit(-1); }
  IND_NODE& operator=(const IND_NODE&) { cerr<<"assignment for IND_NODE n/a.\n", exit(-1); }

  IND       *ind;
  IND_NODE  *next;
};


class PT_NODE : public Pareto {
 public:
  PT_NODE(const Grid_Loc& loc, PT_NODE* parent, IND* pind=0);
  ~PT_NODE()          { assert(seq--); delete first_child; delete next_sibling; delete firstInd; }
  PT_NODE(const PT_NODE&)             { cerr << "copy constructor for PT_NODE n/a.\n", exit(-1); }
  PT_NODE& operator=(const PT_NODE&)        { cerr << "assignment for PT_NODE n/a.\n", exit(-1); }

  pRel compare(const Grid_Loc& _loc) const  { return loc.compare(_loc); }

  void niching_reducing(int noDel); 
  void reset();                      // reset _count recursively.
  int& count()                       { return _count; }
  void del_tree();                   // delete the subtree.
  int differentInds(bool& deleted);  // del genotypically extra ind,ret # of different inds.

 /* for inserting and iterating the individual list */
 private: 
  IND_NODE* curIndNode; 
 public:
  void insert(IND* ind) { firstInd = new IND_NODE(ind, firstInd); }
  void init_iterate()   { curIndNode = firstInd; }
  IND* curInd()         { return curIndNode? curIndNode->ind:0; }
  IND* nextInd()        { assert(curIndNode); curIndNode = curIndNode->next; return curInd(); }

  /* for iterating the whole tree. */
  PT_NODE*  parent() const      { return _parent; }
  PT_NODE*  right_node();       // return right(next) root node of this forest.
  PT_NODE*  leftest_child();    // ret 'child' node in level 1 of this forest.
                                // beware it may be the child of the right_node().
 private:
  Grid_Loc loc;                 // binary strings encoded location of this node.

  PT_NODE*   _parent;            
  PT_NODE*   next_sibling;
  PT_NODE*   first_child;

  int _count;                   // # of inds(or accumulated inds) in this subtree 
  IND_NODE*  firstInd;          // list head of inds occupying in this node.

  // given allowed total resident and original residents, calculate # of del from each child node.
  void reduce(int noChildren, int const *originalResidents, int* del, int totalResidentAllowed);


#ifdef DEBUG    
  static int seq; 
public:
  static int livingNODEs()      { return seq; }
  int subtreeSize() const;      // size of this sub tree.

  int subtreeDepth() const;
  int curDepth() const          { return tree_root()->subtreeDepth(); }
  int level() const;            // return this node's level in the tree 
  PT_NODE*  tree_root() const;  // return the Pareto tree root node.
#endif
};


#endif
