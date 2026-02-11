/**
 * @file       bsp_sim.c
 * @copyright  Copyright (C) 2025 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-01-17
 * @author     Hai Tran
 *
 * @brief      BSP driver for SIM module (UART DMA Idle)
 *              - Init SIM module
 *              - API contact SMS
 *              - API call via phone number
 *              - API contact with firebase
 *
 */

/* Includes ----------------------------------------------------------- */
#include "bsp_sim.h"

#include "a7680c.h"
#include "os_lib.h"

/* Private defines ---------------------------------------------------- */
#define SIM_SEND_CMD_RETRY (3)
/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
#define SIM_SEND(data)     (bsp_uart_write(&SIM_UART_HANDLE, (data), strlen((data))))

/* Public variables --------------------------------------------------- */
static bool is_sim_rsp = false;

/* Private variables -------------------------------------------------- */
uint8_t           sim_rx_buffer[SIM_RX_BUFFER_SIZE];
volatile uint16_t sim_rx_len = 0;

static uint16_t old_pos = 0;
/* Private function prototypes ---------------------------------------- */
/**
 * @brief  Send command and wait for specific response or timeout
 */
static bool bsp_sim_send_and_wait_response(const char *cmd, const char *resp, uint32_t timeout);

/**
 * @brief  SIM response callback
 */
static void bsp_sim_rsp_callback(void);

/* Function definitions ----------------------------------------------- */
status_function_t bsp_sim_init(void)
{
  bsp_uart_init(&SIM_UART_HANDLE, bsp_sim_rsp_callback, sim_rx_buffer, SIM_RX_BUFFER_SIZE);

  SIM_SEND("ATE0\r\n");
  OS_DELAY_MS(50);

  // Tắt các URC không cần thiết
  bsp_sim_send_and_wait_response("AT*URCMODE=0\r\n", "OK", 1000);
  bsp_sim_send_and_wait_response("AT+CFUN=1\r\n", "OK", 2000);
  bsp_sim_send_and_wait_response("AT+CIURC=0\r\n", "OK", 1000);
  bsp_sim_send_and_wait_response("AT+CNMI=0,0,0,0,0\r\n", "OK", 1000);
  bsp_sim_send_and_wait_response("AT+CGEREP=0,0\r\n", "OK", 1000);
  bsp_sim_send_and_wait_response("AT+CREG=0\r\n", "OK", 1000);
  bsp_sim_send_and_wait_response("AT+CGREG=0\r\n", "OK", 1000);
  bsp_sim_send_and_wait_response("AT+STSF=0\r\n", "OK", 1000);

  bsp_sim_send_and_wait_response("AT\r\n", "OK", 2000);
  bsp_sim_send_and_wait_response("AT+CPIN?\r\n", "+CPIN: READY", 2000);
  bsp_sim_send_and_wait_response("AT+CREG?\r\n", "+CREG: 0,1", 3000);
  bsp_sim_send_and_wait_response("AT+CGATT?\r\n", "+CGATT: 1", 3000);
  bsp_sim_send_and_wait_response("AT+CGDCONT=1,\"IP\",\"v-internet\"\r\n", "OK", 2000);
  bsp_sim_send_and_wait_response("AT+CGACT=1,1\r\n", "OK", 3000);
  bool res = bsp_sim_send_and_wait_response("AT+HTTPINIT\r\n", "OK", 2000);
  if (res == false)
  {
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

status_function_t bsp_sim_send_data_firebase(sim_data_field_t field, void *data)
{
  char request[64]    = { 0 };
  char json_data[128] = { 0 };

  switch (field)
  {
  case SIM_DATA_FIELD_BATTERY_LEVEL:
    uint8_t *battery_level = (uint8_t *) data;
    sprintf(json_data, "{\"Battery level\":\"%d\"}", *battery_level);
    break;

  case SIM_DATA_FIELD_POSITION:
    // Do nothing
    break;

  default:
    // Case fail
    return STATUS_ERROR;
  }

  sprintf(request, "AT+HTTPDATA=%d,10000\r\n", strlen(json_data));

  bool res = false;

  res = bsp_sim_send_and_wait_response("AT+HTTPPARA=\"SSLCFG\",0\r\n", "OK", 100);
  if (res == false)
  {
    return STATUS_ERROR;
  }
  res = bsp_sim_send_and_wait_response(
    "AT+HTTPPARA=\"URL\",\"https://tracking-project-cf57e-default-rtdb.firebaseio.com/Device%201.json\"\r\n", "OK",
    100);
  if (res == false)
  {
    return STATUS_ERROR;
  }
  res = bsp_sim_send_and_wait_response("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n", "OK", 100);
  if (res == false)
  {
    return STATUS_ERROR;
  }
  res = bsp_sim_send_and_wait_response(request, "DOWNLOAD", 1000);
  if (res == false)
  {
    return STATUS_ERROR;
  }
  res = bsp_sim_send_and_wait_response(json_data, "OK", 1000);
  if (res == false)
  {
    return STATUS_ERROR;
  }
  res = bsp_sim_send_and_wait_response("AT+HTTPACTION=4\r\n", "+HTTPACTION: ", 15000);
  res = bsp_sim_send_and_wait_response("AT+HTTPREAD=0,100\r\n", "}", 1000);
  // res = bsp_sim_send_and_wait_response("AT+HTTPTERM\r\n", "OK", 100);
  // if (res == false)
  // {
  //   return STATUS_ERROR;
  // }

  return STATUS_OK;
}

/* Private definitions ----------------------------------------------- */
static bool bsp_sim_send_and_wait_response(const char *cmd, const char *resp, uint32_t timeout)
{
  SIM_SEND(cmd);
  uint32_t start_tick = OS_GET_TICK();
  while ((OS_GET_TICK() - start_tick) < timeout)
  {
    if (is_sim_rsp)
    {
      is_sim_rsp = false;
      if (strstr((const char *) sim_rx_buffer, resp))
      {
        return true;
      }
    }
    OS_YIELD();
  }
  return false;
}

static void bsp_sim_rsp_callback(void)
{
  is_sim_rsp = true;
}

/* End of file -------------------------------------------------------- */
