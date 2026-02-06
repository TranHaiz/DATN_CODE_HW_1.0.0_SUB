/**
 * @file       bsp_uart_boootloader.c
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

/* Includes ----------------------------------------------------------- */
#include "bsp_uart_bootloader.h"

#include "ble_at.h"

/* Private defines ---------------------------------------------------- */
#define UART_DMA_IDLE_BUFFER_SIZE (1024)  // 1KB buffer size

/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Private variables -------------------------------------------------- */
uint8_t buffer_uart1[UART_DMA_IDLE_BUFFER_SIZE];

/* Public variables --------------------------------------------------- */
extern UART_HandleTypeDef huart1;
// clang-format off
uart_handle_t uart1_handle = {
	.is_data_ready = false,
	.data_position = 0,
	.buffer        = buffer_uart1,
	.handle        = &huart1
};
// clang-format on

/* Private function prototypes ---------------------------------------- */
uart_boot_cb uart_boot_callback = NULL;
static void  bsp_uart_boot_check_and_change_baud(uart_handle_t *huart);

/* Function definitions ----------------------------------------------- */
status_function_t bsp_uart_booot_init(uart_handle_t *huart, uart_boot_cb callback)
{
  if (huart == NULL)
  {
    return STATUS_ERROR;
  }

  // Check and change baud rate before start DMA receive
  bsp_uart_boot_check_and_change_baud(huart);

  status_function_t ret;
  uart_boot_callback = callback;
  ret                = (status_function_t) HAL_UART_Receive_DMA(huart->handle, huart->buffer, sizeof(huart->buffer));
  __HAL_UART_ENABLE_IT(huart->handle, UART_IT_IDLE);

  return ret;
}

status_function_t bsp_uart_booot_deinit(uart_handle_t *huart)
{
  status_function_t ret;
  ret = (status_function_t) HAL_UART_DMAStop(huart->handle);
  __HAL_UART_DISABLE_IT(huart->handle, UART_IT_IDLE);

  return ret;
}

status_function_t bsp_uart_boot_send_and_wait(uart_handle_t *huart, uint8_t *data, uint8_t *resp, uint16_t timeout)
{
  if (huart == NULL || data == NULL || resp == NULL)
  {
    return STATUS_ERROR;
  }

  // Send data
  status_function_t ret;
  ret = (status_function_t) HAL_UART_Transmit(huart->handle, data, strlen((char *) data), UART_BOOT_WRITE_TIMEOUT);
  if (ret != STATUS_OK)
  {
    return ret;
  }

  // Wait for response
  uint32_t start_tick = HAL_GetTick();
  uint32_t resp_index = 0;
  while ((HAL_GetTick() - start_tick) < timeout)
  {
    uint32_t idle_count  = UART_BOOT_GET_IDLE_COUNT(*huart);
    uint32_t data_length = sizeof(huart->buffer) - idle_count;
    while (resp_index < data_length)
    {
      if (huart->buffer[resp_index] == resp[resp_index])
      {
        resp_index++;
        if (resp[resp_index] == '\0')
        {
          // Full response received
          if (strcmp((char *) resp, BLE_AT_RESP_OK) == 0)
          {
            return STATUS_OK;
          }
          else
          {
            return STATUS_ERROR;
          }
        }
      }
      else
      {
        // Mismatch response
        return STATUS_ERROR;
      }

      HAL_Delay(1);
    }
  }

  return STATUS_TIMEOUT;
}

/* Private definitions ----------------------------------------------- */
void HAL_UART_IdleCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    // Call callback function if available
    if (uart_boot_callback != NULL)
    {
      uart_boot_callback();
    }
  }
}

static void bsp_uart_boot_check_and_change_baud(uart_handle_t *huart)
{
  for (uint8_t i = 2; i <= 8; i++)
  {
    if (bsp_uart_boot_send_and_wait(huart, BLE_AT_CMD, BLE_AT_RESP_OK, UART_BOOT_READ_TIMEOUT) == STATUS_OK)
    {
      // Change baud rate
      huart->handle->Init.BaudRate = BAUD_RATE_LIST[i];
      HAL_UART_Init(huart->handle);
      break;
    }
  }
}

/* End of file -------------------------------------------------------- */
