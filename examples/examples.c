/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : examples.c
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description :
 *    Self-contained usage examples for DGUS + UART helpers. This file is
 *    optional and can be excluded from production builds. Keep examples
 *    simple, non-blocking, and focused on API demonstration.
 * ----------------------------------------------------------------------------- */

#ifdef T5L_EXAMPLES
void examples(void)
{
    /* WRITE ARRAY TO DGUS REGISTERS */
    u8 buf_out[20] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x50};
    DGUS_WriteBytes(0x2000, buf_out, 20);

    /* READ ARRAY FROM DGUS REGISTERS */
    u8 buf_in[20] = {0};
    DGUS_ReadBytes(0x2000, buf_in, 20);
    /* SEND ARRAY OVER UART2 */
    uart_send_arr(2, buf_in, 20); // uart in-->31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50

    /* DGUS TEXT EXAMPLE */
    /* WRITE TEXT TO DGUS REGISTERS */
    /* You can see "HELLO WORLD" on the screen if you link VP 0x2000 to a text component. */
    DGUS_WriteText(0x2000, "HELLO WORLD");

    /* UART SEND EXAMPLES */
    /* SEND BYTE OVER UART2 */
    uart_send_byte(2, 0x55);
    /* SEND WORD OVER UART2 */
    uart_send_word(2, 0x1234);
    /* SEND STRING OVER UART2 */
    uart_send_str(2, "Hello\r\n"); // HEX:48 65 6c 6c 6f 0d 0a

    /* WRITE and READ TO DGUS REGISTERS */
    /* First Write a u16 to DGUS VP 0x2000 */
    DGUS_Write_VP(0x2000, 0x1234);
    // Then Read it back
    u16 v = DGUS_Read_VP(0x2000);
    // Send the read value over UART2
    uart_send_word(2, v); // Should send 0x1234

    /* DGUS PAGE SWITCH EXAMPLE */
    /* Get the current DGUS page ID */
    u8 page_id = DGUS_GetPageID();
    // Switch to page 5
    DGUS_SetPageID(5);

    /* -----------NOR FLASH ----------- */
    u8 nor_src[20] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    u8 nor_dst[20] = {0};
    DGUS_WriteBytes(0x2000, nor_src, 20); // write array to DGUS RAM
    DGUS_NOR_Write(0x000000, 0x2000, 20); // write from DGUS RAM to NOR Flash
    DGUS_NOR_Read(0x000000, 0x2100, 20);  // read from NOR Flash to DGUS RAM
    DGUS_ReadBytes(0x2100, nor_dst, 20);  // read from DGUS RAM to array
    uart_send_arr(2, nor_dst, 20);        // send array over UART2
    // should see 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14
    /* -------------------------------- */

    /* REAL-TIME CURVES */
    /* should be set corresponding to DGUS side components */
    for (u16 i = 0; i < 100; i++)
        DGUS_GraphPush(0, i);
    DGUS_GraphClear(0); // clear channel 0

    /* FLAGS HANDLING */
    /* If received UART has flags address, its flag will be set */
    if (uart_start_flag == 1)
    {
        uart_start_flag = 0;
        uart_send_str(2, "Start Button Pressed\r\n");
    }
    /* If start button is pressed, its flag will be set */
    if (vp_start_button_flag == 1)
    {
        vp_start_button_flag = 0;
        uart_send_str(2, "Start Button Pressed\r\n");
    }
}
#endif /* T5L_EXAMPLES */
