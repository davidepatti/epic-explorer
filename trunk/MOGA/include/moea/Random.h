/* this random generator is copied from genesis by Grefenstette. */


#ifndef _RANDOM_H
#define _RANDOM_H

extern unsigned long Seed, originalSeed;

/* random number generator (copied from Grefensttete's genesis) */

#define MASK 2147483647
#define PRIME 65539
#define SCALE 0.4656612875e-9
#define DEFAULT_SEED  123456789
/****************************************************************/
/*	 Rand computes a psuedo-random				*/
/*	 double value between 0 and 1, excluding 1.  Randint	*/
/*	 gives an integer value between low and high inclusive.	*/
/****************************************************************/
#define Rand()  (( Seed = ( (Seed * PRIME) & MASK) ) * SCALE )
#define Randint(low,high) ( (int) ((low) + ((high)-(low)+1) * Rand()))



#endif
