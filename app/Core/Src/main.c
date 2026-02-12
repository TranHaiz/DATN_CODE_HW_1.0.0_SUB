/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "cmsis_os.h"
#include "dma.h"
#include "fatfs.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "bsp_fs.h"
#include "bsp_sim.h"
#include "bsp_uart.h"
#include "cmsis_os2.h"
#include "ff.h"
#include "os_lib.h"
#include "semphr.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define UART_RX_BUFF_SIZE (512)

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t uart1_rx_buff[UART_RX_BUFF_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include "sd_card.h"

fil_custom_t      test_file;
status_function_t fs_ret;

/* Debug variables - check in debugger */
volatile uint8_t dbg_sd_type      = 0xFF;
volatile FRESULT dbg_mount_result = 0xFF;
volatile FRESULT dbg_mkdir_result = 0xFF;
volatile FRESULT dbg_open_result  = 0xFF;

/* Declare thread (global scope) - 2048 bytes stack for FatFS */
OS_THREAD_DECLARE(FS_Test, osPriorityNormal, 2048);
OS_THREAD_DECLARE(uart_Test, osPriorityNormal, 1024);
OS_SEM_DEFINE_STAIC(uart1_sem_rx);

void uart_rx_callback(void)
{
  // Handle received data in bsp_uart1_handle.buff with length bsp_uart1_handle.rx_len
  uint32_t len = BSP_UART_RX_SIZE(&bsp_uart1_handle);

  // Process received data (for example, echo back)
  bsp_uart_write(&bsp_uart1_handle, bsp_uart1_handle.buff, len);

  // Reset RX length for next reception
  BSP_UART_RX_RESET(&bsp_uart1_handle);
}

status_function_t g_ret = STATUS_ERROR;
void              test_uart(void *argument)
{
  g_ret = bsp_sim_init();

  while (1)
  {
    if (g_ret == STATUS_OK)
    {
      firebase_data_t data_firebase_test    = { 0 };
      data_firebase_test.batt_level         = rand() % 100 + 1;
      data_firebase_test.position.latitude  = (float) (rand() % 18000) / 100.0f - 90.0f;   // -90.0 to +90.0
      data_firebase_test.position.longitude = (float) (rand() % 36000) / 100.0f - 180.0f;  // -180.0 to +180.0
      data_firebase_test.speed              = (float) (rand() % 2000) / 10.0f;             // 0.0 to 200.0

      g_ret = bsp_sim_send_data_firebase(&data_firebase_test);
    }
    OS_DELAY_MS(1000);
  }
}
void test_fs(void *argument)
{
  /* Step 1: Test SD card hardware */
  dbg_sd_type = sd_card_init();
  if (dbg_sd_type == SD_TYPE_ERR)
  {
    // SD card init failed - check sd_init_step for details
    for (;;)
    {
      osDelay(1000);
    }
  }

  /* Step 2: Mount filesystem */
  extern FATFS USERFatFS;
  extern char  USERPath[];
  dbg_mount_result = f_mount(&USERFatFS, USERPath, 1);

  if (dbg_mount_result == FR_NO_FILESYSTEM)
  {
    // No filesystem - need to format
    static uint8_t work_buf[4096] __attribute__((aligned(4)));
    dbg_mount_result = f_mkfs(USERPath, FM_FAT32, 0, work_buf, sizeof(work_buf));
    if (dbg_mount_result == FR_OK)
    {
      dbg_mount_result = f_mount(&USERFatFS, USERPath, 1);
    }
  }

  if (dbg_mount_result != FR_OK)
  {
    // Mount failed
    for (;;)
    {
      osDelay(1000);
    }
  }

  /* Step 3: Create LOG folder */
  dbg_mkdir_result = f_mkdir("0:/LOG");
  if ((dbg_mkdir_result != FR_OK) && (dbg_mkdir_result != FR_EXIST))
  {
    for (;;)
    {
      osDelay(1000);
    }
  }

  /* Step 4: Open file */
  dbg_open_result = f_open(&test_file.fhandle, "0:/LOG/test.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
  if (dbg_open_result == FR_OK)
  {
    const char test_data[] = "Hello, File System!\r\n";
    UINT       bw;
    f_write(&test_file.fhandle, test_data, sizeof(test_data) - 1, &bw);
    f_close(&test_file.fhandle);
    fs_ret = STATUS_OK;
  }
  else
  {
    fs_ret = STATUS_ERROR;
  }

  /* Thread must never return - infinite loop */
  for (;;)
  {
    osDelay(1000);
  }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  OS_THREAD_CREATE(uart_Test, test_uart);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
  RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM            = 8;
  RCC_OscInitStruct.PLL.PLLN            = 100;
  RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ            = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
