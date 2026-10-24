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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "nau7802.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "queue.h"
#include "semphr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* NAU7802 I2C Address */

/* Register Addresses */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
int32_t rawValue;
int32_t tare;
char uartBuffer[120];
int32_t zeroReading;
float calibrationFactor = 789.950980f;

struct weight_grams
{
    float weight;
};
struct weight_grams W;

struct uart_output
{
    float weight_read;
    float reference_weight;
    int level;
    GPIO_PinState monitor_button;
};
struct uart_output UART;

struct weight_level
{
    int level;
};
struct weight_level lvl;


volatile float monitoring_weight = 0.0f;
volatile char currentTaskName[configMAX_TASK_NAME_LEN];
volatile uint8_t taskNameUpdated = 0;
BaseType_t result;

QueueHandle_t weight_queue;
QueueHandle_t uart_queue;
QueueHandle_t status_queue;

SemaphoreHandle_t LogSemaphore;
SemaphoreHandle_t LogMutex;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
void sendmessageTask(void *argument);
void LoadCellTask(void *argument);
void MonitorTask(void *argument);
void actuatorTask(void *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  xTaskCreate(
	  sendmessageTask,             // Task function
      "sendmessageTask",           // Task name
      256,                        // Stack size
      NULL,                       // No parameter
      1,                          // Priority
      NULL                        // Don't need handle yet
  );

  xTaskCreate(
      LoadCellTask,
      "LoadCellTask",
      256,
      NULL,
      1,
      NULL
  );

  xTaskCreate(
      MonitorTask,
      "MonitorTask",
      256,
      NULL,
      1,
      NULL
  );

  xTaskCreate(
		actuatorTask,
        "actuatorTask",
        256,
        NULL,
        1,
        NULL
    );

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  if (NAU7802_Init() != HAL_OK)
  {
      Error_Handler();
  }

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  LogSemaphore = xSemaphoreCreateBinary();
  LogMutex = xSemaphoreCreateMutex();
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  weight_queue = xQueueCreate(10, sizeof(struct weight_grams));
  uart_queue = xQueueCreate(10, sizeof(struct uart_output));
  status_queue = xQueueCreate(10, sizeof(struct weight_level));

  if (weight_queue == NULL ||
      uart_queue == NULL ||
      status_queue == NULL)
  {
      Error_Handler();
  }


  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10D19CE4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Monitor_Start_Pin */
  GPIO_InitStruct.Pin = Monitor_Start_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(Monitor_Start_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void sendmessageTask(void *argument)
{
    struct uart_output data;

    for (;;)
    {
        if (xSemaphoreTake(LogSemaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            /* Process every log message currently queued. */
            while (xQueueReceive(uart_queue, &data, 0) == pdPASS)
            {
                int len = 0;

                switch (data.level)
                {
                    case 0:
                        len = snprintf(
                            uartBuffer, sizeof(uartBuffer),
                            "Weight: %.3f g | MonitorW: %.3f g\r\n",
                            data.weight_read, data.reference_weight);
                        break;

                    case 1:
                        len = snprintf(
                            uartBuffer, sizeof(uartBuffer),
                            "Weight: %.3f g - RUNNING LOW\r\n",
                            data.weight_read);
                        break;

                    case 2:
                        len = snprintf(
                            uartBuffer, sizeof(uartBuffer),
							"Weight: %.3f g - RAN OUT\r\n",
                            data.weight_read);
                        break;

                    default:
                        len = snprintf(
                            uartBuffer, sizeof(uartBuffer),
                            "Weight: %.2f g | Status: UNKNOWN\r\n",
                            data.weight_read);
                        break;
                }

                if (len > 0 && len < (int)sizeof(uartBuffer))
                {
                    if (xSemaphoreTake(LogMutex,
                                       pdMS_TO_TICKS(100)) == pdTRUE)
                    {
                        HAL_UART_Transmit(
                            &huart2,
                            (uint8_t *)uartBuffer,
                            (uint16_t)len,
                            HAL_MAX_DELAY);

                        xSemaphoreGive(LogMutex);
                    }
                }
            }
        }
      vTaskDelay(pdMS_TO_TICKS(50));
    }
}


void LoadCellTask(void *argument)
{
    for (;;)
    {
        if (NAU7802_ReadRaw(&rawValue) == HAL_OK)
        {
            if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET)
            {
                tare = rawValue;
            }

            zeroReading = rawValue - tare;
            W.weight = (float)zeroReading / calibrationFactor;

            if (xQueueSend(weight_queue, &W,
                           pdMS_TO_TICKS(100)) != pdPASS)
            {
                Error_Handler();
            }
        }

        /* Avoid starving lower-priority tasks. */
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


void MonitorTask(void *argument)
{
    struct weight_grams W;
    struct uart_output logData;
    struct weight_level actuatorData;

    float monitoringWeight = 100.0f;

    for (;;)
    {
        GPIO_PinState currentState =
             HAL_GPIO_ReadPin(Monitor_Start_GPIO_Port,
                              Monitor_Start_Pin);
        if (currentState == GPIO_PIN_SET)
        {
            // Save starting/reference weight
        	monitoringWeight = W.weight;
        }

        if (xQueueReceive(weight_queue, &W, pdMS_TO_TICKS(100)) == pdPASS)
        {
            logData.weight_read = W.weight;
            logData.reference_weight = monitoringWeight;
            logData.monitor_button =
                HAL_GPIO_ReadPin(Monitor_Start_GPIO_Port,
                                 Monitor_Start_Pin);

            if (W.weight < monitoringWeight * 0.10f)
            {
                logData.level = 2;
            }
            else if (W.weight < monitoringWeight * 0.20f)
            {
                logData.level = 1;
            }
            else
            {
                logData.level = 0;
            }

            actuatorData.level = logData.level;

            /* Queue log data first, then signal the logger. */
            if (xQueueSend(uart_queue, &logData,
                           pdMS_TO_TICKS(100)) == pdPASS)
            {
                xSemaphoreGive(LogSemaphore);
            }

            /* Send actuator command independently. */
            xQueueSend(status_queue, &actuatorData,
                       pdMS_TO_TICKS(100));
        }
      vTaskDelay(pdMS_TO_TICKS(50));
    }
}


void actuatorTask(void *argument)
{
    struct weight_level actuatorData;
    int old_status;

    for (;;)
    {
        if (xQueueReceive(status_queue, &actuatorData,
                          pdMS_TO_TICKS(100)) == pdPASS)
        {
            if (actuatorData.level == 1 ||
                actuatorData.level == 2)
            {
                uint32_t frequency =
                    (actuatorData.level == 1) ? 1600U : 3200U;

                /* Assumes an 80 MHz timer counter clock.
                   Verify this against your TIM3 configuration. */
                uint32_t arr = (80000000U / frequency) - 1U;

                __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
                __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2,
                                      (arr + 1U) / 2U);

                HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
            }
            else
            {
                HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
            }

            if (old_status != actuatorData.level)
                    {
                        char msg[40];
                        int len = snprintf(msg, sizeof(msg), "Actuator status updated %d\r\n", actuatorData.level);

                        if (xSemaphoreTake(LogMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                        {
                            HAL_UART_Transmit(
                                &huart2,
                                (uint8_t *)msg,
                                (uint16_t)len,
                                HAL_MAX_DELAY);

                            xSemaphoreGive(LogMutex);
                        }
                        old_status = actuatorData.level;
                    }
        old_status = actuatorData.level;
        }
      vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
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
