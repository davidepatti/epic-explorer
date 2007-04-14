/*-----------------------------------------------------------------------------------------*
 * Pareto class defines static data members common to all objects in MOEA. They include:   *
 *     minimization or maximization optimization.                                          *
 *     objective dimensions.                                                               *
 *     niching kinds and parameters.                                                       *
 * Some enumeration constants are also defined here.                                       *
 *                                                                                         *
 *------------------------------------------------------------xianming Chen, Feb 2, 2001 --*/

#ifndef _PARETO_H
#define _PARETO_H

#include <iostream>
#include <assert.h>

using namespace std;

#define MOEADefineIdentity(name, sname, id) \
     virtual const char * className() const {return name;} \
     virtual const char * classShortName() const {return sname;} \
     virtual int classID() const {return id;}

#define DOUBLE_LARGE 1.0E30
#define LARGE 2147483647          //maximal positive integer for 4 byte implementation.
#define INVALID_VALUE  (- LARGE) 

#define DEFAULT_MINMAX            Pareto::MIN
#define DEFAULT_OBJ_DIMENS        2


class Pareto { 
 public:
  enum minMaxType{ MIN, MAX };  // minimization optimization or maximization.
  enum pRel { DOMINATED, SAME, DOMINATING, INDIFFERENT }; // pareto comparison.

  enum NICHING_ON { NOT_USED, OBJECTIVE_SPACE, PHENOTYPIC_SPACE, GENOTYPIC_SPACE };
  enum COPY_TYPE  { SHALLOW, DEEP };

  friend ostream& operator<< (ostream& os, minMaxType mM);
  friend ostream& operator<< (ostream& os, pRel rel);

  static void maximize()                      { minMax(MAX); }
  static void minimize()                      { minMax(MIN); }
  static bool isMaximize()                    { return mM == MAX; }
  static bool isMinimize()                    { return mM == MIN; }
  static minMaxType minMax()                  { return mM; }
  static minMaxType minMax(minMaxType which)  { return mM = which; }

  static int objectiveDimensions()            { return dimens; }
  static int objectiveDimensions(int ds)      { return dimens = ds; }

 protected:
  static minMaxType mM;              // minimize or maximize?
  static int dimens;                 // objective dimensions.
};



/*--------------------- stream output overload -------------------------------*/

inline ostream& operator<< (ostream& os, Pareto::pRel rel) {
  switch(rel) {
  case Pareto::DOMINATING: os << "dominating ";  break;
  case Pareto::DOMINATED : os << "dominated by ";break;
  case Pareto::SAME      : os << "same to ";     break;
  default                : os << "indifferent to "; 
  }
  return os;
}

inline ostream& operator<< (ostream& os, Pareto::minMaxType mM) {
  os << ((mM==Pareto::MIN)? "minimization " : "maximization ");
  return os;
}



/*------------- negative of enumerate variable  ---------------------------*/

inline Pareto::pRel _not(Pareto::pRel rel) { 
  switch(rel) {
  case Pareto::DOMINATING: return Pareto::DOMINATED;
  case Pareto::DOMINATED : return Pareto::DOMINATING;
  default                 : return rel;
  }
}

inline Pareto::minMaxType _not(Pareto::minMaxType mM) { 
  if(mM==Pareto::MIN) return Pareto::MAX;
  return Pareto::MIN;
}

#endif





