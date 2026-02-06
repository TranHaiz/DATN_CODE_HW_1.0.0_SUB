/**
 * @file       timer.c
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2025-8-8
 * @author     Hai Tran
 *
 * @brief      This is bsp layer for timer, base on HAL timer
 *
 */

/* Includes ----------------------------------------------------------- */
#include "bsp_timer.h"

#include "stm32f4xx_hal.h"

/* Private defines ---------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
/* Private variables -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
/* Function definitions ----------------------------------------------- */
void bsp_timer_init(timer_t *timer, timer_callback_t tim_cb, uint8_t second)
{
  assert_param(timer != NULL);
  assert_param(tim_cb != NULL);
  assert_param(second > 0);

  timer->tim_cb           = tim_cb; /*!< Assign user callback function */
  timer->second_threshold = second; /*!< Set threshold (in seconds) before triggering callback function */
}

/**
 * @brief  Period elapsed callback for TIM base interrupt.
 *
 * This function is automatically called by the HAL layer whenever
 * the specified timer (linked to timer) generates an update event.
 * It increments the internal tick counter and triggers the user-defined
 * callback once the configured threshold is reached.
 *
 * @param[in]     htim  Pointer to the timer handle that triggered the interrupt.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
}
/* Private definitions ----------------------------------------------- */

/* End of file ------------------------------------------------------- */
