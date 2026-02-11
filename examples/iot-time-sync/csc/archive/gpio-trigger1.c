#include "contiki.h"
#include <msp430.h>
#include "dev/leds.h"
#include <stdio.h>

/* On TelosB, we want P2.0 */
#define TRIGGER_PIN  (1 << 0)

PROCESS(gpio_trigger_process, "P2.0 Trigger Process");
AUTOSTART_PROCESSES(&gpio_trigger_process);

/*---------------------------------------------------------------------------*/
/* This function is called by the system's Port 2 ISR                        */
/*---------------------------------------------------------------------------*/
int
port2_callback(unsigned char pin)
{
  if(pin == 0) { // P2.0
    /* We are inside an ISR here! Keep it fast. */
    process_poll(&gpio_trigger_process);
    return 1; // Indicate the interrupt was handled
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(gpio_trigger_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Initializing P2.0 via manual register config...\n");

  /* Hardware Setup */
  P2SEL &= ~TRIGGER_PIN; // GPIO mode
  P2DIR &= ~TRIGGER_PIN; // Input
  P2IES &= ~TRIGGER_PIN; // Rising Edge
  
  /* Clear and Enable */
  P2IFG &= ~TRIGGER_PIN;
  P2IE  |= TRIGGER_PIN;

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    printf("P2.0 Triggered!\n");
    leds_toggle(LEDS_GREEN);
  }

  PROCESS_END();
}