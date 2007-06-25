#ifndef _SELECT_H
#define _SELECT_H

#include "Population.h"

using namespace std;

class Selector : public Pareto {
 public:
  MOEADefineIdentity("Selector", "S", 600);
  Selector() : ppop1(NULL), ppop2(NULL), pool(NULL), curIdx(-1) { }
  virtual ~Selector() { }

  /* 
   * Select poolSz genomes from pop1 and pop2 to the pool   
   * If no poolSz is given, it defaults to size of pop1     
   * If ppop2=0, only population 1 is used. Otherwise select from two populations.
   * The actually selection is implemented by derived class
   */
  virtual void initialize(const POP* p1, const POP* p2=0, int poolSize=0);

  IND* select() { assert(curIdx>=0 && curIdx<poolSz+2); return pool[curIdx++]; }

 protected:
  const POP* ppop1;
  const POP* ppop2;
  int sz1;             // population 1 size.
  int sz2;             // population 2 size.

  IND** pool;          // mating pool.
  int   poolSz;
  int   curIdx;        // current index into pool[].
};


/* 
 * Tournament selection with replacement.
 * The greater fitness value, the worse the individual.
 */
class TournamentSelector : public Selector { 
 public: 
  MOEADefineIdentity("TournamentSelector", "tS", 601);
  TournamentSelector(int sz=2) : tournamentSz(sz), tournament(new int[sz]) { }
  ~TournamentSelector() { }

  void initialize(const POP* p1, const POP* p2=0, int poolSize=0);

 protected:
  int* tournament;  
  int  tournamentSz;

  // generate 'sz' different random index between [0,sz1+sz2-1] to tournament[].
  void generateTournament(int sz); 

  IND* individual(int i) { // get individual from pop1 or pop2 given its index.
    assert(i>=0 && i<sz1+sz2); return i<sz1? (*ppop1)[i] : (*ppop2)[i-sz1]; }
};

#define BinarySelector  TournamentSelector(2)     


//NPGA selector, only population 1 is used.
class ParetoTournamentSelector : public TournamentSelector {
 public:
  MOEADefineIdentity("ParetoTournamentSelector", "ptS", 602);
  ParetoTournamentSelector(int compSz, double nicheRadius, int sz=2);
  ~ParetoTournamentSelector() { }

  void initialize(const POP* p1, const POP* p2=0, int poolSize=0);

 protected:
  int compSz;          // size of random comparison set.
  double nicheRadius;  // in objective space.
};



// VEGA selector.(binary tounament is used).
class VEGATournamentSelector : public TournamentSelector {
 public: 
  MOEADefineIdentity("VEGATournamentSelector", "sS", 603);  // Shaffel selector
  VEGATournamentSelector(int sz=2) : TournamentSelector(sz) { }
  ~VEGATournamentSelector() { }

  void initialize(const POP* p1, const POP* p2=0, int poolSize=0);
};



#endif

