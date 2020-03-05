/*
 * it_handlers.c
 *
 *  Created on: Mar 5, 2020
 *      Author: rodrigocelmer
 */
#include "stm32l053xx.h"

extern int	flag_EXTI,	//flag for EXTI interrupt
			flag_TIM6;	//flag for TIM6 interrupt

void TIM6_DAC_IRQHandler(void)
{
	if(TIM6->SR & TIM_SR_UIF)
	{
		flag_TIM6 = 1;				//sets TIM6 flag
		TIM6->SR &= ~TIM_SR_UIF;	//Clear timer interrupt flag
	}
}

void EXTI4_15_IRQHandler (void)
{
	if(EXTI->PR & EXTI_PR_PIF10)	//if there's EXTI interrupt
	{
		flag_EXTI=1;				//sets EXTI flag
		EXTI->PR |= EXTI_PR_PIF10;	//clears the EXTI flag
	}
}
