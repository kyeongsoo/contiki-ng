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
#include "floortest.h"

// int64_t local_ftn(const int64_t i, const int64_t D, const int64_t A)
// {
//     // return (int64_t) floor((i*(float)D/(float)A) + 0.5);
//     return (int64_t)((i*D)+(A/2))/A;
// }

int64_t local_ftn(const int64_t i, const int64_t D, const int64_t A)
{
int64_t quotient = (i / A) * D;
int64_t remainder = i % A;
int64_t rounding = (remainder * D + (A / 2)) / A;
return quotient + rounding;
}

PROCESS(big_int_process, "Big integer debug process");
AUTOSTART_PROCESSES(&big_int_process);

PROCESS_THREAD(big_int_process, ev, data)
{
    static struct etimer timer;
    static int64_t i = 1000000000000000LL;
    static int64_t D = 1000000LL;
    static int64_t A = 1000075LL;

    PROCESS_BEGIN();
 
    etimer_set(&timer, CLOCK_SECOND * 10); // initial delay

    printf("##### BEGIN #####\n");
    printf("local function: %"PRId64"\n", local_ftn(i, D, A));
    printf("external function: %"PRId64"\n", external_ftn(i, D, A));
    printf("##### END #####\n");
        
    PROCESS_END();
}
