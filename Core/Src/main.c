/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body for a security system.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "stm32l4xx_hal.h"  // Asegúrate de que este archivo esté incluido


#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_uart.h"
#include "stm32l4xx_hal_gpio.h"
/* Private defines -----------------------------------------------------------*/
#define OLED_ADDRESS 0x3C  // Dirección I2C del OLED
#define MAX_CLAVE_LENGTH 6  // Tamaño máximo de la clave

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

uint8_t claveIngresada[MAX_CLAVE_LENGTH];
uint8_t claveCorrecta[MAX_CLAVE_LENGTH] = {1, 2, 3, 4, 5, 6};  // Documento del estudiante
uint8_t claveIndex = 0;
uint8_t claveVerificada = 0;

/* Mensajes para UART */
uint8_t successMsg[] = "Success\r\n";
uint8_t errorMsg[] = "Error\r\n";

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void mostrarError(void);
void mostrarExito(void);
void actualizarDisplay(uint8_t *clave, uint8_t size);
void verificarClave(void);
uint8_t detectarTecla(uint16_t GPIO_Pin); // Prototipo de la función para detectar teclas

/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, 10);
    return len;
}

// Función para verificar la clave ingresada
void verificarClave() {
    if (memcmp(claveIngresada, claveCorrecta, MAX_CLAVE_LENGTH) == 0) {
        mostrarExito();
        HAL_UART_Transmit(&huart2, successMsg, sizeof(successMsg), 10);
    } else {
        mostrarError();
        HAL_UART_Transmit(&huart2, errorMsg, sizeof(errorMsg), 10);
    }
    claveIndex = 0;  // Reiniciar el índice de clave para nueva entrada
}

// Función para mostrar "Error" en la pantalla OLED
void mostrarError() {
    // Código para mostrar el mensaje "Error" en la pantalla OLED
    ssd1306_WriteString("Error", Font_7x10, SSD1306_COLOR_WHITE);
}

// Función para mostrar "Success" en la pantalla OLED
void mostrarExito() {
    // Código para mostrar el mensaje "Success" en la pantalla OLED
    ssd1306_WriteString("Success", Font_7x10, SSD1306_COLOR_WHITE);
}

// Actualiza la pantalla OLED con los dígitos ingresados
void actualizarDisplay(uint8_t *clave, uint8_t size) {
    // Limpia la pantalla y muestra los dígitos de la clave ingresada
ssd1306_Clear();
    for (int i = 0; i < size; i++) {
        // Mostrar cada dígito en la pantalla
    }
  ssd1306_UpdateScreen();
}

// Función para manejar las teclas del teclado
uint8_t detectarTecla(uint16_t GPIO_Pin) {
    // Implementación para detectar teclas y retornarlas
    return 0;
}

/* Callback para manejar las interrupciones de GPIO */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    uint8_t tecla = detectarTecla(GPIO_Pin);
    if (tecla == '*') {
        claveIndex = 0;  // Resetear clave ingresada
        memset(claveIngresada, 0, sizeof(claveIngresada));  // Limpiar buffer
        // Actualizar la pantalla para mostrar que se reinició la clave
    } else if (tecla == '#') {
        verificarClave();  // Verificar la clave cuando se presiona '#'
    } else {
        if (claveIndex < MAX_CLAVE_LENGTH) {
            claveIngresada[claveIndex++] = tecla;  // Agregar tecla al buffer de clave
            actualizarDisplay(claveIngresada, claveIndex);  // Mostrar clave en pantalla
        }
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();

    printf("Sistema de seguridad iniciado...\r\n");

    while (1) {
        // Código principal del sistema
    }
}
static void MX_GPIO_Init(void) {
    // Inicialización de GPIO
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);  // Cambia el pin según tu configuración

    /*Configure GPIO pin : PA5 */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void MX_USART2_UART_Init(void) {
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10909CEC;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_USART1_UART_Init(void) {
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE END Main */
