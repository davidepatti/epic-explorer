#ifndef _ASSIGN_H
#define _ASSIGN_H

#include "Population.h"

using namespace std;

class Assigner : public Pareto {
 public:
  MOEADefineIdentity("Assigner", "Asiner", 400);

  Assigner() : ppop1(NULL), ppop2(NULL) { }
  virtual ~Assigner() { }

  virtual void initialize(const POP* p1, const POP* p2=0);
  virtual void assignFitnesses() = 0;

 protected:
  const POP* ppop1;
  const POP* ppop2;
  int sz1;   // population 1 size.
  int sz2;   // population 2 size.
};



// pop1 is traditional population, and pop2 is nondominated external pop.
class StrengthAssigner : public Assigner {
 public:
  MOEADefineIdentity("StrengthAssigner", "sA", 401);

  StrengthAssigner() : Assigner() { }
  ~StrengthAssigner() { }

  void assignFitnesses() { _calcStrength(); _calcFitness(); }

 private:
  void _calcStrength();
  void _calcFitness();
};



class NSGAassigner : public Assigner {
 public:
  MOEADefineIdentity("NSGAassigner", "pA", 402);  // peeling off asigner.

  NSGAassigner() : not_cur_nondominated(0), niche_count(0) { }
  ~NSGAassigner() { delete not_cur_nondominated; delete niche_count; }

  virtual void initialize(const POP* p1, const POP* p2=0);
  void assignFitnesses();

 private:
  double* niche_count;
  bool*   not_cur_nondominated;
  int     remainedSz;
  double  niche_radius;
};

#endif



