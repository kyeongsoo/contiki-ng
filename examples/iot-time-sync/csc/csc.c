/**
 * \brief Analysis of clock skew compensation (CSC) algorithms.
 * 
 * \author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 * 
 * \note The following options are controlled by macro definitions:
 * - CSC_INT_SIZE: The number of bytes for 'i', 'D', and 'A' (4 or 8).
 * - CSC_NO_ITER_COUNT: Turn off iteration couting in iterative algorithms.
 * - CSC_NO_DIV_CHECK: Turn off checking the value of 'A' in division algorithms.
 */

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "csc.h"

/**
 * \brief CSC based on double-precision FP division.
 */
csc_int_t csc_dp(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter)
{
#ifndef CSC_NO_DIV_CHECK
    if (A == 0) {
        return 0;
    }
#endif
    *p_num_iter = 1;
    // return (csc_int_t) floor((i*(double)D/(double)A) + 0.5); // floor() not working for uint64_t on TelosB platform
    return (csc_int_t)((i*(double)D/(double)A) + 0.5);
}

/**
 * \brief CSC based on single-precision FP division.
 */
csc_int_t csc_sp(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter)
{
#ifndef CSC_NO_DIV_CHECK
    if (A == 0) {
        return 0;
    }
#endif
    *p_num_iter = 1;
    // return (csc_int_t) floor((i*(float)D/(float)A) + 0.5); // floor() not working for uint64_t on TelosB platform
    return (csc_int_t)((i*(float)D/(float)A) + 0.5);
}

/**
 * \brief CSC based on the "direct search" algorithm.
 * 
 * \remarks For details, refer to the following paper:
 * - K. S. Kim, "Direct search algorithm for clock skew compensation immune to
 *   floating-point precision loss," arXiv:2504.15039 [cs.NI], Apr. 2025.
 *   [Online]. Available: https://arxiv.org/abs/2504.15039
 */
csc_int_t csc_ds(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter)
{
    csc_int_t j = 0;
    // csc_int_t k = floor(i*(float)D/(float)A + 0.5); // a starting point; floor() not working for uint64_t on TelosB platform
    csc_int_t k = (csc_int_t)(i*(float)D/(float)A + 0.5); // a starting point
    csc_int_t td = (k - i)*A + i*(A - D); // "triangle down" to avoid overflow
    assert(td == k*A - i*D); // for debugging

#ifdef CSC_NO_ITER_COUNT    
    *p_num_iter = 1;
#else
    *p_num_iter = 0;
#endif
    if (td == 0) {
        j = k;
#ifndef CSC_NO_ITER_COUNT
        (*p_num_iter)++;
#endif
    }
    else if (td > 0) {
        while (true) {
            if (k == 0) {
                j = 0;
#ifndef CSC_NO_ITER_COUNT
                (*p_num_iter)++;
#endif
                break;
            }
            else {
                if (td - A == 0) {
                    j = k - 1;
#ifndef CSC_NO_ITER_COUNT
                    (*p_num_iter)++;
#endif
                    break;
                }
                else if (td - A > 0) {
                    k--;
                    td -= A;
#ifndef CSC_NO_ITER_COUNT
                    (*p_num_iter)++;
#endif
                }
                else {
                    j = k - (ABS(td - A) < ABS(td)); // branchless programming
#ifndef CSC_NO_ITER_COUNT
                    (*p_num_iter)++;
#endif
                    break;
                }
            }
        } // end of while loop
    }
    else { // td < 0
        while (true) {
            if (td + A == 0) {
                j = k + 1;
#ifndef CSC_NO_ITER_COUNT
                (*p_num_iter)++;
#endif
                break;
            }
            else if (td + A > 0) {
                j = k + (ABS(td + A) < ABS(td)); // branchless programming
#ifndef CSC_NO_ITER_COUNT
                (*p_num_iter)++;
#endif
                break;
            }
            else {
                k++;
                td += A;
#ifndef CSC_NO_ITER_COUNT
                (*p_num_iter)++;
#endif
            }
        } // end of while loop
    } // td < 0
    return (csc_int_t) j;
}

/**
 * \brief CSC based on the "improved direct search" algorithm.
 * 
 * \remarks For details, refer to the following paper:
 * - K. S. Kim, "Direct search algorithm for clock skew compensation immune to
 *   floating-point precision loss," arXiv:2504.15039 [cs.NI], Apr. 2025.
 *   [Online]. Available: https://arxiv.org/abs/2504.15039
 */
csc_int_t csc_ds2(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter)
{
    csc_int_t j = 0;
    // csc_int_t k = floor(i*(float)D/(float)A + 0.5); // a starting point; floor() not working for uint64_t on TelosB platform
    // csc_int_t k = (csc_int_t)(i*(float)D/(float)A + 0.5); // a starting point
    csc_int_t k = i; // a starting point avoiding overflow resulting from the original one
    csc_int_t td = (k - i)*A + i*(A - D); // "triangle down" to avoid overflow
    assert(td == k*A - i*D); // for debugging

    *p_num_iter = 1;
    if (td == 0) {
        j = k;
    }
    else if (td > 0) {
        k -= (td / A);
        td %= A;
        if (td == 0) {
            j = k;
        }
        else {
            j = k - (ABS(td - A) < ABS(td)); // branchless programming
        } 
    }
    else { // td < 0
        k += (-td / A);
        td = td % A; // N.B.: different from mathematical modulo
        if (td + A > 0) {
            j = k + (ABS(td + A) < ABS(td)); // branchless programming
        }
        else {
            j = k;
        }
    } // td < 0
    return (csc_int_t) j;
}

/**
 * \brief CSC based on the "improved direct search" algorithm with no floating-point operations.
 * 
 * \remarks For details, refer to the following paper:
 * - K. S. Kim, "Direct search algorithm for clock skew compensation immune to
 *   floating-point precision loss," arXiv:2504.15039 [cs.NI], Apr. 2025.
 *   [Online]. Available: https://arxiv.org/abs/2504.15039
 */
csc_int_t csc_ds3(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter)
{
    csc_int_t j = 0;
    // csc_int_t k = floor(i*(float)D/(float)A + 0.5); // a starting point; floor() not working for uint64_t on TelosB platform
    // csc_int_t k = (i / A) * D; // a starting point without floating-point operations
    csc_int_t k = i; // a starting point avoiding overflow resulting from the original one
    csc_int_t td = (k - i) * A + i * (A - D); // "triangle down" to avoid overflow
    assert(td == k * A - i * D); // for debugging

    *p_num_iter = 1;
    if (td == 0) {
        j = k;
    }
    else if (td > 0) {
        k -= (td / A);
        td %= A;
        if (td == 0) {
            j = k;
        }
        else {
            j = k - (ABS(td - A) < ABS(td)); // branchless programming
        } 
    }
    else { // td < 0
        k += (-td / A);
        td = td % A; // N.B.: different from mathematical modulo
        if (td + A > 0) {
            j = k + (ABS(td + A) < ABS(td)); // branchless programming
        }
        else {
            j = k;
        }
    } // td < 0
    return (csc_int_t) j;
}

/**
 * \brief CSC based on the "Euclidean Decomposition Scaling (EDS)", which relies on "Euclidean Division" and the "Midpoint" Principle
 */
csc_int_t csc_eds(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter)
{
#ifndef CSC_NO_DIV_CHECK
    if (A == 0) {
        return 0;
    }
#endif
    *p_num_iter = 1;
    csc_int_t quotient = (i / A) * D;
    csc_int_t remainder = i % A;
    csc_int_t rounding = (remainder * D + A / 2) / A;
    return quotient + rounding;
}