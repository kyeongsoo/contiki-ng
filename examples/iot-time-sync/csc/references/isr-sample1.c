// Project 1
#include <msp430.h>
int count = 0;
int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;
  P1DIR |= 0x07;
  P1OUT  = 0x30;
  P1REN |= 0x30;
  P1IE  |= 0x30;
  P1IES |= 0x30;
  P1IFG &= ~0x30;
  __bis_SR_register(LPM4_bits + GIE);     // Enter LPM4 w/interrupt
}
// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  if(count%3==0)
  {
  P1OUT ^= BIT1;
  P1IFG &= ~0x30;
  count++;
  }
  else if(count%3==1)
  {
      P1OUT ^= BIT1;
      P1IFG &= ~0x30;
      count++;
  }
  else
  {
      P1OUT ^= BIT2;
      P1IFG &= ~0x30;
      count++;
  }
}