#include <stdio.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "sys/rtimer.h"

/* Define the bit mask for P2.X */
// #define P2_PIN  (1 << P2_EXT)

/* Link to the variables in button-sensor.c */
extern rtimer_clock_t gpio_timestamp;
extern volatile uint8_t gpio_triggered;

/*---------------------------------------------------------------------------*/
PROCESS(gpio_trigger_process, "GPIO Trigger Process");
struct process *p2_ext_process = &gpio_trigger_process; // for the button-sensor extension
AUTOSTART_PROCESSES(&gpio_trigger_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(gpio_trigger_process, ev, data)
{
  PROCESS_BEGIN();

  /* 1. ACTIVATE BUTTON (The Probe) */
  SENSORS_ACTIVATE(button_sensor);

  // /* 2. CONFIGURE P2_PIN (The Jumper) */
  // P2SEL &= ~P2_PIN; // as GPIO
  // P2DIR &= ~P2_PIN; // as input
  // P2IES &= ~P2_PIN; // rising edge
  // P2IE  |= P2_PIN;  // enable interrupt
  // P2IFG &= ~P2_PIN; // clear any initial noise

  printf("TelosB Sync App Started.\n");
  printf("Hold Jumper to VCC and press Button to inspect P2IFG.\n");

  /* Read-back for Debugging */
  printf("--- Hardware Debug Check ---\n");
  printf("P2DIR: 0x%02X (Should have bit %d as 0)\n", P2DIR, P2_EXT);
  printf("P2SEL: 0x%02X (Should have bit %d as 0)\n", P2SEL, P2_EXT);
  printf("P2IES: 0x%02X (Should have bit %d as 0 for Rising Edge)\n", P2IES, P2_EXT);
  printf("P2IE:  0x%02X (Bit %d MUST be 1)\n", P2IE, P2_EXT);
  printf("P2IN:  0x%02X (Current logic levels on all Port 2 pins)\n", P2IN);  
  printf("--- Hardware Debug Check ---\n");
  printf("TelosB PORT 2.%d initialized.\n", P2_EXT);
  
  while(1) {
    PROCESS_WAIT_EVENT();

    /* CASE A: Our Pin 6 actually worked! */
    if(ev == PROCESS_EVENT_POLL && gpio_triggered == 1) {
      printf(">>> SUCCESS: P2.%d (Jumper) Triggered the ISR directly!\n", P2_EXT);
#ifdef RTIMER_EXT
      printf("Captured 32-bit timestamp: %10"RTIMER_PRI"\n", gpio_timestamp);
#else
      printf("Captured 16-bit timestamp: %5"RTIMER_PRI"\no", gpio_timestamp);
#endif
      // printf("Captured P2IFG: 0x%02X\n", debug_flags);
      // printf("Captured P2SEL: 0x%02X\n", debug_sel);
      // printf("Captured P2DIR: 0x%02X\n", debug_dir);
      gpio_triggered = 0;
      // leds_off(LEDS_RED);
    }

    /* CASE B: The Button was pressed (The Probe) */
    if(ev == sensors_event && data == &button_sensor) {
      // printf("\n--- PORT 2 CRIME SCENE CAPTURE ---\n");
      // printf("P2IFG: 0x%02X\n", debug_flags);
      // printf("P2SEL: 0x%02X\n", debug_sel);
      // printf("P2DIR: 0x%02X\n", debug_dir);
      
      // if(!(debug_flags & 0x40)) {
      //   printf("RESULT: Bit 6 is MISSING in hardware. Forcing Software Trigger Test...\n");
        
      //   /* THE NUCLEAR TEST: Manually set the flag */
      //   P2IFG |= 0x40; 
      //   printf("Action: P2IFG |= 0x40 executed. Check if Red LED turned on.\n");
      // }
    }
  }

//       /* Wait for the poll signal from the ISR */
//     //   PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
//     //   PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event);

//       printf("PORT2 event detected\n");
//       printf("P2IE:  0x%02X (Current logic P2IE value)\n", P2IE);
//       printf("P2IN:  0x%02X (Current logic levels on all Port 2 pins)\n", P2IN);
      
//       printf("\n--- PORT 2 STATE CAPTURE ---\n");
//       printf("P2IFG (Flags): 0x%02X\n", p2_ifg);
//       printf("P2SEL (Select): 0x%02X\n", p2_sel);
//       printf("P2DIR (Direction): 0x%02X\n", p2_dir);

//       if(p2_ifg & 0x40) {
//         printf("Result: Bit 6 Flag IS set!\n");
//       } else {
//         printf("Result: Bit 6 Flag is MISSING.\n");
//       }

//       if (gpio_triggered == 1) {
//           printf("Edge detected on P2.6 at %u\n", (unsigned)gpio_timestamp);
//           leds_toggle(LEDS_GREEN);
//           printf("P2IE:  0x%02X (Current logic P2IE value)\n", P2IE);
//           printf("P2IN:  0x%02X (Current logic levels on all Port 2 pins)\n", P2IN);  
//           gpio_triggered = 0; // clear the flag


//       }
//       leds_toggle(LEDS_RED);
//   }

  PROCESS_END();
}
