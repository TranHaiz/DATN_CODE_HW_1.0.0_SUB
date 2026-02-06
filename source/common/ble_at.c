/**
 * @file       ble_at.c
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief      BLE AT command definitions
 *
 */

/* Includes ----------------------------------------------------------- */
#include "ble_at.h"

/* Private defines ---------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
// clang-format off
const uint32_t BAUD_RATE_LIST[BLE_MAX_BAUDRATE_SETTING] = {
    2400,
    4800,
    9600,
    19200,
    38400,
    57600,
    115200
};
// clang-format on

/* Private variables -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
/* Function definitions ----------------------------------------------- */
/* Private definitions ----------------------------------------------- */

/* End of file ------------------------------------------------------- */
