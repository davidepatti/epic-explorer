#include "Grid_Loc.h"

int Grid_Loc :: sz = 0;           
int Grid_Loc :: cur_depth = 0;
int Grid_Loc :: strLenPerObj = 1;
unsigned int Grid_Loc :: PCC[maxINTs] = { 0 };
const unsigned int Grid_Loc :: BIT_MASK[sizeof(int)*8] = { 
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,\
  0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000,\
  0x010000, 0x020000, 0x040000, 0x080000, 0x100000, 0x200000, 0x400000, 0x800000,\
  0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000 
};


bool Grid_Loc::update(int curdepth) { //update cur_depth, and others if necessary.
  assert(sizeof(int)==4); //otherwise, BIT_MASK need changing.
  cur_depth = curdepth;
  if(strLenPerObj > cur_depth) return false;  // no change needed.

  while( (strLenPerObj *= 2) <= cur_depth ); 
  sz = (strLenPerObj * dimens) / INTbits; 
  if( (strLenPerObj * dimens) % INTbits) sz++;

  for(int i=0; i<sz; i++) PCC[i] = 0; 
  for(int obj=0; obj<dimens; obj++) { // compute new PCC.
    int pos = (obj+1)*strLenPerObj - 1;
    PCC[pos/INTbits] |= BIT_MASK[pos%INTbits]; 
  }

  return true;
}


Pareto::pRel Grid_Loc::compare(const Grid_Loc& other) const {
  assert(sz>=0 && sz<maxINTs);
  assert(strLenPerObj>1 && strLenPerObj <= INTbits && !(strLenPerObj%2));

  pRel preRel, curRel;
  for(int i=0; i<sz; i++) {
    if(code[i]==other.code[i]) curRel = SAME;
    else if(code[i] < other.code[i]) {
      if(!(((code[i]|PCC[i])-other.code[i] ) & PCC[i]))
	curRel= mM==MIN? DOMINATING : DOMINATED;    // all '0' for PCBs.
      else return INDIFFERENT;
    }
    else {
      if(!(((other.code[i]|PCC[i])-code[i] ) & PCC[i]))
	curRel= mM==MAX? DOMINATING : DOMINATED;    // all '0' for PCBs.
      else return INDIFFERENT;
    }
    if(i && preRel != curRel) return INDIFFERENT;
    preRel = curRel;
  }
  return curRel;
}



#ifdef DEBUG

char* Grid_Loc::str(int i) const {
  assert(i<dimens && i>=0);
  char* string = new char[cur_depth+1];
  for(int lvl=1; lvl<=cur_depth; lvl++) {
    if(setted(i,lvl)) 
      string[lvl-1] = '1';
    else
      string[lvl-1] = '0';
  }
  string[cur_depth] = 0;
  return string;
}


ostream& operator<< (ostream& os, const Grid_Loc& loc) {
  for(int i=loc.dimens-1; i>=0; i--) {
    char* string = loc.str(i);
    cout << string << ' ';
    delete [] string;
  }
  cout << endl;
  return os;
}

#endif
