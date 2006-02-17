#ifndef _ParameterList_H
#define _ParameterList_H
#include <iostream>

using namespace std;


enum PType { Help, Int, Long, Float, Double, minMaxT };

class ParameterList {

  struct Tuple{ 
    Tuple(char* vb, char* fn, char* sn, PType t, void* pv) :
      verbose(vb), fname(fn), sname(sn), type(t), pval(pv) { }
    char* verbose;  // detailed explanation of the parameter.
    char* fname;    // Full name. Same as corresponding member function.
    char* sname;    // Short name.
    PType type; 
    void* pval; 
  };

 public:
  ParameterList(int chunk=2) : N(chunk), pL(NULL), sz(0), n(0) { }
  ~ParameterList() { for(int i=0; i<sz; i++) delete pL[i]; delete pL; }

  void add(char* vb, char* fn, char* sn, PType t, void* pv);
  void readCommandLine(int argc, char** argv);
  void helpMessage() const;
  char* str() const; // get parameter list as a string. give up ownship of string. 

  friend ostream& operator<< (ostream&, const ParameterList&);
 
 private:
  Tuple** pL;
  int sz;     // current tuples in the list.
  int n;      // current capacity of the list.
  int N;      // allocation chunksize.
};

#endif

