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
extern FIL    *g_file_handle;
extern FRESULT g_fs_last_result;

/* Public function prototypes ----------------------------------------- */
/**
 * @brief  Initialize File System BSP
 */
status_function_t bsp_fs_init(void);

/**
 * @brief  Open file in file system
 */
status_function_t bsp_fs_open(fil_custom_t *file, const char *path, uint8_t ops, uint8_t partNum);

/**
 * @brief  Write data to file system
 */
void bsp_fs_write(fil_custom_t *fp, void *buf, uint32_t btw);

/**
 * @brief  Read data from file system
 */
void bsp_fs_read(fil_custom_t *fp, void *buf, uint32_t btr);

/**
 * @brief  Rename file in file system
 */
void bsp_fs_rename(const char *oldPath, const char *newPath, uint8_t partNum);

/**
 * @brief  Delete file in file system
 */
void bsp_fs_delete(char *path, uint8_t partNum);

#endif /*End file _BSP_FS_H_*/

/* End of file -------------------------------------------------------- */
