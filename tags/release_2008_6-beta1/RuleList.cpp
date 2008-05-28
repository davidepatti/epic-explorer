#include "RuleList.h"

CRuleList::CRuleList(int maxrules, int indim, int outdim)
{
	MaxRules = maxrules;
	InDim = indim;
	OutDim = outdim;
	RuleList = new Rule[maxrules];
	if (RuleList == NULL) ; //??
	counter = 0;
	pos = 0;
}

CRuleList::~CRuleList(void)
{
	//delete RuleList;
}

int CRuleList::insertRule(Rule newRule) {
	
	if (counter > 0 && ((pos = searchRule(newRule)) > -1)) {
		for(int j=0; j<OutDim; ++j) {
				if (newRule.degrees[j] > getRule(pos).degrees[j]) {
					//getRule(pos).consequents[j] = (newRule.consequents[j]+getRule(pos).consequents[j])/2;
					getRule(pos).consequents[j] = newRule.consequents[j];
					getRule(pos).degrees[j] = newRule.degrees[j];
				}
		}
		return (pos);
	}

	if (counter < MaxRules-1) {
		pos = (counter++);
		RuleList[pos].antecedents = new int[InDim];
		if (RuleList[pos].antecedents == NULL) return (-1);
		RuleList[pos].consequents = new double[OutDim];
		if (RuleList[pos].consequents == NULL) return (-1);
		RuleList[pos].degrees = new double[OutDim];
		if (RuleList[pos].degrees == NULL) return (-1);
	}
	else
		pos = deleteWorstRule();
	
	memcpy(RuleList[pos].antecedents,newRule.antecedents,sizeof(int)*InDim);
	memcpy(RuleList[pos].consequents,newRule.consequents,sizeof(double)*OutDim);
	memcpy(RuleList[pos].degrees,newRule.degrees,sizeof(double)*OutDim);
		
	return (pos);
};
int CRuleList::searchRule(Rule newRule){	
	for(int i=0;i<counter;++i) {
		if (memcmp(newRule.antecedents,getRule(i).antecedents,sizeof(int)*InDim) == 0) 
			return (i);
	}
	return (-1);
};
Rule CRuleList::getConsequents(Rule leftPart){
	for(int i=0;i<counter;++i) {
		if (memcmp(leftPart.antecedents,getRule(i).antecedents,sizeof(int)*InDim) == 0) 
			return (getRule(i));
	}
	return (leftPart);
};
int CRuleList::getRuleNumber(){
	return (counter);
};
Rule CRuleList::getRule(int ID){
	return (RuleList[ID]);
};
int CRuleList::getNextRuleID(){
	return (pos+1);
};
int CRuleList::deleteWorstRule() {
	double degree = 1.0f,min = 1.0f;
	for(int i=0;i<counter;++i) {
		degree = 1.0f;
		for (int j=0; j<OutDim; ++j) 
			degree *= getRule(i).degrees[j];
		if (degree < min) {
			pos = i;
			min = degree;
		}
	}
	return (pos);
}

