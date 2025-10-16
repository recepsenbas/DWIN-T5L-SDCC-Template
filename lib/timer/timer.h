/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : timer.c / timer.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : System timing library for DWIN T5L.
 *                Provides 1ms base tick, delay function, and UART timeout handling.
 * ----------------------------------------------------------------------------- */
#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#include "t5l1.h"
#include "uart.h"

extern volatile u32 t0_count;
extern volatile u32 t1_count;
extern volatile u16 sys_tick_ms;
extern volatile u16 sys_tick_rtc;
extern volatile u16 monitor_ms;

void delay_ms(u16 ms);
void Timer0_Init(void);
void Timer1_Init(void);
void Timer2_Init(void);
void Timer0_ISR(void) __interrupt(1);
void Timer1_ISR(void) __interrupt(3);
void Timer2_ISR(void) __interrupt(5);

#endif