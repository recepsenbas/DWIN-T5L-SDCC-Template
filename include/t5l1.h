/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : t5l1.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description :
 *    T5L/T5L51 special function register (SFR) and bit mapping definitions
 *    for use with the SDCC compiler. Provides compatibility macros for
 *    IntelliSense and unified naming for DGUS and peripheral control.
 * ----------------------------------------------------------------------------- */
#ifndef __T5L1_H__
#define __T5L1_H__
#define u8 unsigned char
#define s8 signed char
#define u16 unsigned int
#define s16 signed int
#define u32 unsigned long
#define s32 signed long

/* ------------- IntelliSense helpers (editor-only) ------------- */
/* These avoid red squiggles in VS Code when SDCC is not the parser. */
#if !defined(__SDCC__) && defined(__INTELLISENSE__)
#ifndef __sfr __at()
#define __sfr __at() volatile unsigned char
#endif
#ifndef __sbit
#define __sbit volatile unsigned char
#endif
#ifndef __at
#define __at(x)
#endif
#ifndef __xdata
#define __xdata
#endif
#ifndef __idata
#define __idata
#endif
#ifndef __data
#define __data
#endif
#ifndef __bit
#define __bit unsigned char
#endif
#ifndef __interrupt
#define __interrupt(x)
#endif
#endif
/* --------------------------------------------------------------- */

// Special function registers (for SDCC)
__sfr __at(0x80) P0;   // Port 0
__sfr __at(0x90) P1;   // Port 1
__sbit __at(0x98) RI0; // SCON0 bit0: Receive Interrupt Flag (RI)
__sbit __at(0x99) TI0; // SCON0 bit1: Transmit Interrupt Flag (TI)

__sfr __at(0x87) PCON; // Power/Control (SMOD bit7 burada)
__sfr __at(0x89) TMOD; // Timer Mode (Timer0/1 modları)
__sfr __at(0x88) TCON; // Timer Control (TF1/TR1/TF0/TR0 vs.)

__sfr __at(0x8E) CKCON;     // Clock Control
__sfr __at(0x93) DPC;       // DPTR Cycle Control (MOVX sonrası DPTR ilerleme)
__sfr __at(0x94) PAGESEL;   // Code Memory Page Select (T5L specific)
__sfr __at(0x95) D_PAGESEL; // Data Memory Page Select (T5L specific)
__sfr __at(0xC9) MUX_SEL;   // Peripheral MUX Control (UART2/3/WDT seçimi)
__sfr __at(0xF8) RAMMODE;   // DGUS XRAM Interface Control (T5L specific)
__sbit __at(0xF8 + 7) APP_REQ;
__sbit __at(0xF8 + 6) APP_EN;
__sbit __at(0xF8 + 5) APP_RW;
__sbit __at(0xF8 + 4) APP_ACK;
__sfr __at(0xF1) ADR_H;
__sfr __at(0xF2) ADR_M;
__sfr __at(0xF3) ADR_L;
__sfr __at(0xF4) ADR_INC;
__sfr __at(0xFA) DATA3;
__sfr __at(0xFB) DATA2;
__sfr __at(0xFC) DATA1;
__sfr __at(0xFD) DATA0;
__sfr __at(0xF9) PORTDRV; // I/O Drive Strength Control (T5L specific)

__sfr __at(0x8D) TH1;
__sfr __at(0x8B) TL1;

__sfr __at(0xA8) IEN0;
__sbit __at(0xA8 + 7) EA;  // IEN0 bit7: Global Interrupt Enable
__sbit __at(0xA8 + 5) ET2; // IEN0 bit5: Timer2 Interrupt Enable
__sbit __at(0xA8 + 4) ES0; // IEN0 bit4: Serial Port 2 Interrupt Enable
__sbit __at(0xA8 + 3) ET1; // IEN0 bit3: Timer1 Interrupt Enable
__sbit __at(0xA8 + 2) EX1; // IEN0 bit2: External Interrupt 1 Enable
__sbit __at(0xA8 + 1) ET0; // IEN0 bit1: Timer0 Interrupt Enable
__sbit __at(0xA8 + 0) EX0; // IEN0 bit0: External Interrupt 0 Enable

__sfr __at(0xB8) IEN1;
__sbit __at(0xB8 + 5) ES5R; /*****UART5 accepts interrupt enable control bit****/
__sbit __at(0xB8 + 4) ES5T; /*****UART5 accepts interrupt enable control bit****/
__sbit __at(0xB8 + 3) ES4R; /*****UART4 accepts interrupt enable control bit****/
__sbit __at(0xB8 + 2) ES4T; /*****UART4 accepts interrupt enable control bit****/
__sbit __at(0xB8 + 1) ECAN; /********CAN interrupt enable control bit******/
__sfr __at(0x9A) IEN2;
__sfr __at(0xA9) IP0;
__sfr __at(0xB9) IP1;

__sfr __at(0xB7) P0MDOUT; // Port 0 Output Mode Control
__sfr __at(0xBC) P1MDOUT; // Port 1 Output Mode Control
__sfr __at(0xBD) P2MDOUT; // Port 2 Output Mode Control
__sfr __at(0xBE) P3MDOUT; // Port 3 Output Mode Control
#define RTC_DIR_REG P3MDOUT

__sfr __at(0xB9) IP1; /********Interrupt priority controller 0*******/
__sfr __at(0xBF) IRCON2;
__sfr __at(0xC0) IRCON;
__sbit __at(0xC0 + 6) TF2; /********T2 interrupt trigger flag*******/
__sfr __at(0xC8) T2CON;    /********T2 control register********/
__sbit __at(0xC8 + 0) TR2; /***********T2 enable***********/
__sfr __at(0xCB) TRL2H;
__sfr __at(0xCA) TRL2L;
__sfr __at(0xCD) TH2;
__sfr __at(0xCC) TL2;
__sfr __at(0xA0) P2;
__sfr __at(0xB0) P3;

__sbit __at(0xB0 + 3) RTC_SDA;
__sbit __at(0xB0 + 2) RTC_SCL;

__sfr __at(0x89) TMOD; /********T0 T1 mode selection, same as 8051*******/
__sfr __at(0x8C) TH0;
__sfr __at(0x8A) TL0;
__sfr __at(0x8D) TH1;
__sfr __at(0x8B) TL1;

// TCON=0x88 /********T0 T1 control register*******/
__sbit __at(0x88 + 7) TF1; /********T1 ifnterrupt trigger*******/
__sbit __at(0x88 + 6) TR1;
__sbit __at(0x88 + 5) TF0; /********T0 interrupt trigger*******/
__sbit __at(0x88 + 4) TR0;
__sbit __at(0x88 + 3) IE1; /********External interrupt 1*******/
__sbit __at(0x88 + 2) IT1; /********External interrupt 1 trigger mode 0: Low level trigger 1: Falling edge trigger*******/
__sbit __at(0x88 + 1) IE0; /********External interrupt 0*******/
__sbit __at(0x88 + 0) IT0; /********External interrupt 0 trigger mode 0: Low level trigger 1: Falling edge trigger*******/

__sfr __at(0x98) SCON2;
__sbit __at(0x98 + 1) TI2; // UART2 transmit interrupt flag
__sbit __at(0x98 + 0) RI2; // UART2 receive interrupt flag
__sfr __at(0x99) SBUF2;    /********UART2 transceiver data interface*******/
__sfr __at(0xBA) SREL2H;   /********Set the baud rate, when ADCON is 0x80*******/
__sfr __at(0xAA) SREL2L;
__sfr __at(0xD8) ADCON;

__sfr __at(0x9B) SCON3; /********UART3 control interface*******/
__sbit __at(0x9B + 1) TI3;
__sbit __at(0x9B + 0) RI3;
__sfr __at(0x9C) SBUF3;
__sfr __at(0xBB) SREL3H;
__sfr __at(0x9D) SREL3L;

// UART4
__sfr __at(0x96) SCON4T;      /******UART4 send control********/
__sbit __at(0x96 + 0) TI4;    // UART4 send interrupt flag
__sfr __at(0x97) SCON4R;      /******UART4 receive control*********/
__sbit __at(0x97 + 0) RI4;    // UART4 receive interrupt flag
__sfr __at(0xD9) BODE4_DIV_H; /******Baud rate setting********/
__sfr __at(0xE7) BODE4_DIV_L;
__sfr __at(0x9E) SBUF4_TX; /******UART4 sending data interface********/
__sfr __at(0x9F) SBUF4_RX; /******UART4 receiving data interface*********/
__sfr __at(0x80) P0;       // Port 0
__sbit __at(0x80 + 0) TR4; /******485 direction control of port 4**********/
// UART5
__sfr __at(0xA7) SCON5T;
__sbit __at(0xA7 + 0) TI5; // UART5 send interrupt flag
__sfr __at(0xAB) SCON5R;
__sbit __at(0xAB + 0) RI5; // UART5 receive interrupt flag
__sfr __at(0xAE) BODE5_DIV_H;
__sfr __at(0xAF) BODE5_DIV_L;
__sfr __at(0xAC) SBUF5_TX;
__sfr __at(0xAD) SBUF5_RX;
__sbit __at(0x80 + 1) TR5; /******485 direction control of port 5**********/
// CAN communication
__sfr __at(0x8F) CAN_CR;
__sfr __at(0x91) CAN_IR;
__sfr __at(0xE8) CAN_ET;

// Definition of system main frequency and 1ms timing value
#define DTHD1 0X5A // Frame header 1
#define DTHD2 0XA5 // Frame header 2
#define FOSC 206438400UL
#define FRAME_LEN 255 // Frame length

#endif // __T5L51_H__