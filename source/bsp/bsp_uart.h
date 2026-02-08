/**
 * @file       bsp_uart.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-02-08
 * @author     Hai Tran
 *
 * @brief      Generic UART BSP driver (DMA RX, DMA TX, IDLE interrupt)
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _BSP_UART_H_
#define _BSP_UART_H_

/* Includes ----------------------------------------------------------- */
#include "common_type.h"

/* Public defines ----------------------------------------------------- */

/* Public enumerate/structure ----------------------------------------- */
typedef void (*uart_cb)(void);

typedef struct
{
  UART_HandleTypeDef *huart;          // UART instance
  uint8_t            *buff;           // RX buffer
  uint32_t            size_buff;      // RX buffer size
  volatile uint32_t   rx_len;         // Received length
  uart_cb             idle_callback;  // IDLE callback
} bsp_uart_handle_t;

/* Public macros ------------------------------------------------------ */
#define BSP_UART_RX_RESET(h) \
  do                         \
  {                          \
    (h)->rx_len = 0;         \
  } while (0)

// Macro: Get received data size via DMA IDLE
#define BSP_UART_RX_SIZE(h) ((h)->rx_len)

/* Public variables --------------------------------------------------- */
extern bsp_uart_handle_t bsp_uart1_handle;
extern bsp_uart_handle_t bsp_uart2_handle;
extern bsp_uart_handle_t bsp_uart3_handle;

/* Public function prototypes ----------------------------------------- */
void              bsp_uart_init(bsp_uart_handle_t *h, uart_cb callback, uint8_t *buff, uint32_t size_buff);
status_function_t bsp_uart_write(bsp_uart_handle_t *h, const uint8_t *data, uint32_t len);
/**
 * @brief UART RX IDLE handler
 *        This function is called in stm32f4xx_it.c
 */
void bsp_uart_rx_handler(bsp_uart_handle_t *h);

#endif /*End file _BSP_UART_H_*/

/* End of file -------------------------------------------------------- */
