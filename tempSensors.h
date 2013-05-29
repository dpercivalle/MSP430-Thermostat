/**
 * CPE329-07/08 | Project 3 - Environmental Sensing
 * OneWire Temperature sensor based thermostat
 * Header file
 * @author: Donny Percivalle
 *          Alex Lin
 */

#ifndef TEMPSENSORS_H_
#define TEMPSENSORS_H_
#include <msp430.h>
#include <MSP430_lcd.c>
#include <stdio.h>
#include <intrinsics.h>
#include "onewire.h"
#include "delay.h"

#define CONFIG_BYTE 0x7F
#define COPY_SP     0x48
#define SKIP_ROM    0xCC
#define T_CONVER    0x44
#define READ_SP     0xBE
#define WRITE_SP    0x4E
#define ALARM_SRCH  0xEC
#define RECALL_E2   0xB8
#define CPY_SP      0x48
#define TEMP_UP     BIT6
#define TEMP_DWN    BIT7
#define RELAY_CTL   BIT3
#define RELAY_ON    (P2OUT &= ~BIT3)
#define RELAY_OFF   (P2OUT |= BIT3)

#define TRUE        1
#define FALSE       0

static volatile uint8_t thres_temp = 81;
static volatile uint8_t lower_temp = 0;
static volatile uint8_t upper_temp = 0;
static volatile uint8_t timer_counts = 0;
static volatile uint8_t temp = 0;
static volatile int is_relay_on = FALSE;
static volatile unsigned int farenheight = 0;
static onewire_t ow;

// Function prototypes
void getTemp();

#endif /* TEMPSENSORS_H_ */
