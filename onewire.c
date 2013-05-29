/*
 * CPE329-07/08 | Project 3 - Environmental Sensing
 * One-wire interface function library
 *
 * @author: Donny Percivalle
 *          Alex Lin
 */

#include <msp430.h>
#include "delay.h"
#include "onewire.h"
/**
  * Sends the reset signal to the provided one-wire peripheral.
  * Returns 0 if the process succeeded.
  **/
int onewire_reset(onewire_t *ow)
{
  onewire_line_low(ow);
  DELAY_US(550); // 480us minimum
  onewire_line_release(ow);
  DELAY_US(70); // slave waits 15-60us
  if (*(ow->port_in) & ow->pin) return 1; // line should be pulled down by slave
  DELAY_US(500); // slave TX presence pulse 60-240us
  if (!(*(ow->port_in) & ow->pin)) return 2; // line should be "released" by slave
  DELAY_US(50);
  return 0;
}

/**
  * Writes a bit out to the one-wire peripheral
  **/
void onewire_write_bit(onewire_t *ow, int bit)
{
  DELAY_US(2); // recovery, min 1us
  onewire_line_low(ow);
  if (bit)
    DELAY_US(6); // max 15us
  else
    DELAY_US(64); // min 60us
  onewire_line_release(ow);
  // rest of the write slot
  if (bit)
    DELAY_US(64);
  else
    DELAY_US(6);
}

/**
  * Reads a bit from the given one-wire peripheral
  **/
int onewire_read_bit(onewire_t *ow)
{
  int bit;
  DELAY_US(2); // recovery, min 1us
  onewire_line_low(ow);
  DELAY_US(5); // hold min 1us
  onewire_line_release(ow);
  DELAY_US(6); // 15us window
  bit = *(ow->port_in) & ow->pin;
  DELAY_US(60); // rest of the read slot
  return bit;
}

/**
  * Writes a full byte out to the provided one-wire peripheral
  **/
void onewire_write_byte(onewire_t *ow, uint8_t byte)
{
  int i;
  for(i = 0; i < 8; i++)
  {
    onewire_write_bit(ow, byte & 1);
    byte >>= 1;
  }
}

/**
  * Reads an entire bytes from the given one-wire peripheral
  **/
uint8_t onewire_read_byte(onewire_t *ow)
{
  int i;
  uint8_t byte = 0;
  for(i = 0; i < 8; i++)
  {
    byte >>= 1;
    if (onewire_read_bit(ow)) byte |= 0x80;
  }
  return byte;
}

/**
  * Holds the one-wire peripheral line low, persuant to the interface
  * specifications.
  **/
inline void onewire_line_low(onewire_t *ow)
{
  *(ow->port_dir) |= ow->pin;
  *(ow->port_out) &= ~ow->pin;
  *(ow->port_ren) &= ~ow->pin;
}

/**
  * Holds the one-wire peripheral line high, persuant to the interface
  * specifications.
  **/
inline void onewire_line_high(onewire_t *ow)
{
  *(ow->port_dir) |= ow->pin;
  *(ow->port_out) |= ow->pin;
  *(ow->port_ren) &= ~ow->pin;
}


/**
  * Relaeses the one-wrie peripheral line.  This is used when running in
  * parasitic power mode.
  **/
inline void onewire_line_release(onewire_t *ow)
{
  *(ow->port_dir) &= ~ow->pin;
  *(ow->port_ren) |= ow->pin;
  *(ow->port_out) |= ow->pin;
}

