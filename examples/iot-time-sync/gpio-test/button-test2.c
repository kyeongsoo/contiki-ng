#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(button_test2, "TelosB Legacy Button 2 Process");
AUTOSTART_PROCESSES(&button_test2);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(button_test2, ev, data)
{
  PROCESS_BEGIN();

  /* The button is treated as a sensor on the Sky platform */
  SENSORS_ACTIVATE(button_sensor2);

  printf("TelosB Button 2 initialized. Press User Button 2 to toggle Red LED.\n");

  while(1) {
    /* Wait for a sensor event specifically from the button 2 */
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor2);

    printf("Button 2 Clicked!\n");
    leds_toggle(LEDS_RED);
  }

  PROCESS_END();
}
