/**
 * @file       bootloader.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief      Bootloader header file
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_
/* Includes ----------------------------------------------------------- */
#include "deivce_info.h"
#include "memory_info.h"
#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <stdint.h>

/* Public defines ----------------------------------------------------- */
#define BOOTLOADER_START_TIMEOUT        (30)    // 5 seconds timeout for bootloader
#define BOOTLOADER_PROGRESS_TIMEOUT     (120)   // 2 minutes progress time
#define BOOTLOADER_JUMP_NEW_APP_TIMEOUT (30)    // 30 seconds progress time
#define BOOTLOADER_DATA_BUF_SIZE        (1024)  // 1 KB data buffer size

#define BOOTLOADER_CMD_START            "START"
#define BOOTLOADER_CMD_END              "END"
#define BOOTLOADER_CMD_UPDATE           "UPDATE"
#define BOOTLOADER_CMD_INFO             "INFO="

#define BOOTLOADER_CHUNK_SIZE_RECEIVE   (512)

/* Public enumerate/structure ----------------------------------------- */
typedef enum
{
  BOOTLOADER_IDLE = 0,
  BOOTLOADER_DATA__RECEIVE_PROCESS,  // Receiving data process
  BOOTLOADER_DATA_HANDLE_PROCESS,    // Waiting for handling data process
} bootloader_status_t;

typedef struct
{
  // Timeout boolean information
  bool     is_timeout;
  uint16_t current_second;
  // Current status of bootloader
  bootloader_status_t status;
  // Received data information
  bool is_receive_data_info;
  // Handle data receive
  bool     is_data_received;
  uint16_t curr_data_length;
  uint32_t total_data_length;
  uint8_t  data_buf[BOOTLOADER_DATA_BUF_SIZE];
  // Is update Firmware
  bool    is_update_app;
  modem_t modem;
} bootloader_info_t;

typedef struct
{
  uint32_t stack_addr;
  uint32_t reset_handler;
} JumpStruct;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
extern bootloader_info_t g_bootloader_info;

/* Public function prototypes ----------------------------------------- */
/**
 * @brief      This function initializes the bootloader module
 */
void bootloader_init(void);

/**
 * @brief      This function jumps to application at specified address
 * @param[in]  address: Start address of application
 */
void bootloader_jump_to_app(uint32_t address);

/**
 * @brief his function handles data received in bootloader via USB CDC or other interface
 */
void bootloader_handle_data_receive(uint8_t *buf, uint32_t len);

/**
 * @brief      This function checks the CRC of application area
 * @return     true if CRC matches, false otherwise
 */
bool bootloader_checksum_check(uint32_t start_address, uint32_t size, uint8_t crc_value);

/**
 * @brief      This function cleans the flash area of the non-running application
 */
void bootloader_clean_flash_app_area(void);

#endif /*End file _BOOTLOADER_H_*/

/* End of file -------------------------------------------------------- */
