/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 * @defgroup uart_example_main main.c
 * @{
 * @ingroup uart_example
 * @brief UART Example Application main file.
 *
 * This file contains the source code for a sample application using UART.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"
#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "UserFunctions.h"

#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */

#define UART_HWFC APP_UART_FLOW_CONTROL_DISABLED

#define HTTPRequestWAITMode 0x00
#define HTTPRequestGETMode  0x01
#define HTTPRequestPOSTMode 0x02


    uint32_t err_code;
    uint8_t TerminalFSM = 0x00;
    uint32_t outBufrSize = 0x00000000, inBufrSize = 0x00000000;
    uint8_t outBufr[UART_RX_BUF_SIZE];
    uint8_t inBufr[UART_TX_BUF_SIZE];
    uint8_t RequestRetransmit = 0x0A;

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

/*********************************************************************************************************
  Get request from terminal. If last three bytes of arrayis eq to 0x0d, 0x0a, 0x00, then outBufr is full.
    uint8_t *outBufr - pointer to output buffer

*********************************************************************************************************/

uint32_t GetReqFromTerminal(uint8_t *outBufr){
    uint8_t EndDataFlags = 0x00;
    uint32_t BufrSize = 0x00000000;
    while(EndDataFlags != 0x07){
        while (app_uart_get(*outBufr) != NRF_SUCCESS);
        if(EndDataFlags == 0x00){
            EndDataFlags = (*outBufr == 0x0D)? 0x01 : 0x00;
        }
        else if(EndDataFlags == 0x01){
            EndDataFlags = (*outBufr == 0x0A)? 0x03 : 0x00;
        }
        else if(EndDataFlags == 0x03){
            EndDataFlags = (*outBufr == 0x00)? 0x07 : 0x00;
        }
        outBufr = outBufr + BufrSize;
        BufrSize++;
    }
    return BufrSize;
}

/*******************************************************************************************************
  Put response to terminal.
    uint8_t *outBufr - pointer to output buffer
    uint32_t size - buffer siz in bytes

*******************************************************************************************************/
uint32_t PutRespToTerminal(uint8_t *inBufr, uint32_t size){
    uint32_t BufrSize = size;
    while(BufrSize--){
        while (app_uart_put(*inBufr) != NRF_SUCCESS);
        inBufr++;
    }
    return size;
}

/***********************************************************************************************************************/
int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);

    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          UART_HWFC,
          false,
          NRF_UART_BAUDRATE_115200
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);

    APP_ERROR_CHECK(err_code);

    while (true)
    {
        switch(TerminalFSM){
            case HTTPRequestWAITMode:{
                outBufrSize = GetReqFromTerminal(outBufr);
                if(outBufrSize){
                    TerminalFSM = ParseTerminalCMD(outBufr);
                }
                break;
            }
            case HTTPRequestGETMode:{
                while(!inBufrSize && RequestRetransmit--){
                    write(outBufr, outBufrSize);
                    inBufrSize = read(inBufr);
                }
                if(inBufrSize){
                    PutRespToTerminal(inBufr, inBufrSize);
                }
                RequestRetransmit = 0x0A;
                TerminalFSM = HTTPRequestWAITMode;
                break;
            }
            case HTTPRequestPOSTMode:{
                write(outBufr, outBufrSize);
                TerminalFSM = HTTPRequestWAITMode;
                break;
            }
            default:
                break;
        }
    }
}
