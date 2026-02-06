/**
 * @file       fs.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief     File System operations
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _FS_LIB_H_
#define _FS_LIB_H_
/* Includes ----------------------------------------------------------- */
#include "common_type.h"

#include <ff.h>

/* Public defines ----------------------------------------------------- */
#define FIL_MAX_PATH (40)
/* Public enumerate/structure ----------------------------------------- */
typedef struct
{
  const char *const Volume[_VOLUMES];
  char              VolumeLabel[_VOLUMES][16];
  const char *const CheckPoint[_VOLUMES];
} VolumeConfig_T;

typedef struct
{
  FATFS    FatFs[_VOLUMES];
  uint32_t StartSector[_VOLUMES];
} VolumeInfo_T;

typedef struct
{
  FIL *fhandle;
  char fpath[FIL_MAX_PATH];
} fil_custom_t;
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */

#endif /*End file _FS_LIB_H_*/

/* End of file -------------------------------------------------------- */
