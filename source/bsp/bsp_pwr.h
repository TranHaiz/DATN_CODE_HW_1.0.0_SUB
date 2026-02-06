/**
 * @file       bsp_pwr.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-18
 * @author     Hai Tran
 *
 * @brief      MCU power management header file
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _BSP_PWR_H_
#define _BSP_PWR_H_
/* Includes ----------------------------------------------------------- */
/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
void bsp_pwr_init(void);
void bsp_pwr_reboot(void);

#endif /*End file _BSP_PWR_H_*/

/* End of file -------------------------------------------------------- */
