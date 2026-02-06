/**
 * @file       bsp_fs.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief     BSP for File System operations
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _BSP_FS_H_
#define _BSP_FS_H_
/* Includes ----------------------------------------------------------- */
#include "common_type.h"
#include "fs.h"

#include <ff.h>

/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief  Initialize File System BSP
 */
status_function_t bsp_fs_init(void);

/**
 * @brief  Open file in file system
 */
void bsp_fs_open(fil_custom_t *file, const char *path, uint8_t ops, uint8_t partNum);

/**
 * @brief  Write data to file system
 */
void bsp_fs_write(fil_custom_t *file, uint8_t *data, uint32_t length);

/**
 * @brief  Read data from file system
 */
void bsp_fs_read(fil_custom_t *file, uint8_t *data, uint32_t length);

#endif /*End file _BSP_FS_H_*/

/* End of file -------------------------------------------------------- */
