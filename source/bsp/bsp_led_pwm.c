/**
 * @file       bsp_led_pwm.c
 * @copyright  Copyright (C) 2025 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief     LED PWM BSP module source file
 *
 */

/* Includes ----------------------------------------------------------- */
#include "bsp_led_pwm.h"

/**
 * @brief Channel mapping for RGB LED
 *        Red   -> TIM2 Channel 1
 *        Green -> TIM2 Channel 2
 *        Blue  -> TIM2 Channel 3
 */

/* Private defines ---------------------------------------------------- */
#define TIM_HANDLER    htim2
#define MAX_DUTY_CYCLE (1000)  // 1KHz PWM frequency
#define ACTIVE_HIGH_LEVEL

/* Private enumerate/structure ---------------------------------------- */
typedef struct
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} led_color_info_t;

/* Private macros ----------------------------------------------------- */
// clang-format off
#define COLOR_LED_INFO(c, r, g, b) [c] = { .red = r, .green = g, .blue = b }
const led_color_info_t COLOR_LIST[LED_COLOR_MAX] = {
  //            +===================+=====+=======+======+
  //            |COLOR              | Red | Green | Blue |
  //            +-------------------+-----+-------+------+
  COLOR_LED_INFO(LED_COLOR_RED,     255,        0,    0  ),
  COLOR_LED_INFO(LED_COLOR_GREEN,     0,      255,    0  ),
  COLOR_LED_INFO(LED_COLOR_BLUE,      0,        0,  255  ),
  COLOR_LED_INFO(LED_COLOR_YELLOW,  255,      255,    0  ),
  COLOR_LED_INFO(LED_COLOR_PURPLE,  255,        0,  255  ), // Magenta
  COLOR_LED_INFO(LED_COLOR_CYAN,      0,      255,  255  ),
  COLOR_LED_INFO(LED_COLOR_WHITE,   255,      255,  255  ),
  //            +===================+=====+=======+======+
};
// clang-format on

#define SET_LED_DUTY_CYCLE(channel, duty_cycle)               \
  do                                                          \
  {                                                           \
    __HAL_TIM_SET_COMPARE(&TIM_HANDLER, channel, duty_cycle); \
  } while (0)

/* Public variables --------------------------------------------------- */
extern TIM_HandleTypeDef TIM_HANDLER;

/* Private variables -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
/* Function definitions ----------------------------------------------- */
void bsp_led_pwm_start()
{
  HAL_TIM_PWM_Start(&TIM_HANDLER, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&TIM_HANDLER, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&TIM_HANDLER, TIM_CHANNEL_3);
}

void bsp_led_pwm_stop()
{
  HAL_TIM_PWM_Stop(&TIM_HANDLER, TIM_CHANNEL_1);
  HAL_TIM_PWM_Stop(&TIM_HANDLER, TIM_CHANNEL_2);
  HAL_TIM_PWM_Stop(&TIM_HANDLER, TIM_CHANNEL_3);
}

void bsp_led_pwm_solid(led_color_t color, uint8_t brightness)
{
  if (color >= LED_COLOR_MAX)
  {
    return;
  }

#ifdef ACTIVE_HIGH_LEVEL
  uint16_t red_value   = (uint16_t) ((COLOR_LIST[color].red * brightness * MAX_DUTY_CYCLE) / (255 * 100));
  uint16_t green_value = (uint16_t) ((COLOR_LIST[color].green * brightness * MAX_DUTY_CYCLE) / (255 * 100));
  uint16_t blue_value  = (uint16_t) ((COLOR_LIST[color].blue * brightness * MAX_DUTY_CYCLE) / (255 * 100));
#elif ACTIVE_HIGH_LEVEL
  uint16_t red_value =
    MAX_DUTY_CYCLE - (uint16_t) ((COLOR_LIST[color].red * brightness * MAX_DUTY_CYCLE) / (255 * 100));
  uint16_t green_value =
    MAX_DUTY_CYCLE - (uint16_t) ((COLOR_LIST[color].green * brightness * MAX_DUTY_CYCLE) / (255 * 100));
  uint16_t blue_value =
    MAX_DUTY_CYCLE - (uint16_t) ((COLOR_LIST[color].blue * brightness * MAX_DUTY_CYCLE) / (255 * 100));
#endif

  SET_LED_DUTY_CYCLE(TIM_CHANNEL_1, red_value);
  SET_LED_DUTY_CYCLE(TIM_CHANNEL_2, green_value);
  SET_LED_DUTY_CYCLE(TIM_CHANNEL_3, blue_value);
}

void bsp_led_pwm_flash(led_color_t color, uint8_t brightness, uint16_t period_ms)
{
  // TODO: Implement LED flash functionality
}

void bsp_led_pwm_pulse(led_color_t color, uint8_t brightness, uint16_t pulse_ms)
{
  // TODO: Implement LED pulse functionality
}

/* Private definitions ----------------------------------------------- */
/* End of file -------------------------------------------------------- */
