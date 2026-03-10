/**
 * \file
 *         Debug issues related with big integers
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

PROCESS(csc_debug_process, "CSC debug process");
AUTOSTART_PROCESSES(&csc_debug_process);

PROCESS_THREAD(csc_debug_process, ev, data)
{
    static struct etimer timer;
    static csc_int_t is[] = {6260998858, 6349136858, 6475719945, 6650781983, 6730639020};
    static csc_int_t Ds[] = {7985053352, 8072801192, 8199548072, 8375043760, 8453041832};
    static csc_int_t As[] = {8012942942, 8101110627, 8228194770, 8404234237, 8482536472};
    static uint32_t num_iter;

    PROCESS_BEGIN();
 
    etimer_set(&timer, CLOCK_SECOND * 10); // initial delay

    // int N = sizeof(is) / sizeof(is[0]);
    for (int i = 0; i < 5; i++) {
        printf("i=%"CSC_INT_PRI", D=%"CSC_INT_PRI", A=%"CSC_INT_PRI", ds2=%"CSC_INT_PRI", eds=%"CSC_INT_PRI"\n",
            is[i], Ds[i], As[i], csc_ds2(is[i], Ds[i], As[i], &num_iter), csc_eds(is[i], Ds[i], As[i], &num_iter));        
    }
        
    PROCESS_END();
}
