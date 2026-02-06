/**
 * @file       timer.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2025-8-8
 * @author     Hai Tran
 *
 * @brief      This is bsp layer for timer, base on HAL timer
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

/* Includes ----------------------------------------------------------- */
#include "stm32f4xx_hal.h"

#include <stdbool.h>

/* Public defines ----------------------------------------------------- */
// clang-format off
#define BSP_TIMER_DEFINE_INSTANCE (timx)  \
  extern TIM_HandleTypeDef htim##timx;    \
  timer_t timer##timx = {                 \
    .htim = &htim##timx,                  \
    .tim_irq = TIM##timx##_IRQn,          \
  };
// clang-format on

/* Public enumerate/structure ----------------------------------------- */
/**
 * @brief Alias for STM32 HAL Timer handle
 */
typedef TIM_HandleTypeDef timer_handle;

/**
 * @brief Function pointer used to callback
 */
typedef void (*timer_callback_t)(void);

/**
 * @brief Timer abstraction for BSP layer
 *
 * This structure defines a timer instance managed by the BSP.
 *
 */
typedef struct
{
  timer_handle    *htim;             /*!< Struct used to choose which timer is used*/
  IRQn_Type        tim_irq;          /*!< Timer interrupt number */
  timer_callback_t tim_cb;           /*!< system service callback for timer event */
  uint8_t          second_tick;      /*!< Current second counter */
  uint8_t          second_threshold; /*!< Threshold to generate timeout event */
} timer_t;

/* Public macros ------------------------------------------------------ */
#define bsp_timer_start_it(timer)    HAL_TIM_Base_Start_IT((timer)->htim)
#define bsp_timer_stop_it(timer)     HAL_TIM_Base_Stop_IT((timer)->htim)
#define bsp_timer_start(timer)       HAL_TIM_Base_Start((timer)->htim)
#define bsp_timer_get_counter(timer) __HAL_TIM_GET_COUNTER((timer)->htim)
#define bsp_timer_reset(timer)       __HAL_TIM_SET_COUNTER((timer)->htim, 0)
#define bsp_timer_stop(timer)        HAL_TIM_Base_Stop((timer)->htim)
#define bsp_timer_delay(miliseconds) HAL_Delay(miliseconds)

/* Public variables --------------------------------------------------- */
extern timer_t timer; /*!< Public structure of timer used in system service*/

/* Private variables -------------------------------------------------- */

/* Public function prototypes ----------------------------------------- */

/**
 * @brief  Initialize the BSP timer with callback and threshold
 *
 * @param[in]     timer   Pointer to BSP timer instance
 * @param[in]     tim_cb      Callback function to be called on timeout
 * @param[in]     second      Threshold in seconds before timeout
 *
 * @attention  Must be called before starting the timer. NVIC priority is also set here.
 */
void bsp_timer_init(timer_t *timer, timer_callback_t tim_cb, uint8_t second);

#endif  //__BSP_TIMER_H

/* End of file ------------------------------------------------------- */
