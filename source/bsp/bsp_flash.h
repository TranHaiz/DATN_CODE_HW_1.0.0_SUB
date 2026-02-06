/**
 * @file       bsp_flash.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2025-08-21
 * @author     Hai Tran
 *
 * @brief      Board support packet MCU's flash provide api to write and read
 * 			   flash memory.
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _BSP_FLASH_H
#define _BSP_FLASH_H

/* Includes ----------------------------------------------------------- */
#include <stdbool.h>
#include <stdint.h>
#include <stm32f4xx_hal.h>
#include <string.h>


/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief  Write data to internal Flash memory.
 *
 * This function programs a block of data into Flash starting
 * from the specified address. Flash must be erased before writing.
 *
 * @param[in]  addr   Start address in Flash memory.
 * @param[in]  data   Pointer to data buffer to be written.
 * @param[in]  size   Number of bytes to write.
 *
 * @return Bytes written
 */
uint32_t bsp_flash_write(uint32_t addr, void *data, uint32_t size);

/**
 * @brief  Read data from internal Flash memory.
 *
 * This function reads a block of data from Flash starting
 * from the specified address into the provided buffer.
 *
 * @param[in]  addr   Start address in Flash memory.
 * @param[out] data   Pointer to destination buffer.
 * @param[in]  size   Number of bytes to read.
 *
 * @return None
 */
void bsp_flash_read(uint32_t addr, void *data, uint32_t size);
;

/**
 * @brief  Erase Flash memory pages.
 *
 * This function erases one or more Flash pages starting
 * from the specified page address.
 *
 * @param[in]  start_page  Start address of the first page to erase.
 * @param[in]  num_pages        Number of pages to erase.
 *
 * @return
 *  - BSP_STORAGE_OP_DONE : Erase operation successful
 *  - BSP_STORAGE_OP_FAIL : Erase operation failed
 */
void bsp_flash_erase(uint32_t start_page, uint32_t num_pages);

#endif  // _BSP_FLASH_H

/* End of file -------------------------------------------------------- */
