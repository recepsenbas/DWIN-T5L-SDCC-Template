/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : uart_flags.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description :
 *    DGUS UART change flag utility (header-only).
 *    Provides a lightweight mechanism to detect updated UART-linked VP values
 *    and raise software flags accordingly (usually from UART RX ISR or frame
 *    parser). Flags can be polled and cleared in the main loop.
 * ----------------------------------------------------------------------------- */

/**
 * @file    uart_flags.h
 * @brief   DGUS uart change flags utility (header‑only).
 *
 * This header provides a lightweight mechanism to monitor received UART data
 * and raise software flags whenever any of those addresses are written/changed
 * (typically detected inside a UART RX ISR or frame parser).
 *
 * How it works
 *  - Define the watch list in Flags_Uart[] and keep a parallel Flag_States[] array.
 *  - Call Flags_SetByUart(vp) from your ISR/parser when you detect that UART data changed.
 *  - In the main loop, poll the flag macros (e.g., start_button_flag) or the
 *    Flag_States[] entries, handle the event, then clear the flag (write 0).
 *
 * One-definition rule
 *  - In exactly ONE C file, define UART_FLAGS_DEFINE before including this header so
 *    that the storage for Flags_Uart[] and Flag_States[] is emitted there.
 *    Example (uart.c):
 *      #define UART_FLAGS_DEFINE
 *      #include "uart_flags.h"
 *  - In all other files, simply:
 *      #include "uart_flags.h"
 *
 * Concurrency / ISR notes
 *  - Flag_States[] lives in XRAM and is declared 'volatile' because it is written
 *    from ISRs and read from the main context.
 *  - Clearing a single-byte flag is effectively atomic on 8051. For multi-flag
 *    operations that must be atomic, briefly mask UART interrupts or wrap with
 *    EA save/restore as your system allows.
 *
 * Customization
 *  - Update FLAGS_COUNT and the initializer of Flags_Uart[] if you add/remove addresses.
 *  - The convenience macros (flag_1..flag_4,g) map indices to
 *    readable names—rename them to match your UI semantics.
 *
 * Example usage
 *  // In UART RX ISR after parsing a complete frame:
 *  //   Flags_SetByUart(last_addr);
 *
 *  // In main loop:
 *  //   if (start_button_flag) {
 *  //       start_button_flag = 0;   // consume
 *  //       // ... handle Start action ...
 *  //   }
 */
#ifndef __UART_FLAGS_H__
#define __UART_FLAGS_H__

#include "t5l1.h"

/* One-TU storage selector: define UART_FLAGS_DEFINE in exactly ONE C file */
#ifdef UART_FLAGS_DEFINE
#define UART_FLAGS_EXTERN
#else
#define UART_FLAGS_EXTERN extern
#endif

/* Change count here if you add/remove VPs */
#define UART_FLAGS_COUNT 5

/* --- Watch list of DGUS UARTs --- */
#ifdef UART_FLAGS_DEFINE
const u16 Flags_Uart[UART_FLAGS_COUNT] = {0x2100, 0x2102, 0x2104, 0x2106, 0x3000};
#else
UART_FLAGS_EXTERN const u16 Flags_Uart[UART_FLAGS_COUNT];
#endif

/* --- Per-UART software flags (lives in XRAM, written in ISR) --- */
#ifdef UART_FLAGS_DEFINE
volatile __xdata u8 Uart_Flag_States[UART_FLAGS_COUNT] = {0, 0, 0, 0, 0};
#else
UART_FLAGS_EXTERN volatile __xdata u8 Uart_Flag_States[UART_FLAGS_COUNT];
#endif

/* --- Convenience macros for named access --- */
// #define Uart_FLAG_1 0
// #define Uart_FLAG_2 1
// #define Uart_FLAG_3 2
// #define Uart_FLAG_4 3
// #define Uart_FLAG_5 4
enum
{
    Uart_FLAG_1 = 0,
    Uart_FLAG_2,
    Uart_FLAG_3,
    Uart_FLAG_4,
    Uart_FLAG_5
};

#define uart_flag_1 (Uart_Flag_States[Uart_FLAG_1])
#define uart_flag_2 (Uart_Flag_States[Uart_FLAG_2])
#define uart_flag_3 (Uart_Flag_States[Uart_FLAG_3])
#define uart_flag_4 (Uart_Flag_States[Uart_FLAG_4])
#define uart_start_flag (Uart_Flag_States[Uart_FLAG_5])

/* Helpers (header-inline) */
static inline void Flags_SetByUart(u16 vp)
{
    u8 i;
    for (i = 0; i < UART_FLAGS_COUNT; i++)
    {
        if (Flags_Uart[i] == vp)
        {
            Uart_Flag_States[i] = 1;
            break;
        }
    }
}

static inline void Uart_Flags_ClearAll(void)
{
    u8 i;
    for (i = 0; i < UART_FLAGS_COUNT; i++)
    {
        Uart_Flag_States[i] = 0;
    }
}

#endif /* __UART_FLAGS_H__ */