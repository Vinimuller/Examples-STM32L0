#include "stm32l053xx.h"

//------ DEFINES
//--- INPUTS
#define USR_BT_PRESS		(!(GPIOA->IDR & GPIO_IDR_ID10_Msk))

//--- OUTPUTS
#define Red_Led_On			(GPIOB->ODR |= GPIO_ODR_OD0_Msk)
#define Red_Led_Off			(GPIOB->ODR &= ~GPIO_ODR_OD0_Msk)
#define Green_Led_On		(GPIOA->ODR |= GPIO_ODR_OD7_Msk)
#define	Green_Led_Off		(GPIOA->ODR &= ~GPIO_ODR_OD7_Msk)
#define Blue_Led_On			(GPIOB->ODR |= GPIO_ODR_OD1_Msk)
#define Blue_Led_Off		(GPIOB->ODR &= ~GPIO_ODR_OD1_Msk)
#define Toggle_Red_Led		(GPIOB->ODR ^= GPIO_IDR_ID0_Msk)
#define Toggle_Green_Led	(GPIOA->ODR ^=GPIO_IDR_ID7_Msk)
#define Toggle_Blue_Led		(GPIOB->ODR ^= GPIO_IDR_ID1_Msk)

//--- Global Variables
int user_bt_count=0, count=0;

void TIM6_DAC_IRQHandler(void){
	if(TIM6->SR == 0x01){
		TIM6->SR &= ~TIM_SR_UIF;	//Clear timer interrupt flag
		if(!USR_BT_PRESS){
			count++;
			if(count==100){
				Toggle_Red_Led;
				Green_Led_Off;
				count=0;
			}
		}
		else{
			count=0;
		}
	}
}

void EXTI4_15_IRQHandler (void){
	if(EXTI->PR == 0x0400){
		EXTI->PR |= EXTI_PR_PIF10;
		Green_Led_On;
		Red_Led_Off;
	}
}

int main (void){
	//------ Clock init
	//--- GPIO Clock init
	RCC->IOPENR 	|= RCC_IOPENR_GPIOAEN;
	RCC->IOPENR 	|= RCC_IOPENR_GPIOBEN;
	//--- TIM6 Clock init
	RCC->APB1ENR 	|= RCC_APB1ENR_TIM6EN;

	//------ GPIO init
	//--- LEDs config
	//Green
	GPIOA->MODER 	&= ~GPIO_MODER_MODE7_1;	//Set GPIOA pin 7 as output
	GPIOA->BSRR 	|= GPIO_BSRR_BR_7;		//Set pin to low
	//Blue
	GPIOB->MODER	= GPIO_MODER_MODE1_0;	//Set GPIOB pin 1 as output
	GPIOB->BSRR		|= GPIO_BSRR_BR_1;		//Set pin to low
	//Red
	GPIOB->MODER	|= GPIO_MODER_MODE0_0;	//Set GPIOB pin 0 as outuput
	GPIOB->BSRR		|= GPIO_BSRR_BR_0;		//Set pin to low

	//--- Buttons config
	//User button
	GPIOA->MODER	&= ~GPIO_MODER_MODE10_1;
	GPIOA->MODER	&= ~GPIO_MODER_MODE10_0;	//Set GPIOB pin 10 as input
	GPIOA->PUPDR	|= GPIO_PUPDR_PUPD10_0;		//GPIO pin 10 has a pull-up

	//------ TIMER init
	//--- TIM6 config
	TIM6->DIER 		|= TIM_DIER_UIE;	//Enable interrupt
	TIM6->PSC = 209;					//timer prescaler
	TIM6->ARR = 9;						//counter counts up to
	TIM6->EGR 		|= TIM_EGR_UG;		//Update registers values
	//Enables IRQ and IRQ priority
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	NVIC_SetPriority(TIM6_DAC_IRQn, 0);
	TIM6->CR1 		|= TIM_CR1_CEN;		//enables the counter

	//----- EXTI init
	EXTI->IMR	|= EXTI_IMR_IM10;
	EXTI->FTSR	|= EXTI_FTSR_FT10;
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn, 0);

	while(1){

	}

	return 0;
}
