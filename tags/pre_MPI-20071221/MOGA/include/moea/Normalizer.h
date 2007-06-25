/*------------------------------------------------------------------------------------------*
 * Linear Normalizer Class:                                                                 *
 *  This class can linear mapping from original range [min, max] to normalized range [0,1]. *
 *----------------------------------------------------- Jan 23, 2001, xianming Chen---------*/

#include <iostream>

using namespace std;

class Normalizer {
  struct oneDim {
    double min;
    double max;
    double   a;  // parameters for linear normalization: 
    double   b;  // f = ax + b.
  };
  typedef struct oneDim Range;

 public:
  Normalizer() : dims(0), pRange(NULL), N(10), n(0) { }
  Normalizer(const Normalizer& other) : n(0), pRange(NULL) { copy(other); }
  Normalizer& operator= (const Normalizer& rhs) { copy(rhs); return *this; }
  ~Normalizer() { delete [] pRange; }

  void copy(const Normalizer& other);
  virtual Normalizer* clone() const { return new Normalizer(*this); }

  void add(double min, double max);

  double range(int i) const  { return pRange[i].max - pRange[i].min; }
  double max(int i) const    { return pRange[i].max; }
  double min(int i) const    { return pRange[i].min; }
  double a(int i) const      { return pRange[i].a; }
  double b(int i) const      { return pRange[i].b; }
  int dimensions() const     { return dims; }

  friend ostream& operator<< (ostream& os, const Normalizer& n);

 private:
  int dims;
  Range* pRange;

  int n;   // current capacity.
  int N;   // allocation unit.
};

