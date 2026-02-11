/**
 * @file       sd_card.c
 * @copyright  Copyright (C) 2025 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-02-06
 * @author     Hai Tran
 *
 * @brief      SD card driver file.
 *
 */

/* Includes ----------------------------------------------------------- */
#include "sd_card.h"

#include "device_info.h"
#include "os_lib.h"
#include "spi.h"

#include <string.h>

#if SD_USE_DMA && defined(CONFIG_FREE_RTOS)
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "semphr.h"

#endif

/* Private defines ---------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
#define SD_CARD_ENABLE_CS()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define SD_CARD_DISABLE_CS() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

/* Public variables --------------------------------------------------- */
extern SPI_HandleTypeDef hspi1;

#ifdef CONFIG_SD_DEBUG_MODE
// Debug variables - check in debugger
volatile uint32_t sd_write_count    = 0;  // Total writes
volatile uint32_t sd_write_fail     = 0;  // Write failures
volatile uint8_t  sd_last_write_err = 0;  // 1=CMD fail, 2=response fail, 3=busy timeout, 4=DMA error
volatile uint32_t sd_dma_tx_cplt    = 0;  // DMA TX complete count
volatile uint32_t sd_dma_rx_cplt    = 0;  // DMA RX complete count
volatile uint32_t sd_dma_txrx_cplt  = 0;  // DMA TX/RX complete count
volatile uint32_t sd_dma_error      = 0;  // DMA error count
/**
 * @brief  Track initialization progress for debugging
 *        1: start init
 *        2: DMA buffers ready
 *        3: SPI ready, sending dummy clocks
 *        4: sending CMD0
 *        5: CMD0 OK, checking card type
 *        10: init complete
 *        100+: error occurred
 */
volatile uint8_t sd_init_step     = 0;     // Track init progress for debugging
volatile uint8_t sd_cmd0_response = 0xFF;  // Last CMD0 response for debugging
#endif

/* Private variables -------------------------------------------------- */
static uint8_t SD_Type = 0;

// DMA status flag
static volatile SD_DMA_Status_t sd_dma_status = SD_DMA_IDLE;

#if SD_USE_DMA && defined(CONFIG_FREE_RTOS)
// Binary semaphore for DMA completion signaling (use FreeRTOS native for ISR safety)
static SemaphoreHandle_t sd_dma_sem_handle = NULL;
static StaticSemaphore_t sd_dma_sem_buffer;
#endif

// DMA buffers - MUST be word-aligned for DMA
__attribute__((aligned(4))) static uint8_t sd_dma_dummy_tx[512];  // Filled with 0xFF for receive
__attribute__((aligned(4))) static uint8_t sd_dma_dummy_rx[512];  // Dummy receive for transmit

/* Private function prototypes ---------------------------------------- */
/**
 * @brief  Set SPI speed: 0=slow (init), 1=fast (data)
 *         Set SPI speed - slow for init, fast for data transfer
 *         APB2 = 96MHz: /256 = 375kHz (init), /32 = 3MHz (data - more stable for breadboard)
 */
static void spi_set_speed(uint8_t speed);

/**
 * @brief  SPI transmit/receive single byte (blocking)
 */
static uint8_t spi_read_write(uint8_t data);

/**
 * @brief  Wait for SD card ready (increased timeout for write operations)
 */
static uint8_t sd_card_wait_ready(void);

/**
 * @brief  Send command to SD card (CS must already be LOW)
 *         For CMD0 during init, skip_wait=1 to avoid WaitReady
 */
static void sd_card_init_dma_buffer(void);

/**
 * @brief  Standard SendCmd with WaitReady
 */
static uint8_t sd_card_cmd(uint8_t cmd, uint32_t arg);

/**
 * @brief  Send command to SD card (CS must already be LOW)
 *         For CMD0 during init, skip_wait=1 to avoid WaitReady
 */
static uint8_t sd_card_send_cmd_ex(uint8_t cmd, uint32_t arg, uint8_t skip_wait);

/**
 * @brief  Receive data via DMA
 */
static uint8_t sd_card_dma_tx(uint8_t *buff, uint16_t len);

/**
 * @brief  Receive data via DMA
 */
static uint8_t sd_card_dma_receive(uint8_t *buff, uint16_t len);

/* Function definitions ----------------------------------------------- */
uint8_t sd_card_init(void)
{
  uint8_t  i, res;
  uint16_t retry;
  uint8_t  buf[4];

#ifdef CONFIG_SD_DEBUG_MODE
  sd_init_step = 1;  // Start init
#endif

#if SD_USE_DMA
  sd_card_init_dma_buffer();  // Initialize DMA buffers early

#if defined(CONFIG_FREE_RTOS)
  // Create binary semaphore for DMA synchronization (only once)
  if (sd_dma_sem_handle == NULL)
  {
    sd_dma_sem_handle = xSemaphoreCreateBinaryStatic(&sd_dma_sem_buffer);
  }
#endif
#endif

#ifdef CONFIG_SD_DEBUG_MODE
  sd_init_step = 2;  // DMA buffers ready
#endif

  // Ensure SPI is ready for blocking operations
  hspi1.State     = HAL_SPI_STATE_READY;
  hspi1.ErrorCode = HAL_SPI_ERROR_NONE;
  sd_dma_status   = SD_DMA_IDLE;

  spi_set_speed(0);
  SD_CARD_DISABLE_CS();
  OS_DELAY_MS(10);

#ifdef CONFIG_SD_DEBUG_MODE
  sd_init_step = 3;  // SPI ready, sending dummy clocks
#endif

  for (i = 0; i < 10; i++) spi_read_write(0xFF);

#ifdef CONFIG_SD_DEBUG_MODE
  sd_init_step = 4;  // Sending CMD0
#endif

  SD_CARD_ENABLE_CS();
  retry = 20;
  do
  {
    // Skip WaitReady for CMD0 - card may not be ready yet during init
    res = sd_card_send_cmd_ex(CMD0, 0, 1);
#ifdef CONFIG_SD_DEBUG_MODE
    sd_cmd0_response = res;  // Store for debugging
#endif
  } while ((res != 0x01) && retry--);

  if (res != 0x01)
  {
#ifdef CONFIG_SD_DEBUG_MODE
    sd_init_step = 100;  // CMD0 failed
#endif
    SD_CARD_DISABLE_CS();
    return SD_TYPE_ERR;
  }

#ifdef CONFIG_SD_DEBUG_MODE
  sd_init_step = 5;  // CMD0 OK, checking card type
#endif

  if (sd_card_cmd(CMD8, 0x1AA) == 1)
  {
    for (i = 0; i < 4; i++) buf[i] = spi_read_write(0xFF);

    if (buf[2] == 0x01 && buf[3] == 0xAA)
    {
      retry = 0xFFFE;
      do
      {
        sd_card_cmd(CMD55, 0);
        res = sd_card_cmd(ACMD41, 0x40000000);
      } while (res && retry--);

      if (retry && sd_card_cmd(CMD58, 0) == 0)
      {
        for (i = 0; i < 4; i++) buf[i] = spi_read_write(0xFF);
        SD_Type = (buf[0] & 0x40) ? SD_TYPE_V2HC : SD_TYPE_V2;
      }
    }
  }
  else
  {
    sd_card_cmd(CMD55, 0);
    res = sd_card_cmd(ACMD41, 0);
    if (res <= 1)
    {
      SD_Type = SD_TYPE_V1;
      retry   = 0xFFFE;
      do
      {
        sd_card_cmd(CMD55, 0);
        res = sd_card_cmd(ACMD41, 0);
      } while (res && retry--);
    }
    else
    {
      SD_Type = SD_TYPE_MMC;
      retry   = 0xFFFE;
      do
      {
        res = sd_card_cmd(CMD1, 0);
      } while (res && retry--);
    }
    if (retry == 0 || sd_card_cmd(CMD16, 512) != 0)
      SD_Type = SD_TYPE_ERR;
  }

  SD_CARD_DISABLE_CS();
  spi_read_write(0xFF);

  if (SD_Type != SD_TYPE_ERR)
  {
    spi_set_speed(1);
#ifdef CONFIG_SD_DEBUG_MODE
    sd_init_step = 10;  // Init complete success
#endif
  }
  else
  {
#ifdef CONFIG_SD_DEBUG_MODE
    sd_init_step = 101;  // Init failed - card type error
#endif
  }

  return SD_Type;
}

// Read single block (DMA or blocking SPI)
uint8_t sd_card_read_block(uint8_t *buff, uint32_t sector)
{
  uint8_t  res;
  uint32_t tick;

  if (SD_Type != SD_TYPE_V2HC)
    sector <<= 9;

  SD_CARD_ENABLE_CS();
  res = sd_card_cmd(CMD17, sector);

  if (res != 0)
  {
    SD_CARD_DISABLE_CS();
    spi_read_write(0xFF);
    return 1;
  }

  /* Wait for data token 0xFE with time-based timeout */
  tick = OS_GET_TICK();
  while ((spi_read_write(0xFF) != 0xFE))
  {
    if ((OS_GET_TICK() - tick) > 200)
    {
      SD_CARD_DISABLE_CS();
      spi_read_write(0xFF);
      return 1;
    }
  }

#if SD_USE_DMA
  /* Read 512 bytes via DMA */
  if (sd_card_dma_receive(buff, 512) != 0)
  {
    SD_CARD_DISABLE_CS();
    spi_read_write(0xFF);
    return 1;
  }
#else
  /* Read 512 bytes (blocking) */
  for (uint16_t i = 0; i < 512; i++)
  {
    buff[i] = spi_read_write(0xFF);
  }
#endif

  /* Discard CRC */
  spi_read_write(0xFF);
  spi_read_write(0xFF);

  SD_CARD_DISABLE_CS();
  spi_read_write(0xFF);
  return 0;
}

// Write single block (DMA or blocking SPI)
uint8_t sd_card_write_block(uint8_t *buff, uint32_t sector)
{
  uint8_t  res;
  uint32_t tick;

#ifdef CONFIG_SD_DEBUG_MODE
  sd_write_count++;
#endif

  if (SD_Type != SD_TYPE_V2HC)
    sector <<= 9;

  SD_CARD_ENABLE_CS();
  res = sd_card_cmd(CMD24, sector);

  if (res != 0)
  {
#ifdef CONFIG_SD_DEBUG_MODE
    sd_write_fail++;
    sd_last_write_err = 1;  // CMD24 failed
#endif
    SD_CARD_DISABLE_CS();
    spi_read_write(0xFF);
    return 1;
  }

  /* One byte gap */
  spi_read_write(0xFF);

  /* Data token */
  spi_read_write(0xFE);

#if SD_USE_DMA
  /* Send 512 bytes via DMA */
  if (sd_card_dma_tx(buff, 512) != 0)
  {
#ifdef CONFIG_SD_DEBUG_MODE
    sd_write_fail++;
    sd_last_write_err = 4;  // DMA error
#endif
    SD_CARD_DISABLE_CS();
    spi_read_write(0xFF);
    return 1;
  }
#else
  /* Send 512 bytes (blocking) */
  for (uint16_t i = 0; i < 512; i++)
  {
    spi_read_write(buff[i]);
  }
#endif

  /* Dummy CRC */
  spi_read_write(0xFF);
  spi_read_write(0xFF);

  /* Data response - wait for valid response */
  tick = OS_GET_TICK();
  do
  {
    res = spi_read_write(0xFF);
    if ((OS_GET_TICK() - tick) > 200)
    {
#ifdef CONFIG_SD_DEBUG_MODE
      sd_write_fail++;
      sd_last_write_err = 2;  // Data response timeout
#endif
      SD_CARD_DISABLE_CS();
      spi_read_write(0xFF);
      return 1;
    }
  } while ((res & 0x11) != 0x01);  // Wait for valid data response token

  if ((res & 0x1F) != 0x05)
  {
#ifdef CONFIG_SD_DEBUG_MODE
    sd_write_fail++;
    sd_last_write_err = 2;  // Data rejected
#endif
    SD_CARD_DISABLE_CS();
    spi_read_write(0xFF);
    return 1;
  }

  /* Wait while card is busy (DO=0) - INCREASE for format operations */
  tick = OS_GET_TICK();
  while (spi_read_write(0xFF) == 0x00)
  {
    if ((OS_GET_TICK() - tick) > 1000)  // 1 second for format
    {
#ifdef CONFIG_SD_DEBUG_MODE
      sd_write_fail++;
      sd_last_write_err = 3;  // Busy timeout
#endif
      SD_CARD_DISABLE_CS();
      spi_read_write(0xFF);
      return 1;
    }
  }

  SD_CARD_DISABLE_CS();
  spi_read_write(0xFF);
  return 0;
}

// Read multiple blocks
uint8_t sd_card_read_multi_block(uint8_t *buff, uint32_t sector, uint32_t count)
{
  uint8_t res = 0;
  for (uint32_t i = 0; i < count; i++)
  {
    if (sd_card_read_block(buff + (i * 512), sector + i) != 0)
    {
      res = 1;
      break;
    }
  }
  return res;
}

// Write multiple blocks
uint8_t sd_card_write_multi_block(uint8_t *buff, uint32_t sector, uint32_t count)
{
  uint8_t res = 0;
  for (uint32_t i = 0; i < count; i++)
  {
    if (sd_card_write_block(buff + (i * 512), sector + i) != 0)
    {
      res = 1;
      break;
    }
  }
  return res;
}

// Get SD card sector count (read CSD register)
uint32_t sd_card_get_sector_count(void)
{
  uint8_t  csd[16];
  uint32_t capacity = 0;
  uint8_t  n;
  uint32_t tick;
  SD_CARD_ENABLE_CS();
  if (sd_card_cmd(CMD9, 0) != 0)  // Send CMD9 to get CSD
  {
    SD_CARD_DISABLE_CS();
    spi_read_write(0xFF);
    return 0;
  }

  // Wait for data token with time-based timeout
  tick = OS_GET_TICK();
  while ((spi_read_write(0xFF) != 0xFE))
  {
    if ((OS_GET_TICK() - tick) > 200)
    {
      SD_CARD_DISABLE_CS();
      spi_read_write(0xFF);
      return 0;
    }
  }

  // Read 16 bytes CSD
  for (uint8_t i = 0; i < 16; i++)
  {
    csd[i] = spi_read_write(0xFF);
  }
  // Discard CRC
  spi_read_write(0xFF);
  spi_read_write(0xFF);

  // Calculate capacity based on CSD version
  if ((csd[0] & 0xC0) == 0x40)  // CSD Version 2.0 (SDHC/SDXC)
  {
    // C_SIZE is in csd[7..9]
    capacity = (uint32_t) (csd[7] & 0x3F) << 16;
    capacity |= (uint32_t) csd[8] << 8;
    capacity |= csd[9];
    capacity = (capacity + 1) * 1024;  // Number of 512-byte sectors
  }
  else  // CSD Version 1.0 (SDSC)
  {
    n        = (csd[5] & 0x0F) + ((csd[10] & 0x80) >> 7) + ((csd[9] & 0x03) << 1) + 2;
    capacity = (csd[8] >> 6) + ((uint32_t) csd[7] << 2) + ((uint32_t) (csd[6] & 0x03) << 10) + 1;
    capacity <<= (n - 9);  // Number of 512-byte sectors
  }

  SD_CARD_DISABLE_CS();
  spi_read_write(0xFF);

  return capacity;
}

// DMA Callbacks - called from HAL SPI interrupt handlers
void sd_card_tx_rx_callback(void)
{
#ifdef CONFIG_SD_DEBUG_MODE
  sd_dma_txrx_cplt++;
#endif
  sd_dma_status = SD_DMA_COMPLETE;
#if SD_USE_DMA && defined(CONFIG_FREE_RTOS)
  if (sd_dma_sem_handle != NULL)
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sd_dma_sem_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
#endif
}

void sd_card_tx_callback(void)
{
#ifdef CONFIG_SD_DEBUG_MODE
  sd_dma_tx_cplt++;
#endif
  sd_dma_status = SD_DMA_COMPLETE;
#if SD_USE_DMA && defined(CONFIG_FREE_RTOS)
  if (sd_dma_sem_handle != NULL)
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sd_dma_sem_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
#endif
}

void sd_card_rx_callback(void)
{
#ifdef CONFIG_SD_DEBUG_MODE
  sd_dma_rx_cplt++;
#endif
  sd_dma_status = SD_DMA_COMPLETE;
#if SD_USE_DMA && defined(CONFIG_FREE_RTOS)
  if (sd_dma_sem_handle != NULL)
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sd_dma_sem_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
#endif
}

void sd_card_error_callback(void)
{
#ifdef CONFIG_SD_DEBUG_MODE
  sd_dma_error++;
#endif
  sd_dma_status = SD_DMA_ERROR;
#if SD_USE_DMA && defined(CONFIG_FREE_RTOS)
  if (sd_dma_sem_handle != NULL)
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sd_dma_sem_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
#endif
}

SD_DMA_Status_t sd_card_get_dma_status(void)
{
  return sd_dma_status;
}

#if SD_USE_DMA
// Reset SPI and DMA to known good state
static void sd_card_reset_dma_spi(void)
{
  // Abort any ongoing operation
  HAL_SPI_Abort(&hspi1);

  // Just reset the SPI state, don't touch DMA handles
  // They are properly initialized in HAL_SPI_MspInit
  hspi1.State     = HAL_SPI_STATE_READY;
  hspi1.ErrorCode = HAL_SPI_ERROR_NONE;

  sd_dma_status = SD_DMA_IDLE;
}

// Wait for DMA transfer to complete with timeout
static uint8_t sd_card_wait_dma(uint32_t timeout_ms)
{
#if defined(CONFIG_FREE_RTOS)
  // Use FreeRTOS semaphore wait - CPU can do other tasks
  if (sd_dma_sem_handle != NULL)
  {
    if (xSemaphoreTake(sd_dma_sem_handle, pdMS_TO_TICKS(timeout_ms)) == pdTRUE)
    {
      return (sd_dma_status == SD_DMA_COMPLETE) ? 0 : 1;
    }
    else
    {
      // Timeout - reset DMA
      sd_card_reset_dma_spi();
      sd_dma_status = SD_DMA_ERROR;
      return 1;
    }
  }
#endif
  // Callback to polling (HAL only mode or semaphore not created)
  uint32_t tick = HAL_GetTick();
  while (sd_dma_status == SD_DMA_BUSY)
  {
    if ((HAL_GetTick() - tick) > timeout_ms)
    {
      sd_card_reset_dma_spi();
      sd_dma_status = SD_DMA_ERROR;
      return 1;
    }
  }
  return (sd_dma_status == SD_DMA_COMPLETE) ? 0 : 1;
}

// DMA receive 512 bytes (send 0xFF, receive data)
static uint8_t sd_card_dma_receive(uint8_t *buff, uint16_t len)
{
  HAL_StatusTypeDef status;
  uint8_t           retry = 3;

  while (retry--)
  {
    // Ensure SPI is ready
    if (hspi1.State != HAL_SPI_STATE_READY)
    {
      sd_card_reset_dma_spi();
    }

    sd_dma_status = SD_DMA_BUSY;

    // Use TransmitReceive_DMA: send 0xFF bytes, receive actual data
    status = HAL_SPI_TransmitReceive_DMA(&hspi1, sd_dma_dummy_tx, buff, len);
    if (status != HAL_OK)
    {
      sd_card_reset_dma_spi();
      continue;
    }

    // Wait for completion
    if (sd_card_wait_dma(200) == 0)
    {
      return 0;  // Success
    }
  }

  sd_dma_status = SD_DMA_ERROR;
  return 1;
}

// DMA transmit 512 bytes
static uint8_t sd_card_dma_tx(uint8_t *buff, uint16_t len)
{
  HAL_StatusTypeDef status;
  uint8_t           retry = 3;

  while (retry--)
  {
    // Ensure SPI is ready
    if (hspi1.State != HAL_SPI_STATE_READY)
    {
      sd_card_reset_dma_spi();
    }

    sd_dma_status = SD_DMA_BUSY;

    // Use TransmitReceive_DMA: send actual data, receive to dummy buffer
    status = HAL_SPI_TransmitReceive_DMA(&hspi1, buff, sd_dma_dummy_rx, len);
    if (status != HAL_OK)
    {
      sd_card_reset_dma_spi();
      continue;
    }

    // Wait for completion
    if (sd_card_wait_dma(200) == 0)
    {
      return 0;  // Success
    }
  }

  sd_dma_status = SD_DMA_ERROR;
  return 1;
}
#endif

/* Private definitions ----------------------------------------------- */
static void spi_set_speed(uint8_t speed)
{
  // Wait for any pending operations
  while (hspi1.State != HAL_SPI_STATE_READY && hspi1.State != HAL_SPI_STATE_RESET)
  {
    OS_DELAY_MS(1);
  }

  __HAL_SPI_DISABLE(&hspi1);

  // Directly modify the baud rate in CR1 register to avoid re-init DMA
  MODIFY_REG(hspi1.Instance->CR1, SPI_CR1_BR, (speed == 0) ? SPI_BAUDRATEPRESCALER_256 : SPI_BAUDRATEPRESCALER_32);

  // Update the handle struct to match
  hspi1.Init.BaudRatePrescaler = (speed == 0) ? SPI_BAUDRATEPRESCALER_256 : SPI_BAUDRATEPRESCALER_32;

  __HAL_SPI_ENABLE(&hspi1);

  // Ensure state is ready after reconfiguration
  hspi1.State = HAL_SPI_STATE_READY;
}

static uint8_t spi_read_write(uint8_t data)
{
  uint8_t rx = 0xFF;

  // Simple blocking transfer - SPI state is managed elsewhere
  if (hspi1.State == HAL_SPI_STATE_READY)
  {
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, 100);
  }
  else
  {
    // SPI not ready, force state and try anyway
    hspi1.State = HAL_SPI_STATE_READY;
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, 100);
  }
  return rx;
}

static uint8_t sd_card_wait_ready(void)
{
  uint32_t timeout = 500;  // 500ms timeout
  uint32_t tick    = OS_GET_TICK();
  uint8_t  rx;

  while ((OS_GET_TICK() - tick) < timeout)
  {
    rx = spi_read_write(0xFF);
    if (rx == 0xFF)
      return 0;
  }
  return 1;
}

static void sd_card_init_dma_buffer(void)
{
  memset(sd_dma_dummy_tx, 0xFF, 512);
}

static uint8_t sd_card_send_cmd_ex(uint8_t cmd, uint32_t arg, uint8_t skip_wait)
{
  uint8_t res;
  uint8_t retry;

  if (!skip_wait && sd_card_wait_ready())
    return 0xFF;

  spi_read_write(0x40 | cmd);
  spi_read_write((uint8_t) (arg >> 24));
  spi_read_write((uint8_t) (arg >> 16));
  spi_read_write((uint8_t) (arg >> 8));
  spi_read_write((uint8_t) (arg));

  if (cmd == CMD0)
    spi_read_write(0x95);
  else if (cmd == CMD8)
    spi_read_write(0x87);
  else
    spi_read_write(0x01);

  if (cmd == CMD12)
    spi_read_write(0xFF);

  retry = 200;
  do
  {
    res = spi_read_write(0xFF);
  } while ((res & 0x80) && retry--);

  return res;
}

static uint8_t sd_card_cmd(uint8_t cmd, uint32_t arg)
{
  return sd_card_send_cmd_ex(cmd, arg, 0);
}

/* End of file -------------------------------------------------------- */
