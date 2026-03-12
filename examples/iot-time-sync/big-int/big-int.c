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

#ifdef __MSP430__
typedef union {
  float f;
  uint32_t u;
} float_cast;

int64_t ftoi64(float f)
{
    float_cast fc;

    fc.f = f;
    uint32_t s = (fc.u >> 31) & 0x1; // 0 = positive, 1 = negative
    printf("DBG: s=%"PRIu32"\n", (uint32_t)s);
    uint32_t e = (fc.u >> 23) & 0xFF;
    printf("DBG: e=%"PRIu32"\n", (uint32_t)e);
    uint32_t m = fc.u & 0x7FFFFF; // mantissa/significand
    printf("DBG: m=%"PRIu32"\n", (uint32_t)m);

    uint64_t m_value = (uint64_t)(m + (1UL << 23));
    printf("DBG: m_value=%"PRIu64"\n", m_value);
    int16_t e_value = e - 150; // 127 + 23
    printf("DBG: e_value=%"PRIu16"\n", e_value);
    
    int64_t result;
    if (e_value >= 0) {
        result = (int64_t)(m_value << e_value);
    } else {
        result = (int64_t)(m_value >> (-e_value));
    }
    printf("DBG: result=%"PRId64"\n", result);
    return ((s & 1) ? -1 : 1) * result;
}
#endif

int64_t local_ftn(const int64_t i, const int64_t D, const int64_t A)
{
    // printf("DBG: i=%"PRId64"\n", i);
    // printf("DBG: D=%"PRId64"\n", D);
    // printf("DBG: A=%"PRId64"\n", A);
    // float R = (float)D / (float)A; // type cast from int64_t to float seems OK.
    // printf("DBG: R=%"PRId32".%"PRId32"\n",
    //     (int32_t)R, (int32_t)((R - (int32_t)R)*10000000)); // type cast from float to int32_t seems OK.
    // float rtn = i * R;
    // // printf("DBG: rtn=%"PRId64".%"PRId64"\n",
    // //     (int64_t)rtn, (int64_t)((rtn - (int64_t)rtn)*10000000));
    // printf("DBG: rtn=%"PRId32".%"PRId32"\n",
    //     (int32_t)(rtn / 10), (int32_t)((rtn - 10.0*(float)((int32_t)(rtn/10)))*1000)); // type cast from float to int64_t seems not OK!
    // int64_t rtn_ll = float_to_int64(rtn);
    // // int64_t rtn_ll = (int64_t)rtn;
    // printf("DBG: rtn_ll=%"PRId64"\n", rtn_ll);
    // return (int64_t) floor((i*(float)D/(float)A) + 0.5);
    return ftoi64((i*(float)D/(float)A) + 0.5);
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
        
        // // simple test of type casting
        // float_cast d1 = { .f = (float)Ds[i] };
        // printf("DBG: (sign, exponent, mantisa) of D = (%x,%x,%lx)\n",
        //     d1.parts.sign, d1.parts.exponent, (uint32_t)d1.parts.mantisa);
        // float x = 6707235905.198178;
        // printf("DBG: int64_t(0.3)=%"PRId64"\n", (int64_t)x);
    }
        
    PROCESS_END();
}
