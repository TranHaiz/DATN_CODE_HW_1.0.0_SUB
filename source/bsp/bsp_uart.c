/**
 * @file       bsp_uart.c
 * @copyright  Copyright (C) 2025 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-02-08
 * @author     Hai Tran
 *
 * @brief      Generic UART BSP driver (DMA RX, DMA TX, IDLE interrupt)
 *
 */

/* Includes ----------------------------------------------------------- */
#include "bsp_uart.h"

#include "usart.h"

/* Private defines ---------------------------------------------------- */
#define BSP_UART_MAX (3)  // Maximum number of UART instances

/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
// clang-format off
#define UART_INFO(x) \
[x-1] = &bsp_uart##x##_handle
// clang-format on

/* Public variables --------------------------------------------------- */
extern UART_HandleTypeDef huart1;
// extern UART_HandleTypeDef huart2;
// extern UART_HandleTypeDef huart3;

/* Private variables -------------------------------------------------- */
bsp_uart_handle_t bsp_uart1_handle = {
  .huart = &huart1,
};
// bsp_uart_handle_t bsp_uart2_handle = {
//   .huart = &huart2,
// };
// bsp_uart_handle_t bsp_uart3_handle = {
//   .huart = &huart3,
// };

static bsp_uart_handle_t *bsp_uart_list[BSP_UART_MAX] = {
  UART_INFO(1),
  // UART_INFO(2),
  // UART_INFO(3),
};

/* Private function prototypes ---------------------------------------- */
/**
 * @brief Find bsp_uart_handle_t by UART_HandleTypeDef
 */
bsp_uart_handle_t *bsp_uart_find_handle(UART_HandleTypeDef *huart);

/* Function definitions ----------------------------------------------- */
void bsp_uart_init(bsp_uart_handle_t *h, uart_cb callback, uint8_t *buff, uint32_t size_buff)
{
  h->buff          = buff;
  h->size_buff     = size_buff;
  h->rx_len        = 0;
  h->idle_callback = callback;
  // Start DMA RX
  HAL_UART_Receive_DMA(h->huart, h->buff, h->size_buff);
  __HAL_UART_ENABLE_IT(h->huart, UART_IT_IDLE);
}

status_function_t bsp_uart_write(bsp_uart_handle_t *h, const uint8_t *data, uint32_t len)
{
  status_function_t res = STATUS_OK;

  res = HAL_UART_Transmit_DMA(h->huart, data, len);

  return res;
}

void bsp_uart_rx_handler(bsp_uart_handle_t *h)
{
  __HAL_UART_CLEAR_IDLEFLAG(h->huart);
  HAL_UART_DMAStop(h->huart);
  h->rx_len = h->size_buff - __HAL_DMA_GET_COUNTER(h->huart->hdmarx);
  HAL_UART_Receive_DMA(h->huart, h->buff, h->size_buff);
  if (h->idle_callback)
  {
    h->idle_callback();
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  // Do nothing
}

/* Private definitions ----------------------------------------------- */
bsp_uart_handle_t *bsp_uart_find_handle(UART_HandleTypeDef *huart)
{
  for (uint8_t i = 0; i < BSP_UART_MAX; i++)
  {
    if (bsp_uart_list[i] == NULL)
      continue;

    if (bsp_uart_list[i]->huart == huart)
      return bsp_uart_list[i];
  }
  return NULL;
}

/* End of file -------------------------------------------------------- */