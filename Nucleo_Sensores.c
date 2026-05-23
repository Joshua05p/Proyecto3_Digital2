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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Neopixel.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
DMA_HandleTypeDef hdma_tim1_ch1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#define TXBUFFERSIZZE 1
#define RXBUFFERSIZZE 4

int count = 0;

uint8_t aRxBuffer[RXBUFFERSIZZE];
uint8_t aTxBuffer[TXBUFFERSIZZE];

uint8_t estado[8]; // 0 = ocupado, 1 = libre
uint8_t espacios;
uint8_t estadoByte;

float brilloled = 50;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void mostrarNumero(uint8_t num){

    HAL_GPIO_WritePin(GPIOA, E_Pin|F_Pin|G_Pin|C_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, A_Pin|B_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(D_GPIO_Port, D_Pin, GPIO_PIN_SET);

    switch(num){
        case 0:
            HAL_GPIO_WritePin(GPIOC, A_Pin|B_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, C_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(D_GPIO_Port, D_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, E_Pin|F_Pin, GPIO_PIN_RESET);
            break;

        case 1:
            HAL_GPIO_WritePin(GPIOC, B_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, C_Pin, GPIO_PIN_RESET);
            break;

        case 2:
            HAL_GPIO_WritePin(GPIOC, A_Pin|B_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, E_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(D_GPIO_Port, D_Pin, GPIO_PIN_RESET);
            break;

        case 3:
            HAL_GPIO_WritePin(GPIOC, A_Pin|B_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, C_Pin|G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(D_GPIO_Port, D_Pin, GPIO_PIN_RESET);
            break;

        case 4:
            HAL_GPIO_WritePin(GPIOA, F_Pin|G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, B_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, C_Pin, GPIO_PIN_RESET);
            break;

        case 5:
            HAL_GPIO_WritePin(GPIOC, A_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, F_Pin|G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, C_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(D_GPIO_Port, D_Pin, GPIO_PIN_RESET);
            break;

        case 6:
            HAL_GPIO_WritePin(GPIOC, A_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, F_Pin|G_Pin|E_Pin|C_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(D_GPIO_Port, D_Pin, GPIO_PIN_RESET);
            break;

        case 7:
            HAL_GPIO_WritePin(GPIOC, A_Pin|B_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, C_Pin, GPIO_PIN_RESET);
            break;

        case 8:
            HAL_GPIO_WritePin(GPIOC, A_Pin|B_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, C_Pin|E_Pin|F_Pin|G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(D_GPIO_Port, D_Pin, GPIO_PIN_RESET);
            break;

        case 9:
            HAL_GPIO_WritePin(GPIOC, A_Pin|B_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, C_Pin|F_Pin|G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(D_GPIO_Port, D_Pin, GPIO_PIN_RESET);
            break;
    }
}

uint8_t leerSensores(void)
{
    uint8_t libres = 0;

    estado[0] = (HAL_GPIO_ReadPin(S1_GPIO_Port, S1_Pin) == GPIO_PIN_SET);
    estado[1] = (HAL_GPIO_ReadPin(S2_GPIO_Port, S2_Pin) == GPIO_PIN_SET);
    estado[2] = (HAL_GPIO_ReadPin(S3_GPIO_Port, S3_Pin) == GPIO_PIN_SET);
    estado[3] = (HAL_GPIO_ReadPin(S4_GPIO_Port, S4_Pin) == GPIO_PIN_SET);
    estado[4] = (HAL_GPIO_ReadPin(S5_GPIO_Port, S5_Pin) == GPIO_PIN_SET);
    estado[5] = (HAL_GPIO_ReadPin(S6_GPIO_Port, S6_Pin) == GPIO_PIN_SET);
    estado[6] = (HAL_GPIO_ReadPin(S7_GPIO_Port, S7_Pin) == GPIO_PIN_SET);
    estado[7] = (HAL_GPIO_ReadPin(S8_GPIO_Port, S8_Pin) == GPIO_PIN_SET);

    for (int i = 0; i < 8; i++){
        if (estado[i]) libres++;
    }

    return libres;
}

void actualizarLeds(void)
{
    for (int i = 0; i < 8; i++) 
    {
        for (int j = 0; j < 2; j++) 
        {
            int ledIndex = i * 2 + j;

            if (estado[i] == 1)
            {
                setPixelColor(ledIndex, 255, 0, 0); 
            }
            else 
            {
                setPixelColor(ledIndex, 0, 255, 0);
            }
        }
    }

    pixelShow();
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  aTxBuffer[0] = 1;
  aTxBuffer[1] = 2;
  aTxBuffer[2] = 3;
  aTxBuffer[3] = 4;

  if (HAL_I2C_EnableListen_IT(&hi2c1) != HAL_OK){
	  Error_Handler();
  }

  pixelClear();
  setPixelColor(0, 255, 0, 0);
  setPixelColor(1, 0, 255, 0);
  setPixelColor(2, 0, 0, 255);
  setPixelColor(3, 255, 0, 0);
  setPixelColor(4, 0, 255, 0);
  setPixelColor(5, 0, 0, 255);
  setPixelColor(6, 255, 0, 0);
  setPixelColor(7, 0, 255, 0);
  setBrightness(50);
  pixelShow();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		espacios = leerSensores();

		actualizarLeds();
		estadoByte = 0;
		mostrarNumero(espacios);
		for (int i = 0; i < 8; i++)
		{
			if (estado[i] == 0) // 0 = ocupado
			{
				estadoByte |= (1 << i);
			}
		}
		setBrightness(15);
		HAL_Delay(50);


	    //HAL_Delay(100);
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 48;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

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
  HAL_GPIO_WritePin(GPIOA, E_Pin|F_Pin|G_Pin|LD2_Pin
                          |C_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, NEOPIXEL_Pin|D_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, A_Pin|B_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : E_Pin F_Pin G_Pin LD2_Pin
                           C_Pin */
  GPIO_InitStruct.Pin = E_Pin|F_Pin|G_Pin|LD2_Pin
                          |C_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : S3_Pin S2_Pin S1_Pin */
  GPIO_InitStruct.Pin = S3_Pin|S2_Pin|S1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : S8_Pin S7_Pin S6_Pin */
  GPIO_InitStruct.Pin = S8_Pin|S7_Pin|S6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : NEOPIXEL_Pin D_Pin */
  GPIO_InitStruct.Pin = NEOPIXEL_Pin|D_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : S5_Pin S4_Pin */
  GPIO_InitStruct.Pin = S5_Pin|S4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : A_Pin B_Pin */
  GPIO_InitStruct.Pin = A_Pin|B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode){


	if(TransferDirection == I2C_DIRECTION_TRANSMIT){
		if( HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, (uint8_t*) aRxBuffer, 1, I2C_FIRST_AND_LAST_FRAME) != HAL_OK ){
    		Error_Handler();
    	}
    }else if(TransferDirection == I2C_DIRECTION_RECEIVE){
    	if( HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, (uint8_t*) aTxBuffer, 1, I2C_FIRST_AND_LAST_FRAME) != HAL_OK ){
    		Error_Handler();
    	}
    }
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){
    HAL_I2C_EnableListen_IT(hi2c);
}

//Funcion para enviar al maestro.
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *I2cHandle){
    //aTxBuffer[0]++;
    //aTxBuffer[1]++;
    //aTxBuffer[2]++;
    //aTxBuffer[3]++;
}

//Funcion para recibir del maestro.
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *I2cHandle){
    if(aRxBuffer[0] == 83){
    	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    	aTxBuffer[0] = estadoByte;
    }
    aRxBuffer[0] = 0x00;
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle){
	if(HAL_I2C_GetError(I2cHandle) != HAL_I2C_ERROR_AF){
		Error_Handler();
	}
}
/* USER CODE END 4 */

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
