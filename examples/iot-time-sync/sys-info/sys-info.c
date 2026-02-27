/**
 * \file
 *         Display various system information on a given platform.
 * \author
 *         Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 */

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "os/net/linkaddr.h"
#include "sys/rtimer.h"
#if defined(CONTIKI_TARGET_SKY)
#include "dev/sensor/sht11/sht11-sensor.h" // TelosB Temperature/Humidity
#include "dev/light-sensor.h" // TelosB Light
#endif

/* unsigned long getSMCLK() { */
/*     unsigned long sourceFreq; */
/*     unsigned int divider; */

/*     // Check SELS bit in BCSCTL2 */
/*     if (BCSCTL2 & SELS) { */
/*         // Source is XT2 (assuming XT2 is active and used) */
/*         sourceFreq = 8000000; // Example: 8MHz crystal */
/*     } else { */
/*         // Source is DCO */
/*         // Need to calculate DCO frequency based on DCOCTL/BCSCTL1 */
/*         sourceFreq = 1048576; // Example default */
/*     } */

/*     // Determine Divider */
/*     divider = (BCSCTL2 & DIVS1) ? ((BCSCTL2 & DIVS0) ? 8 : 4) : ((BCSCTL2 & DIVS0) ? 2 : 1); */
/*     // Alternatively: divider = 1 << ((BCSCTL2 >> 4) & 0x03); */

/*     return sourceFreq / divider; */
/* } */

PROCESS(sys_info_process, "System information process");
AUTOSTART_PROCESSES(&sys_info_process);

PROCESS_THREAD(sys_info_process, ev, data)
{
    static struct etimer timer;
    rtimer_clock_t t1, t2;
    uint32_t elapsed_microseconds;

    PROCESS_BEGIN();

#if defined(CONTIKI_TARGET_SKY)
    // Initialize sensors
    SENSORS_ACTIVATE(sht11_sensor);
    SENSORS_ACTIVATE(light_sensor);
#endif
  
    // Initial delay of 10 seconds before the first information display
    etimer_set(&timer, CLOCK_SECOND * 10);

    t1 = RTIMER_NOW();
    printf("##### BEGIN #####\n");
    printf("# Defined macros:\n");
#ifdef RTIMER_EXT
    printf("# - RTIMER_EXT=%d: For 32-bit extension\n", RTIMER_EXT);
#endif
#ifdef US_EXT
    printf("# - US_EXT=%d: For faster rtimer extension\n", US_EXT);
#endif
    printf("link address size: %d\n", (unsigned) LINKADDR_SIZE);
    /* printf("link address: %s\n", linkaddr_node_addr.u8); */
    printf("link address: ");
    for (int i = 0; i < LINKADDR_SIZE; i++) {
        printf("%x", (unsigned)linkaddr_node_addr.u16[i]);
    }
    printf("\n");
    printf("sizeof(int): %lu bytes\n", (long unsigned)sizeof(int));
    printf("sizeof(long): %lu bytes\n", (long unsigned)sizeof(long));
    printf("sizeof(long long): %lu bytes\n", (long unsigned)sizeof(long long));
    printf("sizeof(float): %lu bytes\n", (long unsigned)sizeof(float));
    printf("sizeof(double): %lu bytes\n", (long unsigned)sizeof(double));
    // printf("sizeof(rtimer_clock_t): %lu bytes\n", (long unsigned)(sizeof t1));
    printf("F_CPU: %lu Hz\n", (long unsigned)F_CPU);
    printf("MSP430_CPU_SPEED: %lu Hz\n", (long unsigned)MSP430_CPU_SPEED);
    /* printf("SMCLK: %lu Hz\n", getSMCLK()); */
    printf("CLOCK_SIZE: %u\n", (unsigned)CLOCK_SIZE);
    printf("CLOCK_SECOND: %lu\n", (long unsigned)CLOCK_SECOND);
    printf("RTIMER_CLOCK_SIZE: %u\n", (unsigned)RTIMER_CLOCK_SIZE);
    printf("RTIMER_SECOND: %lu\n", (long unsigned)RTIMER_SECOND);
    printf("RTIMER_CLOCK_MAX: %lu\n", (long unsigned)RTIMER_CLOCK_MAX);
    printf("BCSCTL2: %x\n", BCSCTL2);
#if defined(CONTIKI_TARGET_SKY)
    // test sensors
    int temp = sht11_sensor.value(SHT11_SENSOR_TEMP);
    int light = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
    printf("Temperature: %d C, Light: %d\n", (temp/10), light);
#endif
    t2 = RTIMER_NOW();
    printf("elapsed ticks: %lu\n", (long unsigned)(t2 - t1));
    elapsed_microseconds = (1E6 * (t2 - t1)) / RTIMER_SECOND;
    printf("elapsed microseconds: %"PRIu32"\n", elapsed_microseconds);
    printf("##### END #####\n");
        
    PROCESS_END();
}
