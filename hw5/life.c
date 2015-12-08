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

// Define the number of threads to be 8
#define num_threads 8
#define row_num_slice 4
#define col_num_slice 2

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

	int row_slice = nrows / row_num_slice;
	int col_slice = ncols / col_num_slice;
	int row_start = tid % row_num_slice * row_slice;
	int row_end = row_start + row_slice;
	int col_start = tid / row_num_slice * col_slice;
	int col_end = col_start + col_slice;

	const int LDA = nrows;
	int curgen, i, j;
	int ii, jj;
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
	// Let T be either 32 or 16 smallest_slice, if smallest_slice is not large
	int smallest_slice = (row_slice < col_slice) ? row_slice : col_slice;
	int T = (smallest_slice < 32) ? smallest_slice : 32;

	char self, nw, n, ne, e, se, s, sw, w;
	char iself, inw, in, ine, ie, ise, is, isw, iw;

	for (curgen = 0; curgen < gens_max; curgen++)
	{
		for (i = col_start; i < col_end; i+=T)
		{
			// computing north and south at most outer loop will improve performance
        	const int north = mod (i-1, nrows);
        	const int south = mod (i+1, nrows);
			for (j = row_start; j < row_end; j+=T)
			{
	            for(jj = j; jj < j+T; jj++)
	            {
	            	//compute j here to speed up inner loop
	            	const int east = mod (jj+1, ncols);
	            	const int west = mod (jj-1, ncols);
	            	// Here we have two cases: when we first read the element of the board in this block vs any other time
	            	// If it is not our first time, we can essentially use 6/9 of the previous iteration
	            	// and avoid reading the board every time to reduce load overhead
	            	if (jj == j) {
	            		iself = self = BOARD (inboard, i, jj);
	            		inw = nw = BOARD (inboard, north, west);
	            		in = n = BOARD (inboard, north, jj);
	            		ine = ne = BOARD (inboard, north, east);
	            		iw = w = BOARD (inboard, i, west);
	            		ie = e = BOARD (inboard, i, east);
	            		isw = sw = BOARD (inboard, south, west);
	            		is = s = BOARD (inboard, south, jj);
	            		ise = se = BOARD (inboard, south, east);
	            	}
                	/*
                	 * ooo-	   <- we overwrite the 3 most west cells because we no longer need their value
                	 * ox*-	   <- x: old self, jj, *: new self, jj + 1
                	 * ooo-	   <- read these three new cell for this iteration
                	 */
	            	else {
	            		inw = nw = n;
	            		iw = w = self;
	            		isw = sw = s;
	            		in = n = ne;
	            		iself = self = e;
	            		is = s = se;
	            		ine = ne = BOARD (inboard, north, east);
	            		ie = e = BOARD (inboard, i, east);
	            		ise = se = BOARD (inboard, south, east);
	            	}
    				const char neighbor_count =
    				inw + in + ine + ie + ise + is + isw + iw;
    				BOARD(outboard, i, jj) = alivep (neighbor_count, iself);

    				// run the rest of the iteration, from i+1
	                for (ii = i + 1; ii < i+T; ii++)
	                {
	                	// similarly, for everytime we go down the column, we should be able to use 6/9 of the elements
	                	/*
	                	 * ooo	   <- we overwrite these north cells because we no longer need their value
	                	 * oxo	   <- x: old self, ii
	                	 * o*o	   <- *: new self, ii + 1
	                	 * ---     <- read these three new cell for this iteration
	                	 */
						const int isouth = mod (ii+1, nrows);
						inw = iw;
						in = iself;
						ine = ie;
						iw = isw;
						iself = is;
						ie = ise;
						isw = BOARD (inboard, isouth, west);
						is = BOARD (inboard, isouth, jj);
						ise = BOARD (inboard, isouth, east);
	    				const char neighbor_count =
	    				inw + in + ine + ie + ise + is + isw + iw;
	    				BOARD(outboard, ii, jj) = alivep (neighbor_count, iself);
	                }
	            }
			}
		}
		// using barrior function that was covered in lecture to synchronize
		barrier(arrived, mutex, cond, outboard, inboard);
		SWAP_BOARDS( outboard, inboard );
	}
	pthread_exit(NULL);
}

// function to synchronize the threads before doing another iteration
void barrier (int* arrived, pthread_mutex_t* mutex, pthread_cond_t* cond, char* outboard, char* inboard) {
	pthread_mutex_lock(mutex);
	(*arrived)++;
	if (*arrived < num_threads) {
		pthread_cond_wait(cond, mutex);
	}
	else {
		*arrived = 0; /* be prepared for next barrier */
		//SWAP_BOARDS( outboard, inboard );
		pthread_cond_broadcast(cond);
  	}
	pthread_mutex_unlock(mutex);
}
