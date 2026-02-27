#include <stdio.h>
#include <isr_compat.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "sys/rtimer.h"

PROCESS(rtimer_ext_process, "32-Bit Rtimer Extension");
AUTOSTART_PROCESSES(&rtimer_ext_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rtimer_ext_process, ev, data)
{
  PROCESS_BEGIN();

  const int period = 0.1 * CLOCK_SECOND;
  static struct etimer et;
  static rtimer_clock_t now = 0, prev = 0;
  static int i;

  etimer_set(&et, 5*CLOCK_SECOND); // initial delay
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  printf("\n\n\n");
  printf("##### BEGIN\n"); 
  printf("# Defined macros:\n");
#ifdef RTIMER_EXT
  printf("# - RTIMER_EXT=%d: For 32-bit extension\n", RTIMER_EXT);
#endif
  printf("t,diff\n");
  prev = RTIMER_NOW();
  for (i = 0; i < 1000; i++) {
      etimer_set(&et, period);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      now = RTIMER_NOW();
      printf("%"RTIMER_PRI",%"RTIMER_PRI"\n", now, now-prev);
      prev = now;
  }
  printf("##### END\n");
  
  PROCESS_END();
}
