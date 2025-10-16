/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : addresses.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : DGUS VP address definitions for page control, curve buffers,
 *                NOR Flash access, and system reset. Values follow the DGUS
 *                development guide; comments reference the relevant pages.
 * ----------------------------------------------------------------------------- */
#ifndef __ADDRESSES_H__
#define __ADDRESSES_H__

#define PIC_Now_VP 0x0014 // Display current page ID.  Read only. p.50
#define PIC_Set_VP 0x0084 // 5A01 0001 sets page to 1.  p.52

// Curve control (DGUS curve widget)
#define Curve_Data_VP 0x0310 // Write start to curve  buffer. p. 191
#define Curve_Ch0_VP 0x0301  // Clear curve channel 0 buffer. p. 191
#define Curve_Ch1_VP 0x0303  // Clear curve channel 1 buffer
#define Curve_Ch2_VP 0x0305  // Clear curve channel 2 buffer
#define Curve_Ch3_VP 0x0307  // Clear curve channel 3 buffer
#define Curve_Ch4_VP 0x0309  // Clear curve channel 4 buffer
#define Curve_Ch5_VP 0x030B  // Clear curve channel 5 buffer
#define Curve_Ch6_VP 0x030D  // Clear curve channel 6 buffer
#define Curve_Ch7_VP 0x030F  // Clear curve channel 7 buffer

// System & storage
#define NOR_FLASH_RW_VP 0x0008 // Read and write to NOR flash. p. 48
#define System_Reset_VP 0x0004 // Write 0x5A to reset. p. 48

#endif // __ADDRESSES_H__