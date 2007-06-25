#include "ParameterList.h"
#include "Pareto.h"
#include <cstdlib>
#include <string>
#include <iomanip>
#include <sstream>

void ParameterList::add(char* vb, char* fn, char* sn, PType t, void* pv) { 
  if(sz==n) {
    Tuple** old_pL = pL;
    pL = new Tuple* [n+=N];
    for(int i=0; i<sz; i++) pL[i] = old_pL[i];
    delete old_pL;
  }
  pL[sz] = new Tuple(vb, fn, sn, t, pv);
  sz++;
}


void ParameterList::readCommandLine(int argc, char** argv) {
  for(int i=1; i<argc; i++) { 
    for(int j=0; j<sz; j++) {
      if(!pL[j]->fname) continue;  // dynamic parameter can not be set.
      if(strcmp(pL[j]->fname, argv[i])==0|| strcmp(pL[j]->sname, argv[i])==0){
	switch(pL[j]->type) {
	case Help:    helpMessage(); exit(0);
	case minMaxT: *((Pareto::minMaxType *)pL[j]->pval) = \
			atoi(argv[++i])? Pareto::MAX : Pareto::MIN;  break; 
	case Int:     *((int*)pL[j]->pval) = atoi(argv[++i]);  break; 
	case Long:    *((long*)pL[j]->pval) = atol(argv[++i]); break;
	case Double:  *((double*)pL[j]->pval) = atof(argv[++i]); break;
	case Float:   *((float*)pL[j]->pval) = atof(argv[++i]); break;
	default:      cerr << "should not get here!\n";
	}
	break;
      }
    }
  }
}

char* ParameterList::str() const {

  ostringstream oss;

  int pop1_initSz, pop2_initSz;
  int pop1_maxSz, pop2_maxSz;
  int count1 = 0, count2 = 0;

  for(int i=0; i<sz; i++) {
    if(!pL[i]->fname || pL[i]->type == Help) continue;
    
    if(strcmp(pL[i]->fname, "pop1Size") == 0) continue;
    if(strcmp(pL[i]->fname, "pop2Size") == 0) continue;
    
    bool valid_parameter = true;
    switch(pL[i]->type) {
    case Int:    valid_parameter =    *((int*)pL[i]->pval) != INVALID_VALUE;  break; 
    case Long:   valid_parameter =   *((long*)pL[i]->pval) != INVALID_VALUE;  break; 
    case Double: valid_parameter = *((double*)pL[i]->pval) != INVALID_VALUE;  break; 
    case Float:  valid_parameter =  *((float*)pL[i]->pval) != INVALID_VALUE;  break; 
    }
    if(!valid_parameter) continue;

    bool cont = false;
    if(count1<2) {
      if(strcmp(pL[i]->fname, "pop1_initSize") == 0)
	{ pop1_initSz = *((int*)pL[i]->pval); count1++; cont = true;}
      if(strcmp(pL[i]->fname, "pop1_maxSize") == 0)
	{ pop1_maxSz = *((int*)pL[i]->pval); count1++;  cont = true;}
      if(count1==2) {
	if(pop1_initSz==pop1_maxSz)
	  oss << "_pop1Sz" << pop1_initSz;
	else 
	  oss << "_pop1InitSz" << pop1_initSz << "_pop1MaxSz" << pop1_maxSz;
      }
    }
    if(count2<2) {
      if(strcmp(pL[i]->fname, "pop2_initSize") == 0)
	{ pop2_initSz = *((int*)pL[i]->pval); count2++; cont = true;}
      if(strcmp(pL[i]->fname, "pop2_maxSize") == 0)
	{ pop2_maxSz = *((int*)pL[i]->pval); count2++;  cont = true;}
      if(count2==2) {
	if(pop2_initSz==pop2_maxSz)
	  oss << "_pop2Sz" << pop2_initSz;
	else 
	  oss << "_pop2InitSz" << pop2_initSz << "_pop2MaxSz" << pop2_maxSz;
      }
    }
    if(cont) continue;

    oss << "_" << pL[i]->sname;
    switch(pL[i]->type) {
    case minMaxT: oss << (*(Pareto::minMaxType *)pL[i]->pval == Pareto::MIN ? 0:1); break; 
    case Int:     oss << *((int*)pL[i]->pval);  break; 
    case Long:    oss << *((long*)pL[i]->pval); break;
    case Double:  oss << *((double*)pL[i]->pval); break;
    case Float:   oss << *((float*)pL[i]->pval); break;
    default:      cerr << "should not get here!\n";
    }
  }
  return (char *)(oss.str().c_str());
}


void ParameterList::helpMessage() const {

  cout << " Parameters for the moea algorithm can be set by member functions, as well as command line arguments. When setting parameters in command line, just enter parameter name, and followed by its value. The name can be either a full name (same as member function name if exist), or a short name.  The parameters for this program are listed below: \n\n";

  int fname_maxLen = 0, sname_maxLen = 0;
  int argument_maxLen = 16;
  for(int i=0; i<sz; i++) {
    if(!pL[i]->fname) continue;  // skip dynamic parameter.
    int len;
    if( (len=strlen(pL[i]->fname)) > fname_maxLen) 
      fname_maxLen = len;
    if( (len=strlen(pL[i]->sname)) > sname_maxLen) 
      sname_maxLen = len;
  }
  fname_maxLen += 2;
  sname_maxLen += 2;
  argument_maxLen += 2;

  cout.setf(ios::left, ios::adjustfield);

  cout << setw(fname_maxLen) << "full name";
  cout << setw(sname_maxLen) << "short name";
  cout << setw(argument_maxLen) << "argument type";
  cout << "the argument will set...\n";
  cout << setw(fname_maxLen) << "---------";
  cout << setw(sname_maxLen) << "----------";
  cout << setw(argument_maxLen) << "-------------";
  cout << "------------------------\n\n";
 
  int pop1 = 0, pop2 = 0;
  for(int i=0; i<sz; i++) {
    if(!pL[i]->fname || strcmp(pL[i]->fname, "--help")==0) continue;
    if(strcmp(pL[i]->fname, "pop1Size")==0) 
      if(pop1++) continue;
    if(strcmp(pL[i]->fname, "pop2Size")==0) 
      if(pop2++) continue;

    cout << setw(fname_maxLen) << pL[i]->fname;
    cout << setw(sname_maxLen) << pL[i]->sname;
    cout << setw(argument_maxLen);
    switch(pL[i]->type) {
    case minMaxT:            cout << "0 or 1"; break;
    case Long: case Int:     cout << "integer"; break;
    case Float: case Double: cout << "double"; break;
    }
    cout << pL[i]->verbose << endl;   
  }
  cout << endl;
}


ostream& operator<< (ostream& os, const ParameterList& list) {

  int verbose_maxLen = 0;
  for(int i=0; i<list.sz; i++) {
    int len;
    if( (len=strlen(list.pL[i]->verbose)) > verbose_maxLen) 
      verbose_maxLen = len;
  }
  verbose_maxLen += 2;
  cout.setf(ios::left, ios::adjustfield);

  for(int i=0; i<list.sz; i++) {
    if(list.pL[i]->type == Help) continue;
    if(list.pL[i]->fname && strcmp(list.pL[i]->fname, "pop1Size") == 0) continue;
    if(list.pL[i]->fname && strcmp(list.pL[i]->fname, "pop2Size") == 0) continue;
    
    bool valid_parameter = true;
    switch(list.pL[i]->type) {
    case Int:    valid_parameter =    *((int*)list.pL[i]->pval) != INVALID_VALUE;  break; 
    case Long:   valid_parameter =   *((long*)list.pL[i]->pval) != INVALID_VALUE;  break; 
    case Double: valid_parameter = *((double*)list.pL[i]->pval) != INVALID_VALUE;  break; 
    case Float:  valid_parameter =  *((float*)list.pL[i]->pval) != INVALID_VALUE;  break; 
    }
    if(!valid_parameter) continue;

    os << setw(verbose_maxLen) << list.pL[i]->verbose;
    switch(list.pL[i]->type) {
    case minMaxT: os << ((*(Pareto::minMaxType *)list.pL[i]->pval==Pareto::MIN)? 0:1); break; 
    case Int:     os << *((int*)list.pL[i]->pval);  break; 
    case Long:    os << *((long*)list.pL[i]->pval); break;
    case Double:  os << *((double*)list.pL[i]->pval); break;
    case Float:   os << *((float*)list.pL[i]->pval); break;
    default:      cerr << "should not get here!\n";
    }
    os << endl;
  }

  return os;
}



