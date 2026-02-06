/**
 * @file       bsp_fs.c
 * @copyright  Copyright (C) 2025 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief     BSP for File System operations
 *
 */

/* Includes ----------------------------------------------------------- */
#include "bsp_fs.h"

#include "sd_card.h"

#include <fatfs.h>
#include <ff.h>

/* Private defines ---------------------------------------------------- */
#define SD_CHECKPOINT_0    "checkpoint-0"
#define SD_CHECKPOINT_1    "checkpoint-1"
#define SD_DEVICE_NAME     "Tracing_Device"
#define SD_MOUNT_RETRY_MAX (5)

/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
/* Private variables -------------------------------------------------- */
static VolumeConfig_T volume_cfg = {
  .Volume[0]      = "0:",
  .VolumeLabel[0] = "",
  .CheckPoint[0]  = SD_CHECKPOINT_0,
  .Volume[1]      = "1:",
  .VolumeLabel[1] = SD_DEVICE_NAME,
  .CheckPoint[1]  = SD_CHECKPOINT_1,
};
static VolumeInfo_T volume_info = {
  .FatFs          = { 0 },
  .StartSector[0] = 0,
  .StartSector[1] = 0,
};
static uint8_t work[4096] __attribute__((aligned(4)));
static FATFS   FatFs;
static FIL     File;

/* Private function prototypes ---------------------------------------- */

/**
 * @brief  Initialize device folders
 */
status_function_t bsp_fs_init_folders(void);

/**
 * @brief  Mount file system
 */
static status_function_t bsp_fs_mount(void);

/**
 * @brief  Format file system
 */
static status_function_t bsp_fs_format(void);

/**
 * @brief  Build full path for file operations
 */
static void bsp_fs_make_path(uint8_t partNum, const char *path, char *pathBuild);

/* Function definitions ----------------------------------------------- */
status_function_t bsp_fs_init(void)
{
  // Check SD detect pin
  // if (!bsp_io_read(IO_SD_DETECT))
  // {
  //   return STATUS_ERROR;
  // }

  // Build Public Drive Label

  // Mount, Check flag, Checkpoint
  for (uint8_t i = 0; i < SD_MOUNT_RETRY_MAX; i++)
  {
    if ((bsp_fs_mount() == STATUS_OK) && (bsp_fs_init_folders() == STATUS_OK))
    {
      return STATUS_OK;
    }
    else
    {
      // Try format
      if (bsp_fs_format() == STATUS_OK)
      {
        if ((bsp_fs_mount() == STATUS_OK) && (bsp_fs_init_folders() == STATUS_OK))
        {
          return STATUS_OK;
        }
      }
    }
  }

  return STATUS_ERROR;
}

void bsp_fs_open(fil_custom_t *file, const char *path, uint8_t ops, uint8_t partNum)
{
  uint32_t len  = strlen(path);
  uint32_t size = sizeof(file->fpath);

  assert_param(partNum < _VOLUMES);

  char pathTemp[SD_MAX_PATH];
  bsp_fs_make_path(partNum, path, pathTemp);

  if (len < size)
  {
    strcpy(file->fpath, path);
  }
  else
  {
    strcpy(file->fpath, &path[len - size + 1]);
  }

  FRESULT result = f_open(file->fhandle, pathTemp, ops);
  // Handle error if needed
}

/* Private definitions ----------------------------------------------- */
static status_function_t bsp_fs_mount(void)
{
  // Mount SD card
  for (uint_fast8_t i = 0; i < _VOLUMES; i++)
  {
    if (f_mount(&volume_info.FatFs[i], volume_cfg.Volume[i], 1) != FR_OK)
    {
      return false;
    }
    else
    {
      char label[128];

      f_getlabel(volume_cfg.Volume[i], label, 0);
      label[sizeof(label) - 1] = '\0';

      if (strcmp(label, volume_cfg.VolumeLabel[i]) != 0)
      {
        snprintf(label, sizeof(label), "%s%s", volume_cfg.Volume[i], volume_cfg.VolumeLabel[i]);
        f_setlabel(label);
      }
    }
  }

  return true;
}

/**
 * @brief  Format file system
 */
static status_function_t bsp_fs_format(void)
{
  FRESULT fres;
  FRESULT fres_mkfs = f_mkfs((TCHAR const *) USERPath, FM_FAT32, 0, work, sizeof(work));
  if (fres_mkfs == FR_OK)
  {
    // Mount lại sau khi format
    fres = f_mount(&FatFs, (TCHAR const *) USERPath, 1);
  }
  else
  {
    fres = fres_mkfs;  // Pass error to main fres
  }

  return (fres == FR_OK) ? true : false;
}

status_function_t bsp_fs_init_folders(void)
{
  FRESULT fres;
  // Create folders
  fres = f_mkdir("0:/LOG");
  if ((fres != FR_OK) && (fres != FR_EXIST))
  {
    return STATUS_ERROR;
  }

  fres = f_mkdir("1:/DATA");
  if ((fres != FR_OK) && (fres != FR_EXIST))
  {
    return STATUS_ERROR;
  }

  return STATUS_OK;
}

static void bsp_fs_make_path(uint8_t partNum, const char *path, char *pathBuild)
{
  assert_param(partNum < _VOLUMES);

  if (path != NULL)
  {
    if (((path[0] == (volume_cfg.Volume[0][0])) && (path[1] == volume_cfg.Volume[0][1]))
        || ((path[0] == (volume_cfg.Volume[1][0])) && (path[1] == volume_cfg.Volume[1][1])))
    {
      strcpy(pathBuild, path);
    }
    else
    {
      sprintf(pathBuild, "%d:%s", partNum, path);
    }
  }
  else
  {
    sprintf(pathBuild, "%d:", partNum);
  }
}

/* End of file -------------------------------------------------------- */
