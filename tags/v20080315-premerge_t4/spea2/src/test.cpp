#include <iostream>
#include "containers.h"

using namespace std;

int individual::uid = 0;

int main(char *argc, char **argv) {
	individual ind1(1,1);
	individual ind2(1,1);
	individual ind3(1,1);
	cout << ind1.index << endl << ind2.index << endl << ind3.index << endl;
	population pop;
	pop.push_back(ind1);
	pop.push_back(ind2);
	pop.push_back(ind3);
	for (int i=0; i<3; i++)
		cout << pop.elements.at(i).index << endl;
}