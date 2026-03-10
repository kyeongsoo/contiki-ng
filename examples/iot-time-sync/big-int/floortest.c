/**
 * \file
 *         A function for testing external call
 * \author
 *         Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 */

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "floortest.h"

// int64_t external_ftn(const int64_t i, const int64_t D, const int64_t A)
// {
//     // return (int64_t) floor((i*(float)D/(float)A) + 0.5);
//     return (int64_t)(i*D/A);
// }

int64_t external_ftn(const int64_t i, const int64_t D, const int64_t A)
{
int64_t quotient = (i / A) * D;
int64_t remainder = i % A;
int64_t rounding = (remainder * D + (A / 2)) / A;
return quotient + rounding;
}