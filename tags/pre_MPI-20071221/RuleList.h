#ifndef _RuleList_
#define _RuleList_
#include "common.h"
#include "memory.h"



class CRuleList
{
public:
	CRuleList(void) {};
	CRuleList(int MaxRules, int InDim, int OutDim);
	~CRuleList(void);

	int insertRule(Rule newRule);
	int searchRule(Rule newRule);
	Rule getConsequents(Rule leftPart);
	int getRuleNumber();
	Rule getRule(int ID);
	int getNextRuleID();
	int deleteWorstRule();

private:
	bool compare(int* left,int*right,int dim);
	int counter;
	int pos;
	int InDim;
	int OutDim;
	int MaxRules;
	Rule* RuleList;
};
#endif
