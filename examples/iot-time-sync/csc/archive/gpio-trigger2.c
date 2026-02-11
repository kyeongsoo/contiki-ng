#include "contiki.h"
#include <msp430.h>
#include "dev/leds.h"
#include <stdio.h>

#define TRIGGER_PIN  (1 << 0) // P2.0 (U2 Pin 10)

static struct etimer test_timer;
static volatile rtimer_clock_t sync_timestamp;

PROCESS(gpio_trigger_process, "P2.0 Rising Edge Trigger");
AUTOSTART_PROCESSES(&gpio_trigger_process);

// /*---------------------------------------------------------------------------*/
// /* The ISR Hook                                                              */
// /* Note: If this fails to link, see the 'Conflict' note below.               */
// /*---------------------------------------------------------------------------*/
// #if defined(__GNUC__) && defined(__MSP430__)
// __attribute__((interrupt(PORT2_VECTOR)))
// void port2_interrupt_handler(void)
// {
//   if(P2IFG & TRIGGER_PIN) {
//     P2IFG &= ~TRIGGER_PIN; // Clear flag
//     process_poll(&gpio_trigger_process);
//   }
// }
// #endif

/*---------------------------------------------------------------------------*/
/* This function is called by the system's Port 2 ISR                        */
/*---------------------------------------------------------------------------*/
int
port2_callback(unsigned char pin)
{
  if(pin == 0) { // P2.0
    /* Capture the high-resolution timer IMMEDIATELY */
    sync_timestamp = RTIMER_NOW();

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

  printf("Configuring P2.0 for RISING EDGE detection...\n");

  /* Hardware Configuration */
  P2SEL &= ~TRIGGER_PIN; // GPIO mode
  P2DIR &= ~TRIGGER_PIN; // Input mode
  
  /* EDGE SELECT: Clear bit for Rising Edge (0) */
  P2IES &= ~TRIGGER_PIN; 
  
  P2IFG &= ~TRIGGER_PIN; // Clear any pending flags
  P2IE  |= TRIGGER_PIN;  // Enable Interrupt

  /* Set a timer for the first self-test pulse (5 seconds) */
  etimer_set(&test_timer, CLOCK_SECOND * 5);

  while(1) {
    PROCESS_WAIT_EVENT();

    /* Check if the event is a physical interrupt (Poll) OR our test timer */
    if(ev == PROCESS_EVENT_POLL || (ev == PROCESS_EVENT_TIMER && data == &test_timer)) {
      
      if(ev == PROCESS_EVENT_TIMER) {
        printf("[Self-Test] Simulating P2.0 trigger...\n");
        // Reset timer for next self-test in 10 seconds
        etimer_set(&test_timer, CLOCK_SECOND * 10);
      } else {
        printf("[Hardware] Physical Rising Edge detected on P2.0!\n");
      }

      /* --- YOUR EVENT PROCESSING LOGIC HERE --- */
      leds_toggle(LEDS_GREEN);
      printf("Event Processing Executed.\n");
      /* ---------------------------------------- */
    }
  }

  PROCESS_END();
}