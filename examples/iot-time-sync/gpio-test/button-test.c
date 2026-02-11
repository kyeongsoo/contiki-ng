#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(button_test, "TelosB Legacy Button Process");
AUTOSTART_PROCESSES(&button_test);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(button_test, ev, data)
{
  PROCESS_BEGIN();

  /* The button is treated as a sensor on the Sky platform */
  SENSORS_ACTIVATE(button_sensor);

  printf("TelosB Button initialized. Press User Button to toggle Red LED.\n");

  while(1) {
    /* Wait for a sensor event specifically from the button */
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);

    printf("Button Clicked!\n");
    leds_toggle(LEDS_RED);
  }

  PROCESS_END();
}
