/**
 * @file       ble_at.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief      BLE AT command definitions
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _BLE_AT_COMMAND_H_
#define _BLE_AT_COMMAND_H_
/* Includes ----------------------------------------------------------- */
#include "common_type.h"

/* Public defines ----------------------------------------------------- */
#define BLE_AT_CMD               "AT\r\n"
#define BLE_AT_CMD_BAUDRATE      "AT+BAUD\r\n"
#define BLE_AT_CMD_NAME          "AT+NAME\r\n"
#define BLE_AT_CMD_RESET         "AT+RESET\r\n"
#define BLE_AT_CMD_SETBAUD(x)    "AT+BAUD" STRING2NUMBER(x) "\r\n"

#define BLE_AT_RESP_OK           "OK\r\n"

#define BLE_AT_CMD_PREFIX        "AT+"
#define BLE_AT_CMD_SUFFIX        "\r\n"

#define BLE_MAX_BAUDRATE_SETTING (7)  // Total 8 baud rate settings (2 to 8)

/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
extern const uint32_t BAUD_RATE_LIST[BLE_MAX_BAUDRATE_SETTING];

/* Public function prototypes ----------------------------------------- */

#endif /*End file _BLE_AT_COMMAND_H_*/

/* End of file -------------------------------------------------------- */
