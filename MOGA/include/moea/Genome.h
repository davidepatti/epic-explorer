/*************************************************************************************
 *                                                                                   *
 * ----------------- Genome(Individual) Class for MOEA ------------------------------*
 *                                                                                   *
 * Implemented by inheriting AlleleString class and embedding ObjectiveVector class, *
 * the former for genome infomation, the latter is the objectives of this ind.       *
 *                                                                                   *
 * The IND class should be created after static member "dimens" is already           *
 * initialized, because it is needed when creating embedded ObjectiveVector object.  *
 *                                                                                   *
 *                                                   xianming Chen, Aug, 2000        *
 *                                                   modified on Jan 15, 2001        *
 *************************************************************************************/


#ifndef _Genome_H_
#define _Genome_H_

#include "AlleleStr.h"
#include "ObjectiveVector.h"

using namespace std;

class  POP;

class IND : public AlleleString, public Pareto {
 public:
  typedef void(*Evaluator)(IND& ind, ObjectiveVector& obj, void *stuff);
  typedef double (* Distance) (const IND&, const IND&);

 public:
  MOEADefineIdentity("IND", "IND", 100);

  IND(int len, Evaluator efp, int alleles = 2);

  IND(vector<vector<Allele> > alss, Evaluator efp, void *_user_data);
  
  IND(const IND& org) : AlleleString(org), pscores(NULL) { assert(++seq); copy(org); }
  virtual ~IND() { assert(seq--); } 

  void copy(const IND& org);
  IND& operator= (const IND& ind) { copy(ind); return *this; }

  virtual IND* clone()       const { IND* p=new IND(*this); return p; }
  virtual IND* randomClone() const { IND* p=new IND(*this); p->randomize();return p; }
  IND&         randomize()         { AlleleString::randomize(); invalidateCaches(); return *this;}

  void crossover(float pcross, const IND& dad, IND*& sis, IND*& bro) const;
  void mutate(float pmutate);

  int  genes() const               { return length(); }

  virtual void invalidateCaches()  { invalidateScores(); }
  bool scoresValid() const         { return scores_valid; }
  void invalidateScores()          { scores_valid = false; }

  Normalizer* objectiveNormalizer() const { return pscores->normalizer(); }
  Normalizer* objectiveNormalizer(const Normalizer& n)const { return pscores->normalizer(n); }

  double  distance(IND& ind) const          { return distFp(*this, ind); }
  static  Distance metric()                 { return distFp; }
  static  Distance metric(Distance dist_fp) { return distFp = dist_fp; }

  const   ObjectiveVector& objectiveVector() const;

  static  Evaluator evaluator()             { return evalFp; }
  static  Evaluator evaluator(Evaluator eFp){ 
    if(evalFp && evalFp != eFp) cout << "evaluator changed!\n"; return evalFp=eFp; }

  pRel   compare(const IND& ind) { return objectiveVector().compare(ind.objectiveVector()); }
  /*+++++++++++++++++++ NOTE: != means indifferent. +++++++++++++++++++++++++++*/
  bool operator!= (const IND &r) { return compare(r)==INDIFFERENT ? true:false; }
  bool operator>  (const IND &r) { return compare(r)==DOMINATING ? true:false; }
  bool operator<  (const IND &r) { return compare(r)==DOMINATED  ? true:false; }
  bool operator== (const IND &r) { return compare(r)==SAME        ? true:false; }
  bool operator>= (const IND &r) { return *this > r || *this == r ; }
  bool operator<= (const IND &r) { return *this < r || *this == r ; } 

  double fitness() const       { return fit; }
  double fitness(double f)     { return fit = f; }

  void printGenome(ostream& os=cout) { operator<< (os,(const AlleleString&) *this); }
  friend ostream& operator<< (ostream & os,const IND& ind);  // output phenotypes


 protected:
  ObjectiveVector* pscores;     // multi-dimensional objective scores.
  bool             scores_valid;
  double           fit;               

  static Distance  distFp;
  static Evaluator evalFp;
  static void      *user_data;
  
#ifdef DEBUG
 protected:
  static int seq;
 public:
  int livingINDs() { return seq; }
#endif
};

inline double DefaultObjectiveDistance(const IND& ind1, const IND& ind2) {
  return ind1.objectiveVector().distance(ind2.objectiveVector());
}

inline double DefaultNormalizedObjectiveDistance(const IND& ind1, const IND& ind2) {
  return ind1.objectiveVector().normalizedDistance(ind2.objectiveVector());
}

inline double DefaultGenotypicHammingDistance(const IND& ind1, const IND& ind2) {
  return (double) ind1.hammingDistance(ind2);
}



#endif
