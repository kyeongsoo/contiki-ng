#include "contiki.h"
#include <msp430.h>
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "sys/rtimer.h"
#include <stdio.h>

/* #define TRIGGER_PIN (1 << 0) // P2.0 */
#define TRIGGER_PIN (1 << 7) // P2.7

static volatile rtimer_clock_t sync_timestamp;

PROCESS(gpio_trigger_process, "TelosB Sync Process");
AUTOSTART_PROCESSES(&gpio_trigger_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(gpio_trigger_process, ev, data)
{
  PROCESS_BEGIN();

  /* 1. Activate the button sensor to 'turn on' the Port 2 ISR */
  SENSORS_ACTIVATE(button_sensor);

  /* 2. Configure P2.0 manually */
  P2SEL &= ~TRIGGER_PIN;
  P2DIR &= ~TRIGGER_PIN;
  P2IES &= ~TRIGGER_PIN; // Rising Edge
  P2IFG &= ~TRIGGER_PIN;
  P2IE  |= TRIGGER_PIN;  // Enable interrupt for our pin

  printf("P2.0 Trigger Active. Waiting for Raspberry Pi...\n");

  while(1) {
    /* * On the Sky platform, when ANY Port 2 pin triggers, 
     * the button_sensor posts a sensors_event.
     */
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event);
    printf("Port 2 event detected\n");
    
    /* 3. Check if OUR pin was the one that caused the interrupt */
    if(P2IFG & TRIGGER_PIN) {
       sync_timestamp = RTIMER_NOW();
       P2IFG &= ~TRIGGER_PIN; // Clear the flag manually

       printf("Sync Pulse Detected! Rtimer: %u\n", sync_timestamp);
       leds_toggle(LEDS_BLUE);
    }
  }

  PROCESS_END();
}
