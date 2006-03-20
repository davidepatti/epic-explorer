//Fuzzy Function Approximation
#define VERBOSE_ON // activates verbose mode
#include <memory.h>
#include <math.h>
#include "FuzzyApprox.h"


CFuzzyFunctionApproximation::CFuzzyFunctionApproximation() {

  InDim = 0;
  OutDim = 0;
  InputSetsNumber = 0;
  InputsMin = 0;
  InputsMax = 0;
  InputCenters = 0;
  RuleTable = 0;
  newRule.antecedents = NULL;
  newRule.degrees = NULL;
  newRule.consequents = NULL;
  estimatedValues = 0;
  degrees = 0;
  alpha = 0;
  Sets = 0;
  count = 0;
  calcola = 0;
  errore = 0;
  errmatrix = NULL;
  prove = 0;
  stima = 0;
  threshold = 0;
  char *stri = getenv("HOME");
  char pathx[50];

    time_t t = time(NULL);
    char * data;

    data = asctime(localtime(&t));

  sprintf(pathx,"%s/trimaran-workspace/epic-explorer/fuzzy_log.txt",stri);
  fuzzy_log = fopen(pathx,"a");
  fprintf(fuzzy_log,"\n\n--------------------------------\n %s - Fuzzy Function Approximation Initialized\n----------------------------\n\n",data);
  fflush(fuzzy_log);
}
bool CFuzzyFunctionApproximation::GenerateInputFuzzySets(int dim, int *numbers, REAL *Min, REAL *Max) {


	//Total number of sets
	InDim = dim;
	int number = 0;
	
	if (Sets != NULL) delete Sets;
	Sets = new int[InDim];
	if (Sets == NULL) return (false);

	for(int i=0;i<InDim;++i) {
		Sets[i] = number;
		number += numbers[i];
	}

	//Alloacate vectors
	if (InputCenters != NULL) delete InputCenters;
	InputCenters = new REAL[number];
	if (InputCenters == NULL) return (false);
	memset(InputCenters,0,sizeof(REAL)*number);
	
	if (InputSetsNumber != NULL) delete InputSetsNumber;
	InputSetsNumber = new int[InDim];
	if (InputSetsNumber == NULL) return (false);

	if (InputsMin != NULL) delete InputsMin;
	InputsMin = new REAL[InDim];
	if (InputsMin == NULL) return (false);

	if (InputsMax != NULL) delete InputsMax;
	InputsMax = new REAL[InDim];
	if (InputsMax == NULL) return (false);

	if (alpha != NULL) delete alpha;
	alpha = new REAL[InDim];
	if (alpha == NULL) return (false);

	//Copy variables
	memcpy(InputSetsNumber,numbers,sizeof(int)*InDim);
	memcpy(InputsMin,Min,sizeof(REAL)*InDim);
	memcpy(InputsMax,Max,sizeof(REAL)*InDim);

	//Calculate Input Set Center values
	REAL step = 0;
	for (int i=0;i<InDim;++i) {
		if (InputSetsNumber[i] == 1) {
			InputCenters[Sets[i]] = InputsMax[i]+0.1f;
			continue;
		}
		InputsMax[i] += InputsMax[i]*0.01f;
		InputsMin[i] -= InputsMin[i]*0.01f;
		step = (InputsMax[i]-InputsMin[i])/REAL(InputSetsNumber[i]-1);
		alpha[i] = step / sqrt(5.545f);
		for (int j=0;j<InputSetsNumber[i];++j) {
			InputCenters[Sets[i]+j] = InputsMin[i]+(step*REAL(j));
		}
	}
			
	return (true);
};


bool CFuzzyFunctionApproximation::StartUp(int N, REAL thres,int _min, int _max) {
	// Creates the Rule Table

  	//OutDim = 2; // Provvisorio

	threshold = thres;
	min_sims = _min;
	if (min_sims <= ERR_MEMORY) min_sims = ERR_MEMORY+1;
	max_sims = _max;
		
	if (RuleTable != NULL) delete RuleTable;
	RuleTable = new CRuleList(N,InDim,OutDim);

	count = 0;

	if (estimatedValues != NULL) delete estimatedValues;
	estimatedValues = new REAL[OutDim];
	if (estimatedValues == NULL) return (false);

	
	if (degrees != NULL) delete degrees;
	degrees = new REAL[OutDim];
	if (degrees == NULL) return (false);

	
	if (newRule.antecedents != NULL) delete newRule.antecedents;
	newRule.antecedents = new int[InDim];
	if (newRule.antecedents == NULL) return (false);

	if (newRule.consequents != NULL) delete newRule.consequents;
	newRule.consequents = new REAL[OutDim];
	if (newRule.consequents == NULL) return (false);

	if (newRule.degrees != NULL) delete newRule.degrees;
	newRule.degrees = new REAL[OutDim];
	if (newRule.degrees == NULL) return (false);

	if (errore != NULL) delete errore;
	errore = new REAL[OutDim];
	if (errore == NULL) return (false);

	if (errmatrix != NULL) {
	    for (int i=0; i<OutDim; ++i) {
		if (errmatrix[i] != NULL) delete errmatrix[i];
	    }
	    delete errmatrix;
	}
	errmatrix = new REAL*[OutDim];
	if (errmatrix == NULL) return (false);
	for (int i=0; i<OutDim; ++i) {
	    errmatrix[i] = new REAL[ERR_MEMORY];
	    if (errmatrix[i] == NULL) return (false);
	    memset(errmatrix[i],0,sizeof(REAL)*ERR_MEMORY);
	}
	

	if (stima != NULL) delete stima;
	stima = new REAL[OutDim];
	if (stima == NULL) return (false);

	calcola = false;

	prove = 0;

	return (true);
}

int CFuzzyFunctionApproximation::position() {
    int temp = posx;
    posx = (posx+1)%ERR_MEMORY;
    return (temp);
}

bool CFuzzyFunctionApproximation::Learn(REAL* InputValue, REAL* OutputValue) {

	int i,j;

	fprintf(fuzzy_log,"\n----------------------- Fuzzy System is Learning ------------------------------------\n");
	for (i=0;i<OutDim; i++)
	  fprintf(fuzzy_log,"%lf \t", OutputValue[i]);
	fprintf(fuzzy_log, "---- %u \n", prove);

	fflush(fuzzy_log);

	int rulen = RuleTable->getRuleNumber();
	REAL m = 1.0f, mm;
	memset(stima,0,sizeof(REAL)*OutDim);
	memset(newRule.antecedents,0,sizeof(int)*InDim);
	memset(newRule.consequents,0,sizeof(int)*OutDim);
	memset(newRule.degrees,0,sizeof(REAL)*OutDim);
	
	if (rulen > 0 && prove < max_sims) 
	{
		EstimateG(InputValue,stima);
		j = position();
		for(i=0;i<OutDim;++i) {
			errore[i] = fabs(OutputValue[i] - stima[i]);
			errmatrix[i][j] = REAL(errore[i]/OutputValue[i]);
		} 
	}

	prove++;
	
	for(i=0;i<InDim;++i) {;
		if (InputValue[i] > InputCenters[Sets[i]]) {
			if (InputValue[i] < InputCenters[Sets[i]+InputSetsNumber[i]-1]) {
				for(j=0;j<InputSetsNumber[i]-1;++j) {
					if (InputValue[i] <= InputCenters[Sets[i]+j+1]) {
						mm = ((InputCenters[Sets[i]+j+1]-InputValue[i])/(InputCenters[Sets[i]+j+1]-InputCenters[Sets[i]+j]));
						if (mm>=0.5f) m *= mm;					
						else {m *= (1-mm);j++;}
						newRule.antecedents[i] = j;
						j=InputSetsNumber[i];
					}
				}
			} else {
				//m *= 1.0f;
				newRule.antecedents[i] = (InputSetsNumber[i]-1);
			}
		} 
		//else m *= 1.0f;
	}
	
	for(i=0;i<OutDim;++i) {
		newRule.consequents[i] = OutputValue[i];
	}
	
	RuleTable->insertRule(newRule);

	calcola = true;

	return (true);
}

bool CFuzzyFunctionApproximation::Reliable() {
    REAL errmax = 0.0f;
    
    memset(errore,0,sizeof(REAL)*OutDim);
    
    if (prove < min_sims) return (false);
    if (prove == min_sims) {
	    fprintf(fuzzy_log,"\nMinimum number of simulations %d reached.\n", min_sims);
	    fflush(fuzzy_log);
    }
    if (prove == max_sims) {
	    fprintf(fuzzy_log,"\nMaximum number of simulation %d reached.\n", max_sims);
	    fflush(fuzzy_log);
    }
    if (prove > max_sims) return (true);
    
    for(int i=0; i<OutDim; ++i) {
	//fprintf(fuzzy_log,"\n");
	//fflush(fuzzy_log);
	for(int j=0; j<ERR_MEMORY; ++j) {
	    errore[i] += errmatrix[i][j];
	    //fprintf(fuzzy_log,"%.2lf,", errmatrix[i][j]*100);
	}
	//fflush(fuzzy_log);
	errore[i] /= ERR_MEMORY;
        errmax += errore[i]*errore[i];
    }

    errmax = sqrt(errmax)*100;

    if (errmax < threshold) {
	    fprintf(fuzzy_log,"\nNumber of sims %d, error %% is %lf < threshold %lf.",prove,errmax,threshold); 
	    fflush(fuzzy_log);
	    return (true);
    }

    fprintf(fuzzy_log,"\nNumber of sims %d, error %% is %lf > threshold %lf.",prove,errmax,threshold);
    fflush(fuzzy_log);
    return (false);
}


bool CFuzzyFunctionApproximation::EstimateG(REAL* InputValue, REAL* Outputs) {
	int i,j,k;
	REAL maxdegree = 0;
	int rulen = RuleTable->getRuleNumber();
	Rule currentRule;
	REAL degree = 0;
	estimatedValues = Outputs;
	memset(estimatedValues,0,sizeof(REAL)*OutDim);	
	memset(degrees,0,sizeof(REAL)*OutDim);

	for(i=0;i<rulen;++i) {
		currentRule = RuleTable->getRule(i);
		degree = 1.0f; 
		for(j=0;j<InDim;++j) {
			if (InputValue[j]>InputCenters[Sets[j]] && InputValue[j]<InputCenters[Sets[j]+InputSetsNumber[j]-1])
				degree *= exp(-0.5f*((InputValue[j]-InputCenters[Sets[j]+currentRule.antecedents[j]])*(InputValue[j]-InputCenters[Sets[j]+currentRule.antecedents[j]]))/(alpha[j]*alpha[j]));
		}
		if (degree > maxdegree) maxdegree = degree;
		for(k=0;k<OutDim;++k) {
			//estimatedValues[k] += (OutputCenters[OSets[k]+currentRule.consequents[k]]*degree);
			estimatedValues[k] += ((currentRule.consequents[k])*degree);
			degrees[k] += degree;
		}		
	}

	for(j=0;j<OutDim;++j) {
		if (degrees[j] > 0.0f)
			estimatedValues[j] /= degrees[j];
		else
			estimatedValues[j] = 0.0f;
		fprintf(fuzzy_log,"\nIl valore stimato per l'obiettivo %d è %f",j,estimatedValues[j]);
		fflush(fuzzy_log);

	}


	if (maxdegree < pow(0.5f,InDim)) return (false);
	return (true);
}

int CFuzzyFunctionApproximation::GetRules() {
	return (RuleTable->getRuleNumber());
}

void CFuzzyFunctionApproximation::Clean() {
  InDim = 0;
  OutDim = 0;
  prove=0;  
  count=0;
  threshold=0;
  calcola=false;
  delete InputSetsNumber;
  delete InputsMin;
  delete InputsMax;
  delete InputCenters;
  delete degrees;
  delete alpha;
  delete Sets;
  delete errore;
  for(int i=0; i<OutDim; ++i) {
      delete errmatrix[i];
  }
  delete errmatrix;
  delete stima;
  delete RuleTable;
}

CFuzzyFunctionApproximation::~CFuzzyFunctionApproximation()
{
	Clean();
	fclose(fuzzy_log);
};
