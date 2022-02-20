
/* USER CODE BEGIN 2 */
#include "main.h"


#include <stdio.h>


#define VERSION    "V_1.0"
#define PRESSED    0
#define NOTPRESSED 1

UART_HandleTypeDef      huart1;
UART_HandleTypeDef      huart2;

#define PC_UART         huart2
#define BLUETOOTH_UART  huart1

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

static void GoToAPP( void );


int main(void)
{
  //Initialization
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();

  printf("Starting BootLoader %s\r\n",VERSION);
  //Turn ON LED to indicate that you are in the BootLoader mode
  HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin, GPIO_PIN_SET );

  printf("Press The BootLoader Button to Trigger OTA Update...\r\n");
  GPIO_PinState OTAButtonState;
  uint32_t CurrentTick = HAL_GetTick();
  uint32_t EndTick     = HAL_GetTick() + 3000;   // from now to 3 Seconds

  /* Check the button if  pressed or not for 3seconds */
  while(CurrentTick < EndTick)
  {
	OTAButtonState=HAL_GPIO_ReadPin( GPIOC, GPIO_PIN_13 );
    if(OTAButtonState== PRESSED )
    {
      break;
    }
    CurrentTick = HAL_GetTick();
  }

  /*Start the Firmware or Application update */
  if(OTAButtonState== PRESSED )
  {
    printf("Starting Firmware Download...\r\n");
    // OTA Request. Receive the data from the UART1 and flash it
    if( OTA_ReceiveAndFlashTheCode () != OTA_ERROR_OK )
    {
      printf("OTA Update : ERROR!!! HALT!!!\r\n");
      while( 1 );
    }
    else
    {
      /* Reset to load the new application */
      printf("Firmware Update Done Rebooting...\r\n");
      HAL_NVIC_SystemReset();
    }
  }

  GoToAPP();

  while (1)
  {

  }

}

//Consider it as an implementation for printf() to use UART
int __io_putchar(int ch)
{
  /* e.g. write a character to the PC_UART and Loop until the end of transmission */
  HAL_UART_Transmit(&PC_UART, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

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
  RCC_OscInitStruct.PLL.PLLN = 90;
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

static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART2_UART_Init(void)
{

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

}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

static void GoToAPP(void)
{
  printf("Jump to Application\r\n");
  // Turn OFF the LED since Bootloader is not running
  HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin, GPIO_PIN_RESET );

  //pointer to the Reset Handler of the Application
  void (*APP_ResetHandler)(void) = (void*)(*((volatile uint32_t*) (OTA_APP_ADDRESS + 4U)));
  APP_ResetHandler();
}


void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }

}

/* USER CODE END 2 */



