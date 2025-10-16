/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : config.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : Central project configuration for RTC selection, UART enables,
 *                RX buffer sizes, baud rates, protocol behavior (OK reply, auto
 *                upload, CRC). Uses a clean, consistent naming scheme and keeps
 *                backward-compatibility aliases for legacy macros.
 * ----------------------------------------------------------------------------- */
#define SELECT_RTC_TYPE 0 // RX8130=1;SD2058=2 ;No RTC=0

#define UART2_ENABLE 1 // T5L serial port 2 open and close settings, 1 opens, 0 closes
#define UART3_ENABLE 1 // T5L serial port 3 open and close settings, 1 opens, 0 closes
#define UART4_ENABLE 1 // T5L serial port 4 open and close settings, 1 to open, 0 to close
#define UART5_ENABLE 1 // T5L serial port 5 open and close settings, 1 opens, 0 closes

#define UART2_RX_LENTH 1024 // T5L serial port 2 receive array length
#define UART3_RX_LENTH 1024 // T5L serial port 3 receive array length
#define UART4_RX_LENTH 1024 // T5L serial port 4 receive array length
#define UART5_RX_LENTH 1024 // T5L serial port 5 receive array length

#define BAUD_UART2 115200 // T5L serial port 2 baud rate setting
#define BAUD_UART3 115200 // T5L serial port 3 baud rate setting
#define BAUD_UART4 115200 // T5L serial port 4 baud rate setting
#define BAUD_UART5 115200 // T5L serial port 5 baud rate setting

#define RESPONSE_UART2 1 // Serial port 2 response 4F4B is turned on and off, RESPONSE_UART2=1 is turned on, RESPONSE_UART2=0 is turned off
#define RESPONSE_UART3 1 // Serial port 3 response 4F4B is turned on and off, RESPONSE_UART3=1 is turned on, RESPONSE_UART3=0 is turned off
#define RESPONSE_UART4 1 // Serial port 4 response 4F4B is turned on and off, RESPONSE_UART4=1 is turned on, RESPONSE_UART4=0 is turned off
#define RESPONSE_UART5 1 // Serial port 5 response 4F4B is turned on and off, RESPONSE_UART5=1 is turned on, RESPONSE_UART5=0 is turned off

#define DATA_UPLOAD_UART2 1 // Serial port 2 data automatic upload setting, 1 uploads, 0 does not upload
#define DATA_UPLOAD_UART3 1 // Serial port 3 data automatic upload setting, 1 to upload, 0 not to upload
#define DATA_UPLOAD_UART4 1 // Serial port 4 data automatic upload setting, 1 to upload, 0 not to upload
#define DATA_UPLOAD_UART5 1 // Serial port 5 data automatic upload setting, 1 uploads, 0 does not upload

#define USE_CRC 0 // Whether to use CRC check, 1 is used, 0 is not used