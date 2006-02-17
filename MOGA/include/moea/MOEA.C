#include "MOEA.h"
#include <stdlib.h>
#include <string.h>
//#include <strstream.h>
#include <sstream>

using namespace std;

MOEA::MOEA() : 
  pop1_initSz(INVALID_VALUE), pop2_initSz(INVALID_VALUE),
  pop1_maxSz(INVALID_VALUE), pop2_maxSz(INVALID_VALUE),
  ppop1(NULL), ppop2(NULL), 
  indTemplate(NULL),
  slctor(NULL), asiner(NULL), razor(NULL), 
  niche_radius(INVALID_VALUE),
  maxGen(DEFAULT_MAX_GENERATIONS), 
  pcross(DEFAULT_PCROSS), pmut(DEFAULT_PMUT) {

  paraList.add("help message", "--help", "-h", Help,0);
  paraList.add("initial population 1 size", "pop1_initSize", "pop1InitSz", Int, &pop1_initSz);
  paraList.add("initial populaiton 2 size", "pop2_initSize", "pop2InitSz", Int, &pop2_initSz);
  paraList.add("maximum population 1 size", "pop1_maxSize", "pop1MaxSz", Int, &pop1_maxSz);
  paraList.add("maximum population 2 size", "pop2_maxSize", "pop2MaxSz", Int, &pop2_maxSz);
  paraList.add("initial and maximum population 1 size", "pop1Size", "pop1Sz", Int, &pop1_initSz);
  paraList.add("initial and maximum population 1 size", "pop1Size", "pop1Sz", Int, &pop1_maxSz);
  paraList.add("initial and maximum population 2 size", "pop2Size", "pop2Sz", Int, &pop2_initSz);
  paraList.add("initial and maximum population 2 size", "pop2Size", "pop2Sz", Int, &pop2_maxSz);
  paraList.add("maximum generations", "maxGenerations", "gens", Int, &maxGen);
  paraList.add("current generation", NULL, NULL, Int, &curGen);
  paraList.add("probability of crossover", "pCrossover", "pc", Float, &pcross);
  paraList.add("probability of mutation", "pMutate", "pm", Float, &pmut);
  paraList.add("minimize(0) or maximize(1)", "minMax", "mM", minMaxT, &mM);
  paraList.add("dimensions of objective space", "objectiveDimensions", "objs", Int, &dimens);
  paraList.add("random seed", "seed", "s", Long, &originalSeed);
  paraList.add("niche radius", "nicheRadius", "nr", Double, &niche_radius);
}

MOEA::~MOEA() {
  delete ppop1; delete ppop2; 
  delete slctor; delete asiner; delete razor;
}

void MOEA::initialize(const IND& ind) {
  curGen = 0; 
  Seed = originalSeed; 
  indTemplate = ind.clone();
} 


void MOEA::generate(POP& tmpPop) {

  assert(!tmpPop.currentSize());

  init_select();

  const IND *mom, *dad;
  IND *sis, *bro;
  while((mom=select(), dad=select())) {
    mom->crossover(pcross, *dad, sis, bro);
    sis->mutate(pmut);
    bro->mutate(pmut);

    if( strcmp(className(), "PTSGA")==0 && sis->scoresValid())
      delete sis;
    else 
      tmpPop.append(sis);
    if(strcmp(className(), "PTSGA")==0 && bro->scoresValid())
      delete bro;
    else 
      tmpPop.append(bro);
  }
}


/*--------------- output member functions -----------------------------------------*/

void MOEA :: parameters(ostream& os) const  { 

  os << ".....Algorithms " << className() << ".... current generation ";
  os << currentGeneration() << ".........\n\n";

  if(ptr2pop1()) 
    os << "current population 1 size              " << population1().currentSize() << endl;
  if(ptr2pop2()) 
    os << "current populaiton 2 size              " << population2().currentSize() << endl;
  os << paraList;

  if(razor)
    os << "reduction strategy                     " << razor->className() << endl;
  if(asiner)
    os << "fitness  strategy                      " << asiner->className() << endl;
  if(slctor)    
    os << "selection strategy                     " << slctor->className() << endl;

  if(!ppop1 || !population1().currentSize()) return;
  IND::Distance dist_fp = population1()[0]->metric(); 
  if(!dist_fp) return;
  os << "distance metric                        ";
  if(dist_fp == DefaultObjectiveDistance)             os << "Default Objective Metric"; 
  else if(dist_fp==DefaultNormalizedObjectiveDistance)os << "Default Normalized Objective Metric";
  else if(dist_fp==DefaultPhenotypicDistance)         os << "Default Phenotypic Metric";
  else if(dist_fp==DefaultNormalizedPhenotypicDistance)os<<"Defalt Normalized Phenotypic Metric"; 
  else if(dist_fp==DefaultGenotypicHammingDistance)     os<<"Default Genotypic Hamming Metric"; 
  else                                                 os<<"user defined metric";
  os << endl;
}

const char* MOEA::parametersString() const { // out put parameters as a string.

  ostringstream oss;

  oss << className() << '_';
  if(razor)
    oss << "_" << razor->classShortName();
  if(asiner)
    oss << "_" << asiner->classShortName();
  if(slctor)    
    oss << "_" << slctor->classShortName();

  IND::Distance dist_fp = 0;
  if(ppop1 && population1().currentSize())
    dist_fp = population1()[0]->metric(); 
  if(!dist_fp && ppop2 && population2().currentSize())
    dist_fp = population2()[0]->metric(); 
  if(dist_fp) {
    if(dist_fp == DefaultObjectiveDistance)              oss << "_oM"; 
    else if(dist_fp==DefaultNormalizedObjectiveDistance) oss << "_noM";
    else if(dist_fp==DefaultPhenotypicDistance)          oss << "_pM";
    else if(dist_fp==DefaultNormalizedPhenotypicDistance)oss << "_npM"; 
    else if(dist_fp==DefaultGenotypicHammingDistance)     oss << "_ghM"; 
    else /* user defined metric */                       oss << "_uM";   
  }

  char *tmp = paraList.str();
  oss << tmp; delete tmp;

  return (const char *)(oss.str().c_str());
}


void MOEA :: printFitnesses(ostream& os) const { 
  MOEA* This = (MOEA*) this;
  This->assignFitnesses();
  os << "\n++++ Fitnesses infomation on generation " << curGen << "++++++++++++\n";
  if(ptr2pop1()) {
    os << "in population 1........\n";
    population1().printFitnesses(os); 
  } 
  if(ptr2pop2()) {
    os << "\nin population 2........\n";
    population2().printFitnesses(os); 
  }
  os << endl;
}


void MOEA :: printScores(ostream& os) const { 
  os << "\n++++++++ Scores infomation on generation " << curGen << "++++++++++++\n";
  if(ptr2pop1()) {
    os << "..........in population 1........\n";
    population1().printScores(os); 
  }
  if(ptr2pop2()) {
    os << "\n..........in population 2........\n";
    population2().printScores(os); 
  }
  os << endl;
}

void MOEA :: printResult(ostream& os) const { 
  if(ptr2pop1()) 
    population1().printScores(os); 
  if(ptr2pop2())
    population2().printScores(os); 
  os << endl;
}

void MOEA :: printSolutions(ostream& os) const { 
  os << "\n++++ Solutions infomation on generation " << curGen << "++++++++++++\n";
  if(ptr2pop1()) {
    os << "in population 1........\n";
    population1().printSolutions(os); 
  }
  if(ptr2pop2()) {
    os << "in population 2........\n";
    population2().printSolutions(os); 
  }
  os << endl;
}

void MOEA :: printGenomes(ostream& os) const { 
  os << "\n++++ Genomes infomation on generation " << curGen << "++++++++++++\n";
  if(ptr2pop1()) {
    os << "in population 1........\n";
    population1().printGenomes(os); 
  }
  if(ptr2pop2()) {
    os << "in population 2........\n";
    population2().printGenomes(os); 
  }
  os << endl;
}

void MOEA :: printAll(ostream& os) const { 
  MOEA* This = (MOEA*) this;
  This->assignFitnesses();
  os << "\n++++ All infomation on generation " << curGen << "++++++++++++\n";
  if(ptr2pop1()) {
    os << "in population 1........\n";
    population1().printAll(os); 
  }
  if(ptr2pop2()) {
    os << "in population 2........\n";
    population2().printAll(os); 
  }
  os << endl;
}
