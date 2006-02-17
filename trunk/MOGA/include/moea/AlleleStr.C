#include "AlleleStr.h"

AlleleString::AlleleString(int ln, int als = 2)
{
  len = ln; 
  _alleles = als; 
  data = new Allele [len];

  vector<Allele> s;  
  for (int j=0; j<_alleles; j++)
    s.push_back(j);

  for (int i=0; i<len; i++)
    alleles_set.push_back(s);
}

AlleleString::AlleleString(vector<vector<Allele> > alss)
{
  alleles_set = alss;
  len = alss.size();
  _alleles = -1; // means that alleles should be taken from alleles_set
  data = new Allele [len];
}

void AlleleString :: copy(const AlleleString& other) {
  if(this == &other) return;
  if(len != other.len) {
    delete [] data;
    data = new Allele [len = other.len];
  }
  _alleles = other._alleles;
  alleles_set = other.alleles_set;
  for(int i=0; i<len; i++) data[i] = other.data[i];
}


bool operator== (const AlleleString& a, const AlleleString& b) {
  if(a.len != b.len) return false;
  int len = a.len; bool isEqual = true;
  for(int i=0; isEqual && i<len; i++)
    isEqual = (a[i]==b[i]);
  return isEqual;
}


ostream& operator<< (ostream& os, const AlleleString& astr) {
  for(int i=0; i<astr.len; i++) 
    os << astr.data[i] << " ";
  return os;
}

void AlleleString::randomize() 
{ 
  /*
    for(int i=0; i<len; i++) 
    data[i] = Randint(0,_alleles-1) + (_alleles>10 ? 'a':'0');
  */
  for(int i=0; i<len; i++) 
    {
      int allele_idx = Randint(0, alleles_set[i].size()-1);
      data[i] = alleles_set[i][allele_idx];
    }
}

