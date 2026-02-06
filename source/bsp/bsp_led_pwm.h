/**
 * @file       bsp_led_pwm.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief     LED PWM BSP module header file
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _BSP_LED_PWM_H_
#define _BSP_LED_PWM_H_
/* Includes ----------------------------------------------------------- */
#include "common_type.h"

/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
typedef enum
{
  LED_COLOR_RED = 0,
  LED_COLOR_GREEN,
  LED_COLOR_BLUE,
  LED_COLOR_YELLOW,
  LED_COLOR_PURPLE,
  LED_COLOR_CYAN,
  LED_COLOR_WHITE,
  LED_COLOR_MAX
} led_color_t;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
void bsp_led_pwm_start();
void bsp_led_pwm_stop();
void bsp_led_pwm_solid(led_color_t color, uint8_t brightness);
void bsp_led_pwm_flash(led_color_t color, uint8_t brightness, uint16_t period_ms);
void bsp_led_pwm_pulse(led_color_t color, uint8_t brightness, uint16_t pulse_ms);

#endif /*End file _BSP_LED_PWM_H_*/

/* End of file -------------------------------------------------------- */
