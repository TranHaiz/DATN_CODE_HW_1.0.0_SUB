/**
 * @file       common_type.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief      Common type definitions used across the project
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _COMMON_TYPE_H_
#define _COMMON_TYPE_H_
/* Includes ----------------------------------------------------------- */
#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
typedef enum
{
  STATUS_OK = 0,
  STATUS_ERROR,
  STATUS_BUSY,
  STATUS_TIMEOUT
} status_function_t;

/* Public macros ------------------------------------------------------ */
#define STRING2NUMBER(x) #x
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */

#endif /*End file _COMMON_TYPE_H_*/

/* End of file -------------------------------------------------------- */
