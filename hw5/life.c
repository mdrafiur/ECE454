/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define OPTIMIZATION
/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/
/*
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

// Determine value given board and its coordinate
#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

// Define the number of threads to be 4
#define num_threads 4

// Define a structure of variables to be passed into the threads as arguments
typedef struct {
	int tid;
	char* outboard;
	char* inboard;
	int nrows;
	int ncols;
	int gens_max;
	int* arrived;
	pthread_mutex_t* mutex;
	pthread_cond_t* cond;
} thread_args;
/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
	#ifdef OPTIMIZATION
		if (nrows < 32) return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);
		else return optimized_game_of_life (outboard, inboard, nrows, ncols, gens_max);
	#else
		return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);
	#endif
}

char*
optimized_game_of_life (char* outboard,
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
	int i;
	// allocate threads and their arguments
	//
	pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
	thread_args** args = (thread_args**)malloc(num_threads * sizeof(thread_args*));

	//setup locks and cv for synchronization
	pthread_mutex_t* mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_cond_t* cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	pthread_mutex_init(mutex, NULL);
	pthread_cond_init(cond, NULL);

	//setup arrive variable for barrier to block threads for synchronization
	int* arrived = (int*)malloc(sizeof(int));
	*arrived = 0;

	// initialize threads arguments and create threads with pthread_create
	for (i = 0; i < num_threads; i++){
		args[i] = (thread_args*)malloc(sizeof(thread_args));
		(args[i])->tid = i;
		(args[i])->outboard = outboard;
		(args[i])->inboard = inboard;
		(args[i])->nrows = nrows;
		(args[i])->ncols = ncols;
		(args[i])->gens_max = gens_max;
		(args[i])->arrived = arrived;
		(args[i])->mutex = mutex;
		(args[i])->cond = cond;
		//printf("Passing %d\n", tid[i]);
		pthread_create(&(threads[i]), NULL, &thread_game_of_life, (void*) args[i]);
	}
	// do thread_join in the end
	for (i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}
	// return finished board
	return inboard;

	/*
	const int LDA = nrows;
	int curgen, i, j;

	for (curgen = 0; curgen < gens_max; curgen++)
	{
		for (i = 0; i < nrows; i++)
		{
			for (j = 0; j < ncols; j++)
			{
			const int inorth = mod (i-1, nrows);
			const int isouth = mod (i+1, nrows);
			const int jwest = mod (j-1, ncols);
			const int jeast = mod (j+1, ncols);

			const char neighbor_count =
			BOARD (inboard, inorth, jwest) +
			BOARD (inboard, inorth, j) +
			BOARD (inboard, inorth, jeast) +
			BOARD (inboard, i, jwest) +
			BOARD (inboard, i, jeast) +
			BOARD (inboard, isouth, jwest) +
			BOARD (inboard, isouth, j) +
			BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));
			}
		}
		SWAP_BOARDS( outboard, inboard );
	}
	*/
}

void* thread_game_of_life (void* targs)
{
	thread_args* args = (thread_args*) targs;
	int tid = args->tid;
	char* outboard = args->outboard;
	char* inboard = args->inboard;
	const int nrows = args->nrows;
	const int ncols = args->ncols;
	const int gens_max = args->gens_max;
	int* arrived = args->arrived;
	pthread_mutex_t* mutex = args->mutex;
	pthread_cond_t* cond = args->cond;

	int slice = nrows / (num_threads / 2);
	int row_start = tid % (num_threads / 2) * slice;
	int row_end = row_start + slice;
	int col_start = tid / (num_threads / 2) * slice;
	int col_end = col_start + slice;

	const int LDA = nrows;
	int curgen, i, j;
	//int ii, jj;
	/*
	 * Here we will take advantage of lab 2 optimization as shown below

    T = 32;
    for(i = 0; i < dim; i+=T)
        for(j = 0; j < dim; j+=T)
            for(jj = j; jj < j+T; jj++)
                for(ii = i; ii < i+T; ii++)
                	...
     *
     */
	// Let T be either 32 or slice, if slice is smaller
	//int T = (slice < 32) ? slice : 32;

	for (curgen = 0; curgen < gens_max; curgen++)
	{
		for (i = row_start; i < row_end; i++)
		{
			for (j = col_start; j < col_end; j++)
			{
			const int inorth = mod (i-1, nrows);
			const int isouth = mod (i+1, nrows);
			const int jwest = mod (j-1, ncols);
			const int jeast = mod (j+1, ncols);

			const char neighbor_count =
			BOARD (inboard, inorth, jwest) +
			BOARD (inboard, inorth, j) +
			BOARD (inboard, inorth, jeast) +
			BOARD (inboard, i, jwest) +
			BOARD (inboard, i, jeast) +
			BOARD (inboard, isouth, jwest) +
			BOARD (inboard, isouth, j) +
			BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));
			}
		}
		barrier(arrived, mutex, cond);
		SWAP_BOARDS( outboard, inboard );
	}
	pthread_exit(NULL);
}

void barrier (int* arrived, pthread_mutex_t* mutex, pthread_cond_t* cond) {
	pthread_mutex_lock(mutex);
	(*arrived)++;
	if (*arrived < num_threads) {
		pthread_cond_wait(cond, mutex);
	}
	else {
		pthread_cond_broadcast(cond);
    	*arrived = 0; /* be prepared for next barrier */
  	}
	pthread_mutex_unlock(mutex);
}
