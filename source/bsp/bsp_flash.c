/**
 * @file       bsp_flash.c
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

/* Includes ----------------------------------------------------------- */
#include "bsp_flash.h"

/* Private defines ---------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
/* Private variables -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
/* Function definitions ----------------------------------------------- */
uint32_t bsp_flash_write(uint32_t addr, void *data, uint32_t size)
{
  uint8_t *src = (uint8_t *) data;
  uint64_t dword;
  uint32_t bytes_written = 0;

  __disable_irq();

  if (HAL_OK != HAL_FLASH_Unlock())
  {
    return 0;
  }

  while (size > 0)
  {
    dword         = 0xFFFFFFFFFFFFFFFFULL;
    uint32_t copy = (size >= 8) ? 8 : size;
    memcpy(&dword, src, copy);

    if (HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, dword))
    {
      break;
      ;
    }

    addr += 8;
    src += copy;
    size -= copy;
    bytes_written += copy;
  }

  if (HAL_OK != HAL_FLASH_Lock())
  {
    // Do nothing
  }
  __enable_irq();

  return bytes_written;
}

void bsp_flash_read(uint32_t addr, void *data, uint32_t size)
{
  uint32_t *pSrc = (uint32_t *) addr;
  uint32_t *pDst = (uint32_t *) data;

  assert_param(size % 4 == 0);
  size = size / 4;  // byte -> word

  while (size--) *(pDst++) = *(pSrc++);
}

void bsp_flash_erase(uint32_t start_page, uint32_t num_pages)
{
  uint32_t               sectorError;
  FLASH_EraseInitTypeDef flashEraseConfig;

  __disable_irq();

  if (HAL_OK != HAL_FLASH_Unlock())
  {
    // Do nothing
  }

  flashEraseConfig.TypeErase = FLASH_TYPEERASE_PAGES;
  flashEraseConfig.Page      = start_page;
  flashEraseConfig.NbPages   = num_pages;

  if (HAL_OK != HAL_FLASHEx_Erase(&flashEraseConfig, &sectorError))
  {
    // Do nothing
  }

  if (HAL_OK != HAL_FLASH_Lock())
  {
    // Do nothing
  }

  __enable_irq();
}

/* Private definitions ----------------------------------------------- */

/* End of file -------------------------------------------------------- */
