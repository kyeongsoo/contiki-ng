/**
 * \file
 *         Analysis of clock skew compensation (CSC) algorithms
 * \author
 *         Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 */

/* #include <inttypes.h> */
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NO_LLABS // llabs() is missing on MSP430 platforms

/* // optimization level */
/* #define _OPT1 // turn off iteration couting in DS */
/* #define _OPT2 // enable branchless programming in DS */
/* #define _OPT3 // turn off checking the value of A in division algos */

#ifdef NO_LLABS
long long llabs(long long n) {
    return (n < 0) ? -n : n;
}
#endif

// CSC based on double-precision FP division (reference algo.)
int64_t csc_dp_div(int64_t i, int64_t D, int64_t A, int *p_num_iter)
{
    *p_num_iter = 1;
#ifndef _OPT3
    if (A == 0) {
        return 0;
    }
#endif
    return (int64_t) floor((i * (double) D / (double) A) + 0.5);
}

// CSC based on single-precision FP division
int64_t csc_sp_div(int64_t i, int64_t D, int64_t A, int *p_num_iter)
{
    *p_num_iter = 1;
#ifndef _OPT3
    if (A == 0) {
        return 0;
    }
#endif
    return (int64_t) floor((i * (float) D / (float) A) + 0.5);
}

// CSC based on the direct search algorithm (i.e., arXiv:2504.15039)
int64_t csc_ds(int64_t i, int64_t D, int64_t A, int *p_num_iter)
{
    int64_t j = 0;
    int64_t k = floor(i*(float)D/(float)A + 0.5); // y coordinate
    int64_t td = (k - i)*A + i*(A - D); // triangle down for 32-bit integers
    // assert(td == k*A - i*D); // for debugging

#ifdef _OPT1    
    *p_num_iter = 1;
#else
    *p_num_iter = 0;
#endif
    if (td == 0) {
        j = k;
#ifndef _OPT1
        (*p_num_iter)++;
#endif
    }
    else if (td > 0) {
        while (true) {
            if (k == 0) {
                j = 0;
#ifndef _OPT1
                (*p_num_iter)++;
#endif
                break;
            }
            else {
                if (td - A == 0) {
                    j = k - 1;
#ifndef _OPT1
                    (*p_num_iter)++;
#endif
                    break;
                }
                else if (td - A > 0) {
                    k--;
                    td -= A;
#ifndef _OPT1
                    (*p_num_iter)++;
#endif
                }
                else {
#ifdef _OPT2
                    j = k - (llabs(td - A) < llabs(td));
#else
                    if (llabs(td - A) < llabs(td)) {
                        j = k - 1;
                    }
                    else {
                        j = k;
                    }
#endif
#ifndef _OPT1
                    (*p_num_iter)++;
#endif
                    break;
                }
            }
        }
    }
    else { // td < 0
        while (true) {
            if (td + A == 0) {
                j = k + 1;
#ifndef _OPT1
                (*p_num_iter)++;
#endif
                break;
            }
            else if (td + A > 0) {
#ifdef _OPT2
                j = k + (llabs(td + A) < llabs(td));
#else
                if (llabs(td + A) < llabs(td)) {
                    j = k + 1;
                }
                else {
                    j = k;
                }
#endif
#ifndef _OPT1
                (*p_num_iter)++;
#endif
                break;
            }
            else {
                k++;
                td += A;
#ifndef _OPT1
                (*p_num_iter)++;
#endif
            }
        }
    }
    return j;
}
