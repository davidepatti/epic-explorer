/*
 *  utility.c
 *  spea2
 *
 *  Created by Gianmarco De Francisci Morales [gmorales(at)diit.unict.it] on 04/12/07.
 *
 */

#include <stdlib.h>
#include "utility.h"

int irand(int range){
	int j;
	j=(int) ((double)range * (double) rand() / (RAND_MAX+1.0));
	return (j);
}

double drand(double range){
	double j;
	j=(range * (double) rand() / (RAND_MAX + 1.0));
	return (j);
}

