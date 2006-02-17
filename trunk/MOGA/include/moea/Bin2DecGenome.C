#include "Bin2DecGenome.h"
#include <math.h>

Bin2DecMapping* Bin2DecIND::_map = 0;

void Bin2DecMapping::copy(const Bin2DecMapping& org) {
  if(n!=org.n) {
    delete[] pRange;
    pRange = new Range [n=org.n];
  }
  dims = org.dims; N=org.N;
  for(int i=0; i<dims; i++) 
    pRange[i] = org.pRange[i];
}

void Bin2DecMapping::add(int bits, int min, int max) {   //add one dimension.
  if( n==dims ) { //reallocate memory.
    Range* old_ptr = pRange;
    pRange = new Range[n += N];
    for(int i=0; i<dims; i++)
      pRange[i] = old_ptr[i];
    delete [] old_ptr;
  }
  pRange[dims].min = min;
  pRange[dims].max = max;
  pRange[dims].bits = bits;
  pRange[dims].reso = (int) pow((double)2, bits);
  dims++;
}
int Bin2DecMapping::MSB(int i)   const {// most significant(leftmost) bit of dimen i. 
  int rt = 0;                           // NOTE: count from left to right.  
  for(int d = 0; d<i; d++) rt += pRange[d].bits;
  return rt;
}



const PhenotypicVector& Bin2DecIND::phenotypicVector() const {
  if(!mapped) {
    Bin2DecIND* This =  (Bin2DecIND*) this;
    for(int i=_phenotypicVector.dimensions()-1; i>=0; i--) {
      int rawVal = 0;
      int lsb = _map->LSB(i);
      for(int j=_map->MSB(i); j<=lsb; rawVal *= 2, j++) {
	if(data[j] == '1') rawVal++;
      }
      rawVal /= 2;  //undo the last shift operation.
      This->_phenotypicVector[i] = rawVal  * _map->range(i) / _map->resolution(i);
    }
    This->mapped = true;
  }
  return _phenotypicVector;
}

