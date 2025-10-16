/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : uart.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : UART driver header for DWIN T5L/T5L51 (8051-core) controller.
 *                Defines prototypes, flags, and ISR declarations for UART2–UART5.
 * ----------------------------------------------------------------------------- */
#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include "t5l1.h"
#include "config.h"
#include "crc16.h"
#include "sys.h"
#include "timer.h"

extern __bit Response_flog;    // for response
extern __bit Auto_data_upload; // For automatic data upload
extern __bit Crc_check_flog;   // Crc check mark
extern __bit g_in_download_mode;
// uart.h
extern volatile __xdata u8 T_O2;
extern volatile __xdata u8 T_O3; // Port 3 timeout counter
extern volatile __xdata u8 T_O4; // Port 4 timeout counter
extern volatile __xdata u8 T_O5; // Port 5 timeout counter

void Uart_Init(void);
void uart2_ISR(void) __interrupt(4);
void uart3_ISR(void) __interrupt(16);
void uart4_RISR(void) __interrupt(11);
void uart5_RISR(void) __interrupt(13);

void uart_send_byte(u8 Uart_number, u8 Dat);
void uart_send_word(u8 Uart_number, u16 data);
void uart_send_str(u8 Uart_number, u8 *str);
void uart_send_arr(u8 Uart_number, u8 *arr, u8 len);
void DGUS_MonitorAndSendUpdates(void);
void DGUS_HandleCmd82(u8 uart, u8 *frame);
void DGUS_HandleCmd83(u8 uart, u8 *response, const u8 *request);
void DGUS_ParseUartFrame(u8 *rx_buf, u16 *data_len, u8 uart, __bit response, __bit crc_check);
void DGUS_ProcessAllUarts(void);
void uart_4_5_pin_ctrl(u8 uart_num, u8 state);
#endif