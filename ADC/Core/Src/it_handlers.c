/*
 * it_handlers.c
 *
 *  Created on: Mar 10, 2020
 *      Author: rodrigocelmer
 */
#include "stm32l053xx.h"
#include "macros.h"

extern uint8_t	flag_TIM6,	//flag for TIM6 interrupt
				T_Start,
				T_ADC_Conv;

void TIM6_DAC_IRQHandler(void)		//interrupt every 10 us
{
	if(TIM6->SR & TIM_SR_UIF)
	{
		flag_TIM6 = 1;				//sets TIM6 flag
		TOGGLE_BLUE_LED;
		if(T_Start > 0)
		{
			T_Start--;
		}
		if(T_ADC_Conv > 0)
		{
			T_ADC_Conv--;
		}
		TIM6->SR &= ~TIM_SR_UIF;	//Clear timer interrupt flag
	}
}

