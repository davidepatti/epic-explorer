#include "Random.h"

unsigned long Seed = 0;//DEFAULT_SEED;
unsigned long originalSeed = 0;//DEFAULT_SEED;

double Rand() {return ((( (Seed * PRIME) & MASK) ) * SCALE ); }
int Randint(int low, int high) { return ((int) ((low) + ((high)-(low)+1) * Rand()));}
