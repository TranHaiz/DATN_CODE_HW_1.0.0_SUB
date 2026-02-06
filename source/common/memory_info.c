/**
 * @file       memory_info.c
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-21
 * @author     Hai Tran
 *
 * @brief      Flash memory information - Store device info in Internal Flash
 *
 */

/* Includes ----------------------------------------------------------- */
#include "memory_info.h"

#include "bsp_flash.h"
#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Private defines ---------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
// RAM cache for device info (to avoid frequent Flash reads)
app_information_t  g_device_info_cache = { 0 };
app_information_t *g_device_info       = &g_device_info_cache;

/* Private variables -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
static uint32_t calculate_checksum(const app_information_t *info);
static bool     is_device_info_valid(const app_information_t *info);

/* Function definitions ----------------------------------------------- */

/**
 * @brief      Calculate checksum for device info structure
 * @param[in]  info  Pointer to device info structure
 * @return     Checksum value
 */
static uint32_t calculate_checksum(const app_information_t *info)
{
  uint32_t       checksum = 0;
  const uint8_t *data     = (const uint8_t *) info;

  // Calculate checksum for all fields except checksum field itself
  for (size_t i = 0; i < offsetof(app_information_t, checksum); i++)
  {
    checksum += data[i];
  }
  return checksum;
}

/**
 * @brief      Validate device info data from Flash
 * @param[in]  info  Pointer to device info structure
 * @return     true if data is valid, false otherwise
 */
static bool is_device_info_valid(const app_information_t *info)
{
  // Check magic number
  if (info->magic_number != DEVICE_INFO_MAGIC_NUMBER)
  {
    return false;
  }

  // Check checksum
  if (info->checksum != calculate_checksum(info))
  {
    return false;
  }

  // Check valid app values
  if (info->current_app != APP_1 && info->current_app != APP_2 && info->current_app != APP_UNKNOWN)
  {
    return false;
  }

  return true;
}

void device_info_init(void)
{
  // Read from Flash directly into RAM cache first
  // Note: bsp_flash_read requires 4-byte aligned destination
  bsp_flash_read(DEVICE_INFO_FLASH_ADDRESS, &g_device_info_cache, sizeof(app_information_t));

  if (!is_device_info_valid(&g_device_info_cache))
  {
    // Invalid data (first run or corrupted), reset to default
    device_info_reset();
  }
}

void device_info_save(void)
{
  // Update magic number and checksum before saving
  g_device_info_cache.magic_number = DEVICE_INFO_MAGIC_NUMBER;
  g_device_info_cache.checksum     = calculate_checksum(&g_device_info_cache);

  // Erase Flash page before writing
  bsp_flash_erase(DEVICE_INFO_FLASH_PAGE, 1);

  // Write data to Flash
  bsp_flash_write(DEVICE_INFO_FLASH_ADDRESS, (uint8_t *) &g_device_info_cache, sizeof(app_information_t));
}

void device_info_reset(void)
{
  g_device_info_cache.magic_number       = DEVICE_INFO_MAGIC_NUMBER;
  g_device_info_cache.is_bootloader_mode = BOOTLOADER_MODE_DISABLE;
  g_device_info_cache.current_app        = APP_1;
  g_device_info_cache.app_1_size         = 0;
  g_device_info_cache.app_2_size         = 0;
  g_device_info_cache.app_1_crc          = 0;
  g_device_info_cache.app_2_crc          = 0;
  g_device_info_cache.checksum           = calculate_checksum(&g_device_info_cache);

  // Save to Flash
  device_info_save();
}

// Backward compatibility - deprecated function
void ram_memory_info_init(void)
{
  device_info_init();
}

/* End of file -------------------------------------------------------- */
