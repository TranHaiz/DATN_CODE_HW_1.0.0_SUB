/**
 * @file       memory_info.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-21
 * @author     Hai Tran
 *
 * @brief      Flash memory information
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _FLASH_INFORMATION_H_
#define _FLASH_INFORMATION_H_
/* Includes ----------------------------------------------------------- */
#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <stdint.h>

/* Public defines ----------------------------------------------------- */
// clang-format off
/*
 * ====================== FLASH MEMORY MAP ============================
 *
 *  Flash size: FULL_SIZE_FLASH = 512KB (0x80000)
 *
 *  0x08000000  ----------------------------------------------
 *  |                  Bootloader (32KB)                     |
 *  |  BOOTLOADER_START_ADRESS                              |
 *  |  Size: BOOT_LOADER_SIZE = 0x8000                      |
 *  0x08008000  ----------------------------------------------
 *  |                Application 1 (APP_1)                   |
 *  |  Start: APP_1_START_ADRESS                            |
 *  |  Size : APP_SIZE = (FULL_SIZE_FLASH - BOOT_LOADER_SIZE |
 *  |                     - DEVICE_INFO_SIZE) / 2 = 239KB    |
 *  |                     = 0x3BC00                          |
 *  0x08043C00  ----------------------------------------------
 *  |                Application 2 (APP_2)                   |
 *  |  Start: APP_2_START_ADRESS = APP_1_START_ADRESS        |
 *  |                       + APP_SIZE                       |
 *  |  Size : APP_SIZE = 239KB = 0x3BC00                     |
 *  0x0807F800  ----------------------------------------------
 *  |              Device Info (2KB - last page)             |
 *  |  Start: DEVICE_INFO_FLASH_ADDRESS                      |
 *  |  Size : DEVICE_INFO_SIZE = PAGE_SIZE_FLASH = 2KB       |
 *  0x08080000  ----------------------------------------------  End of flash
 *
 * ======================== SRAM MEMORY MAP ===========================
 *
 *  SRAM size: SRAM_FULL_SIZE = 128KB (0x20000)
 *
 *  0x20000000  ----------------------------------------------
 *  |            Application code/data in SRAM               |
 *  |  Start: SRAM_START_ADDRESS                            |
 *  |  Size : CODE_SRAM_SIZE = 0x1FE00                      |
 *  0x2001FE00  ----------------------------------------------
 *  |          Application information structure             |
 *  |  Start: APP_INFORMATION_START_ADDRESS                  |
 *  |  Size : APP_INFORMATION_SIZE = SRAM_FULL_SIZE          |
 *  |                              - CODE_SRAM_SIZE          |
 *  |                              = 512 bytes (0x200)       |
 *  0x20020000  ----------------------------------------------  End of SRAM
 */
// clang-format on

#define SRAM_START_ADDRESS            (0x20000000)  // Start address of SRAM
#define SRAM_FULL_SIZE                (0x20000)     // Full size SRAM
#define CODE_SRAM_SIZE                (0x1FE00)

#define PAGE_SIZE_FLASH               (2 * 1024)    // 2KB for page size flash memory
#define FULL_SIZE_FLASH               (512 * 1024)  // 256KB for full size flash memory

#define BOOT_LOADER_SIZE              (0x8000)     // 32KB for bootloader
#define BOOTLOADER_START_ADRESS       (0x8000000)  // Default start address MCU

#define APP_INFORMATION_SIZE          (SRAM_FULL_SIZE - CODE_SRAM_SIZE)  // 512B for application information
#define APP_INFORMATION_START_ADDRESS (SRAM_START_ADDRESS + CODE_SRAM_SIZE)

// Device info stored in Flash (last page of flash: page 255)
#define DEVICE_INFO_SIZE              (PAGE_SIZE_FLASH)  // 2KB for device info
#define DEVICE_INFO_FLASH_PAGE        (255)              // Last page (0x0807F800 - 0x0807FFFF)
#define DEVICE_INFO_FLASH_ADDRESS     (BOOTLOADER_START_ADRESS + (DEVICE_INFO_FLASH_PAGE * PAGE_SIZE_FLASH))
#define DEVICE_INFO_MAGIC_NUMBER      (0xDEADBEEFU)  // Magic number to verify valid data

// App size calculation: (Total Flash - Bootloader - Device Info) / 2
#define APP_SIZE                      ((FULL_SIZE_FLASH - BOOT_LOADER_SIZE - DEVICE_INFO_SIZE) / 2)  // 239KB each
#define APP_1_START_ADRESS            (BOOTLOADER_START_ADRESS + BOOT_LOADER_SIZE)
#define APP_2_START_ADRESS            (APP_1_START_ADRESS + APP_SIZE)

/* Public enumerate/structure ----------------------------------------- */
typedef enum
{
  BOOTLOADER_MODE_ENABLE  = 3,
  BOOTLOADER_MODE_DISABLE = 0,
} bootloader_mode_t;

typedef enum
{
  APP_UNKNOWN = 0,
  APP_1       = 1,
  APP_2       = 2,
} app_select_t;

typedef struct
{
  uint32_t          magic_number;  // Magic number to verify valid data
  bootloader_mode_t is_bootloader_mode;
  app_select_t      current_app;  // 1: app1, 2: app2
  uint32_t          app_1_size;
  uint32_t          app_2_size;
  uint32_t          app_1_crc;
  uint32_t          app_2_crc;
  uint32_t          checksum;  // Checksum for data integrity
} app_information_t;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
extern app_information_t *g_device_info;
extern app_information_t  g_device_info_cache;

/* Public function prototypes ----------------------------------------- */
/**
 * @brief      Initialize device info from Flash, load to RAM cache
 *             If Flash data is invalid, reset to default values
 */
void device_info_init(void);

/**
 * @brief      Save device info from RAM cache to Flash
 *             Call this before reboot to persist changes
 */
void device_info_save(void);

/**
 * @brief      Reset device info to default values and save to Flash
 */
void device_info_reset(void);

/**
 * @brief      Initialize RAM memory information (deprecated, use device_info_init)
 * @note       Kept for backward compatibility
 */
void ram_memory_info_init(void);

#endif /*End file _FLASH_INFORMATION_H_*/

/* End of file -------------------------------------------------------- */
