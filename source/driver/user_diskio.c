/**
 ******************************************************************************
 * @file    diskio_spi.c
 * @brief   This file includes a diskio driver skeleton to be completed by the user.
 ******************************************************************************
 *
 * COPYRIGHT(c) 2016 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* USER CODE BEGIN 0 */

/* Includes ------------------------------------------------------------------*/
#include "diskio_spi.h"
#include "../bsp_common.h"
#include "../bsp_io.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* MMC/SD command */
#define CMD0   (0)         /* GO_IDLE_STATE */
#define CMD1   (1)         /* SEND_OP_COND (MMC) */
#define ACMD41 (0x80 + 41) /* SEND_OP_COND (SDC) */
#define CMD8   (8)         /* SEND_IF_COND */
#define CMD9   (9)         /* SEND_CSD */
#define CMD10  (10)        /* SEND_CID */
#define CMD12  (12)        /* STOP_TRANSMISSION */
#define ACMD13 (0x80 + 13) /* SD_STATUS (SDC) */
#define CMD16  (16)        /* SET_BLOCKLEN */
#define CMD17  (17)        /* READ_SINGLE_BLOCK */
#define CMD18  (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23  (23)        /* SET_BLOCK_COUNT (MMC) */
#define ACMD23 (0x80 + 23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)        /* WRITE_BLOCK */
#define CMD25  (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD32  (32)        /* ERASE_ER_BLK_START */
#define CMD33  (33)        /* ERASE_ER_BLK_END */
#define CMD38  (38)        /* ERASE */
#define CMD55  (55)        /* APP_CMD */
#define CMD58  (58)        /* READ_OCR */

#define MMC_CD (!0) /* Card detect (yes:true, no:false, default:true) */
#define MMC_WP (0)  /* Write protected (yes:true, no:false, default:false) */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC   (0x01)            /* MMC ver 3 */
#define CT_SD1   (0x02)            /* SD ver 1 */
#define CT_SD2   (0x04)            /* SD ver 2 */
#define CT_SDC   (CT_SD1 | CT_SD2) /* SD */
#define CT_BLOCK (0x08)            /* Block addressing */

#define FATFS_ASSERT   BSP_IO_WritePin(IO_SD_CS, 0)
#define FATFS_DEASSERT BSP_IO_WritePin(IO_SD_CS, 1)

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;
static BYTE CardType;                /* Card type flags */
static volatile UINT Timer1, Timer2; /* 1kHz decrement timer stopped at zero (disk_timerproc()) */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize(BYTE);
DSTATUS USER_status(BYTE);
DRESULT USER_read(BYTE, BYTE *, DWORD, UINT);
#if _USE_WRITE == 1
DRESULT USER_write(BYTE, const BYTE *, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
DRESULT USER_ioctl(BYTE, BYTE, void *);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef USER_Driver = {
  USER_initialize, USER_status, USER_read,
#if _USE_WRITE == 1
  USER_write,
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/
static SPI_Device_T spiDev = SPI_DEV_SD_INIT;

// Exchange a byte
static BYTE xchg_spi(BYTE dat // Data to send
)
{
  HAL_SPI_Config(spiDev);

  BYTE dumy;
  HAL_SPI_TransmitReceive(hspi[spiDev], &dat, &dumy, 1, 1000);
  return dumy;
}

// Receive multiple byte
static void rcvr_spi_multi(BYTE *buff, // Pointer to data buffer
                           UINT btr    // Number of bytes to receive (even number)
)
{
  HAL_SPI_Config(spiDev);

#if (1)
  HAL_SPI_Receive_DMA(hspi[spiDev], (uint8_t *)buff, btr);
  while (HAL_SPI_GetState(hspi[spiDev]) != HAL_SPI_STATE_READY)
    ;
#else
  HAL_SPI_Receive(hspi[spiDev], (uint8_t *)buff, btr, SPIx_TIMEOUT_MAX);
#endif
}

#if _USE_WRITE
// Send multiple byte
static void xmit_spi_multi(const BYTE *buff, // Pointer to the data
                           UINT btx          // Number of bytes to send (even number)
)
{
  HAL_SPI_Config(spiDev);

#if (1)
  HAL_SPI_Transmit_DMA(hspi[spiDev], (uint8_t *)buff, btx);
  while (HAL_SPI_GetState(hspi[spiDev]) != HAL_SPI_STATE_READY)
    ;
#else
  HAL_SPI_Transmit(hspi[spiDev], (uint8_t *)buff, btx, SPIx_TIMEOUT_MAX);
#endif
}
#endif

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/
// 1:Ready, 0:Timeout
static int wait_ready(UINT wt // Timeout [ms]
)
{
  BYTE d = 0xFF;
  Timer2 = wt;
  do
  {
    d = xchg_spi(0xFF);
    // This loop takes a time. Insert rot_rdq() here for multitask envilonment.
  } while (d != 0xFF && Timer2); // Wait for card goes ready or timeout
  return (d == 0xFF) ? 1 : 0;
}

// 1:Ready, 0:Timeout
static int wait_ready_init(UINT wt // Timeout [ms]
)
{
  BYTE d = 0xFF;
  Timer2 = wt;
  do
  {
    d = xchg_spi(0xFF);
    // This loop takes a time. Insert rot_rdq() here for multitask envilonment.
  } while (d != 0xFF && Timer2); // Wait for card goes ready or timeout
  return 1;
}

/*-----------------------------------------------------------------------*/
/* Deselect card and release SPI                                         */
/*-----------------------------------------------------------------------*/
static void deselect(void)
{
  FATFS_DEASSERT; // CS = H
  xchg_spi(0xFF); // Dummy clock (force DO hi-z for multiple slave SPI)
}

/*-----------------------------------------------------------------------*/
/* Select card and wait for ready                                        */
/*-----------------------------------------------------------------------*/
// 1:OK, 0:Timeout
static int select(void)
{
  FATFS_ASSERT;
  xchg_spi(0xFF); // Dummy clock (force DO enabled)
  if (wait_ready(500))
    return 1; // OK
  deselect();
  return 0; // Timeout
}

/* 1:OK, 0:Timeout */
static int select_init(void)
{
  FATFS_ASSERT;
  xchg_spi(0xFF); // Dummy clock (force DO enabled)
  if (wait_ready_init(500))
    return 1; // OK
  deselect();
  return 0; // Timeout
}

/*-----------------------------------------------------------------------*/
/* Control SPI module (Platform dependent)                               */
/*-----------------------------------------------------------------------*/
// Enable SSP module and attach it to I/O pads
static void power_on(void)
{
  FATFS_DEASSERT;
  for (Timer1 = 10; Timer1;)
    ; // 10ms
}

/* Disable SPI function */
static void power_off(void)
{
  select(); // Wait for card ready
  deselect();
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the MMC                                    */
/*-----------------------------------------------------------------------*/
// 1:OK, 0:Error
static int rcvr_datablock(BYTE *buff, // Data buffer
                          UINT btr    // Data block length (byte)
)
{
  BYTE token;

  memset(buff, 0xFF, btr);

  Timer1 = 200;
  do
  { // Wait for DataStart token in timeout of 200ms
    token = xchg_spi(0xFF);
    // This loop will take a time. Insert rot_rdq() here for multitask envilonment
  } while ((token == 0xFF) && Timer1);
  if (!(token & 0xE0))
    return 0; // Function fails if invalid DataStart token or timeout

  rcvr_spi_multi(buff, btr); // Store trailing data to the buffer
  xchg_spi(0xFF);
  xchg_spi(0xFF); // Discard CRC

  return 1; // Function succeeded
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to the MMC                                         */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE
// 1:OK, 0:Failed
static int xmit_datablock(const BYTE *buff, // Ponter to 512 byte data to be sent
                          BYTE token        // Token
)
{
  BYTE resp;

  if (!wait_ready(500))
    return 0; // Wait for card ready

  xchg_spi(token); // Send token
  if (token != 0xFD)
  {                            // Send data if token is other than StopTran
    xmit_spi_multi(buff, 512); // Data
    xchg_spi(0xFF);
    xchg_spi(0xFF); // Dummy CRC

    resp = xchg_spi(0xFF);     // Receive data resp
    if ((resp & 0x1F) != 0x05) // Function fails if the data packet was not accepted
      return 0;
  }

  return 1;
}
#endif

/*-----------------------------------------------------------------------*/
/* Send a command packet to the MMC                                      */
/*-----------------------------------------------------------------------*/
// Return value: R1 resp (bit7==1:Failed to send)
static BYTE send_cmd(BYTE cmd, // Command index
                     DWORD arg // Argument
)
{
  BYTE n, res;

  if (cmd & 0x80)
  { // Send a CMD55 prior to ACMD<n>
    cmd &= 0x7F;
    res = send_cmd(CMD55, 0);
    if (res > 1)
      return res;
  }

  // Select the card and wait for ready except to stop multiple block read
  if (cmd != CMD12)
  {
    deselect();
    if (cmd == CMD0)
    {
      if (!select_init())
        return 0xFF;
    }
    else
    {
      if (!select())
        return 0xFF;
    }
  }

  // Send command packet
  xchg_spi(0x40 | cmd);        // Start + command index
  xchg_spi((BYTE)(arg >> 24)); // Argument[31..24]
  xchg_spi((BYTE)(arg >> 16)); // Argument[23..16]
  xchg_spi((BYTE)(arg >> 8));  // Argument[15..8]
  xchg_spi((BYTE)arg);         // Argument[7..0]
  n = 0x01;                    // Dummy CRC + Stop
  if (cmd == CMD0)
    n = 0x95; // Valid CRC for CMD0(0)
  if (cmd == CMD8)
    n = 0x87; // Valid CRC for CMD8(0x1AA)
  xchg_spi(n);

  // Receive command resp
  if (cmd == CMD12)
    xchg_spi(0xFF); // Diacard following one byte when CMD12
  n = 10;           // Wait for response (10 bytes max)
  do
    res = xchg_spi(0xFF);
  while ((res & 0x80) && --n);

  return res; // Return received response
}

/**
 * @brief  Initializes a Drive
 * @param  lun : not used
 * @retval DSTATUS: Operation status
 */
DSTATUS USER_initialize(BYTE lun)
{
  Stat = STA_NOINIT;
  BYTE n, cmd, ty, ocr[4];

  power_on();               // Initialize SPI
  spiDev = SPI_DEV_SD_INIT; // Slow speed

  for (n = 10; n; n--)
    xchg_spi(0xFF); // Send 80 dummy clocks

  ty = 0;
  if (send_cmd(CMD0, 0) == 1)
  {                // Put the card SPI/Idle state
    Timer1 = 1000; // Initialization timeout = 1 sec
    if (send_cmd(CMD8, 0x1AA) == 1)
    { // SDv2?
      for (n = 0; n < 4; n++)
        ocr[n] = xchg_spi(0xFF); // Get 32 bit return value of R7 resp
      if (ocr[2] == 0x01 && ocr[3] == 0xAA)
      { // Is the card supports vcc of 2.7-3.6V?
        while (Timer1 && send_cmd(ACMD41, 1UL << 30))
          ; // Wait for end of initialization with ACMD41(HCS)
        if (Timer1 && send_cmd(CMD58, 0) == 0)
        { // Check CCS bit in the OCR
          for (n = 0; n < 4; n++)
            ocr[n] = xchg_spi(0xFF);
          ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; // Card id SDv2
        }
      }
    }
    else
    { /* Not SDv2 card */
      if (send_cmd(ACMD41, 0) <= 1)
      { // SDv1 or MMC?
        ty  = CT_SD1;
        cmd = ACMD41; // SDv1 (ACMD41(0))
      }
      else
      {
        ty  = CT_MMC;
        cmd = CMD1; // MMCv3 (CMD1(0))
      }
      while (Timer1 && send_cmd(cmd, 0))
        ;                                       // Wait for end of initialization
      if (!Timer1 || send_cmd(CMD16, 512) != 0) // Set block length: 512
        ty = 0;
    }
  }
  CardType = ty; // Card type

  deselect();

  if (ty)
  {                      // OK
    Stat &= ~STA_NOINIT; // Clear STA_NOINIT flag
  }
  else
  { // Failed
    power_off();
    Stat = STA_NOINIT;
  }

  spiDev = SPI_DEV_SD; // High speed

  return Stat;
}

/**
 * @brief  Gets Disk Status
 * @param  lun : not used
 * @retval DSTATUS: Operation status
 */
DSTATUS USER_status(BYTE lun)
{
  Stat = STA_NOINIT;
  Stat &= ~STA_NOINIT;

  return Stat;
}

/**
 * @brief  Reads Sector(s)
 * @param  lun : not used
 * @param  *buff: Data buffer to store read data
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to read (1..128)
 * @retval DRESULT: Operation result
 */
DRESULT USER_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
  /* USER CODE HERE */
  BYTE cmd;

  if (!count)
    return RES_PARERR; // Check parameter
  if (Stat & STA_NOINIT)
    return RES_NOTRDY; // Check if drive is ready
  if (!(CardType & CT_BLOCK))
    sector *= 512; // LBA ot BA conversion (byte addressing cards)

  cmd = count > 1 ? CMD18 : CMD17; //  READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK
  if (send_cmd(cmd, sector) == 0)
  {
    do
    {
      if (!rcvr_datablock(buff, 512))
      {
        break;
      }
      buff += 512;
    } while (--count);
    if (cmd == CMD18)
      send_cmd(CMD12, 0); // STOP_TRANSMISSION
  }

  deselect();

  return count ? RES_ERROR : RES_OK; // Return result
}

/**
 * @brief  Writes Sector(s)
 * @param  lun : not used
 * @param  *buff: Data to be written
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to write (1..128)
 * @retval DRESULT: Operation result
 */
#if _USE_WRITE == 1
DRESULT USER_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  /* USER CODE HERE */
  if (!count)
    return RES_PARERR; // Check parameter
  if (Stat & STA_NOINIT)
    return RES_NOTRDY; // Check drive status
  if (Stat & STA_PROTECT)
    return RES_WRPRT; // Check write protect

  if (!(CardType & CT_BLOCK))
    sector *= 512; // LBA ==> BA conversion (byte addressing cards)

  if (count == 1)
  {                                    // Single sector write
    if ((send_cmd(CMD24, sector) == 0) // WRITE_BLOCK
        && xmit_datablock(buff, 0xFE))
      count = 0;
  }
  else
  { // Multiple sector write
    if (CardType & CT_SDC)
      send_cmd(ACMD23, count); // Predefine number of sectors
    if (send_cmd(CMD25, sector) == 0)
    { // WRITE_MULTIPLE_BLOCK
      do
      {
        if (!xmit_datablock(buff, 0xFC))
          break;
        buff += 512;
      } while (--count);
      if (!xmit_datablock(0, 0xFD)) // STOP_TRAN token
        count = 1;
    }
  }

  deselect();

  return count ? RES_ERROR : RES_OK; // Return result
}
#endif /* _USE_WRITE == 1 */

/**
 * @brief  I/O control operation
 * @param  lun : not used
 * @param  cmd: Control code
 * @param  *buff: Buffer to send/receive control data
 * @retval DRESULT: Operation result
 */
#if _USE_IOCTL == 1
DRESULT USER_ioctl(BYTE lun, BYTE cmd, void *buff)
{
  DRESULT res;
  BYTE n, csd[16], *ptr = buff;
  DWORD *dp, st, ed, csize;

  if (Stat & STA_NOINIT)
    return RES_NOTRDY; // Check if drive is ready

  res = RES_ERROR;

  switch (cmd)
  {
  case CTRL_SYNC: // Wait for end of internal write process of the drive
    if (select())
      res = RES_OK;
    break;

  case GET_SECTOR_COUNT: // Get drive capacity in unit of sector (DWORD)
    if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
    {
      if ((csd[0] >> 6) == 1)
      { // SDC ver 2.00
        csize          = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
        *(DWORD *)buff = csize << 10;
      }
      else
      { // SDC ver 1.XX or MMC ver 3
        n              = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize          = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
        *(DWORD *)buff = csize << (n - 9);
      }
      res = RES_OK;
    }
    break;

  case GET_BLOCK_SIZE: // Get erase block size in unit of sector (DWORD)
    if (CardType & CT_SD2)
    { // SDC ver 2.00
      if (send_cmd(ACMD13, 0) == 0)
      { // Read SD status
        xchg_spi(0xFF);
        if (rcvr_datablock(csd, 16))
        { // Read partial block
          for (n = 64 - 16; n; n--)
            xchg_spi(0xFF); // Purge trailing data
          *(DWORD *)buff = 16UL << (csd[10] >> 4);
          res            = RES_OK;
        }
      }
    }
    else
    { // SDC ver 1.XX or MMC
      if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
      { // Read CSD
        if (CardType & CT_SD1)
        { // SDC ver 1.XX
          *(DWORD *)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
        }
        else
        { // MMC
          *(DWORD *)buff =
            ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
        }
        res = RES_OK;
      }
    }
    break;

  case CTRL_TRIM: // Erase a block of sectors (used when _USE_ERASE == 1)
    if (!(CardType & CT_SDC))
      break; // Check if the card is SDC */
    if (USER_ioctl(0, MMC_GET_CSD, csd))
      break; // Get CSD // lun = 0, not used [TrietLuu]
    if (!(csd[0] >> 6) && !(csd[10] & 0x40))
      break; // Check if sector erase can be applied to the card
    dp = buff;
    st = dp[0];
    ed = dp[1]; // Load sector block
    if (!(CardType & CT_BLOCK))
    {
      st *= 512;
      ed *= 512;
    }
    if (send_cmd(CMD32, st) == 0 && send_cmd(CMD33, ed) == 0 && send_cmd(CMD38, 0) == 0 &&
        wait_ready(30000)) /* Erase sector block */
      res = RES_OK;        // FatFs does not check result of this command
    break;

  // Following commands are never used by FatFs module
  case MMC_GET_TYPE: // Get MMC/SDC type (BYTE)
    *ptr = CardType;
    res  = RES_OK;
    break;

  case MMC_GET_CSD:            // Read CSD (16 bytes)
    if (send_cmd(CMD9, 0) == 0 // READ_CSD
        && rcvr_datablock(ptr, 16))
      res = RES_OK;
    break;

  case MMC_GET_CID:             // Read CID (16 bytes)
    if (send_cmd(CMD10, 0) == 0 // READ_CID
        && rcvr_datablock(ptr, 16))
      res = RES_OK;
    break;

  case MMC_GET_OCR: // Read OCR (4 bytes)
    if (send_cmd(CMD58, 0) == 0)
    { // READ_OCR
      for (n = 4; n; n--)
        *ptr++ = xchg_spi(0xFF);
      res = RES_OK;
    }
    break;

  case MMC_GET_SDSTAT: // Read SD status (64 bytes)
    if (send_cmd(ACMD13, 0) == 0)
    { // SD_STATUS
      xchg_spi(0xFF);
      if (rcvr_datablock(ptr, 64))
        res = RES_OK;
    }
    break;

  default:
    res = RES_PARERR;
  }

  deselect();

  return res;
}
#endif // _USE_IOCTL == 1

/*-----------------------------------------------------------------------*/
/* Device timer function                                                 */
/*-----------------------------------------------------------------------*/
/*
This function must be called from timer interrupt routine in period
of 1 ms to generate card control timing.
*/
void disk_timerproc(void)
{
  WORD n;
  BYTE s;

  n = Timer1; // 1kHz decrement timer stopped at 0
  if (n)
    Timer1 = --n;
  n = Timer2;
  if (n)
    Timer2 = --n;

  s = Stat;
  if (MMC_WP) // Write protected
    s |= STA_PROTECT;
  else // Write enabled
    s &= ~STA_PROTECT;
  if (MMC_CD) // Card is in socket
    s &= ~STA_NODISK;
  else // Socket empty
    s |= (STA_NODISK | STA_NOINIT);
  Stat = s;
}

FRESULT set_timestamp(char *obj, // Pointer to the file name
                      int year, int month, int mday, int hour, int min, int sec)
{
  FILINFO fno;

  fno.fdate = (WORD)(((year - 1980) * 512U) | month * 32U | mday);
  fno.ftime = (WORD)(hour * 2048U | min * 32U | sec / 2U);

  return f_utime(obj, &fno);
}
/* USER CODE END 0 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
