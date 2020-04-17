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
		flag_EOC++;					//sets flag_EOC
		ADC1->ISR |= ADC_ISR_EOC;		//clears EOC interrupt flag by writing 1 to it

		if(ADC1->ISR & ADC_ISR_OVR)
		{
			asm("nop");
		}
	}

//	if(ADC1->ISR & ADC_ISR_EOSEQ)		//if there was and End Of Sequence
//	{
//		flag_EOC = 1;					//sets flag_EOC
//		ADC1->ISR |= ADC_ISR_EOSEQ;		//clears EOS interrupt flag by writing 1 to it
//	}
}

