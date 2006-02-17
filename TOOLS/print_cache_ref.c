#include <stdio.h>
#include <stdlib.h>

#define N 100000
int main(int argc,char * argv[])
{
  unsigned long v[N];
  FILE *f;
  int i, n;

  if (argc!=2)
  {
  	printf("\n Usage : printf_cache_ref <binary ref file>\n");
	exit(1);
}

  f=fopen(argv[1], "r");
  n = fread(v, sizeof(unsigned long), N, f);
  for (i=1; i<n; i++)
    printf("%d  (step %d)\n", v[i] , v[i]-v[i-1]);

  fclose(f);

  return 0;
}
