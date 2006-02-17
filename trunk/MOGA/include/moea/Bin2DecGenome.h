/*----------------------------------------------------------------------------------------------*
 * --------- Binary String Encoded Representation for solutions of double vector     ---------- *
 *                                                                                              *
 * As all the inds of this kind of solution share common characteristic of mapping function     *
 * from genotype to phenotype, we use a static point to Bin2DecMapping class object.            *
 *                                                                                              *
 *                                                                   xianming Chen, Jan 18, 2001*
 *----------------------------------------------------------------------------------------------*/


#ifndef _Bin2DecGenome_H
#define _Bin2DecGenome_H

#include "Genome.h"

using namespace std;

/*-------------------------------------------------------------------------*
 * Bin2DecMapping class is used for genome class Bin2DecIND.               *
 * It keeps (for each dimension) float range and encoded bit length        *
 *-------------------------------------------------------------------------*/
class Bin2DecMapping {
  struct oneDim {
    double min;
    double max;
    int    bits;    // bits length.
    int    reso;    // resolution of this dimension: amount of different values.
  };
  typedef struct oneDim Range;

 public:
  Bin2DecMapping() : dims(0), pRange(NULL), N(10), n(0) { }
  Bin2DecMapping(const Bin2DecMapping& other) : n(0), pRange(NULL) { copy(other); }
  ~Bin2DecMapping() { delete [] pRange; }
  Bin2DecMapping& operator= (const Bin2DecMapping& rhs) { copy(rhs); return *this; }
  void copy(const Bin2DecMapping& other);

  void add(int bits, int min, int max);
  int dimensions() const { return dims; }
  int bits(int i)  const { return pRange[i].bits; }      // bits strlen of dimension i.
  int resolution(int i) const { return pRange[i].reso; } // resolution of dimension i.
  int totalBits()  const { int rt=0; for(int i=0;i<dims;i++) rt+=pRange[i].bits; return rt; }

  double range(int i) const { return pRange[i].max - pRange[i].min; }
  int    LSB(int i)   const { return MSB(i) + pRange[i].bits - 1; } 
  int    MSB(int i)   const; // most significant(leftmost)bit (counting from left.)
                             // for binary string encoding of this dimension.

 private:
  int dims;
  Range* pRange;

  int n;   // current capacity.
  int N;   // allocation unit.
};

class BinIND : public IND {
 public:
  BinIND(int len, Evaluator efp) :
    IND(len, efp, 2) { }
  BinIND(const BinIND& other) : IND(other) { }
  IND& operator= (const IND& ind) { copy(ind); return *this; }
  ~BinIND() { }
};


class Bin2DecIND : public BinIND {
public:
  MOEADefineIdentity("Bin2DecIND", "b2dIND", 101);

  Bin2DecIND(const Bin2DecMapping & mp, Evaluator efp) :BinIND(mp.totalBits(), efp), 
    _phenotypicVector(PhenotypicVector(mp.dimensions())), mapped(false)
    { if(!_map) _map = new Bin2DecMapping(mp); }
  Bin2DecIND(const Bin2DecIND& other) : BinIND(other), 
    _phenotypicVector(other._phenotypicVector), mapped(other.mapped) { }
  ~Bin2DecIND() { }

  void invalidateCaches()                  { IND::invalidateCaches(); mapped = false; }
  Normalizer* phenotypicNormalizer()const  { return _phenotypicVector.normalizer(); }
  Normalizer* phenotypicNormalizer(const Normalizer& n)const 
    { return _phenotypicVector.normalizer(n); }

  void copy(const Bin2DecIND& other) { 
    if(this==&other) return;
    BinIND::copy(other); 
    if(mapped = other.mapped)
      _phenotypicVector = other._phenotypicVector;
  }
  Bin2DecIND& operator= (const Bin2DecIND& rhs) { copy(rhs); return *this; }
  virtual IND* randomClone() const { IND *p=new Bin2DecIND(*this); p->randomize();return p; }
  virtual IND* clone()       const { return new Bin2DecIND(*this); }

  const PhenotypicVector&  phenotypicVector() const;
  double phenotype(int i) const { return phenotypicVector()[i].convert2double(); }
 private:
  static Bin2DecMapping* _map;
  PhenotypicVector       _phenotypicVector;
  bool                   mapped;
};


inline double DefaultPhenotypicDistance(const IND& ind1, const IND& ind2) {
  Bin2DecIND& Ind1 = (Bin2DecIND&) ind1;  Bin2DecIND& Ind2 = (Bin2DecIND&) ind2;
  return Ind1.phenotypicVector().distance(Ind2.phenotypicVector());
}

inline double DefaultNormalizedPhenotypicDistance(const IND& ind1, const IND& ind2) { 
  Bin2DecIND& Ind1 = (Bin2DecIND&) ind1;  Bin2DecIND& Ind2 = (Bin2DecIND&) ind2;
  return Ind1.phenotypicVector().normalizedDistance(Ind2.phenotypicVector());
}


#endif

