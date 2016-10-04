//#include "stdafx.h"
//#include <tchar.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#define ARRAY_MIN (1024) /* 1/4 smallest cache */
#define ARRAY_MAX (4096*4096) /* 1/4 largest cache */

using namespace std;

int x[ARRAY_MAX]; /* array going to stride through */
bool check_x[ARRAY_MAX];

/*double get_seconds() 
	{ 
	__time64_t ltime;
	_time64( &ltime );
	return (double) ltime;
	}*/
void clear_check_x()
	{
	int i;
	for(i = 0; i < ARRAY_MAX; ++i)
		{
		check_x[i] = false;
		}
	return;
	}

double gettime() 
    {
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec+t.tv_usec*1e-6;
    }

int label(int i) 
	{/* generate text labels */
	if (i<1e3) 
		printf("%1dB,",i);
	else if (i<1e6) 
		printf("%1dK,",i/1024);
	else if (i<1e9) 
		printf("%1dM,",i/1048576);
	else 
		printf("%1dG,",i/1073741824);
	return 0;
	}

int main(int argc, char* argv[]) 
	{
	int register nextstep, i, index, stride;
	int csize;
	unsigned long int temp_rand;
	double steps, tsteps;
	double loadtime, lastsec, sec0, sec1, sec; /* timing variables */
	srand( time(NULL) );
	/* Initialize output */
	printf(" ,");
	for (stride=1; stride <= ARRAY_MAX/2; stride=stride*2)
		label(stride*sizeof(int));
	printf("\n");
	
	/* Main loop for each configuration */
	for (csize=ARRAY_MIN; csize <= ARRAY_MAX; csize=csize*2) 
		{
		label(csize*sizeof(int)); /* print cache size this loop */
		for (stride=1; stride <= csize/2; stride=stride*2) 
			{
			clear_check_x();
			/* Lay out path of memory references in array */
			for (index=0; index < csize; index=index+stride)
				{
				temp_rand = stride * rand();
				while( check_x[temp_rand % csize] == true)
					{
					temp_rand = temp_rand + stride;
					}
				x[index] = temp_rand % csize; /* pointer to next */
				check_x[ x[index] ] = true;
				//cout << index << ": " << x[index] << endl;
				}
			// x[index-stride] = 0; /* loop back to beginning */
			
			/* Wait for timer to roll over */
			lastsec = gettime();
			do sec0 = gettime(); while (sec0 == lastsec);
			
			/* Walk through path in array for twenty seconds */
			/* This gives 5% accuracy with second resolution */
			steps = 0.0; /* number of steps taken */
			nextstep = 0; /* start at beginning of path */
			sec0 = gettime(); /* start timer */
			do { /* repeat until collect 20 seconds */
				/*for (i=stride;i!=0;i=i-1) 
					{ 
					nextstep = 0;
					do nextstep = x[nextstep];
					while (nextstep != 0);
					}*/
				i = 0;
				index = 0;
				while(i < csize)
					{
					index = x[index];
					++i;
					}
				steps = steps + 1.0; /* count loop iterations */
				sec1 = gettime(); /* end timer */
				} while ((sec1 - sec0) < 0.1); /* collect 20 seconds */

			sec = sec1 - sec0;
			/* Repeat empty loop to loop subtract overhead */
			tsteps = 0.0; /* used to match no. while iterations */
			sec0 = gettime(); /* start timer */
			do { /* repeat until same no. iterations as above */
				/*for (i=stride;i!=0;i=i-1) 
					{ 
					index = 0;
					do index = index + stride;
					while (index < csize);
					}*/
				i = 0;
				index = 0;
				while(i < csize)
					{
					++index;
					++i;
					}
				tsteps = tsteps + 1.0;
				sec1 = gettime(); /* - overhead */
				} while (tsteps<steps); /* until = no. iterations */
			
			sec = sec - (sec1 - sec0);
			loadtime = (sec*1e9)/(steps*csize);
			/* write out results in .csv format for Excel */
			printf("%4.1f,", (loadtime<0.1) ? 0.1 : loadtime);
			}; /* end of inner for loop */
		printf("\n");
		}; /* end of outer for loop */
	return 0;
	}