/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "TeamName",              /* Team name */

    "Md Rafiur Rashid",     /* First member full name */
    "rafiur.rashid@mail.utoronto.ca",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
        for (j = 0; j < dim; j++)
            dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * ECE 454 Students: Write your rotate functions here:
 */ 

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    attempt_six(dim, src, dst);
}


/* 
 * second attempt (commented out for now)
 */
char rotate_two_descr[] = "second attempt: Tiling with T = 8";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
    int i, j, i1, j1;
    int T = 8;
    
    for(i = 0; i < dim; i+=T)
        for(j = 0; j < dim; j+=T)
            for(i1 = i; i1 < i+T; i1++)
                for(j1 = j; j1 < j+T; j1++)
                    dst[RIDX(dim-1-j1, i1, dim)] = src[RIDX(i1, j1, dim)];
}

char rotate_three_descr[] = "third attempt: Tiling with T = 32";
void attempt_three(int dim, pixel *src, pixel *dst)
{
    int i, j, i1, j1;
    int T = 32;
    
    for(i = 0; i < dim; i+=T)
        for(j = 0; j < dim; j+=T)
            for(i1 = i; i1 < i+T; i1++)
                for(j1 = j; j1 < j+T; j1++)
                    dst[RIDX(dim-1-j1, i1, dim)] = src[RIDX(i1, j1, dim)];
}

char rotate_four_descr[] = "fourth attempt: Tiling with T = 8 & Loop Reordering";
void attempt_four(int dim, pixel *src, pixel *dst)
{
    int i, j, i1, j1;
    int T = 8;
    
    for(i = 0; i < dim; i+=T)
        for(j = 0; j < dim; j+=T)
            for(j1 = j; j1 < j+T; j1++)
                for(i1 = i; i1 < i+T; i1++)
                    dst[RIDX(dim-1-j1, i1, dim)] = src[RIDX(i1, j1, dim)];
}

char rotate_five_descr[] = "fifth attempt: Tiling with T = 16 & Loop Reordering";
void attempt_five(int dim, pixel *src, pixel *dst)
{
    int i, j, i1, j1;
    int T = 16;
    
    for(i = 0; i < dim; i+=T)
        for(j = 0; j < dim; j+=T)
            for(j1 = j; j1 < j+T; j1++)
                for(i1 = i; i1 < i+T; i1++)
                    dst[RIDX(dim-1-j1, i1, dim)] = src[RIDX(i1, j1, dim)];
}

char rotate_six_descr[] = "sixth attempt: Tiling with T = 32 & Loop Reordering";
void attempt_six(int dim, pixel *src, pixel *dst)
{    
    int i, j, i1, j1;
    int T = 32;
    
    for(i = 0; i < dim; i+=T)
        for(j = 0; j < dim; j+=T)
            for(j1 = j; j1 < j+T; j1++)
                for(i1 = i; i1 < i+T; i1++)
                    dst[RIDX(dim-1-j1, i1, dim)] = src[RIDX(i1, j1, dim)];
}

    /*********************************************************************
     * register_rotate_functions - Register all of your different versions
     *     of the rotate kernel with the driver by calling the
     *     add_rotate_function() for each test function. When you run the
     *     driver program, it will test and report the performance of each
     *     registered test function.  
     *********************************************************************/

    void register_rotate_functions() 
    {
        add_rotate_function(&naive_rotate, naive_rotate_descr);   
        add_rotate_function(&rotate, rotate_descr);   

        add_rotate_function(&attempt_two, rotate_two_descr);   
        add_rotate_function(&attempt_three, rotate_three_descr);   
        add_rotate_function(&attempt_four, rotate_four_descr);   
        add_rotate_function(&attempt_five, rotate_five_descr);   
        add_rotate_function(&attempt_six, rotate_six_descr);   
        //add_rotate_function(&attempt_seven, rotate_seven_descr);   
        //add_rotate_function(&attempt_eight, rotate_eight_descr);   
        //add_rotate_function(&attempt_nine, rotate_nine_descr);   
        //add_rotate_function(&attempt_ten, rotate_ten_descr);   
        //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

        /* ... Register additional rotate functions here */
    }

