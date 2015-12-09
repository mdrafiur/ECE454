#ifndef _life_h
#define _life_h

#include <pthread.h>

/**
 * Given the initial board state in inboard and the board dimensions
 * nrows by ncols, evolve the board state gens_max times by alternating
 * ("ping-ponging") between outboard and inboard.  Return a pointer to 
 * the final board; that pointer will be equal either to outboard or to
 * inboard (but you don't know which).
 */
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max);

/**
 * Same output as game_of_life() above if opmization is not defined, except this is not
 * parallelized.  Useful for checking output.
 */
char*
sequential_game_of_life (char* outboard, 
			 char* inboard,
			 const int nrows,
			 const int ncols,
			 const int gens_max);

// optimized version of the function defined here
char*
optimized_game_of_life (char* outboard,
		 char* inboard,
		 const int nrows,
		 const int ncols,
		 const int gens_max);

// function that each thread run to optimize game_of_life
void* thread_game_of_life (void* targs);

// function to synchronize the threads before doing another iteration
void barrier(int* arrived, pthread_mutex_t* mutex, pthread_cond_t* cond);

#endif /* _life_h */
