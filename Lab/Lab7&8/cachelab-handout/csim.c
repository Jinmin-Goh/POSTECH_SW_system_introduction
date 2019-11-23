// Student name: 고진민
// Student ID: 20160074
// POVIS ID: eric9709

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "cachelab.h"

// define cache type
typedef struct _cachetype {
	int valid;
	long long int tag;
}cachetype;


int cacheCheck(cachetype** cache, int s, int E, int b, long long address);

int main(int argc, char *argv[])
{
	int hit_count = 0, miss_count = 0, eviction_count = 0,
		s, E, b, size, vflag = 0, parameter,
		cacheflag;
	long long address;
	char option, *tracefile;
	

	// parsing command line parameter
	extern char *optarg;
	extern int optind;
	while((parameter = getopt(argc, argv, "hvs:E:b:t:")) != -1)
	{
		switch(parameter)
		{
			case 'h':
			break;

			case 'v':
			vflag = 1;
			break;

			case 's':
			s = atoi(optarg);
			break;

			case 'E':
			E = atoi(optarg);
			break;

			case 'b':
			b = atoi(optarg);
			break;

			case 't':
			tracefile = optarg;
			break;
		}
	}
	
	// dataSet memory allocation & initialization
	cachetype **cache = (cachetype**) malloc (sizeof(cachetype) * (1 << s));
	for (int i = 0; i < (1 << s); i++)
		cache[i] = (cachetype*) malloc (sizeof(cachetype) * E);
	for (int i = 0; i < (1 << s); i++)
		for (int j = 0; j < E; j++)
			cache[i][j].valid = 0;

	// get valgrind memory trace data
	FILE *readfile;
	readfile = fopen(tracefile, "r");
	while(fscanf(readfile, "%c %llx,%d", &option, &address, &size) != EOF)
	{
		// cache check
		// M의 경우에 대해서 세심하게 진행할것 
		switch(option)
		{
			case 'I':
			break;	// ignore instruction cache access

			case 'L':
			case 'S':
			cacheflag = cacheCheck(cache, s, E, b, address);
			switch(cacheflag)
			{
				case 0:	// hit
				hit_count++;
				break;

				case 1: // miss
				miss_count++;
				break;

				case 2: // miss eviction
				miss_count++;
				eviction_count++;
				break;
			}
			break;

			case 'M':	// check first, second is always hit
			cacheflag = cacheCheck(cache, s, E, b, address);
			switch(cacheflag)
			{
				case 0:	// hit
				hit_count++;
				break;

				case 1: // miss
				miss_count++;
				break;

				case 2: // miss eviction
				miss_count++;
				eviction_count++;
				break;
			}
			hit_count++;
			break;
		}
		
		
		// in case of -v option
		if(vflag)
		{
			if(option == 'M')
			{
				switch(cacheflag)
				{
					case 0:	// hit
					printf("%c %llx, %d hit hit\n", option, address, size);
					break;
	
					case 1: // miss
					printf("%c %llx, %d miss hit\n", option, address, size);
					break;
	
					case 2: // miss eviction
					printf("%c %llx, %d miss eviction hit\n", option, address, size);
					break;
				}
			}
			else if((option == 'S') || (option == 'L'))
			{
				switch(cacheflag)
				{
					case 0:	// hit
					printf("%c %llx, %d hit\n", option, address, size);
					break;
	
					case 1: // miss
					printf("%c %llx, %d miss\n", option, address, size);
					break;
	
					case 2: // miss eviction
					printf("%c %llx, %d miss eviction\n", option, address, size);
					break;
				}
			}
		}
	}

	// free memory
	fclose(readfile);
	for (int i = 0; i < (1 << s); i++)
		free(cache[i]);
	free(cache);	

	// print summary
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

int cacheCheck(cachetype** cache, int s, int E, int b, long long address)
{
	int setbit, /*blockbit,*/ tagbit, addsize = 0, i, j, hitflag = 0, evictionflag = 1;
	cachetype cachetemp;
	long long int temp;

	// calculate instructions
	temp = address;
	while (temp)
	{
		temp = temp / 2;
		addsize++;
	}

	temp = address;
	for(i = 0; i < b; i++)
		temp = temp / 2;
	setbit = temp % (1 << s);

	temp = address;
	for(i = 0; i < (b + s); i++)
		temp = temp / 2;
	tagbit = temp % (1 << (addsize - b - s));

	// check cache
	for(i = 0; i < E; i++)
	{
		if(cache[setbit][i].valid && (tagbit == cache[setbit][i].tag))
		{
			hitflag++;
			break;
		}
	}

	// in case of hit
	if(hitflag)
	{
		cachetemp = cache[setbit][i];

		// shift the cache lines and move used line to first position
		// this is to check which is most recently used
		for(j = i - 1; j >= 0; j--)
			cache[setbit][j + 1] = cache[setbit][j];
		cache[setbit][0] = cachetemp;
		return 0;
	}

	// in case of miss
	else
	{
		for(j = 0; j < E; j++)
		{
			if (!cache[setbit][j].valid)
			{
				evictionflag--;
				break;
			}
		}

		// in case of eviction
		if(evictionflag)
		{
			// overlay the LRU data(delete) and insert new data
			for(int k = E-2; k >= 0; k--)
				cache[setbit][k + 1] = cache[setbit][k];

			cache[setbit][0].valid = 1;
			cache[setbit][0].tag = tagbit;
			return 2;
		}

		// shift old lines and insert new data
		for(int k = j - 1; k >= 0; k--)
			cache[setbit][k + 1] = cache[setbit][k];
		
		cache[setbit][0].valid = 1;
		cache[setbit][0].tag = tagbit;

		return 1;
	}
}