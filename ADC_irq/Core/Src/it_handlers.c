/*
 * it_handlers.c
 *
 *  Created on: Apr 15, 2020
 *      Author: rodrigocelmer
 */
#include "stm32l053xx.h"

extern uint8_t flag_EOC;	//flag for End Of Conversion (ADC converted one single channel)

void ADC1_COMP_IRQHandler(void)
{
	if(ADC1->ISR & ADC_ISR_EOC)			//if there was and End Of Conversion
	{
		flag_EOC = 1;					//sets flag_EOC
		//EOC flag is cleared by reading the ADC_DR register
		//ADC1->ISR &= ~ADC_ISR_EOC;		//clears EOC interrupt flag
	}

//	if(ADC1->ISR & ADC_ISR_EOS)		//if there was and End Of Sequence
//	{
//
//	}
}
