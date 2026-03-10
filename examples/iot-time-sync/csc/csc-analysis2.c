/**
 * \file
 *         Analysis of clock skew compensation (CSC) algorithms
 * \author
 *         Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 */

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "net/netstack.h"
#include "os/lib/random.h"
#include "sys/log.h"
// #include "sys/rtimer.h"
// #include "csc.h"

#define LOG_MODULE "CSC-Analysis"
#define LOG_LEVEL LOG_LEVEL_NONE

#if CSC_INT_SIZE == 4
typedef int32_t csc_int_t;
#define CSC_INT_MAX INT32_MAX
#define CSC_INT_PRI PRId32
#elif CSC_INT_SIZE == 8
typedef int64_t csc_int_t;
#define CSC_INT_MAX INT64_MAX
#define CSC_INT_PRI PRId64
#else
#error Unsupported CSC_INT_SIZE
#endif

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
 * \brief CSC based on the "improved direct search" algorithm.
 * 
 * \remarks For details, refer to the following paper:
 * - K. S. Kim, "Direct search algorithm for clock skew compensation immune to floating-point precision loss,"
 *   arXiv:2504.15039 [cs.NI], Apr. 2025. [Online]. Available: https://arxiv.org/abs/2504.15039
 */
csc_int_t csc_ds2(const csc_int_t i, const csc_int_t D, const csc_int_t A, uint32_t *p_num_iter)
{
    csc_int_t j = 0;
    // csc_int_t k = floor(i*(float)D/(float)A + 0.5); // a starting point; floor() not working for uint64_t on TelosB platform
    csc_int_t k = (csc_int_t)(i*(float)D/(float)A + 0.5); // a starting point
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

PROCESS(csc_analysis_process, "CSC analysis process");
AUTOSTART_PROCESSES(&csc_analysis_process);

PROCESS_THREAD(csc_analysis_process, ev, data)
{
    static struct etimer timer;

    // experimental parameters
    csc_int_t D = 1000000; // corresponding to 1s
    int skew_max = 100; // skew bound in ppm
    // int N_samples = 10000; // Number of samples for D
    int N_samples = 100; // Number of samples for D
    // csc_int_t is[] = {1000000, 10000000, 100000000, 1000000000};
    csc_int_t is[] = {10000000000, 100000000000, 1000000000000, 10000000000000, 100000000000000, 1000000000000000};
    int N_is = sizeof(is) / sizeof(is[0]);

    // CSC algosrithms
    // N.B.: DS is the reference algorithm; double-precision is not supported in sky (TelosB) platform.
    // csc_int_t (*csc_algs[])(csc_int_t, csc_int_t, csc_int_t, uint32_t*) = {csc_ds, csc_sp, csc_ds2};
    // char *alg_names[] = {"ds", "sp", "ds2"};
    csc_int_t (*csc_algs[])(csc_int_t, csc_int_t, csc_int_t, uint32_t*) = {csc_ds2, csc_sp};
    char *alg_names[] = {"ds2", "sp"};
    int N_algs = sizeof(csc_algs) / sizeof(csc_algs[0]);
    csc_int_t A, i, j[N_algs], diff;
    int skew;
    uint32_t num_iter;
    rtimer_clock_t start_ticks, end_ticks; // 'rtimer_clock_t' -> 'uint16_t'
    rtimer_clock_t elapsed_ticks;

    PROCESS_BEGIN();

    // turn off radio
    NETSTACK_RADIO.off();

    etimer_set(&timer, CLOCK_SECOND * 10);
    random_init();

    printf("##### BEGIN\n"); // post-processing indicator
    printf("alg,i,D,A,j,diff,num_iter,elapsed_ticks\n"); // header for column names in CSV format
    while (1) {
        for (int n = 0; n < N_is; n++) {
            i = is[n];
            for (int k = 0; k < N_samples; k++) {
                skew = (random_rand() % (2*skew_max + 1)) - skew_max; // in ppm
                LOG_INFO("skew=%d ppm\n", skew);
                A = (csc_int_t) ((1 + skew*1.0E-6)*D);
                for (int a = 0; a < N_algs; a++) {
                    start_ticks = RTIMER_NOW();
                    j[a] = csc_algs[a](i, D, A, &num_iter);
                    end_ticks = RTIMER_NOW();
                    elapsed_ticks = (end_ticks < start_ticks) ? end_ticks + (UINT32_MAX - start_ticks) : end_ticks - start_ticks;
                    diff = (a == 0) ? 0 : j[0] - j[a]; // 1st algo. (i.e., DS) is the reference
                    
                    // data row in CSV format
                    printf("%s,%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"PRIu32",%"RTIMER_PRI"\n",
                           alg_names[a], i, D, A, j[a], diff, num_iter, elapsed_ticks);                    
                } // end of for() for a
            } // end of for() for k
        } // end of for() for i

        // indicator for post-processing
        printf("##### END\n");
        break; // end the process
        
        /* Wait for the periodic timer to expire and then restart the timer. */
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        etimer_reset(&timer);
    } // end of while()

    PROCESS_END();
}
