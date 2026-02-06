/**
 * @file       bootloader.c
 * @copyright  Copyright (C) 2025 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief      Bootloader C source file
 *
 */

/* Includes ----------------------------------------------------------- */
#include "bootloader.h"

#include "bsp_flash.h"
#include "bsp_uart_bootloader.h"
#include "sys.h"

/* Private defines ---------------------------------------------------- */
#define BLE_CONNECTED    "+CONNECTED"
#define BLE_DISCONNECTED "+DISCONNECTED"

/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
bootloader_info_t g_bootloader_info = { 0 };

/* Private variables -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
static bool check_info_request(uint8_t *buf, uint32_t rx_len);
static void bootloader_uart_rx_callback(void);

/* Function definitions ----------------------------------------------- */
void bootloader_init(void)
{
  bsp_uart_booot_init(&uart1_handle, bootloader_uart_rx_callback);
}

void bootloader_jump_to_app(uint32_t address)
{
  const JumpStruct *vector = (const JumpStruct *) address;

  if ((vector->stack_addr & 0x2FFE0000) != 0x20000000)
    return;

  __disable_irq();

  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL  = 0;

  for (uint32_t i = 0; i < 8; i++) NVIC->ICER[i] = 0xFFFFFFFF;

  SCB->VTOR = address;

  __set_MSP(vector->stack_addr);

  void (*app_reset)(void);
  app_reset = (void (*)(void)) vector->reset_handler;

  __enable_irq();

  app_reset();
}

void bootloader_handle_data_receive(uint8_t *buf, uint32_t len)
{
  if (memcmp(buf, BOOTLOADER_CMD_START, strlen(BOOTLOADER_CMD_START)) == 0)
  {
    g_bootloader_info.status = BOOTLOADER_DATA__RECEIVE_PROCESS;
  }
  else if (memcmp(buf, BOOTLOADER_CMD_END, strlen(BOOTLOADER_CMD_END)) == 0)
  {
    g_bootloader_info.status = BOOTLOADER_DATA_HANDLE_PROCESS;
  }
  else if (memcmp(buf, BOOTLOADER_CMD_UPDATE, strlen(BOOTLOADER_CMD_UPDATE)) == 0)
  {
    g_bootloader_info.is_update_app = true;
  }
  else if (check_info_request(buf, len))
  {
    g_bootloader_info.is_receive_data_info = true;
  }
  else if ((g_bootloader_info.curr_data_length + len) <= sizeof(g_bootloader_info.data_buf))
  {
    memcpy(&g_bootloader_info.data_buf[g_bootloader_info.curr_data_length], buf, len);
    g_bootloader_info.curr_data_length += len;
    g_bootloader_info.total_data_length += len;

    // Set flag data received
    g_bootloader_info.is_data_received = true;
  }
  else
  {
    // overflow → reset buffer
    g_bootloader_info.curr_data_length = 0;
  }
}

bool bootloader_checksum_check(uint32_t start_address, uint32_t size, uint8_t crc_value)
{
  uint8_t *data_ptr = (uint8_t *) start_address;
  uint8_t  crc_calc = 0;
  uint32_t i        = 0;
  while (i < size)
  {
    crc_calc += *(data_ptr + i);
    i++;
  }

  return (crc_calc == crc_value);
}

void bootloader_clean_flash_app_area(void)
{
  if (g_device_info->current_app == APP_1)
  {
    uint32_t start_page = APP_2_START_ADRESS / FLASH_PAGE_SIZE;
    bsp_flash_erase(start_page, APP_SIZE / FLASH_PAGE_SIZE);
  }
  else
  {
    uint32_t start_page = APP_1_START_ADRESS / FLASH_PAGE_SIZE;
    bsp_flash_erase(start_page, APP_SIZE / FLASH_PAGE_SIZE);
  }
}

/* Private definitions ----------------------------------------------- */
static bool check_info_request(uint8_t *rx_buf, uint32_t rx_len)
{
  if (rx_len < strlen(BOOTLOADER_CMD_INFO))
  {
    return false;
  }

  // Copy to local buffer for processing, add "/0" at the end
  uint8_t buf[64];
  if (rx_len >= sizeof(buf))
  {
    rx_len = sizeof(buf) - 1;
  }
  memcpy(buf, rx_buf, rx_len);
  buf[rx_len] = '\0';

  // Check INFO command
  if (strncmp((char *)buf, "INFO=", 5) != 0)
  {
    return false;
  }

  char *p = (char *)(buf + 5);

  /* Take SIZE */
  uint32_t size  = 0;
  char    *token = strtok(p, ",");
  if (token == NULL)
  {
    return false;
  }

  for (char *c = token; *c != '\0'; c++)
  {
    if (*c < '0' || *c > '9')
      return false;
    size = size * 10u + (uint32_t) (*c - '0');
  }

  /* Take CRC */
  token = strtok(NULL, ",");
  if (token == NULL)
  {
    return false;
  }

  uint32_t crc = 0;
  for (char *c = token; *c != '\0'; c++)
  {
    if (*c < '0' || *c > '9')
      return false;
    crc = crc * 10u + (uint32_t) (*c - '0');
  }

  if (g_device_info->current_app == APP_1)
  {
    g_device_info->app_2_size = size;
    g_device_info->app_2_crc  = crc;
  }
  else
  {
    g_device_info->app_1_size = size;
    g_device_info->app_1_crc  = crc;
  }

  return true;
}

static void bootloader_uart_rx_callback(void)
{
  // Return when USB connected
  if ((g_bootloader_info.modem == MODEM_USB) && (g_bootloader_info.status != BOOTLOADER_IDLE))
  {
    return;
  }

  // Hanle BLE data
  if (strncmp((char *) uart1_handle.buffer, BLE_CONNECTED, strlen(BLE_CONNECTED)) == 0)
  {
    g_bootloader_info.modem = MODEM_BLE;
    UART_BOOT_DISABLE_IDLE(uart1_handle);
    UART_BOOT_START_DMA(uart1_handle);
  }
  else if (strncmp((char *) uart1_handle.buffer, BLE_DISCONNECTED, strlen(BLE_DISCONNECTED)) == 0)
  {
    g_bootloader_info.modem = MODEM_USB;
    UART_BOOT_ENABLE_IDLE(uart1_handle);
    UART_BOOT_START_DMA(uart1_handle);
  }
  // Handle if received enough data
  else if (UART_BOOT_GET_IDLE_COUNT(uart1_handle) == sizeof(uart1_handle.buffer))
  {
    g_bootloader_info.is_data_received = true;
  }
}

/* End of file -------------------------------------------------------- */
