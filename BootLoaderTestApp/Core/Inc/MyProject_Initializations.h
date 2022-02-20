/*
 * MyProject_Initializations.h
 *
 *  Created on: Jan 28, 2022
 *      Author: Ahmed Elhamy
 */

#ifndef INC_MYPROJECT_INITIALIZATIONS_H_
#define INC_MYPROJECT_INITIALIZATIONS_H_


extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART1_UART_Init(void);


#endif /* INC_MYPROJECT_INITIALIZATIONS_H_ */
