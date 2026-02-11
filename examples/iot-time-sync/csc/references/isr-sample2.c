// Project 2
#include <msp430.h>
int i,j;
int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;
  P1DIR |= 0x06;
  P1OUT =  0x10;
  P1REN |= 0x10;
  P1IE |= 0x10;
  P1IES |= 0x10;
  P1IFG &= ~0x10;
  P2DIR |= 0x00;
  P2OUT =  0x01;
  P2REN |= 0x01;
  P2IE |= 0x01;
  P2IES |= 0x01;
  P2IFG &= ~0x01;
  __bis_SR_register(LPM4_bits + GIE);       // Enter LPM4 w/interrupt
}
// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
      P1OUT ^= BIT1;
      P1IFG &= ~0x10;
      for(i=0;i<20000;i++)
      {
      }
      P1OUT ^= BIT1;
}
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P1OUT ^= BIT2;
    P2IFG &= ~0x01;
    for(j=0;j<20000;j++)
         {
         }
    P1OUT ^= BIT2;
}