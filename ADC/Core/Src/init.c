/*
 * init.c
 *
 *  Created on: Mar 10, 2020
 *      Author: rodrigocelmer
 */
#include "stm32l053xx.h"

void GPIO_Init (void)
{
	/*GPIOA
	 * PA14 -> AF mode (SWD_IO)		| Low speed	| Push pull
	 * PA13 -> AF mode (SWD_CK)		| Low speed	| Push pull
	 * PA10	-> Input (User Button)	| Low speed	| Pull-up
	 * PA7	-> Output (Green Led)	| Low speed | Push pull
	 */
	RCC->IOPENR		|= 		RCC_IOPENR_GPIOAEN		;

	GPIOA->MODER 	&= 	~(	GPIO_MODER_MODE7_1 		|
							GPIO_MODER_MODE10_Msk	);
	GPIOA->BSRR 	|= 		GPIO_BSRR_BR_7			;
	GPIOA->PUPDR	|= 		GPIO_PUPDR_PUPD10_0		;

	/*GPIOB
	 * PB1	-> Output (Blue Led)	| Low speed	| Push pull
	 * PB0	-> Output (Red Led)		| Low speed | Push pull
	 */
	RCC->IOPENR 	|= 	RCC_IOPENR_GPIOBEN	;

	GPIOB->MODER	= 	GPIO_MODER_MODE1_0	|
						GPIO_MODER_MODE0_0	;
	GPIOB->BSRR		|= 	GPIO_BSRR_BR_1 		|
						GPIO_BSRR_BR_0		;
}

void TIM6_Init (void)
{
	RCC->APB1ENR 	|= RCC_APB1ENR_TIM6EN;

	TIM6->DIER 		|= TIM_DIER_UIE;	//Enable interrupt
	TIM6->PSC = 0;						//timer prescaler
	TIM6->ARR = 20;						//counter counts up to
	TIM6->EGR 		|= TIM_EGR_UG;		//Update registers values
	//Enables IRQ and IRQ priority
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	NVIC_SetPriority(TIM6_DAC_IRQn, 0);
	TIM6->CR1 		|= TIM_CR1_CEN;		//enables the counter
}

void ADC_Init (void)
{
//	RCC->APB2ENR 	|= 	RCC_APB2ENR_ADC1EN;
//	ADC1->CFGR2 	|= 	ADC_CFGR2_CKMODE_0;	//sets ADC clock as PCLK/2
//	ADC->CCR 		|= 	ADC_CCR_LFMEN;
//	ADC1->SMPR 		|= 	ADC_SMPR_SMPR_1	|	//sets sample time for 12.5 ADC clock cycles
//						ADC_SMPR_SMPR_0	;	//because we need 10.5 ADC clock cycles at least
}
