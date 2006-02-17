#include "Select.h"

void Selector::initialize(const POP* p1, const POP* p2, int poolSize) { 
  ppop1=p1; ppop2=p2;  
  sz1 = ppop1? ppop1->currentSize() : 0;
  sz2 = ppop2? ppop2->currentSize() : 0;
  poolSz = poolSize ? poolSize : sz1;
  if(poolSz%2) poolSz++;
  delete pool; pool = new IND* [poolSz+2];  
  pool[poolSz] = pool[poolSz+1] = NULL;   // end of selected population.
} 

void TournamentSelector::initialize(const POP* p1, const POP* p2, int poolSize) { 

  Selector::initialize(p1, p2, poolSize);

  for(int i=0; i<poolSz; i++) {
    generateTournament(tournamentSz);
    IND* winner = individual(tournament[0]);
    for(int j=1; j<tournamentSz; j++) {
      IND* curInd = individual(tournament[j]);
      if(winner->fitness() > curInd->fitness())
	winner = curInd;
    }
    pool[i] = winner;
  }
  curIdx = 0;
}

void TournamentSelector::generateTournament(int size) {
  int maxIdx = sz1 + sz2 -1, curRandIdx;
  int j = 0;   
  while(j < size) {
    curRandIdx = Randint(0, maxIdx);
    bool isNewIdx = true;
    if(j <= maxIdx) {
      for(int i=0; isNewIdx && i<j; i++)
	isNewIdx = (curRandIdx != tournament[i]);
    }
    if(isNewIdx) tournament[j++] = curRandIdx;
  }
}



ParetoTournamentSelector::ParetoTournamentSelector(int c, double r, int sz) 
  : TournamentSelector(sz), compSz(c), nicheRadius(r) { 
  delete tournament; 
  /* comparison members stored in extended tournament[] */
  tournament = new int [compSz + tournamentSz]; 
}

void ParetoTournamentSelector::initialize(const POP* p1, const POP* p2, int poolSize) {

  Selector::initialize(p1, p2, poolSize);

  int totalSz = compSz + tournamentSz;
  int *nondominated = new int[tournamentSz]; 
  int  nondominatedCount; // among current tournament candidates.

  for(int i=0; i<poolSz; i++) {

    generateTournament(totalSz);  // generate totalSz different index.
    nondominatedCount = 0;
    for(int j=0; j<tournamentSz; j++) {
      IND* cur = individual(tournament[j]);
      bool dominated = false;
      for(int k=tournamentSz; !dominated && k< totalSz; k++) 
	dominated = (*cur) < (*individual(tournament[k]));
      if(!dominated) 
	nondominated[nondominatedCount++] = tournament[j];
    }

    if(nondominatedCount==1) 
      pool[i] = individual(nondominated[0]);

    else { // select winner from a tie by contiuously updated niching.
      int tie_count = tournamentSz;   // # of inds in the tie.
      if(nondominatedCount) {
	tie_count = nondominatedCount;
	for(int j=0; j<nondominatedCount; j++)
	  tournament[j] = nondominated[j];
      } 

      /* now tie candidates are tournament[0..tie_count-1] */

      int minNicheCount = LARGE; 
      IND* winner = 0;
      for(int j=0; j<tie_count; j++) {
	IND* curInd = individual(tournament[j]);
	int curNicheCount = 0;
	for(int k=0; k<i; k++)   // niching in partially filled pool.
	  if(curInd->distance(*pool[k]) < nicheRadius) 
	    curNicheCount++;	
	if(curNicheCount < minNicheCount) {
	  minNicheCount = curNicheCount;
	  winner = curInd;
	}
      }
      pool[i] = winner;
    }
  }

  curIdx = 0;
  return;
}



void VEGATournamentSelector::initialize(const POP* p1, const POP* p2, int poolSize) {

  Selector::initialize(p1, p2, poolSize);
  bool isMaximize = (*ppop1)[0]->isMaximize();

  int quota = poolSz / dimens; 

  for(int i=0; i<poolSz; i++) {

    int obj = i / quota;  // current selection based on this objective.
    if(obj == dimens) 
      obj = Randint(0, dimens-1);

    generateTournament(tournamentSz);
    IND* winner = individual(tournament[0]);
    for(int j=1; j<tournamentSz; j++) {
      IND* curInd = individual(tournament[j]);
      double diff = winner->objectiveVector()[obj] - curInd->objectiveVector()[obj];
      if( (isMaximize && diff < 0) || (!isMaximize && diff > 0) )
	winner = curInd;
    }
    pool[i] = winner;
  }

  curIdx = 0;
}

