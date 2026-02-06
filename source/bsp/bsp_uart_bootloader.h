/**
 * @file       bsp_uart_bootloader.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2012-08-12
 * @author     Hai Tran
 *
 * @brief      This board suppor package for uart peripheral for stm32l411 bootloader.
 *             Hanlde bootloader uart communication.
 *             Include: write and read use DMA and IDLE interrupt
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __UART_BOOTLOADER_H
#define __UART_BOOTLOADER_H

/* Includes ----------------------------------------------------------- */
#include "common_type.h"

/* Public defines ----------------------------------------------------- */
#define UART_BOOT_WRITE_TIMEOUT (100)  // 100 ms timeout
#define UART_BOOT_READ_TIMEOUT  (100)  // 100 ms timeout

/* Public enumerate/structure ----------------------------------------- */
typedef struct
{
  volatile bool       is_data_ready;
  volatile uint32_t   data_position;
  uint8_t            *buffer;
  UART_HandleTypeDef *handle;
} uart_handle_t;

typedef void (*uart_boot_cb)(void);  // Callback function when data ready

/* Public macros ------------------------------------------------------ */
// huart: uart_handle_t *
#define UART_BOOT_GET_IDLE_COUNT(huart) __HAL_DMA_GET_COUNTER((huart).handle->hdmarx)
#define UART_BOOT_ENABLE_IDLE(huart)    __HAL_UART_ENABLE_IT((huart).handle, UART_IT_IDLE)
#define UART_BOOT_DISABLE_IDLE(huart)   __HAL_UART_DISABLE_IT((huart).handle, UART_IT_IDLE)
#define UART_BOOT_START_DMA(huart)      HAL_UART_Receive_DMA((huart).handle, (huart).buffer, sizeof((huart).buffer))

/* Public variables --------------------------------------------------- */
extern uart_handle_t uart1_handle;

/* Public function prototypes ----------------------------------------- */
status_function_t bsp_uart_booot_init(uart_handle_t *huart, uart_boot_cb callback);
status_function_t bsp_uart_booot_deinit(uart_handle_t *huart);
status_function_t
bsp_uart_boot_send_and_wait_poll(uart_handle_t *huart, uint8_t *data, uint8_t *resp, uint16_t timeout);

#endif  // __UART_H

/* End of file -------------------------------------------------------- */
