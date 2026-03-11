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

int64_t local_ftn(const int64_t i, const int64_t D, const int64_t A)
{
    printf("DBG: i=%"PRId64"\n", i);
    printf("DBG: D=%"PRId64"\n", D);
    printf("DBG: A=%"PRId64"\n", A);
    float i_f = (float)i;
    float D_f = (float)D;
    float A_f = (float)A;
    float R = D_f / A_f;
    float rtn = i*D_f/A_f;
    printf("DBG: integer part of i_f=%"PRId64"\n", (int64_t)i_f);
    printf("DBG: fractional part of i_f=%"PRId64"\n", (int64_t)((i_f - (int64_t)i_f)*1000000));
    printf("DBG: integer part of D_f=%"PRId64"\n", (int64_t)D_f);
    printf("DBG: fractional part of D_f=%"PRId64"\n", (int64_t)((D_f - (int64_t)D_f)*1000000));
    printf("DBG: integer part of A_f=%"PRId64"\n", (int64_t)A_f);   
    printf("DBG: fractional part of A_f=%"PRId64"\n", (int64_t)((A_f - (int64_t)A_f)*1000000));
    printf("DBG: integer part of R=%"PRId64"\n", (int64_t)R);
    printf("DBG: fractional part of R=%"PRId64"\n", (int64_t)((R - (int64_t)R)*1000000));
    printf("DBG: integer part of rtn=%"PRId64"\n", (int64_t)rtn);
    printf("DBG: fractional part of rtn=%"PRId64"\n", (int64_t)((rtn - (int64_t)rtn)*1000000));
    return (int64_t) floor((i*(float)D/(float)A) + 0.5);
    // return (int64_t)((i*D)+(A/2))/A;
}

// int64_t local_ftn(const int64_t i, const int64_t D, const int64_t A)
// {
//     int64_t quotient = (i / A) * D;
//     printf("DBG: quotient=%"PRId64"\n", quotient);
//     int64_t remainder = i % A;
//     printf("DBG: remainder=%"PRId64"\n", remainder);
//     int64_t rounding = (remainder * D + (A / 2)) / A;
//     printf("DBG: rounding=%"PRId64"\n", rounding);
//     return quotient + rounding;
// }

PROCESS(big_int_process, "Big integer debug process");
AUTOSTART_PROCESSES(&big_int_process);

PROCESS_THREAD(big_int_process, ev, data)
{
    static struct etimer timer;
    static int64_t is[] = {6260998858, 6349136858, 6475719945, 6650781983, 6730639020};
    static int64_t Ds[] = {7985053352, 8072801192, 8199548072, 8375043760, 8453041832};
    static int64_t As[] = {8012942942, 8101110627, 8228194770, 8404234237, 8482536472};

    PROCESS_BEGIN();
 
    etimer_set(&timer, CLOCK_SECOND * 10); // initial delay

    // int N = sizeof(is) / sizeof(is[0]);
    for (int i = 0; i < 5; i++) {
        printf("i=%"PRId64", D=%"PRId64", A=%"PRId64", local_ftn=%"PRId64", external_ftn=%"PRId64"\n",
            is[i], Ds[i], As[i], local_ftn(is[i], Ds[i], As[i]), external_ftn(is[i], Ds[i], As[i]));        
    }
        
    PROCESS_END();
}
