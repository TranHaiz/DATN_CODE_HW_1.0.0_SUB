/**
 * @file       bsp_pwr.c
 * @copyright  Copyright (C) 2025 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief
 *
 */

/* Includes ----------------------------------------------------------- */
#include "bsp_pwr.h"

#include "stm32f4xx_hal.h"

/* Private defines ---------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
/* Private variables -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
/* Function definitions ----------------------------------------------- */
void bsp_pwr_init(void)
{
}

void bsp_pwr_reboot(void)
{
  HAL_NVIC_SystemReset();
}

/* Private definitions ----------------------------------------------- */
/* End of file -------------------------------------------------------- */
