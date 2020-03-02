/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

//Timer 6 interrupt Handler
void TIM6_DAC_IRQHandler(void)
{
	if(TIM6->SR & TIM_SR_UIF)	//Check if it was a timer event
	{
		TIM6->SR &= ~TIM_SR_UIF;//Clear timer interrupt flag
		GPIOB->ODR ^= GPIO_IDR_ID0_Msk | GPIO_IDR_ID1_Msk;
		GPIOA->ODR ^= GPIO_IDR_ID7_Msk;
	}
}

int main(void)
{
	//GPIO configuration
	RCC->IOPENR 	|= RCC_IOPENR_GPIOBEN;						//Enable GPIOB clock
	GPIOB->MODER 	= GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0;	//Set GPIOB pin 0 and 1 as output
	GPIOB->BSRR 	|= GPIO_BSRR_BR_0 | GPIO_BSRR_BR_1;			//Set pin to low

	RCC->IOPENR 	|= RCC_IOPENR_GPIOAEN;						//Enable GPIOA clock
	GPIOA->MODER 	&= ~GPIO_MODER_MODE7_1;						//Set GPIOA pin 7
	GPIOA->BSRR 	|= GPIO_BSRR_BR_7;							//Set pin to low

	//Timer configuration
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN; //Enable TIM 6 clock
	TIM6->CR1 	|= TIM_CR1_URS; 		//Only counter overflow generate interrupt
	TIM6->DIER 	|= TIM_DIER_UIE;		//Enable interrupt
	TIM6->PSC 	= 2099;					//Set prescaller
	TIM6->ARR 	= 999;					//Set ARR
	TIM6->EGR 	|= TIM_EGR_UG;			//Update registers values

	NVIC_EnableIRQ(TIM6_DAC_IRQn);		//Enable IRQ
	NVIC_SetPriority(TIM6_DAC_IRQn, 0);	//Set IRQ priority

	//tchau
	//qualquer coisa dinweinwe

	TIM6->CR1 	|= TIM_CR1_CEN;			//Start timer


	while(1)
	{

	}

	return 0;
}
