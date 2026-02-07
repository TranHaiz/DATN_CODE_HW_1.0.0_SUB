/**
 * @file       os_lib.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief     Operating System abstraction layer
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __LIB_H_
#define _FS_LIB_H_
/* Includes ----------------------------------------------------------- */
#include "cmsis_os2.h"
#include "common_type.h"

/* Public defines ----------------------------------------------------- */
/* Define one of: CONFIG_HAL_ONLY, CONFIG_FREE_RTOS */
#define CONFIG_FREE_RTOS
// #define CONFIG_FREE_RTOS  // Uncomment when osKernelStart() is enabled

#if defined(CONFIG_HAL_ONLY)
#define OS_DELAY_MS(ms) HAL_Delay(ms)
#elif defined(CONFIG_FREE_RTOS)  // FreeRTOS OS
#define OS_DELAY_MS(ms) osDelay(ms)
#define OS_GET_TICK()   osKernelGetTickCount()

/**
 * @brief  Declare thread (use at global scope, outside functions)
 * @param  thread_name  Thread name (identifier)
 * @param  priority     Priority: osPriorityLow(8) to osPriorityRealtime(48)
 * @param  stack_size   Stack size in bytes
 * @example OS_THREAD_DECLARE(FS_Task, osPriorityNormal, 512);
 */
#define OS_THREAD_DECLARE(thread_name, priority, stack_size)                      \
  osThreadId_t         thread_name##_handle;                                      \
  const osThreadAttr_t thread_name##_attr = {                                     \
    #thread_name, 0, NULL, 0, NULL, (stack_size), (osPriority_t) (priority), 0, 0 \
  }

/**
 * @brief  Create and start thread (use inside function, after osKernelInitialize)
 * @param  thread_name  Thread name (must match OS_THREAD_DECLARE)
 * @param  func         Thread function: void func(void *argument)
 * @example OS_THREAD_CREATE(FS_Task, test_fs);
 */
#define OS_THREAD_CREATE(thread_name, func) thread_name##_handle = osThreadNew((func), NULL, &thread_name##_attr)

#elif defined(CONFIG_PTOTO_THREAD)  // lightweight protothread OS
// Protothread delay implementation (to be defined)
#else
#error "Unsupported OS_TYPE"
#endif

/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */

#endif /*End file _FS_LIB_H_*/

/* End of file -------------------------------------------------------- */
