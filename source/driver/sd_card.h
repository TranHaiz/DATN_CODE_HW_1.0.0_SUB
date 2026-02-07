/**
 * @file       sd_card.h
 * @copyright  Copyright (C) 2019 ITRVN. All rights reserved.
 * @license    This project is released under the Fiot License.
 * @version    major.minor.patch
 * @date       2026-02-06
 * @author     Hai Tran
 *
 * @brief     SD card driver file.
 *
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef _SD_CARD_H_
#define _SD_CARD_H_
/* Includes ----------------------------------------------------------- */
#include "common_type.h"

/* Public defines ----------------------------------------------------- */
// SD Card Commands
#define CMD0           (0)   // GO_IDLE_STATE
#define CMD1           (1)   // SEND_OP_COND
#define CMD8           (8)   // SEND_IF_COND
#define CMD9           (9)   // SEND_CSD
#define CMD10          (10)  // SEND_CID
#define CMD12          (12)  // STOP_TRANSMISSION
#define CMD16          (16)  // SET_BLOCKLEN
#define CMD17          (17)  // READ_SINGLE_BLOCK
#define CMD18          (18)  // READ_MULTIPLE_BLOCK
#define CMD24          (24)  // WRITE_BLOCK
#define CMD25          (25)  // WRITE_MULTIPLE_BLOCK
#define CMD55          (55)  // APP_CMD
#define CMD58          (58)  // READ_OCR
#define ACMD41         (41)  // SD_SEND_OP_COND

// Card Types
#define SD_TYPE_ERR    (0x00)
#define SD_TYPE_MMC    (0x01)
#define SD_TYPE_V1     (0x02)
#define SD_TYPE_V2     (0x04)
#define SD_TYPE_V2HC   (0x06)

#define SD_SECTOR_SIZE (512)  // Size of sector
#define SD_MAX_PATH    (256)  // Max length of path

/**
 * @brief SD Card SPI mode
 *        - 0: polling (blocking)
 *       - 1: DMA
 */
// DMA Configuration - 1=Enable DMA, 0=Polling mode
#define SD_USE_DMA     (1)

/* Public enumerate/structure ----------------------------------------- */
/**
 * @brief SD DMA transfer status
 */
typedef enum
{
  SD_DMA_IDLE = 0,
  SD_DMA_BUSY,
  SD_DMA_COMPLETE,
  SD_DMA_ERROR
} SD_DMA_Status_t;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
// Debug variables
extern volatile uint32_t sd_write_count;     // Total write calls
extern volatile uint32_t sd_write_fail;      // Write failures
extern volatile uint8_t  sd_last_write_err;  // 1=CMD, 2=response, 3=busy timeout, 4=DMA
extern volatile uint32_t sd_dma_tx_cplt;     // DMA TX complete count
extern volatile uint32_t sd_dma_rx_cplt;     // DMA RX complete count
extern volatile uint32_t sd_dma_txrx_cplt;   // DMA TX/RX complete count
extern volatile uint32_t sd_dma_error;       // DMA error count
extern volatile uint8_t  sd_init_step;       // Init progress (10=success, 100+=error)
extern volatile uint8_t  sd_cmd0_response;   // Last CMD0 response (0x01=OK, 0xFF=no response)

/* Public function prototypes ----------------------------------------- */
/**
 * @brief  Initialize SD card
 * @retval SD card type (SD_TYPE_XXX) or SD_TYPE_ERR on failure
 */
uint8_t sd_card_init(void);

/**
 * @brief  Read single block from SD card
 *
 * @return 0: success, 1: failure
 */
uint8_t sd_card_read_block(uint8_t *buff, uint32_t sector);

/**
 * @brief Write single block to SD card
 */
uint8_t sd_card_write_block(uint8_t *buff, uint32_t sector);

/**
 * @brief  Read multiple blocks from SD card
 */
uint8_t sd_card_read_multi_block(uint8_t *buff, uint32_t sector, uint32_t count);

/**
 * @brief Write multiple blocks to SD card
 */
uint8_t sd_card_write_multi_block(uint8_t *buff, uint32_t sector, uint32_t count);

/**
 * @brief  Get SD card sector count
 */
uint32_t sd_card_get_sector_count(void);

/**
 * @brief  Get SD DMA transfer status
 */
SD_DMA_Status_t sd_card_get_dma_status(void);

// DMA callbacks - call from SPI DMA complete interrupt
void sd_card_tx_rx_callback(void);
void sd_card_tx_callback(void);
void sd_card_rx_callback(void);
void sd_card_error_callback(void);

#endif /*End file _NAME_H_*/

/* End of file -------------------------------------------------------- */
