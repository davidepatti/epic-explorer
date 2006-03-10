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
  errmedio = 0;
  prove = 0;
  stima = 0;
  threshold = 0;
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


bool CFuzzyFunctionApproximation::StartUp(int N, REAL thres) {
	// Creates the Rule Table

  	//OutDim = 2; // Provvisorio

	threshold = thres;
		
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

	if (errmedio != NULL) delete errmedio;
	errmedio = new double[OutDim];
	if (errmedio == NULL) return (false);
	memset(errmedio,0,sizeof(double)*OutDim);

	if (stima != NULL) delete stima;
	stima = new REAL[OutDim];
	if (stima == NULL) return (false);

	calcola = false;

	prove = 0;

	return (true);
}

bool CFuzzyFunctionApproximation::Learn(REAL* InputValue, REAL* OutputValue) {

	int i,j;

	fprintf(stdout,"\n----------------------- Fuzzy System is Learning ------------------------------------\n");
	for (i=0;i<OutDim; i++)
	  cout << OutputValue[i] <<"\t";
	fprintf(stdout, "---- %u \n", prove);

	int rulen = RuleTable->getRuleNumber();
	REAL m = 1.0f, mm;
	memset(stima,0,sizeof(REAL)*OutDim);
	memset(newRule.antecedents,0,sizeof(int)*InDim);
	memset(newRule.consequents,0,sizeof(int)*OutDim);
	memset(newRule.degrees,0,sizeof(REAL)*OutDim);
	
	if (rulen > 0) EstimateG(InputValue,stima);
	if (rulen > 0  && stima[0] > 0.0f) {
		for(i=0;i<OutDim;++i) {
			errore[i] = fabs(OutputValue[i] - stima[i]);
			errmedio[i] += double(errore[i]/OutputValue[i]);
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
	if (prove < 2) return (false); //impone di farne almeno 445
	cout << "\n\n\n *************+  Prove = " << prove << endl;
	//REAL erro = 0.0f;
	//for(int i=0;i<OutDim;++i) {
	//	erro += (errmedio[i] * 100.0f )/ REAL(prove);
	//}
	//fprintf(stdout, "\n\n\n\n************ L'errore di stima e' : %f ************\n",erro);
	//if (erro > threshold) return (false);
	return (true);
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
#ifdef VERBOSE_ON
		fprintf(stdout,"\nIl valore stimato per l'obiettivo %d è %f",j,estimatedValues[j]);
		//wait_key();

#endif
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
  delete errmedio;
  delete stima;
  delete RuleTable;
}

CFuzzyFunctionApproximation::~CFuzzyFunctionApproximation()
{
	Clean();
};
