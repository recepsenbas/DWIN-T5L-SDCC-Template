/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : vp_flags.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description :
 *    DGUS VP change-flag handler for event-driven UI.
 *    Allows lightweight detection of changed DGUS VPs (via UART RX ISR)
 *    and flag-based event handling in the main loop.
 * ----------------------------------------------------------------------------- */

/**
 * @file    vp_flags.h
 * @brief   DGUS VP change flags utility (header‑only).
 *
 * This header provides a lightweight mechanism to monitor a fixed set of DGUS
 * VPs (Virtual Parameters) and raise software flags whenever any of those VPs
 * are written/changed (typically detected inside a UART RX ISR or frame parser).
 *
 * How it works
 *  - Define the watch list in Flags_VPs[] and keep a parallel VP_Flag_States[] array.
 *  - Call Flags_SetByVP(vp) from your ISR/parser when you detect that VP 'vp' changed.
 *  - In the main loop, poll the flag macros (e.g., start_button_flag) or the
 *    VP_Flag_States[] entries, handle the event, then clear the flag (write 0).
 *
 * One-definition rule
 *  - In exactly ONE C file, define FLAGS_DEFINE before including this header so
 *    that the storage for Flags_VPs[] and VP_Flag_States[] is emitted there.
 *    Example (uart.c):
 *      #define FLAGS_DEFINE
 *      #include "app/vp_flags.h"
 *  - In all other files, simply:
 *      #include "app/vp_flags.h"
 *
 * Concurrency / ISR notes
 *  - VP_Flag_States[] lives in XRAM and is declared 'volatile' because it is written
 *    from ISRs and read from the main context.
 *  - Clearing a single-byte flag is effectively atomic on 8051. For multi-flag
 *    operations that must be atomic, briefly mask UART interrupts or wrap with
 *    EA save/restore as your system allows.
 *
 * Customization
 *  - Update FLAGS_COUNT and the initializer of Flags_VPs[] if you add/remove VPs.
 *  - The convenience macros (flag_1..flag_4, start_button_flag) map indices to
 *    readable names—rename them to match your UI semantics.
 *
 * Example usage
 *  // In UART RX ISR after parsing a complete frame:
 *  //   Flags_SetByVP(last_addr);
 *
 *  // In main loop:
 *  //   if (start_button_flag) {
 *  //       start_button_flag = 0;   // consume
 *  //       // ... handle Start action ...
 *  //   }
 */
#ifndef __VP_FLAGS_H__
#define __VP_FLAGS_H__

#include "t5l1.h"

/* One-TU storage selector: define FLAGS_DEFINE in exactly ONE C file */
#ifdef VP_FLAGS_DEFINE
#define VP_FLAGS_EXTERN
#else
#define VP_FLAGS_EXTERN extern
#endif

/* Change count here if you add/remove VPs */
#define VP_FLAGS_COUNT 5

/* --- Watch list of DGUS VPs --- */
#ifdef VP_FLAGS_DEFINE
const u16 Flags_VPs[VP_FLAGS_COUNT] = {0x2100, 0x2102, 0x2104, 0x2106, 0x3000};
#else
VP_FLAGS_EXTERN const u16 Flags_VPs[VP_FLAGS_COUNT];
#endif

/* --- Per-VP software flags (lives in XRAM, written in ISR) --- */
#ifdef VP_FLAGS_DEFINE
volatile __xdata u8 VP_Flag_States[VP_FLAGS_COUNT] = {0, 0, 0, 0, 0};
#else
VP_FLAGS_EXTERN volatile __xdata u8 VP_Flag_States[VP_FLAGS_COUNT];
#endif

/* --- Convenience macros for named flags --- */
// #define VP_FLAG_1 0
// #define VP_FLAG_2 1
// #define VP_FLAG_3 2
// #define VP_FLAG_4 3
// #define VP_FLAG_5 4
enum
{
    VP_FLAG_1 = 0,
    VP_FLAG_2,
    VP_FLAG_3,
    VP_FLAG_4,
    VP_FLAG_5
};

#define vp_flag_1 (VP_Flag_States[VP_FLAG_1])
#define vp_flag_2 (VP_Flag_States[VP_FLAG_2])
#define vp_flag_3 (VP_Flag_States[VP_FLAG_3])
#define vp_flag_4 (VP_Flag_States[VP_FLAG_4])
#define vp_start_button_flag (VP_Flag_States[VP_FLAG_5])

/* Helpers (header-inline) */
static inline void Flags_SetByVP(u16 vp)
{
    u8 i;
    for (i = 0; i < VP_FLAGS_COUNT; i++)
    {
        if (Flags_VPs[i] == vp)
        {
            VP_Flag_States[i] = 1;
            break;
        }
    }
}

static inline void VP_Flags_ClearAll(void)
{
    u8 i;
    for (i = 0; i < VP_FLAGS_COUNT; i++)
    {
        VP_Flag_States[i] = 0;
    }
}

#endif /* __VP_FLAGS_H__ */