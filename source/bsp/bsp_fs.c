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
FIL    *g_file_handle;
FRESULT g_fs_last_result;

/* Private variables -------------------------------------------------- */
static VolumeConfig_T volume_cfg = {
  .Volume[0]      = "0:",
  .VolumeLabel[0] = "",
  .CheckPoint[0]  = SD_CHECKPOINT_0,
};
static VolumeInfo_T volume_info = {
  .FatFs          = { 0 },
  .StartSector[0] = 0,
};
static uint8_t work[4096] __attribute__((aligned(4)));
static FATFS   FatFs;

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

status_function_t bsp_fs_open(fil_custom_t *file, const char *path, uint8_t ops, uint8_t partNum)
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

  FRESULT result = f_open(&file->fhandle, pathTemp, ops);
  // Handle error if needed
  return (result == FR_OK) ? STATUS_OK : STATUS_ERROR;
}

void bsp_fs_rename(const char *oldPath, const char *newPath, uint8_t partNum)
{
  FRESULT result;
  assert_param(partNum < _VOLUMES);

  char pathTemp[SD_MAX_PATH];
  bsp_fs_make_path(partNum, oldPath, pathTemp);

  result = f_rename(pathTemp, newPath);
  // Handle error if needed
}

void bsp_fs_write(fil_custom_t *fp, void *buf, uint32_t btw)
{
  FRESULT res   = FR_OK;
  g_file_handle = &fp->fhandle;

  uint32_t bw;

  res = f_write(g_file_handle, buf, btw, &bw);
  /**
   * Handle error if needed
   * if (res != FR_OK || bw != btw)
   */
}

void bsp_fs_read(fil_custom_t *fp, void *buf, uint32_t btr)
{
  g_file_handle = &fp->fhandle;

  uint32_t br;

  g_fs_last_result = f_read(g_file_handle, buf, btr, &br);
  /**
   * Handle error if needed
   * if (g_fs_last_result != FR_OK || br != btr)
   */
}

void bsp_fs_delete(char *path, uint8_t partNum)
{
  assert_param(partNum < _VOLUMES);

  char pathTemp[SD_MAX_PATH];
  bsp_fs_make_path(partNum, path, pathTemp);

  g_fs_last_result = f_unlink(pathTemp);
  /**
   * Handle error if needed
   * if (g_fs_last_result != FR_OK)
   */
}

/* Private definitions ----------------------------------------------- */
static status_function_t bsp_fs_mount(void)
{
  // Mount SD card - single partition, use volume 0 only
  if (f_mount(&volume_info.FatFs[0], volume_cfg.Volume[0], 1) != FR_OK)
  {
    return false;
  }
  else
  {
    char label[128];

    f_getlabel(volume_cfg.Volume[0], label, 0);
    label[sizeof(label) - 1] = '\0';

    if (strcmp(label, volume_cfg.VolumeLabel[0]) != 0)
    {
      snprintf(label, sizeof(label), "%s%s", volume_cfg.Volume[0], volume_cfg.VolumeLabel[0]);
      f_setlabel(label);
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
  // Create folders - all on volume 0
  fres = f_mkdir("0:/LOG");
  if ((fres != FR_OK) && (fres != FR_EXIST))
  {
    return STATUS_ERROR;
  }

  fres = f_mkdir("0:/DATA");
  if ((fres != FR_OK) && (fres != FR_EXIST))
  {
    return STATUS_ERROR;
  }

  return STATUS_OK;
}

static void bsp_fs_make_path(uint8_t partNum, const char *path, char *pathBuild)
{
  (void) partNum;  // Not used - only 1 volume

  if (path != NULL)
  {
    // Check if path already has drive prefix "0:"
    if ((path[0] == volume_cfg.Volume[0][0]) && (path[1] == volume_cfg.Volume[0][1]))
    {
      strcpy(pathBuild, path);
    }
    else
    {
      sprintf(pathBuild, "0:/%s", path);
    }
  }
  else
  {
    sprintf(pathBuild, "0:/");
  }
}

/* End of file -------------------------------------------------------- */
