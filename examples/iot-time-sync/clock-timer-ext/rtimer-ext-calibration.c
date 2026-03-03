#include <inttypes.h>
#include <isr_compat.h>
#include <stdbool.h>
#include <stdio.h>
#include "contiki.h"
#include "dev/button-sensor.h" // for GPIO trigger through button-sensor extension
#include "dev/leds.h"
#include "sys/log.h"
#include "sys/rtimer.h"

#define LOG_MODULE "CSC-Client"
#define LOG_LEVEL LOG_LEVEL_INFO

PROCESS(rtimer_ext_calibration_process, "Rtimer Extension Calibration");
struct process *p2_ext_process = &rtimer_ext_calibration_process; // for P2 extension
AUTOSTART_PROCESSES(&rtimer_ext_calibration_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rtimer_ext_calibration_process, ev, data)
{
  // GPIO trigger external variables
  extern volatile rtimer_clock_t gpio_timestamp;
  extern volatile uint8_t gpio_triggered;

  static bool event_initialized = false;
  static rtimer_clock_t gpio_timestamp_prev = 0;
  static uint32_t event_number = 0;
  static uint32_t iet = 0; // inter-event time

  PROCESS_BEGIN();

  // enable trigger detection at GPIO
  SENSORS_ACTIVATE(button_sensor);

  printf("Now ready for triggers\n");
  while (event_number <= 100) {
    PROCESS_WAIT_EVENT();

    if (ev == PROCESS_EVENT_POLL && gpio_triggered == true) {
      if (event_initialized == false) {
        event_initialized = true;
        LOG_INFO("t=%"RTIMER_PRI": Detect 1st event through GPIO\n", gpio_timestamp);
        printf("##### BEGIN\n");
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
      }
      else {
        // handle timestamp wraparound
        if (gpio_timestamp < gpio_timestamp_prev) {
          iet = (uint32_t)gpio_timestamp + (RTIMER_CLOCK_MAX - gpio_timestamp_prev);
        } else {
          iet = (uint32_t)(gpio_timestamp - gpio_timestamp_prev);
        }
        printf("event_number=%"PRIu32",t=%"RTIMER_PRI",iet=%"PRIu32",elapsed_us=%"PRIu64"\n",
          event_number, gpio_timestamp, iet, 1000000*(uint64_t)iet/RTIMER_SECOND);
      }
      event_number++;
      gpio_timestamp_prev = gpio_timestamp;
      gpio_triggered = 0; // clear the flag
    }
  }
  printf("##### END\n");
  
  PROCESS_END();
}
