/**
 * @file       .h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief     Header file for system-level functions
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _SYS_H_
#define _SYS_H_
/* Includes ----------------------------------------------------------- */
/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief Initial peripherals and system settings
 */
void sys_init(void);

/**
 * @brief Deinitialize peripherals and system settings
 */
void sys_deinit(void);

#endif /*End file _SYS_H_*/

/* End of file -------------------------------------------------------- */
