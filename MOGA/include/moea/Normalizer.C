#include "Normalizer.h"
#include <iomanip>

void Normalizer::copy(const Normalizer& org) {
  if(n!=org.n) {
    delete[] pRange;
    pRange = new Range [n=org.n];
  }
  dims = org.dims; N=org.N;
  for(int i=0; i<dims; i++) 
    pRange[i] = org.pRange[i];
}

void Normalizer::add(double min, double max) {   //add one dimension.
  if( n==dims ) { //reallocate memory.
    Range* old_ptr = pRange;
    pRange = new Range[n += N];
    for(int i=0; i<dims; i++)
      pRange[i] = old_ptr[i];
    delete [] old_ptr;
  }
  pRange[dims].min = min;
  pRange[dims].max = max;
  pRange[dims].a = 1.0 / (max - min);
  pRange[dims].b = - min * pRange[dims].a;
  dims++;
}

ostream& operator<< (ostream& os, const Normalizer& n) {
  os << "range and normalizing factors of each dimension.\n";
  for(int i=0; i<n.dims; i++) {
    os << "dimension " << i+1 << ": ";
    os << "min = " << n.pRange[i].min << ", max = " << n.pRange[i].max;
    os << "\t\ta = " << setw(4) << n.pRange[i].a << "     b = " << setw(4) << n.pRange[i].b << endl;
  }
  return os;
}
