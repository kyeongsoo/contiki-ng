#include <stdio.h>
#include <isr_compat.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "sys/rtimer.h"

// // upper 16 bits of the extended 32-bit rtimer
// extern volatile uint16_t rtimer_high_bits;
// // volatile uint16_t rtimer_high_bits = 0;

// /**
//  * \brief Get the current 32-bit rtimer value
//  */
// uint32_t RTIMER32_NOW(void) {
//   uint16_t high1, high2, low;
  
//   // Strategy to prevent race condition during overflow:
//   // Read high, then low, then high again. If high changed, re-read.
//   do {
//     high1 = rtimer_high_bits;
//     // low = TBR; // On TelosB/MSP430, rtimer uses Timer B
//     low = RTIMER_NOW(); // On TelosB/MSP430, rtimer uses Timer B
//     high2 = rtimer_high_bits;
//   } while (high1 != high2);

//   return ((uint32_t)high1 << 16) | low;
// }

PROCESS(rtimer_ext_process, "32-Bit Rtimer Extension");
struct process *button_sensor_ext_process = &rtimer_ext_process; // for the button-sensor extension
AUTOSTART_PROCESSES(&rtimer_ext_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rtimer_ext_process, ev, data)
{
  PROCESS_EXITHANDLER(goto exit;)
  PROCESS_BEGIN();

  // button_init(NULL);
  // init_extended_rtimer();
  const int period = 1 * CLOCK_SECOND;
  static rtimer32_clock_t now = 0, prev = 0;

  while(1) {
    static struct etimer et;

    etimer_set(&et, period);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    now = RTIMER32_NOW();
    printf("32-bit rtimer value=%10lu with diff=%lu\n", (unsigned long)now, (unsigned long)(now - prev));
    prev = now;

    // etimer_set(&et, period);
    // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    // now = RTIMER32_NOW();
    // printf("RED: 32-bit rtimer value=%lu with diff=%lu\n", (unsigned long)now, (unsigned long)(now - prev));
    // // printf("RED: high=%u and low=%u of the 32-bit rtimer\n", (unsigned)rtimer_high_bits, (unsigned)RTIMER_NOW());
    // leds_on(LEDS_RED);
    // prev = now;

    // etimer_set(&et, period);
    // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    // now = RTIMER32_NOW();
    // printf("GREEN: 32-bit rtimer value=%lu with diff=%lu\n", (unsigned long)now, (unsigned long)(now - prev));
    // // printf("GREEN: high=%u and low=%u of the 32-bit rtimer\n", (unsigned)rtimer_high_bits, (unsigned)RTIMER_NOW());
    // leds_on(LEDS_GREEN);
    // prev = now;
    
    // etimer_set(&et, period);
    // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    // now = RTIMER32_NOW();
    // printf("BLUE: 32-bit rtimer value=%lu with diff=%lu\n", (unsigned long)now, (unsigned long)(now - prev));
    // // printf("YELLOW: high=%u and low=%u of the 32-bit rtimer\n", (unsigned)rtimer_high_bits, (unsigned)RTIMER_NOW());
    // leds_on(LEDS_YELLOW);
    // prev = now;

    // etimer_set(&et, period);
    // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    // now = RTIMER32_NOW();
    // printf("OFF: 32-bit rtimer value=%lu with diff=%lu\n", (unsigned long)now, (unsigned long)(now - prev));
    // // printf("OFF: high=%u and low=%u of the 32-bit rtimer\n", (unsigned)rtimer_high_bits, (unsigned)RTIMER_NOW());
    // printf("REGISTERS - TACTL: %x, TAR: %u\n", TACTL, TAR); // DEBUG
    // leds_off(LEDS_ALL);
    // prev = now;
  }

 exit:
  leds_off(LEDS_ALL);
  PROCESS_END();
}
