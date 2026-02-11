/**
 * \file
 *         Analysis of clock skew compensation (CSC) algorithms
 * \author
 *         Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 */

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "os/lib/random.h"
#include "sys/node-id.h"
#include "sys/rtimer.h"

#define NO_LLABS // llabs() is missing on MSP430 platforms

// set optimization level for the direct search algorithm
#define _OPT1 // turn off iteration couting
#define _OPT2 // enable branchless programming

#ifdef NO_LLABS
long long llabs(long long n) {
    return (n < 0) ? -n : n;
}
#endif

// CSC based on double-precision FP division (reference algo.)
int64_t csc_dp_div(int64_t i, int64_t D, int64_t A, int *p_num_iter)
{
    *p_num_iter = 1;
    if (A == 0) {
        return 0;
    }
    return (int64_t) floor((i * (double) D / (double) A) + 0.5);
}

// CSC based on single-precision FP division
int64_t csc_sp_div(int64_t i, int64_t D, int64_t A, int *p_num_iter)
{
    *p_num_iter = 1;
    if (A == 0) {
        return 0;
    }
    return (int64_t) floor((i * (float) D / (float) A) + 0.5);
}

// CSC based on the direct search algorithm (i.e., arXiv:2504.15039)
int64_t csc_ds(int64_t i, int64_t D, int64_t A, int *p_num_iter)
{
    // initialization
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

/*---------------------------------------------------------------------------*/
PROCESS(csc_process, "CSC process");
AUTOSTART_PROCESSES(&csc_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(csc_process, ev, data)
{
    static struct etimer timer;

    // experimental parameters
    int64_t D = 1000000; // corresponding to 1s
    int skew_max = 100; // skew bound in ppm
    int N_samples = 100; // Number of samples for D
    int64_t is[] = {1000000, 10000000, 100000000, 1000000000};
    int N_is = sizeof(is) / sizeof(is[0]);

    // CSC algosrithms
    int64_t (*csc_algs[])(int64_t, int64_t, int64_t, int*) = {csc_dp_div, csc_sp_div, csc_ds};
    char *csc_alg_names[] = {"csc_dp_div", "csc_sp_div", "csc_ds"}; // 1st one is a reference algo.
    int N_algs = sizeof(csc_algs) / sizeof(csc_algs[0]);

    int64_t A, i, j[N_algs];
    int skew;
    int64_t csc_err, csc_err_min[N_algs], csc_err_max[N_algs];
    double csc_err_sum[N_algs];
    int num_iter, num_iter_min[N_algs], num_iter_max[N_algs];
    double num_iter_sum[N_algs];
    rtimer_clock_t start_ticks, end_ticks; // 'rtimer_clock_t' -> 'uint16_t'
    rtimer_clock_t elapsed_ticks, elapsed_ticks_min[N_algs], elapsed_ticks_max[N_algs];
    double elapsed_ticks_sum[N_algs];

    PROCESS_BEGIN();

    etimer_set(&timer, CLOCK_SECOND * 10);
    /* random_init(node_id); */
    random_init();

    while(1) {
        printf("Starting CSC analysis with rtimer %u ticks per second", RTIMER_SECOND);
        for (int n = 0; n < N_is; n++) {
            i = is[n];

            // initialize statistics
            for (int a = 0; a < N_algs; a++) {
                csc_err_min[a] = INT64_MAX;
                csc_err_max[a] = INT64_MIN;
                csc_err_sum[a] = 0.0;
                elapsed_ticks_min[a] = UINT16_MAX;
                elapsed_ticks_max[a] = 0;
                elapsed_ticks_sum[a] = 0.0;
            }

            for (int k = 0; k < N_samples; k++) {
                skew = (random_rand() % (2*skew_max + 1)) - skew_max; // random skew in ppm
                printf("skew=%d ppm", skew);
                A = (int64_t) ((1 + skew*1.0E-6)*D);
                for (int a = 0; a < N_algs; a++) {
                    start_ticks = RTIMER_NOW();
                    printf("Running %s(i=%lld, D=%lld, A=%lld, &num_iter)", csc_alg_names[a], i, D, A);
                    j[a] = csc_algs[a](i, D, A, &num_iter);
                    end_ticks = RTIMER_NOW();
                    elapsed_ticks = (end_ticks < start_ticks) ? end_ticks + (UINT32_MAX - start_ticks) : end_ticks - start_ticks;
                    printf("Results: j=%lld, num_iter=%d, elapsed cycles=%u", j[a], num_iter, elapsed_ticks);

                    if (k > 0) {
                        csc_err = j[0] - j[a];
                        csc_err_sum[a] += csc_err;
                        if (csc_err < csc_err_min[a]) {
                            csc_err_min[a] = csc_err;
                        }
                        if (csc_err > csc_err_max[a]) {
                            csc_err_max[a] = csc_err;
                        }
                    }

                    num_iter_sum[a] += num_iter;
                    if (num_iter < num_iter_min[a]) {
                        num_iter_min[a] = num_iter;
                    }
                    if (num_iter > num_iter_max[a]) {
                        num_iter_max[a] = num_iter;
                    }

                    elapsed_ticks_sum[a] += elapsed_ticks;
                    if (elapsed_ticks < elapsed_ticks_min[a]) {
                        elapsed_ticks_min[a] = elapsed_ticks;
                    }
                    if (elapsed_ticks > elapsed_ticks_max[a]) {
                        elapsed_ticks_max[a] = elapsed_ticks;
                    }
                } // end of a
            } // end of k

            // display results
            printf("## i=%lld:\n", i);
            for (int a = 0; a < N_algs; a++) {
                printf("### %s\n", csc_alg_names[a]);
                printf("- CSC error (min./max./avg.): %lld/%lld/%.4e\n", csc_err_min[a], csc_err_max[a], csc_err_sum[a]/N_samples);
                printf("- Number of iterations (min./max./avg.): %d/%d/%.4e\n", num_iter_min[a], num_iter_max[a], num_iter_sum[a]/N_samples);
                printf("- Elapsed ticks (min./max./avg.): %u/%u/%.4e\n", elapsed_ticks_min[a], elapsed_ticks_max[a], elapsed_ticks_sum[a]/N_samples);
                /* printf("- Elapsed time [us] (min./max./avg.): %lu/%lu/%.4e\n", elapsed_ticks_min[a]/us_per_tick, elapsed_ticks_max[a]/us_per_tick, (elapsed_ticks_sum[a]/N_samples)/us_per_tick); */
            } // end of a
        } // end of i

        /* Wait for the periodic timer to expire and then restart the timer. */
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        etimer_reset(&timer);
    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
