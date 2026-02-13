/**
 * \file
 *         Analysis of clock skew compensation (CSC) algorithms
 * \author
 *         Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 * \note
 *         Optimization options controlled by macro definition:
 *         - CSC_DS_OPT1 // turn off iteration couting in DS
 *         - CSC_DS_OPT2 // enable branchless programming in DS
 *         - CSC_DIV_OPT // turn off checking the value of A in division algos
 */

/* #include <inttypes.h> */
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define AD_SIZE 4 // size of A and D in bytes: 4 for uint32_t; 8 for uint64_t
#if AD_SIZE == 8
#define NO_LLABS // llabs() is missing on MSP430 platforms
#endif

#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x) : (x))
// #ifdef NO_LLABS
// inline long long llabs(long long n) {
//     return (n < 0) ? -n : n;
// }
// #endif
#endif

// CSC based on double-precision FP division (reference algo.)
#if AD_SIZE == 8
int64_t csc_dp_div(int64_t i, int64_t D, int64_t A, int *p_num_iter)
#elif AD_SIZE == 4
int32_t csc_dp_div(int32_t i, int32_t D, int32_t A, int *p_num_iter)
#endif
{
    *p_num_iter = 1;
#ifndef CSC_DIV_OPT
    if (A == 0) {
        return 0;
    }
#endif
#if AD_SIZE == 8
    return (int64_t) floor((i * (double) D / (double) A) + 0.5);
#elif AD_SIZE == 4
    return (int32_t) floor((i * (double) D / (double) A) + 0.5);
#endif
}

// CSC based on single-precision FP division
#if AD_SIZE == 8
int64_t csc_sp_div(int64_t i, int64_t D, int64_t A, int *p_num_iter)
#elif AD_SIZE == 4
int32_t csc_sp_div(int32_t i, int32_t D, int32_t A, int *p_num_iter)
#endif
{
    *p_num_iter = 1;
#ifndef CSC_DIV_OPT
    if (A == 0) {
        return 0;
    }
#endif
#if AD_SIZE == 8
    return (int64_t) floor((i * (float) D / (float) A) + 0.5);
#elif AD_SIZE == 4
    return (int32_t) floor((i * (float) D / (float) A) + 0.5);
#endif
}

// CSC based on the direct search algorithm (i.e., arXiv:2504.15039)
#if AD_SIZE == 8
int64_t csc_ds(int64_t i, int64_t D, int64_t A, int *p_num_iter)
#elif AD_SIZE == 4
int32_t csc_ds(int32_t i, int32_t D, int32_t A, int *p_num_iter)
#endif
{
#if AD_SIZE == 8
    int64_t j = 0;
    int64_t k = floor(i*(float)D/(float)A + 0.5); // y coordinate
    int64_t td = (k - i)*A + i*(A - D); // triangle down for 32-bit integers
#elif AD_SIZE == 4
    int32_t j = 0;
    int32_t k = floor(i*(float)D/(float)A + 0.5); // y coordinate
    int32_t td = (k - i)*A + i*(A - D); // triangle down for 32-bit integers
#endif
    // assert(td == k*A - i*D); // for debugging

#ifdef CSC_DS_OPT1    
    *p_num_iter = 1;
#else
    *p_num_iter = 0;
#endif
    if (td == 0) {
        j = k;
#ifndef CSC_DS_OPT1
        (*p_num_iter)++;
#endif
    }
    else if (td > 0) {
        while (true) {
            if (k == 0) {
                j = 0;
#ifndef CSC_DS_OPT1
                (*p_num_iter)++;
#endif
                break;
            }
            else {
                if (td - A == 0) {
                    j = k - 1;
#ifndef CSC_DS_OPT1
                    (*p_num_iter)++;
#endif
                    break;
                }
                else if (td - A > 0) {
                    k--;
                    td -= A;
#ifndef CSC_DS_OPT1
                    (*p_num_iter)++;
#endif
                }
                else {
#ifdef CSC_DS_OPT2
                    j = k - (ABS(td - A) < ABS(td));
// #if AD_SIZE == 8
//                     j = k - (llabs(td - A) < llabs(td));
// #elif AD_SIZE == 4
//                     j = k - (labs(td - A) < labs(td));
// #endif
#else
                    if (ABS(td - A) < ABS(td)) {
// #if AD_SIZE == 8
//                     if (llabs(td - A) < llabs(td)) {
// #elif AD_SIZE == 4
//                     if (labs(td - A) < labs(td)) {
// #endif
                        j = k - 1;
                    }
                    else {
                        j = k;
                    }
#endif
#ifndef CSC_DS_OPT1
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
#ifndef CSC_DS_OPT1
                (*p_num_iter)++;
#endif
                break;
            }
            else if (td + A > 0) {
#ifdef CSC_DS_OPT2
                j = k + (ABS(td + A) < ABS(td));
                // j = k + (llabs(td + A) < llabs(td));
#else
                if (ABS(td + A) < ABS(td)) {
                // if (llabs(td + A) < llabs(td)) {
                    j = k + 1;
                }
                else {
                    j = k;
                }
#endif
#ifndef CSC_DS_OPT1
                (*p_num_iter)++;
#endif
                break;
            }
            else {
                k++;
                td += A;
#ifndef CSC_DS_OPT1
                (*p_num_iter)++;
#endif
            }
        }
    }
    return j;
}
