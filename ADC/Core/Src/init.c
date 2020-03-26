/*
 * init.c
 *
 *  Created on: Mar 10, 2020
 *      Author: rodrigocelmer
 */
#include "stm32l053xx.h"

void ADC_Init (void)
{
	RCC->APB2ENR 	|= 	RCC_APB2ENR_ADC1EN;
	ADC1->CFGR2 	|= 	ADC_CFGR2_CKMODE_0;	//sets ADC clock as PCLK/2
	ADC->CCR 		|= 	ADC_CCR_LFMEN;
	ADC1->SMPR 		|= 	ADC_SMPR_SMPR_1	|	//sets sample time for 12.5 ADC clock cycles
						ADC_SMPR_SMPR_0	;	//because we need 10.5 ADC clock cycles at least
}
