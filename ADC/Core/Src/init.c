/*
 * init.c
 *
 *  Created on: Mar 10, 2020
 *      Author: rodrigocelmer
 */
#include "stm32l053xx.h"

void ADC_Init (void)
{
	//--- Initialization procedure
	RCC->APB2ENR 	|= 	RCC_APB2ENR_ADC1EN;		//enables ADC clock
	ADC1->CR 		|=	ADC_CR_ADVREGEN;		//enables the voltage regulator
	ADC1->CFGR2 	|= 	ADC_CFGR2_CKMODE_0;		//sets ADC clock as PCLK/2
	ADC->CCR 		|= 	ADC_CCR_LFMEN;			//low frequency mode enable
	ADC1->SMPR 		|= 	ADC_SMPR_SMPR_1	|		//sets sample time for 12.5 ADC clock cycles
						ADC_SMPR_SMPR_0	;		//because we need 10.5 ADC clock cycles at least

	ADC1->CR |= ADC_CR_ADCAL;				//starting the calibration
	while(ADC1->CR & ADC_CR_ADCAL);			//we have to wait until ADCAL = 0 (Can be handled by interrupt)

	ADC1->CR |= ADC_CR_ADEN;				//then we enable the ADC
	while(!(ADC1->ISR & ADC_ISR_ADRDY));	//and wait for it to be ready (Can be handled by interrupt)
}
