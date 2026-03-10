/**
 * \file
 *         Debug clock skew compensation (CSC) algorithms
 * \author
 *         Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 */

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // for rand() and srand()
#include "csc.h"

int main(void)
{
    // experimental parameters
    csc_int_t i, A, D, j_ds, j_sp, diff;
    uint16_t num_iter;

    printf("i: ");
#if CSC_INT_SIZE == 8
    scanf("%"SCNd64, &i);
#elif CSC_INT_SIZE == 4
    scanf("%"SCNd32, &i);
#else
#error Unsupported CSC_INT_SIZE
#endif
    printf("D: ");
#if CSC_INT_SIZE == 8
    scanf("%"SCNu64, &D);
#elif CSC_INT_SIZE == 4
    scanf("%"SCNu32, &D);
#else
#error Unsupported CSC_INT_SIZE
#endif
    printf("A: ");
#if CSC_INT_SIZE == 8
    scanf("%"SCNu64, &A);
#elif CSC_INT_SIZE == 4
    scanf("%"SCNu32, &A);
#else
#error Unsupported CSC_INT_SIZE
#endif

    j_ds = csc_ds(i, D, A, &num_iter);
    j_sp = csc_sp(i, D, A, &num_iter);
    diff = j_ds - j_sp;
    printf("ds: j=%"CSC_INT_PRI", num_iter=%"PRIu16"\n", j_ds, num_iter);
    printf("sp_div: j=%"CSC_INT_PRI"\n", j_sp);
    printf("diff=%"CSC_INT_PRI"\n", diff);

    return 0;
}
