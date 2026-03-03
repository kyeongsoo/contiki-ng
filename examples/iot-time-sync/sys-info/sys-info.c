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
#include "dev/sensor/sht11/sht11-sensor.h" // for temperature/humidity sensors
#include "dev/light-sensor.h"
#endif

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
    #if RTIMER_EXT == 1
    printf("# - RTIMER_EXT=1: 32-bit rtimer running at 32.768 kHz (32.768 kHz ACLK driven by crystal; default)\n");
    #elif RTIMER_EXT == 2
    printf("# - RTIMER_EXT=2: 32-bit rtimer running at 0.4875 MHz (3.9 MHz SMCLK divided by 8; stable)\n");
    #elif RTIMER_EXT == 3
    printf("# - RTIMER_EXT=3: 32-bit rtimer running at 0.975 MHz (3.9 MHz SMCLK divided by 4; experimental)\n");
    #elif RTIMER_EXT == 4
    printf("# - RTIMER_EXT=4: 32-bit rtimer running at 1.95 MHz (3.9 MHz SMCLK divided by 2; unstable!!!)\n");
    #else
        #error "Unsupported RTIMER_EXT value. Supported values are 1, 2, 3, and 4."
    #endif
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
    printf("F_CPU: %lu Hz\n", (long unsigned)F_CPU);
    printf("MSP430_CPU_SPEED: %lu Hz\n", (long unsigned)MSP430_CPU_SPEED);
    printf("CLOCK_SIZE: %u\n", (unsigned)CLOCK_SIZE);
    printf("CLOCK_SECOND: %lu\n", (long unsigned)CLOCK_SECOND);
    printf("RTIMER_CLOCK_SIZE: %u\n", (unsigned)RTIMER_CLOCK_SIZE);
    printf("RTIMER_SECOND: %lu\n", (long unsigned)RTIMER_SECOND);
    printf("RTIMER_CLOCK_MAX: %lu\n", (long unsigned)RTIMER_CLOCK_MAX);
    printf("BCSCTL2: %x\n", BCSCTL2);
#if defined(CONTIKI_TARGET_SKY)
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
