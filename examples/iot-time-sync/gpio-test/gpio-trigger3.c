#include "contiki.h"
#include <msp430.h>
#include "dev/leds.h"
#include "dev/button-sensor.h" // Include this to hook into the platform ISR
#include <stdio.h>

#define TRIGGER_PIN  (1 << 0) // P2.0

static struct etimer test_timer;

PROCESS(gpio_trigger_process, "P2.0 Manual Hook");
AUTOSTART_PROCESSES(&gpio_trigger_process);

/*---------------------------------------------------------------------------*/
// We keep the callback, but we will also activate the sensor system
int
port2_callback(unsigned char pin)
{
  if(pin == 0) {
    process_poll(&gpio_trigger_process);
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(gpio_trigger_process, ev, data)
{
  PROCESS_BEGIN();

  /* IMPORTANT: On Sky, the Port 2 ISR logic lives inside the button sensor.
     We must activate it to ensure the Port 2 interrupt vector is actually handled. */
  SENSORS_ACTIVATE(button_sensor);

  printf("System Booted. P2.0 Rising Edge. Button Sensor Active.\n");

  /* Hardware Setup for P2.0 */
  P2SEL &= ~TRIGGER_PIN; 
  P2DIR &= ~TRIGGER_PIN; 
  P2IES &= ~TRIGGER_PIN; // Rising Edge
  
  P2IFG &= ~TRIGGER_PIN; 
  P2IE  |= TRIGGER_PIN;  // Enable interrupt for our specific pin

  etimer_set(&test_timer, CLOCK_SECOND * 10);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_POLL) {
      printf("[Hardware] Edge detected on P2.0!\n");
      leds_toggle(LEDS_BLUE);
    } 
    
    else if(ev == sensors_event && data == &button_sensor) {
      /* This is just to prove Port 2 is working—press the actual TelosB button! */
      printf("[System] User Button pressed (P2.7).\n");
    }

    else if(ev == PROCESS_EVENT_TIMER && data == &test_timer) {
      printf("[Self-Test] Pulse...\n");
      leds_toggle(LEDS_GREEN);
      etimer_reset(&test_timer);
    }
  }

  PROCESS_END();
}