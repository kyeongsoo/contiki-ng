/**
 * \file
 *         Analysis of clock skew compensation (CSC) algorithms
 * \author
 *         Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "net/netstack.h"
#include "os/lib/random.h"
#include "sys/log.h"
#include "sys/rtimer.h"
#include "csc.h"

#define LOG_MODULE "CSC-Analysis"
#define LOG_LEVEL LOG_LEVEL_NONE

PROCESS(csc_analysis_process, "CSC analysis process");
AUTOSTART_PROCESSES(&csc_analysis_process);

PROCESS_THREAD(csc_analysis_process, ev, data)
{
    static struct etimer timer;

    // experimental parameters
    csc_int_t D = 1000000; // corresponding to 1s
    int skew_max = 100; // skew bound in ppm
    int N_samples = 1000; // Number of samples for D
    csc_int_t is[] = {1000000, 10000000, 100000000, 1000000000};
    int N_is = sizeof(is) / sizeof(is[0]);

    // CSC algosrithms
    // N.B.: DS is the reference algorithm; double-precision is not supported in sky (TelosB) platform.
    csc_int_t (*csc_algs[])(csc_int_t, csc_int_t, csc_int_t, uint16_t*) = {csc_ds, csc_sp_div};
    char *alg_names[] = {"ds", "sp"};
    int N_algs = sizeof(csc_algs) / sizeof(csc_algs[0]);
    csc_int_t A, i, j[N_algs], diff;
    int skew;
    uint16_t num_iter;
    rtimer_clock_t start_ticks, end_ticks; // 'rtimer_clock_t' -> 'uint16_t'
    rtimer_clock_t elapsed_ticks;
    unsigned long elapsed_ms; // in millisecond

    PROCESS_BEGIN();

    // turn off radio
    NETSTACK_RADIO.off();

    etimer_set(&timer, CLOCK_SECOND * 10);
    random_init();

    printf("##### BEGIN\n"); // post-processing indicator
    printf("alg,i,D,A,j,diff,num_iter,elapsed_ticks,elapsed_ms\n"); // header for column names in CSV format
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
                    elapsed_ms = (unsigned long)((uint64_t)elapsed_ticks * 1000 / RTIMER_SECOND);
                    diff = (a == 0) ? 0 : j[0] - j[a]; // 1st algo. (i.e., DS) is the reference
                    
                    // data row in CSV format
                    printf("%s,%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"PRIu16",%u,%lu\n",
                           alg_names[a], i, D, A, j[a], diff, num_iter, elapsed_ticks, elapsed_ms);                    
                } // end of for() for a
            } // end of for() for k
        } // end of for() for i

        // indicator for post-processing
        printf("##### END\n");

        while (1);
        
        /* Wait for the periodic timer to expire and then restart the timer. */
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        etimer_reset(&timer);
    } // end of while() 

    PROCESS_END();
}
