/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include <string.h> //
#include "keypad.h" //


#include "ring_buffer.h"
#include"ssd1306.h"
#include"ssd1306_fonts.h"
#include <stdio.h>
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

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t data_usart2;
uint8_t newline[] = "\r\n";

#define BUFFER_CAPACITY 10
uint8_t keyboard_buffer_memory[BUFFER_CAPACITY];
ring_buffer_t keyboard_ring_buffer;
uint8_t first_key_pressed = 0;
uint8_t cursor_x_position = 10;
uint8_t cursor_y_position = 30;
uint8_t max_cursor_x_position = 80;

#define MAX_DISPLAY_CHARS 20 // SIZE of chart and screen

//buffer to store the key sequence
static char display_buffer[MAX_DISPLAY_CHARS + 1]; // +1 for nule value
static uint8_t buffer_index = 0;

// variables for actual cursor position on screen
static uint8_t cursor_x = 10;
static uint8_t cursor_y = 30;
// adding variables for LED control
#define SYSTEM_LED_GPIO_Port GPIOA
#define SYSTEM_LED_Pin GPIO_PIN_5


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*
 * Function to redirect printf output to UART.
 *
 * Purpose:
 * This function overrides the standard `_write` function to allow the
 * usage of `printf` in embedded systems without a standard output. It
 * redirects the printf output to UART for debugging or serial communication.
 *
 * Parameters:
 * - file: Not used, required by the function signature.
 * - ptr: Pointer to the data to be transmitted.
 * - len: Length of the data to be transmitted.
 *
 * Functionality:
 * - It sends the data pointed to by `ptr` via UART using HAL_UART_Transmit.
 * - It returns the length of the data transmitted.
 *
 * Use case:
 * By using this function, any call to `printf` will output the message
 * to the UART interface, which can be monitored with a serial console.
 */

int _write(int file, char *ptr, int len)
{
  // to using printf
  HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, 10);
  return len;
}

/*
 * Function to handle external interrupt callbacks for GPIO pins (keypad input).
 *
 * Purpose:
 * This function is called when a GPIO interrupt occurs. It scans the keypad for the pressed key,
 * handles special cases ('*' to reset the sequence, '#' to validate the password), and updates
 * the OLED display and UART based on the input.
 *
 * Parameters:
 * - GPIO_Pin: The pin number where the interrupt was triggered.
 *
 * Functionality:
 * - Detects the key pressed on the keypad.
 * - If '*' is pressed, the input sequence is reset.
 * - If a valid key is pressed, it is added to the ring buffer and displayed on the OLED.
 * - If '#' is pressed, the entered sequence is validated against a predefined correct sequence.
 * - The result of the validation (correct or incorrect) is displayed on the OLED and transmitted via UART.
 * - The buffer is reset after validation.
 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	uint8_t key_pressed = keypad_scan(GPIO_Pin);

	    if (key_pressed != 0xFF) {
	        // when '*' is pressed, the sequence is RESET
	        if (key_pressed == '*') {
	            ring_buffer_reset(&keyboard_ring_buffer);
	            memset(display_buffer, 0, sizeof(display_buffer)); //clean actual buffer on the screen
	            buffer_index = 0; // reset index on buffer

	            ssd1306_Fill(Black);
	            ssd1306_SetCursor(10, 20);
	            ssd1306_WriteString("sequence restarted", Font_6x8, White);
	            ssd1306_UpdateScreen();
	            HAL_UART_Transmit(&huart2, (uint8_t*)"Sequence restarted\n\r", 22, 10);
	            return;
	        }

	        // Write the key to the ring buffer
	        if (key_pressed != '#') {
	            ring_buffer_write(&keyboard_ring_buffer, key_pressed);

	            // add chart to the buffer
	            if (buffer_index < MAX_DISPLAY_CHARS) {
	                display_buffer[buffer_index++] = key_pressed;
	                display_buffer[buffer_index] = '\0'; // Null-terminar el buffer

	                // clean screen and show buffer content on screen
	                ssd1306_Fill(Black);
	                ssd1306_SetCursor(10, 30);
	                ssd1306_WriteString(display_buffer, Font_6x8, White);
	                ssd1306_UpdateScreen();

	                // send chart via UART
	                HAL_UART_Transmit(&huart2, &key_pressed, 1, 10);
	            }
	            return;
	        }



	        // proccoed when  '#' is pressed , we verify the password entered



	        uint8_t byte2 = 0;
	        uint8_t id_incorrect2 = 0;
	        uint8_t my_id2[] = "1004191436";  // correct sequence

	        // Read from buffer and compare with correct key
	        for (uint8_t idx2 = 0; idx2 < sizeof(my_id2) - 1; idx2++) {
	            if (ring_buffer_read(&keyboard_ring_buffer, &byte2) != 0) {
	                if (byte2 != my_id2[idx2]) {
	                    id_incorrect2 = 1;  // Mark as incorrect if no match


	                    break;
	                }
	            } else {
	                id_incorrect2 = 1;  // if there is no space in buffer
	                break;
	            }
	        }

	        HAL_UART_Transmit(&huart2, (uint8_t*)"\n", 1, 10);

	        if (!id_incorrect2) {
	            // success
	            ssd1306_Fill(Black);
	            ssd1306_SetCursor(10, 20);
	            ssd1306_WriteString("correct sequence", Font_6x8, White);
	            ssd1306_UpdateScreen();
	            HAL_UART_Transmit(&huart2, (uint8_t*)"correct sequence\n\r", 21, 10);
	            HAL_UART_Transmit(&huart2, (uint8_t*)"starting...\n\r", 14, 10);


	        } else {
	            //  error
	            ssd1306_Fill(Black);
	            ssd1306_SetCursor(10, 20);
	            ssd1306_WriteString("error ", Font_6x8, White);
	            ssd1306_UpdateScreen();
	            HAL_UART_Transmit(&huart2, (uint8_t*)" incorrect sequence \n\r", 12, 10);

	        }

	        // reset buffer after validation
	        ring_buffer_reset(&keyboard_ring_buffer);
	        memset(display_buffer, 0, sizeof(display_buffer)); // clean buffer on screen
	        buffer_index = 0; // reset index buffer
	        cursor_x = 10;  //Resets the horizontal cursor position
	        cursor_y = 30;  // Restarts the vertical position of the course

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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  ring_buffer_init(&keyboard_ring_buffer, keyboard_buffer_memory, BUFFER_CAPACITY);
  ssd1306_Init();
  ssd1306_Fill(Black);
  ssd1306_SetCursor(10,20);
  ssd1306_WriteString("Starting...\r\n",Font_6x8,White);
  ssd1306_UpdateScreen();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("Starting...\r\n");
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
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
  hi2c1.Init.Timing = 0x10909CEC;
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_HEARTBEAT_Pin|LED_LEFT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ROW_1_GPIO_Port, ROW_1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ROW_2_Pin|ROW_4_Pin|ROW_3_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_RIGHT_GPIO_Port, LED_RIGHT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BUTTON_LEFT_Pin BUTTON_RIGHT_Pin */
  GPIO_InitStruct.Pin = BUTTON_LEFT_Pin|BUTTON_RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_HEARTBEAT_Pin LED_LEFT_Pin */
  GPIO_InitStruct.Pin = LED_HEARTBEAT_Pin|LED_LEFT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : COLUMN_1_Pin */
  GPIO_InitStruct.Pin = COLUMN_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(COLUMN_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : COLUMN_4_Pin */
  GPIO_InitStruct.Pin = COLUMN_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(COLUMN_4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : COLUMN_2_Pin COLUMN_3_Pin */
  GPIO_InitStruct.Pin = COLUMN_2_Pin|COLUMN_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ROW_1_Pin */
  GPIO_InitStruct.Pin = ROW_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(ROW_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ROW_2_Pin ROW_4_Pin ROW_3_Pin */
  GPIO_InitStruct.Pin = ROW_2_Pin|ROW_4_Pin|ROW_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_RIGHT_Pin */
  GPIO_InitStruct.Pin = LED_RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_RIGHT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
