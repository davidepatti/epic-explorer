#include "FuzzyApprox.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#define INS 18
#define OUTS 2
#define PATTERN 2250

CFuzzyFunctionApproximation FuzzyApprox;

int main() {
	int InSets[INS] = {5,5,5,5,	3,3,3,3,3,	7,7,7,	7,7,7,	7,7,7};
	//int InSets[INS] = {5,5,5,5,	3,3,3,3,3,	5,5,5,	5,5,5,	5,5,5};
	//int InSets[INS] = {2,2,2,2,	2,2,2,2,2,	2,2,2,	2,2,2,	2,2,2};
	int OutSets[OUTS] = {9,9};
	REAL InMin[INS] = {1,	1,	1,	2,	 16,	 16,     32,	 32,	 8,	  1024,	 32,	1,	  1024,	 32,	1,	 32768,	 64,	 2};
	REAL InMax[INS] = {6,	6,	4,	4,	128,	128,	128,	128,	16,	131072,	128,	4,	131072,	128,	4,	524288,	256,	16};
	//REAL InMax[INS] = {6,	4,	3,	3,	 64,	 64,	128,	 64,	16,	 65536,	 64,	16,	131072,	 64,	16,	524288,	128,	16};
	//REAL InMin[INS] = {1,	1,	1,	1,	 16,	  8,	 32,	  8,	 8,	   128,	  8,     1,	   128,	  8,	 1,	  8192,	  8,  	1};
	int OutTypes[OUTS] = {1,1};
	REAL OutMin[OUTS] = {0.0f,0.0f};
	REAL OutMax[OUTS] = {10.0f,100.0f};
	REAL *Inputs = new REAL[INS];
	REAL *Outputs = new REAL[OUTS];
	REAL errore = 0.0f;
	REAL errore2 = 0.0f;
	REAL *stima = new REAL[OUTS];

	FuzzyApprox.GenerateInputFuzzySets(INS,InSets,InMin,InMax);

	FuzzyApprox.StartUp(10000,3.0f);	

	srand((unsigned)time(NULL));
	REAL errmedio = 0.0f;
	REAL errmedio2 = 0.0f;
	REAL maxserr = 0.0f;
	REAL maxserr2 = 0.0f;
	int nonstimati = 0,k=0;

	FILE* fin = fopen("DCT-unstable.txt","r");
	//fin = fopen("RWun-pareto.txt","r");

	for(int i=0; i<PATTERN; ++i) {
		//if (feof(fin)) fin = fopen("rawd-stable.txt","r");
		for(int j=0;j<OUTS;++j) {
			fscanf(fin,"%f",&(Outputs[j]));
		}
		for(int j=0;j<INS;++j) {
			fscanf(fin,"%f",&(Inputs[j]));
		}
		FuzzyApprox.Learn(Inputs,Outputs);
		//printf("%d -> %.4f - %.4f = %.2f \n",i,Outputs[0],stima[0],(errore/Outputs[0])*100.0f);
		if (FuzzyApprox.Reliable()) break;
	}
	printf("Numero di regole = %d\n",FuzzyApprox.GetRules());

	FILE *f = fopen("Risultati.txt","w");

	errore = errore2 = errmedio = errmedio2 = 0.0f;

	printf("Completata la fase di learning con %d pattern\n",i);

	int test = PATTERN - i;

	printf("\n\nIniziata la fase di test con %d pattern\n",test);

	for(int i=0; i<test; ++i) {
		for(int j=0;j<OUTS;++j) {
			fscanf(fin,"%f",&(Outputs[j]));
		}
		for(int j=0;j<INS;++j) {
			fscanf(fin,"%f",&(Inputs[j]));
		}
		if (!FuzzyApprox.EstimateG(Inputs,stima)) {
			if (stima[0] == 0.0f) {
				nonstimati++;
				FuzzyApprox.Learn(Inputs,Outputs);
				for(int j=0;j<OUTS;++j) stima[j] = Outputs[j];
			}
		} 
		//else
		//	FuzzyApprox.EstimateGC(Inputs,stima);
		errore = fabs(Outputs[0] - stima[0]);
		errore2 = fabs(Outputs[1] - stima[1]);
		errmedio += (errore/Outputs[0])*100.0f;	
		errmedio2 += (errore2/Outputs[1])*100.0f;
		fprintf(f,"%.4f\t%.4f\t%.4f\t%.4f\n",Outputs[0],stima[0],Outputs[1],stima[1]);
		if ((errore/Outputs[0]) > maxserr) {
			maxserr = errore/Outputs[0];
		}
		if ((errore2/Outputs[1]) > maxserr2) {
			maxserr2 = (errore2/Outputs[1]);
		}
		if (feof(fin)) break;
	}
	
	errmedio /= i;
	errmedio2 /= i;
	printf("Numero di casi non stimati = %d - %.2f percento\n",nonstimati,(REAL(nonstimati)*100.0f)/test);
	printf("Errore massimo percentuale = %.2f \n",maxserr*100.0f);
	printf("Errore medio percentuale= %.2f \n",errmedio);
	printf("Errore massimo percentuale = %.2f \n",maxserr2*100.0f);
	printf("Errore medio percentuale= %.2f \n",errmedio2);

	fclose(f);
	f = fopen("risultati_globali.txt","w");
	maxserr = maxserr2 = errore = errore2 = errmedio = errmedio2 = 0.0f;
	fseek(fin,0,SEEK_SET);

	for(int i=0; i<PATTERN; ++i) {
		for(int j=0;j<OUTS;++j) {
			fscanf(fin,"%f",&(Outputs[j]));
		}
		for(int j=0;j<INS;++j) {
			fscanf(fin,"%f",&(Inputs[j]));
		}
		FuzzyApprox.EstimateG(Inputs,stima);
		if (stima[0] == 0.0f) {
			printf("No good!");
		} else {
			errore = fabs(Outputs[0] - stima[0]);
			errore2 = fabs(Outputs[1] - stima[1]);
			errmedio += (errore/Outputs[0])*100.0f;	
			errmedio2 += (errore2/Outputs[1])*100.0f;
		}
		if ((errore/Outputs[0]) > maxserr) {
			maxserr = errore/Outputs[0];
		}
		if ((errore2/Outputs[1]) > maxserr2) {
			maxserr2 = (errore2/Outputs[1]);
		}
		fprintf(f,"%.4f\t%.4f\t%.4f\t%.4f\n",Outputs[0],stima[0],Outputs[1],stima[1]);
		fflush(f);
		if (feof(fin)) break;
	}
	fclose(f);
	fclose(fin);
	errmedio /= i;
	errmedio2 /= i;
	printf("Numero di regole = %d\n",FuzzyApprox.GetRules());
	printf("Errore massimo percentuale = %.2f \n",maxserr*100.0f);
	printf("Errore medio percentuale= %.2f \n",errmedio);
	printf("Errore massimo percentuale = %.2f \n",maxserr2*100.0f);
	printf("Errore medio percentuale= %.2f \n",errmedio2);

	maxserr = maxserr2 = errore = errore2 = errmedio = errmedio2 = 0.0f;
	i=0;
	f = fopen("pareto.txt","w");
	fin = fopen("RWun-pareto.txt","r");
	while (!feof(fin)) {
		for(int j=0;j<OUTS;++j) {
			fscanf(fin,"%f",&(Outputs[j]));
		}
		for(int j=0;j<INS;++j) {
			fscanf(fin,"%f",&(Inputs[j]));
		}
		FuzzyApprox.EstimateG(Inputs,stima);
		errore = fabs(Outputs[0] - stima[0]);
		errore2 = fabs(Outputs[1] - stima[1]);
		errmedio += (errore/Outputs[0])*100.0f;	
		errmedio2 += (errore2/Outputs[1])*100.0f;
		if ((errore/Outputs[0]) > maxserr) {
			maxserr = errore/Outputs[0];
		}
		if ((errore2/Outputs[1]) > maxserr2) {
			maxserr2 = (errore2/Outputs[1]);
		}
		fprintf(f,"%.4f\t%.4f\t%.4f\t%.4f\n",Outputs[0],stima[0],Outputs[1],stima[1]);
		fflush(f);
		i++;
	}
	errmedio /= i;
	errmedio2 /= i;
	printf("Numero di regole = %d\n",FuzzyApprox.GetRules());
	printf("Errore massimo percentuale = %.2f \n",maxserr*100.0f);
	printf("Errore medio percentuale= %.2f \n",errmedio);
	printf("Errore massimo percentuale = %.2f \n",maxserr2*100.0f);
	printf("Errore medio percentuale= %.2f \n",errmedio2);
	fclose(f);
	fclose(fin);

  unsigned stop2=0;
  scanf("%u",stop2);

};