/*
 * randtrack_global_lock.cc
 *
 *  Created on: Nov 12, 2015
 *      Author: fangxuan
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "defs.h"
#include "hash.h"

#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

void *run_thread (void* tid);
pthread_mutex_t mutex;
//pthread_cond_t* cond;

/*
 * ECE454 Students:
 * Please fill in the following team struct
 */
team_t team = {
    "Eclipse",                  /* Team name */

	"Md Rafiur Rashid",                    /* First member full name */
    "998544240",                 /* First member student number */
	"rafiur.rashid@mail.utoronto.ca",                 /* First member email address */

    "Xuan (Benny) Fang",                           /* Second member full name */
    "999159887",                           /* Second member student number */
    "benny.fang@mail.utoronto.ca"                            /* Second member email address */
};

unsigned num_threads;
unsigned samples_to_skip;

class sample;

class sample {
  unsigned my_key;
 public:
  sample *next;
  unsigned count;

  sample(unsigned the_key){my_key = the_key; count = 0;};
  unsigned key(){return my_key;}
  void print(FILE *f){printf("%d %d\n",my_key,count);}
};

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".
hash<sample,unsigned> h;

int
main (int argc, char* argv[]){
  int i;

  // Print out team information
  printf( "Team Name: %s\n", team.team );
  printf( "\n" );
  printf( "Student 1 Name: %s\n", team.name1 );
  printf( "Student 1 Student Number: %s\n", team.number1 );
  printf( "Student 1 Email: %s\n", team.email1 );
  printf( "\n" );
  printf( "Student 2 Name: %s\n", team.name2 );
  printf( "Student 2 Student Number: %s\n", team.number2 );
  printf( "Student 2 Email: %s\n", team.email2 );
  printf( "\n" );

  // Parse program arguments
  if (argc != 3){
    printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
    exit(1);
  }
  sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
  sscanf(argv[2], " %d", &samples_to_skip);

  // initialize a 16K-entry (2**14) hash of empty lists
  h.setup(14);
  // prevent faulty input

  if (num_threads != 1 && num_threads != 2 && num_threads != 4) {
	  printf ("This program only allows 1,2 or 4 threads. Please try again.\n");
	  exit(0);
  }
  //else printf("Running %d threads\n", num_threads);
  // initialize mutex lock and allocate cv
  pthread_mutex_init(&mutex, NULL);
  /*
  cond = (pthread_cond_t*)malloc(num_threads * sizeof(pthread_cond_t))
  for (i=0; i<num_threads; i++){
	  //initialize cv variable
	  pthread_cond_init(&cond[i], NULL);
  }
  */
  // allocate and initialize thread and thread id
  pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
  int* tid = (int*)malloc(num_threads * sizeof(int));
  // create thread here, equal to number of thread specified
  for (i=0; i<num_threads; i++){
	  tid[i] = i;
	  //printf("Passing %d\n", tid[i]);
	  pthread_create(&(threads[i]), NULL, &run_thread, (void*) &tid[i]);
  }
  for (i=0; i<num_threads; i++){
		pthread_join(threads[i], NULL);
  }
  // process streams starting with different initial numbers

  // print a list of the frequency of all samples
  h.print();
  printf("If diff only shows this line the test for randtrack_global_lock.cc is successful!\n");
  return 0;
}

void* run_thread (void* tid) {
	int i,j,k;
	int slice, from, to, rnum;
	unsigned key;
	sample *s;
	from = (*((int*)tid) * NUM_SEED_STREAMS) / num_threads;
	to = from + NUM_SEED_STREAMS / num_threads;
	//printf("Running from %d to %d\n", from, to);
	for (i = from; i < to; i++) {
		rnum = i;
		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){
			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
				rnum = rand_r((unsigned int*)&rnum);
			}
			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;
			// lock here because we are inserting the data which is shared between threads
			pthread_mutex_lock(&mutex);
			// if this sample has not been counted before
			if (!(s = h.lookup(key))){
				// insert a new element for it into the hash table
				s = new sample(key);
				h.insert(s);
			}
			// increment the count for the sample
			s->count++;
			// unlock here after we are done
			pthread_mutex_unlock(&mutex);
		}
	}
	pthread_exit(NULL);
}


