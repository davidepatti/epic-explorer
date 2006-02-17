/***************************************************************************************
 *                                                                                     *
 * -------------------------- MOEA ObjectiveVector Class  -----------------------------*
 *   -----------------PhenotypicVector class also defined here----------------------   *
 *                                                                                     *
 * ObjectiveVector is used to store objecitve scores of individual in multi-objective  *
 * evolutionary algorithm.                                                             *
 *                                                                                     *
 * The only difference between ObjectiveVector and VectorT<double> & PhenotypicVector  *
 * (defined below for phenotypic representation of genomes) is that the former is      *
 * added with static member mM, which indicate whether we are minimizing or maximizing.*
 * And consequently the compariosn relation is different from the latter too.          *
 *                                                                                     *
 *                                                          xianming Chen, Aug, 2000   *
 *                                                          modified on Jan 13, 2001   *
 ***************************************************************************************/


#ifndef _ObjectiveVector_H
#define _ObjectiveVector_H

#include "Vector.h"
#include "Pareto.h"

using namespace std;

/* 
 * "_double" is defined to enable that there are two static normalizer,
 * The original double type is  used for objective space (class ObjectiveVector below).
 * The defined class _double is used for decision(phenotypic) space.
 */
class _double {
 public:
  _double() { }
  _double(double d) : dval(d) { }

  _double operator= (const _double& rhs) { dval = rhs.dval; }
  double convert2double() const { return dval; }

  friend bool operator==(const _double& d1, const _double& d2) { return d1.dval == d2.dval; }
  friend bool operator!=(const _double& d1, const _double& d2) { return d1.dval != d2.dval; }
  friend bool operator>=(const _double& d1, const _double& d2) { return d1.dval >= d2.dval; }
  friend bool operator<=(const _double& d1, const _double& d2) { return d1.dval <= d2.dval; }
  friend bool operator> (const _double& d1, const _double& d2) { return d1.dval >  d2.dval; }
  friend bool operator< (const _double& d1, const _double& d2) { return d1.dval <  d2.dval; }
  friend double operator+ (const _double& d1, const _double& d2) { return d1.dval+d2.dval; }
  friend double operator- (const _double& d1, const _double& d2) { return d1.dval-d2.dval; }
  friend double operator* (const _double& d1, const _double& d2) { return d1.dval*d2.dval; }
  friend double operator/ (const _double& d1, const _double& d2) { return d1.dval/d2.dval; }
  friend ostream& operator<< (ostream& os, const _double d) { return os << d.dval; }

 private:
  double dval;
};
typedef VectorT<_double>  PhenotypicVector;


class ObjectiveVector : public dVector, public Pareto {  // dVector is VectorT<double>
 public:
  ObjectiveVector() : dVector(dimens) { assert(dimens==dims); }
  ObjectiveVector(const ObjectiveVector& org): dVector(org) { assert(dimens==dims); }
  virtual  ~ObjectiveVector() { }

  void    copy(const ObjectiveVector& org)              { dVector::copy(org); }
  virtual ObjectiveVector* clone() { ObjectiveVector* p = new ObjectiveVector(*this); return p; }
  const   ObjectiveVector& operator= (const ObjectiveVector& rhs) { copy(rhs); return *this; }

  pRel compare(const ObjectiveVector & other) const;

  /*+++++++++++++++++++++++ NOTE: != means indifferent. ++++++++++++++++++++++++++++++*/
  bool operator!= (const ObjectiveVector &r) { return compare(r)==INDIFFERENT ? true:false; }
  bool operator>  (const ObjectiveVector &r) { return compare(r)==DOMINATING ? true:false; }
  bool operator<  (const ObjectiveVector &r) { return compare(r)==DOMINATED  ? true:false; }
  bool operator== (const ObjectiveVector &r) { return compare(r)==SAME        ? true:false; }
  bool operator>= (const ObjectiveVector &r) { return *this > r || *this == r ; }
  bool operator<= (const ObjectiveVector &r) { return *this < r || *this == r ; }
};

inline Pareto::pRel ObjectiveVector::compare(const ObjectiveVector& other) const {
  switch(dVector::compare(other)) {
  case SMALLER: return mM==MIN?  DOMINATING : DOMINATED; break;
  case BIGGER : return mM==MAX?  DOMINATING : DOMINATED; break;
  case EQUAL  : return SAME;     break;
  default     : return INDIFFERENT;
  }
}


#endif

