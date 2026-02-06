/**
 * @file       sys.c
 * @copyright  Copyright (C) 2025 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief     C source file for system-level functions
 *
 */

/* Includes ----------------------------------------------------------- */
#include "sys.h"

#include "stm32f4xx_hal.h"

/* Private defines ---------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
/* Private variables -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
/* Function definitions ----------------------------------------------- */
void sys_init(void)
{
  HAL_Init();
  __enable_irq();
  SystemClock_Config();
}
void sys_deinit(void)
{
  // Deinit port, pin

  // Deint clock source
  HAL_RCC_DeInit();

  // Deinit HAL
  HAL_DeInit();

  // Deinit SysTick
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL  = 0;
}
/* Private definitions ----------------------------------------------- */
/* End of file -------------------------------------------------------- */
