/**
 * CPE329-07/08 | Project 3 - Environmental Sensing
 * OneWire Temperature sensor based thermostat
 * @author: Donny Percivalle
 *          Alex Lin
 */

#include "tempSensors.h"
/////////////////////////////
//
// main()
//
/////////////////////////////
int main()
{
  WDTCTL = WDTPW + WDTHOLD; //Stop WDT
  if (CALBC1_8MHZ == 0XFF){
    while(1);
  }
  DCOCTL = 0;
  BCSCTL1 = CALBC1_8MHZ;
  DCOCTL = CALDCO_8MHZ;

  TA0CTL = TASSEL_2 + MC_2 + ID_3; // Set timer to count SMCLK
                                   // Set mode to count UP to CCR0 value
                                   // Divide the 1MHz clock by 8 = 125kHz
  CCTL0 = CCIE;          // Enable timer interrupts
  CCR0 = 62500;          // Timer will count 62,500 clocks then interrupt
                         // 62500 * (1 / (1MHz / 8)) = 0.5s
                         // Timer interrupts every 0.5s
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
  ow.port_in = &P2IN;
  ow.port_ren = &P2REN;
  ow.port_dir = &P2DIR;
  ow.pin = BIT5;
  //
  // Configure the three two thermostat push buttons
  //
//   P2SEL = 0x00;
//   P2SEL2 = 0x00;
//   P2REN |= (TEMP_UP | TEMP_DWN | ON_OFF);
//   P2IE  |=  (TEMP_UP | TEMP_DWN | ON_OFF);
//   P2IES |= (TEMP_UP | TEMP_DWN | ON_OFF);
  //
  // Configure the relay control pin
  //
//   P2OUT |= RELAY_CTL;
//   P2DIR |= RELAY_CTL;
  //
  // Get the temperature, set threshold values, enable
  // interrupts, enter LPM0 (CPU off; SMCLK, ACLK, DCO on)
  //
  getTemp();
//  getAndSetThresholds();
//  _enable_interrupt();
  P2OUT &= ~BIT3;
  P2DIR |= BIT3;
  _low_power_mode_0();
  return 0;
}
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

   while (!onewire_read_byte(&ow));

   onewire_reset(&ow);
   onewire_write_byte(&ow, SKIP_ROM);
   onewire_write_byte(&ow, READ_SP);
   lower_temp = onewire_read_byte(&ow);
   upper_temp = onewire_read_byte(&ow);
   onewire_reset(&ow);
   DELAY_MS(200);
   temp = ((upper_temp << 4) | (lower_temp >> 4));
   farenheight = ((temp * 9) / 5) + 32;
   char temperature[16];
   sprintf(temperature, "Temp: %d%cF   ", farenheight, 0xDF);
   lcd_goHome();
   DELAY_MS(1000);
   lcd_printString(temperature, NO_DELAY, TOP_LINE);
   return;
}
/**
  * void getAndSetThresholds()
  *   Called when pushbuttons on port 2 are pressed--user changing thermostat
  *   settings.  Retrieves the current flag status from the DS18B20, and changes
  *   the relay control pin accordingly
  * Parameters:
  *   none
  * Returns:
  *   void
  **/
void getAndSetThresholds(){
  // Write the three configuration bytes to the scratchpad
  onewire_reset(&ow);
  onewire_write_byte(&ow, SKIP_ROM);
  onewire_write_byte(&ow, upper_temp);
  onewire_write_byte(&ow, lower_temp);
  onewire_write_byte(&ow, CONFIG_BYTE);
  // Update the temperature reading on the display
  getTemp();
  // Check for alarm status on the DS18B20, and change relay accordingly
}

/**
  * TIMER0_A0_VECTOR: Timer A Interrupt Service Routine
  *     ISR for Timer A.  Called every 1.6 seconds--ensures 8Mhz MCLK,
  *     calls getTemp(), returns to previous LMP0.
  **/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void timerAISR(){
  // Disable nested interrupts
   _disable_interrupt();
  // Ensure 8Mhz clock
  if (CALBC1_8MHZ == 0XFF){
       while(1);
  }
  DCOCTL = 0;
  BCSCTL1 = CALBC1_8MHZ;
  DCOCTL = CALDCO_8MHZ;
  // getTemp()
  getTemp();
  // Offset CCR0
  CCR0 += 62500;
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
    thres_temp++;                            // Increment threshold temp
    onewire_reset(&ow);                      // Resent one-wire bus
    onewire_write_byte(&ow, SKIP_ROM);       // Send skip-rom command
    onewire_write_byte(&ow, WRITE_SP);       // Send write scratchpad command
    onewire_write_byte(&ow, thres_temp);     // Transmit Th byte
    onewire_write_byte(&ow, thres_temp - 2); // Transmit Tl byte
    onewire_write_byte(&ow, CONFIG_BYTE);    // Transmit config byte
  }
  else if (P2IFG & TEMP_DWN){
    thres_temp++;                            // Increment threshold temp
    onewire_reset(&ow);                      // Resent one-wire bus
    onewire_write_byte(&ow, SKIP_ROM);       // Send skip-rom command
    onewire_write_byte(&ow, WRITE_SP);       // Send write scratchpad command
    onewire_write_byte(&ow, thres_temp);     // Transmit Th byte
    onewire_write_byte(&ow, thres_temp - 2); // Transmit Tl byte
    onewire_write_byte(&ow, CONFIG_BYTE);    // Transmit config byte
  }
  else if (P2IFG & ON_OFF){
    if (!is_relay_on){
      is_relay_on = TRUE;
      RELAY_ON;
    }
    else{
      is_relay_on = FALSE;
      RELAY_OFF;
    }
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

