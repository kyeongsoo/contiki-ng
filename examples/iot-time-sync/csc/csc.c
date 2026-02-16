/**
 * \brief Analysis of clock skew compensation (CSC) algorithms.
 * 
 * \author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 * 
 * \note The following options are controlled by macro definitions:
 * - CSC_INT_SIZE: The number of bytes for 'i', 'D', and 'A' (4 or 8).
 * - CSC_DS_OPT1: Turn off iteration couting in DS.
 * - CSC_DS_OPT2: Enable branchless programming in DS.
 * - CSC_DIV_OPT: Turn off checking the value of 'A' in division algorithms.
 */

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "csc.h"

// type-independent implementation
#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x) : (x))
#endif

/**
 * \brief CSC based on double-precision FP division.
 */
csc_int_t csc_dp_div(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint16_t *p_num_iter)
{
    *p_num_iter = 1;
#ifndef CSC_DIV_OPT
    if (A == 0) {
        return 0;
    }
#endif
    return (csc_int_t) floor((i * (double)D / (double)A) + 0.5);
}

/**
 * \brief CSC based on single-precision FP division.
 */
csc_int_t csc_sp_div(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint16_t *p_num_iter)
{
    *p_num_iter = 1;
#ifndef CSC_DIV_OPT
    if (A == 0) {
        return 0;
    }
#endif
    return (csc_int_t) floor((i * (float)D / (float)A) + 0.5);
}

/**
 * \brief CSC based on the "direct search" algorithm.
 * 
 * \remarks For details, refer to the following paper:
 * - K. S. Kim, "Direct search algorithm for clock skew compensation immune to floating-point precision loss,"
 *   arXiv:2504.15039 [cs.NI], Apr. 2025. [Online]. Available: https://arxiv.org/abs/2504.15039
 */
csc_int_t csc_ds(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint16_t *p_num_iter)
{
    csc_int_t j = 0;
    csc_int_t k = floor(i*(float)D/(float)A + 0.5); // a starting point
    csc_int_t td = (k - i)*A + i*(A - D); // "triangle down" to avoid overflow
    assert(td == k*A - i*D); // for debugging

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
#else
                    if (ABS(td - A) < ABS(td)) {
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
        } // end of while loop
    } // td > 0
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
#else
                if (ABS(td + A) < ABS(td)) {
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
        } // end of while loop
    } // td < 0
    return (csc_int_t) j;
}
