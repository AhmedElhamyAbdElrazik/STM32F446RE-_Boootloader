/************************************************************************
 * Author  :Ahmed Elhamy
 * Date    :28/1/2022
 * Version :v_01
 ***********************************************************************/
#include "main.h"


#define PRESSED 0

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  uint8_t  blinkFlag=0;
  uint32_t blinkLastTime=HAL_GetTick();
  uint32_t buttonLastTime=HAL_GetTick();
  uint32_t blinkingDelay=500;
  uint32_t debounceDelay=200;

  while (1)
  {

	  if((PRESSED==HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin))
		 &&((HAL_GetTick()-buttonLastTime)>debounceDelay))
	  {
		  blinkFlag=!blinkFlag;
		  buttonLastTime=HAL_GetTick();
	  }

	  if(blinkFlag)
	  {
		  if((HAL_GetTick()-blinkLastTime)>blinkingDelay)
		   {HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
		   blinkLastTime=HAL_GetTick();
		   }
	  }else
	  {
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	  }

	  printf("Hello Ahmed,blinking value =%d \r\n",blinkFlag);

  }

}


int __io_putchar(int ch)
{
  /* e.g. write a character to the PC_UART and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1,HAL_MAX_DELAY);
  return ch;
}
