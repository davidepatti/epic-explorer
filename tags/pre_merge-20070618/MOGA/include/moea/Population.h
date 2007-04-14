/*-----------------------------------------------------------------------------------*
 *                                                                                   *
 *  This is a content class for individuals. it includes routine operation for       *
 *  content class such as replace(), append(), del(). The only one operation         *
 *  specific to our problem is cull(), which culles all the dominated individuals   *
 *  from the content.                                                                *
 *                                                                                   *
 *  All the genetic operations are dealt by an upper class MOEA, which typically     *
 *  embeds two populations, a generational one, and an external(secondary) one.      *
 *  In a word, I make this class as simple as possible; it knows nothing about       *
 *  the algorithms.                                                                  *
 *                                                                                   *
 *                                                          Aug 2000, xianming chen  *
 *                                                          modified on Jan 17, 2001 *
 *-----------------------------------------------------------------------------------*/


#ifndef _Population_H_
#define _Population_H_

#include "Genome.h"
#include "Bin2DecGenome.h"

using namespace std;
class POP : public Pareto {
 public:
  MOEADefineIdentity("POP", "POP", 200);

  POP(int chunkSize = 100, class MOEA* pAlgo=0);
  POP(const POP& other) : curSz(0), ppind(NULL), curBd(0) { copy(other); }
  virtual ~POP() { for(int i=0; i<curSz; i++) delete ppind[i]; delete [] ppind; }

  void copy(const POP& pop);
  virtual POP* clone() {return new POP(chunkSz, pAlgo); }   // no copy of individuals!!!
  POP& operator= (const POP& pop) { copy(pop); return *this; }

  // randomly generating no inds from an individual template.
  void initialize(const IND&, int no, class MOEA* pAlgo=0);

  void del(int index); 
  IND* remove(int index);      // caller dealloc the removed ind which is returned.
  void replace(int idx,IND*); 
  void replace(int idx,const IND& ind) { replace(idx, ind.clone()); } 
  void append(IND* ind);  
  void append(const IND& ind)          { append(ind.clone()); }
  void append(const POP& pop)  {for(int i=0;i<pop.currentSize();i++)append(pop[i]->clone());}
  void append(POP* ppop)       {for(int i=0;i<ppop->currentSize();i++)append((*ppop)[i]);}

  void cull();   //cull those dominated;

  class MOEA& algorithm() const     { return *pAlgo; }
  int currentSize() const           { return curSz; }
  IND*   operator[] (int idx) const { assert(idx<curSz); return ppind[idx]; }
  IND* & operator[] (int idx)       { assert(idx<curSz); return ppind[idx]; }
  
  void printSolutions(ostream& os=cout) const { os << *this; }
  void printGenomes(ostream& os=cout) const; 
  void printFitnesses(ostream& os=cout) const;
  void printScores(ostream& os=cout) const;
  void printAll(ostream& os=cout) const;
  friend ostream& operator<< (ostream& os, const POP& pop);

 protected:
  IND** ppind;
  int   curSz;            // current individuals.
  int   curBd;            // current capacity.
  int   chunkSz;          // memory allocation unit.

  class MOEA* pAlgo;      // the algorithm this population is related.
};

#endif
