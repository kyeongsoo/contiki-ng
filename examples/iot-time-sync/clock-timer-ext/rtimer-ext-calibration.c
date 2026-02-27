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
  extern volatile rtimer_clock_t gio_timestamp;
  extern volatile uint8_t gio_triggered;

  static bool event_initialized = false;
  static rtimer_clock_t gio_timestamp_prev = 0;
  static uint32_t event_number = 0;
  static uint32_t iet = 0; // inter-event time

  PROCESS_BEGIN();

  // enable trigger detection at GPIO
  SENSORS_ACTIVATE(button_sensor);

  while (event_number < 1000) {
    PROCESS_WAIT_EVENT();

    if (ev == PROCESS_EVENT_POLL && gio_triggered == true) {
      if (event_initialized == false) {
        event_initialized = true;
        LOG_INFO("t=%"RTIMER_PRI": Detect 1st event through GIO\n", gio_timestamp);
        printf("##### BEGIN\n");
      }
      else {
        // handle timestamp wraparound
        if (gio_timestamp < gio_timestamp_prev) {
          iet = (uint32_t)gio_timestamp + (RTIMER_CLOCK_MAX - gio_timestamp_prev);
        } else {
          iet = (uint32_t)(gio_timestamp - gio_timestamp_prev);
        }
        printf("event_number=%"PRIu32",t=%"RTIMER_PRI",iet=%"PRIu32",elapsed_us=%"PRIu64"\n",
          event_number, gio_timestamp, iet, 1000000*(uint64_t)iet/RTIMER_SECOND);
      }
      event_number++;
      gio_timestamp_prev = gio_timestamp;
      gio_triggered = 0; // clear the flag
    }
  }
  printf("##### END\n");
  
  PROCESS_END();
}
