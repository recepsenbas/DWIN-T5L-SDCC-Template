/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : timer.c / timer.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : System timing library for DWIN T5L.
 *                Provides 1ms base tick, delay function, and UART timeout handling.
 * ----------------------------------------------------------------------------- */
#include "timer.h"

volatile u32 t0_count = 0;
volatile u32 t1_count = 0;
volatile u16 sys_tick_ms = 0;
volatile u16 sys_tick_rtc = 0;
volatile u16 monitor_ms = 0;

void delay_ms(u16 ms)
{
    u16 start = sys_tick_ms;
    while ((u16)(sys_tick_ms - start) < ms)
        ;
}

void Timer0_Init(void)
{
    TMOD &= 0xF0; // clear lower nibble
    TMOD |= 0x01; // Timer0 mode 1 (16-bit)
    TH0 = 0xBC;
    TL0 = 0xCD; // 1ms timer
    ET0 = 1;    // enable Timer0 interrupt
    TR0 = 1;    // start Timer0
}

void Timer1_Init(void)
{
    TMOD &= 0x0F; // clear upper nibble
    TMOD |= 0x10; // Timer1 mode 1 (16-bit)
    TH1 = 0xBC;
    TL1 = 0xCD; // 1ms timer
    ET1 = 1;    // enable Timer1 interrupt
    TR1 = 1;    // start Timer1
}

void Timer2_Init(void)
{
    TH2 = 0x00;
    TL2 = 0x00;
    T2CON = 0x70;
    TRL2H = 0xBC; // 1ms timer
    TRL2L = 0xCD;
    IEN0 |= 0x20; // Start timer 2
    TR2 = 0x01;
    ET2 = 1; // T2 timer interrupt enable control bit
    EA = 1;
}

// Timer0 interrupt service routine (1ms interval)
void Timer0_ISR(void) __interrupt(1)
{
    TH0 = 0xBC;
    TL0 = 0xCD;
    t0_count++;
}
// Timer1 interrupt service routine (1ms interval)
void Timer1_ISR(void) __interrupt(3)
{
    TH1 = 0xBC;
    TL1 = 0xCD;
    t1_count++;
}
// Timer2 interrupt service routine (1ms interval)
void Timer2_ISR(void) __interrupt(5)
{
    TF2 = 0;
    sys_tick_ms++;
    monitor_ms++;
    sys_tick_rtc++;

#if UART5_ENABLE
    if (T_O5 > 0)
        T_O5--;
#endif
#if UART4_ENABLE
    if (T_O4 > 0)
        T_O4--;
#endif
#if UART3_ENABLE
    if (T_O3 > 0)
        T_O3--;
#endif
#if UART2_ENABLE

    if (T_O2 > 0)
        T_O2--;
#endif
}