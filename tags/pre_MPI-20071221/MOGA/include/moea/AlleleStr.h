/********************************************************************************
 *                                                                              *
 * -------------------   Class for Allele String -------------------------------*
 *                                                                              *
 * the allele set is consisted of the following symbols(characters):            *
 * 1. if alleles <= 10: 0, 1,..., 9                                             *
 * 2. if alleles <= 26: a, b,..., z                                             *
 *                                                                              *
 *                                                   xianming Chen, Aug, 2000   *
 *                                                   modified on Jan 14, 2001   *
 ********************************************************************************/


#ifndef _AlleleStr_H_
#define _AlleleStr_H_

#include <iostream>
#include <assert.h>
#include <vector>
#include "Random.h"

using namespace std;

class AlleleString  {

 public:
  typedef long Allele;

  AlleleString() : len(0), _alleles(0), data(NULL) { }

  AlleleString(int ln, int als );

  AlleleString(const AlleleString& other) { data = 0; len = -1; copy(other); }

  AlleleString(vector<vector<Allele> > alss);

  virtual ~AlleleString() { delete [] data; }

  void copy(const AlleleString& other);
  AlleleString& operator= (const AlleleString& other) { copy(other); return *this; }

  const Allele& operator[] (int index) const { assert(index<len); return data[index]; }
  Allele& operator[] (int index)             { assert(index<len); return data[index]; }

  void randomize();

  int length()  const { return len; }
  int alleles() const { return _alleles; }

  int hammingDistance(const AlleleString& s2) const { 
    int rt=0; for(int i=0; i<len; i++) if(data[i] != s2.data[i]) rt++; return rt; }

  friend ostream& operator<< (ostream & os,const AlleleString& str);
  friend bool operator== (const AlleleString&, const AlleleString&);
  friend bool operator!= (const AlleleString& a, const AlleleString& b) { return !(a==b); }

 protected:
  Allele* data;
  int len;
  int _alleles;
  vector<vector<Allele> > alleles_set;
};

#endif
