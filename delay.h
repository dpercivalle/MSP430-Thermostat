/*
 * CPE329-07/08 | Project 3 - Environmental Sensing
 * One-wire interface delay definitions
 *
 * @author: Donny Percivalle
 *          Alex Lin
 */

#ifndef __DELAY_H
#define __DELAY_H

#define CYCLES_PER_US 8L // depends on the CPU speed
#define CYCLES_PER_MS (CYCLES_PER_US * 1000L)

#define DELAY_US(x) __delay_cycles((x * CYCLES_PER_US))
#define DELAY_MS(x) __delay_cycles((x * CYCLES_PER_MS))

#endif
