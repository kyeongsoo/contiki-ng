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
    // CSC algosrithms
    // N.B.: DS is the reference algorithm; double-precision is not supported in sky (TelosB) platform.
    csc_int_t (*csc_algs[])(csc_int_t, csc_int_t, csc_int_t, uint32_t*) = {csc_ds, csc_sp_div};
    int N_algs = sizeof(csc_algs) / sizeof(csc_algs[0]);
    csc_int_t D = 1000000; // corresponding to 1s
    csc_int_t A, i, j[N_algs];
    csc_int_t is[] = {1000000, 10000000, 100000000, 1000000000};
    csc_int_t diff;
    char *alg_names[] = {"ds", "sp"};
    int skew_max = 100; // skew bound in ppm
    int N_samples = 1000; // Number of samples for D
    int N_is = sizeof(is) / sizeof(is[0]);
    int skew;
    uint32_t num_iter;
#ifdef CONTIKI_NG
    rtimer_clock_t start_ticks, end_ticks; // 'rtimer_clock_t' -> 'uint16_t'
    rtimer_clock_t elapsed_ticks;
    unsigned long elapsed_ms; // in millisecond
#else
    uint16_t start_ticks, end_ticks; // 'rtimer_clock_t' -> 'uint16_t'
    uint16_t elapsed_ticks;
    unsigned long elapsed_ms; // in millisecond
#endif

#ifdef CONTIKI_NG
    random_init();
#else
    srand(12345); // fixed seed for reproducibility
#endif

    printf("##### BEGIN\n"); // post-processing indicator
    printf("alg,i,D,A,j,diff,num_iter,elapsed_ticks,elapsed_ms\n"); // header for column names in CSV format
        
    for (int n = 0; n < N_is; n++) {
        i = is[n];
        for (int k = 0; k < N_samples; k++) {
#ifdef CONTIKI_NG
            skew = (random_rand() % (2*skew_max + 1)) - skew_max; // in ppm
            LOG_INFO("skew=%d ppm\n", skew);
#else
            skew = (rand() % (2*skew_max + 1)) - skew_max; // in ppm
#endif
            A = (csc_int_t) ((1 + skew*1.0E-6)*D);
            for (int a = 0; a < N_algs; a++) {
#ifdef CONTIKI_NG
                start_ticks = RTIMER_NOW();
#else
                start_ticks = 0; // dummy value for non-Contiki-NG platforms
#endif
                j[a] = csc_algs[a](i, D, A, &num_iter);
#ifdef CONTIKI_NG   
                end_ticks = RTIMER_NOW();
                elapsed_ticks = (end_ticks < start_ticks) ? end_ticks + (UINT32_MAX - start_ticks) : end_ticks - start_ticks;
                elapsed_ms = (unsigned long)((uint64_t)elapsed_ticks * 1000 / RTIMER_SECOND);
#else
                // dummy values for non-Contiki-NG platforms
                end_ticks = 0; 
                elapsed_ticks = end_ticks - start_ticks;
                elapsed_ms = 0;
#endif
                diff = (a == 0) ? 0 : j[0] - j[a]; // 1st algo. (i.e., DS) is the reference
                
                // data row in CSV format
                printf("%s,%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"PRIu32",%u,%lu\n",
                        alg_names[a], i, D, A, j[a], diff, num_iter, elapsed_ticks, elapsed_ms);
                
            } // end of for() for a
        } // end of for() for k
    } // end of for() for i

    // indicator for post-processing
    printf("##### END\n");
    return 0;
}
