/********************************************************************************
 *                                                                              *
 * -------------------  Class Template for Vector ------------------------------*
 *                                                                              *
 * We add a static Normalizer object to Vector class. If it exists, the range of*
 * each dimension of Vector can be normalized to [0, 1] as necessary.           *
 *                                                   xianming Chen, Aug, 2000   *
 *                                                   modified on Jan 13, 20001  *
 ********************************************************************************/

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <iostream>
#include <math.h>
#include <assert.h>

#include "Normalizer.h"  

using namespace std;

class Vector  {
 public:
  enum nRel { SMALLER, EQUAL, BIGGER, INCOMPARABLE };       // numeric comparison.
  friend ostream& operator<< (ostream& os, const nRel rel); 
};

//DAV - gcc 3.4 compatibility

template<class T> class VectorT;
template<class T> ostream& operator<< (ostream& os, const VectorT<T>& tuple);

template<class T>  
class VectorT : public Vector {
 public:
  VectorT(int d=0) : dims(d), vector(d? new T[d] : 0) { }
  VectorT(T *pv, int dimens) {vector=new T[dims=dimens]; for(int i=0;i<dims;i++) vector[i]=pv[i]; }
  VectorT(const VectorT & other) : vector(NULL), dims(0) { copy(other); }
  virtual  ~VectorT() { delete [] vector; }

  void copy(const VectorT& org); 
  const VectorT& operator= (const VectorT& rhs) { copy(rhs); return *this; }

  int  dimensions() const             { return dims; }
  bool dimensions(int dms);           // reset dims, copy old values. ret true if dims changed.
  T& operator[] (int idx)             { assert(idx>=0&&idx<dims); return vector[idx]; }
  T operator[] (int idx) const        { assert(idx>=0&&idx<dims); return vector[idx]; }

  static Normalizer* normalizer()                    { return _normalizer; }
  static Normalizer* normalizer(Normalizer* pn)      { return _normalizer=pn; }
  static Normalizer* normalizer(const Normalizer& n) { return normalizer(n.clone()); }
  double distance(const VectorT & other) const;
  double normalizedDistance(const VectorT & other) const;

  nRel compare(const VectorT & other) const;

  /*+++++++++++++++++++ NOTE: != means not comparable. ++++++++++++++++++++++++++++*/
  bool operator!= (const VectorT &r) { return compare(r)==INCOMPARABLE? true:false; }
  bool operator> (const VectorT  &r) { return compare(r)==BIGGER      ? true:false; }
  bool operator< (const VectorT  &r) { return compare(r)==SMALLER     ? true:false; }
  bool operator== (const VectorT &r) { return compare(r)==EQUAL       ? true:false; }
  bool operator>= (const VectorT &r) { return *this > r || *this == r ; }
  bool operator<= (const VectorT &r) { return *this < r || *this == r ; }

  const VectorT& operator+= (const VectorT &r) { assert(dims==r.dims);
       for(int i=0;i<dims;i++) vector[i] += r.vector[i]; return *this; }

  friend ostream& operator<< <>(ostream& os, const VectorT<T>& tuple);

 protected:
  T* vector;
  int dims;
  static Normalizer* _normalizer;
};

template<class T>
Normalizer* VectorT<T>::_normalizer = 0;

template<class T>
void VectorT<T>::copy(const VectorT& other) {
  if(this == &other) return;
  if(dims != other.dims) {
    delete vector;
    vector = new T [dims=other.dims];
  }
  for(int i=0; i<dims; i++) 
    vector[i] = other.vector[i];
}

template<class T>
bool VectorT<T>::dimensions(int d) {
  if(dims == d) return false;
  T* old = vector; int _dims = dims;
  vector = new T [dims=d];
  for(int i=0; i<d && i<_dims; i++)
    vector[i] = old[i];
  delete old;
  return true;
}


template<class T>
Vector::nRel VectorT<T> :: compare(const VectorT& other) const {
  assert(dims==other.dims);
  bool bigger = false, smaller = false; 
  bool equal = false, indiff = false;
  for(int i=0; !(indiff) && i<dims; i++) {
    if(vector[i] > other.vector[i]) bigger  = true;
    if(vector[i] < other.vector[i]) smaller = true;
    indiff = (bigger && smaller);
  }
  if(indiff) return  INCOMPARABLE;
  if(bigger) return  BIGGER;
  if(smaller)return  SMALLER;
  return             EQUAL;
}


template<class T>
double VectorT<T> :: distance(const VectorT & other) const {
  assert(dims==other.dims);
  double sum = 0.0;
  for(int i=0; i<dims; i++)
    sum += (vector[i]-other.vector[i]) * (vector[i]-other.vector[i]);
  return sqrt(sum);
}


template<class T>
double VectorT<T> :: normalizedDistance(const VectorT & other) const {
  assert(_normalizer);
  assert(dims==other.dims);

  double sum = 0.0; double scaled1, scaled2;
  for(int i=0; i<dims; i++) {
    scaled1 = _normalizer->a(i) * vector[i] + _normalizer->b(i);
    scaled2 = _normalizer->a(i) * other.vector[i] + _normalizer->b(i);
    assert(scaled1>=0 && scaled1 <=1 && scaled2>=0 && scaled2 <=1 );
    sum += (scaled1 - scaled2) * (scaled1 - scaled2);
  }

  return sqrt(sum);
}


template<class T>
ostream& operator<< (ostream& os, const VectorT<T>& tuple) {
  os << "(" << tuple.vector[0];
  for(int i=1; i<tuple.dims; i++)
    os << ", " << tuple.vector[i];
  os << ")";
}


inline ostream& operator<< (ostream& os, const Vector::nRel rel) {
  switch(rel) {
  case Vector::BIGGER  : os << "bigger than ";  break;
  case Vector::SMALLER : os << "smaller than ";break;
  case Vector::EQUAL   : os << "equal to ";     break;
  default              : os << "incomparable to "; 
  }
  return os;
}

inline Vector::nRel _not(Vector::nRel rel) { 
  switch(rel) {
  case Vector::BIGGER : return Vector::SMALLER;
  case Vector::SMALLER: return Vector::BIGGER;
  default             : return rel;
  }
}

typedef VectorT<double>   dVector;
typedef VectorT<int>      iVector;
typedef VectorT<float>    fVector;

#endif
