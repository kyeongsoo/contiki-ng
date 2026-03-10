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
#include <stdlib.h>
#include "contiki.h"
#include "csc.h"

csc_int_t csc_sp_local(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter)
{
#ifndef CSC_NO_DIV_CHECK
    if (A == 0) {
        return 0;
    }
#endif
    *p_num_iter = 1;
    return (csc_int_t) floor((i*(float)D/(float)A) + 0.5); // floor() not working for uint64_t on TelosB platform
    // return (csc_int_t)((i*(float)D/(float)A) + 0.5);
}

PROCESS(csc_debug_process, "CSC debug process");
AUTOSTART_PROCESSES(&csc_debug_process);

PROCESS_THREAD(csc_debug_process, ev, data)
{
    static struct etimer timer;
    static csc_int_t i = 1000000000000000;
    static csc_int_t D = 1000000;
    static csc_int_t A = 1000075;
    static csc_int_t j_ds2, j_sp, k, td, diff;
    static uint32_t num_iter;

    PROCESS_BEGIN();
 
    // Initial delay of 10 seconds before the first information display
    etimer_set(&timer, CLOCK_SECOND * 10);

    printf("##### BEGIN #####\n");
    printf("CSC_INT_SIZE: %d\n", CSC_INT_SIZE);
    printf("i: %"CSC_INT_PRI"\n", i);
    printf("D: %"CSC_INT_PRI"\n", D);
    printf("A: %"CSC_INT_PRI"\n", A);
    printf("i*D: %"CSC_INT_PRI"\n", i*D);
    printf("D/A: %"CSC_INT_PRI"\n", D/A);
    printf("A/D: %"CSC_INT_PRI"\n", A/D);
    printf("(csc_int_t)((i*(float)D/(float)A)+0.5): %"CSC_INT_PRI"\n", (csc_int_t)((i*(float)D/(float)A)+0.5));
    printf("(csc_int_t)floor((i*(float)D/(float)A)+0.5): %"CSC_INT_PRI"\n", (csc_int_t)floor((i*(float)D/(float)A)+0.5));
    printf("original version of sp_div: %"CSC_INT_PRI"\n", csc_sp(i, D, A, &num_iter));
    printf("local version of sp_div: %"CSC_INT_PRI"\n", csc_sp_local(i, D, A, &num_iter));
    k = (csc_int_t)((i * (float)D / (float)A) + 0.5);
    td = (k - i)*A + i*(A - D);
    printf("sp_div w/o floor(): j=%"CSC_INT_PRI"\n", k);
    printf("td: %"CSC_INT_PRI"\n", td);
    k -= (td / A);
    printf("td / A: %"CSC_INT_PRI"\n", td / A);
    printf("k - (td / A): %"CSC_INT_PRI"\n", k);
    td %= A;
    printf("td %% A: %"CSC_INT_PRI"\n", td);
    j_sp = csc_sp(i, D, A, &num_iter);
    printf("sp_div w/ floor(): j=%"CSC_INT_PRI"\n", j_sp);
    j_ds2 = csc_ds2(i, D, A, &num_iter);
    printf("ds2: j=%"CSC_INT_PRI", num_iter=%"PRIu32"\n", j_ds2, num_iter);
    diff = j_ds2 - j_sp;
    printf("diff=%"CSC_INT_PRI"\n", diff);
    printf("##### END #####\n");
        
    PROCESS_END();
}
