/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
#include "contiki.h"
#include "lib/sensors.h"
#include "dev/hwconf.h"
#include "dev/button-sensor.h"
#include "isr_compat.h"

const struct sensors_sensor button_sensor;

static struct timer debouncetimer;
static int status(int type);

HWCONF_PIN(BUTTON, 2, 7);
HWCONF_IRQ(BUTTON, 2, 7);

#ifdef P2_EXT
/* enable GIOx for event detection and high-quality timestamping */
#include "sys/rtimer.h"
#ifdef RTIMER_EXT
volatile rtimer32_clock_t gio_timestamp = 0;
#else
volatile rtimer_clock_t gio_timestamp = 0;
#endif
volatile uint8_t gio_triggered = 0;

/* external process pointer for the poll */
extern struct process *button_sensor_ext_process;

ISR(PORT2, irq_p2)
{
  if(P2IFG & (1 << P2_EXT)) {
#ifdef RTIMER_EXT
    gio_timestamp = RTIMER32_NOW();
#else
    gio_timestamp = RTIMER_NOW();
#endif
    gio_triggered = 1;
    process_poll(button_sensor_ext_process);
    LPM4_EXIT;
  }
  if(BUTTON_CHECK_IRQ()) {
    if(timer_expired(&debouncetimer)) {
      timer_set(&debouncetimer, CLOCK_SECOND / 4);
      sensors_changed(&button_sensor);
      LPM4_EXIT;
    }
  }
  P2IFG = 0x00;
}
#else
ISR(PORT2, irq_p2)
{
  if(BUTTON_CHECK_IRQ()) {
    if(timer_expired(&debouncetimer)) {
      timer_set(&debouncetimer, CLOCK_SECOND / 4);
      sensors_changed(&button_sensor);
      LPM4_EXIT;
    }
  }
  P2IFG = 0x00;
}
#endif
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return BUTTON_READ() || !timer_expired(&debouncetimer);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch (type) {
  case SENSORS_ACTIVE:
    if (c) {
      if(!status(SENSORS_ACTIVE)) {
	timer_set(&debouncetimer, 0);
	BUTTON_IRQ_EDGE_SELECTD();

	BUTTON_SELECT();
	BUTTON_MAKE_INPUT();

	BUTTON_ENABLE_IRQ();
      }
    } else {
      BUTTON_DISABLE_IRQ();
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch (type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return BUTTON_IRQ_ENABLED();
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       value, configure, status);
