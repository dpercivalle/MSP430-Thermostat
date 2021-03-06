/**
 * CPE329-07/08 | Project 3 - Environmental Sensing
 * OneWire Temperature sensor based thermostat
 *
 * @author: Donny Percivalle
 *          Alex Lin
 */

#include "tempSensors.h"
/////////////////////////////
//
// main()
//
/////////////////////////////
int main(){
  WDTCTL = WDTPW + WDTHOLD; //Stop WDT
  // Configure DCO
  if (CALBC1_8MHZ == 0XFF){
    while(1);
  }
  DCOCTL = 0;
  BCSCTL1 = CALBC1_8MHZ;
  DCOCTL = CALDCO_8MHZ;
  // Configure Timer A
  TA0CTL = TASSEL_2 + MC_2 + ID_3; // Set timer to count SMCLK
                                   // Set mode to count UP to CCR0 value
                                   // Divide the 8MHz clock by 8 = 1MHz
  CCTL0 = CCIE;          // Enable timer interrupts
  CCR0 = 65535;          
  //
  // Initialize an LCD in 8-bit Parallel (D7-D0 = P1.0-P1.7)
  // Control lines P2.0-P2.3
  //
  lcd_init(MODE_RT_NOSHIFT);
  //
  // Configure the one-wire interface
  // on P2.5
  //
  ow.port_out = &P2OUT;
  ow.port_in  = &P2IN;
  ow.port_ren = &P2REN;
  ow.port_dir = &P2DIR;
  ow.pin = BIT5;
  //
  // Configure the three two thermostat push buttons
  //
  P2SEL = 0x00;
  P2SEL2 = 0x00;
  P2DIR &= ~(TEMP_UP| TEMP_DWN);
  P2REN |= (TEMP_UP | TEMP_DWN);
  P2IE  |= (TEMP_UP | TEMP_DWN);
  P2IES |= (TEMP_UP | TEMP_DWN);
  //
  // Configure the relay control pin
  //
  P2OUT |= RELAY_CTL;
  P2DIR |= RELAY_CTL;
  //
  // Get the temperature, set threshold values, enable
  // interrupts, enter LPM0 (CPU off; SMCLK, ACLK, DCO on)
  //
  getTemp();
  _enable_interrupt();
  while(1){
     ;
  }
  return 0;
}
//////////////////////////////////
//
// Function Definitions
//
//////////////////////////////////
/**
  * void getTemp()
  *   Retrieves the temperature stored in the first two bytes of the DS18B20's
  *   scratchpad memory, and prints it to the top line of the LCD in F.
  * Parameters:
  *   none
  * Returns:
  *   void
  **/
void getTemp(){
   onewire_reset(&ow);
   onewire_write_byte(&ow, SKIP_ROM);
   onewire_write_byte(&ow, T_CONVER);
   // Wait for temperature conversion to complete
   while (!onewire_read_byte(&ow));
   // Read temperature from sensor scratchpad
   onewire_reset(&ow);
   onewire_write_byte(&ow, SKIP_ROM);
   onewire_write_byte(&ow, READ_SP);
   lower_temp = onewire_read_byte(&ow);
   upper_temp = onewire_read_byte(&ow);
   onewire_reset(&ow);
   // Convert C data from sensor to F
   temp = ((upper_temp << 4) | (lower_temp >> 4));
   farenheight = ((temp * 9) / 5) + 32;
   // Hard coded thermostat logic
   if (farenheight > thres_temp){
    RELAY_ON;
   }
   else{
    RELAY_OFF;
   }
   char temperature[16];
   sprintf(temperature, "Temp: %d%cF   ", farenheight, 0xDF);
   lcd_goHome();
   lcd_printString(temperature, NO_DELAY, TOP_LINE);
   return;
}
//////////////////////////////////
//
// Interrupt Service Routines
//
//////////////////////////////////
/**
  * TIMER0_A0_VECTOR: Timer A Interrupt Service Routine
  *     ISR for Timer A.  Called every 1.6 seconds--ensures 8Mhz MCLK,
  *     calls getTemp(), returns to previous LMP0.
  **/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void timerAISR(){
  // Reset WDT just in case
   WDTCTL = WDTPW + WDTHOLD;
  // Disable nested interrupts
  _disable_interrupt();
  TACCTL1 &= ~CCIFG;
  // Only execute bulk of ISR every 4 interrupts
  if (timer_counts == 20){
     timer_counts = 0;
     // Ensure 8Mhz clock
     if (CALBC1_8MHZ == 0XFF){
        while(1);
     }
      DCOCTL = 0;
      BCSCTL1 = CALBC1_8MHZ;
      DCOCTL = CALDCO_8MHZ;
      // getTemp()
      getTemp();
   }
   // If less than 4 interrupts have occured since last execution,
   // increment the interrupts counter
   else{
    timer_counts++;
   }
   // Offset CCR0
   CCR0 += 65535;
  // Re-enable interrupts and leave ISR
  _enable_interrupt();
}
/**
  * PORT2_VECOTR: Port 2 interrupt vector
  *     TEMP_UP:  increment the threshold temperature value
  *     TEMP_DWN: decrement the threshold temperature value
  *     ON_OFF:   toggle the status of the relay control
  **/
#pragma vector = PORT2_VECTOR
__interrupt void port2ISR(){
  // Disable nested interrupts
   _disable_interrupt();
  // Ensure 8Mhz clock
  if (CALBC1_8MHZ == 0XFF){
       while(1);
  }
  DCOCTL = 0;
  BCSCTL1 = CALBC1_8MHZ;
  DCOCTL = CALDCO_8MHZ;
  // Determine which button was pressed
  if (P2IFG & TEMP_UP){
   P2IFG &= ~0xFF;                      // Clear interrupt flags
   thres_temp++;                            // Increment threshold temp
   char thres[16];
   sprintf(thres, "Thresh: %d%cF   ", thres_temp, 0xDF);
   lcd_printString(thres, NO_DELAY, TOP_LINE);
  }
  else if (P2IFG & TEMP_DWN){
   P2IFG &= ~0xFF;                       // Clear interrupt flags
    thres_temp--;                            // Decrement threshold temp
    char thres[16];
    sprintf(thres, "Thresh: %d%cF   ", thres_temp, 0xDF);
    lcd_printString(thres, NO_DELAY, TOP_LINE);
  }
  else;
  // Re-enable interrupts, and leave ISR, returning to LMP0
  _enable_interrupt();
}
//////////////////////////////////////
//
// Trap all other interrupt vectors
//
//////////////////////////////////////
#pragma vector = PORT1_VECTOR
__interrupt void port1ISR(){while(1);}
#pragma vector = ADC10_VECTOR
__interrupt void adcISR(){while(1);}
#pragma vector = COMPARATORA_VECTOR
__interrupt void compAISR(){while(1);}
#pragma vector = NMI_VECTOR
__interrupt void nmiISR(){while(1);}
#pragma vector = TIMER0_A1_VECTOR
__interrupt void timerA01ISR(){while(1);}
#pragma vector = TIMER1_A0_VECTOR
__interrupt void timer1A0ISR(){while(1);}
#pragma vector = TIMER1_A1_VECTOR
__interrupt void timer1A1ISR(){while(1);}
#pragma vector = USCIAB0RX_VECTOR
__interrupt void usciAB0RXISR(){while(1);}
#pragma vector = USCIAB0TX_VECTOR
__interrupt void usciAB0TXISR(){while(1);}
#pragma vector = WDT_VECTOR
__interrupt void wdtISR(){while(1);}

