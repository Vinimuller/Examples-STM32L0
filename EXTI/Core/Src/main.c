#include "stm32l053xx.h"

//------ DEFINES
//--- INPUTS
#define USR_BT_PRESS		(!(GPIOA->IDR & GPIO_IDR_ID10_Msk))

//--- OUTPUTS
#define RED_LED_ON			(GPIOB->ODR |= GPIO_ODR_OD0_Msk)
#define RED_LED_OFF			(GPIOB->ODR &= ~GPIO_ODR_OD0_Msk)
#define TOGGLE_RED_LED		(GPIOB->ODR ^= GPIO_IDR_ID0_Msk)
#define GREEN_LED_ON		(GPIOA->ODR |= GPIO_ODR_OD7_Msk)
#define	GREEN_LED_OFF		(GPIOA->ODR &= ~GPIO_ODR_OD7_Msk)
#define TOGGLE_GREEN_LED	(GPIOA->ODR ^=GPIO_IDR_ID7_Msk)
#define BLUE_LED_ON			(GPIOB->ODR |= GPIO_ODR_OD1_Msk)
#define BLUE_LED_OFF		(GPIOB->ODR &= ~GPIO_ODR_OD1_Msk)
#define TOGGLE_BLUE_LED		(GPIOB->ODR ^= GPIO_IDR_ID1_Msk)

//--- Global Variables
int user_bt_count=0, count=0, flag_EXTI=0;

void TIM6_DAC_IRQHandler(void)		//interrupts every 1 ms
{
	if(TIM6->SR & TIM_SR_UIF)
	{
		TIM6->SR &= ~TIM_SR_UIF;	//Clear timer interrupt flag
		count++;
		if(count==100)
		{
			TOGGLE_RED_LED;			//toggle the red led every 100 ms
			count=0;
		}
		if(flag_EXTI)										//if there was an EXTI int
		{
			user_bt_count++;								//counts up to 50 (50 ms)
			if(user_bt_count==50)
			{
				TOGGLE_GREEN_LED;							//turns on/off the green led

			}
			if((user_bt_count>=50) && (!USR_BT_PRESS))		//if the led has been toggled and we released the button
			{
				flag_EXTI=0;								//clears flag
			}
		}
		else
		{
			user_bt_count=0;
		}
	}
}

void EXTI4_15_IRQHandler (void)
{
	if(EXTI->PR & EXTI_PR_PIF10)	//if there's EXTI interrupt
	{
		flag_EXTI=1;				//sets aux flag for toggling the green led
		EXTI->PR |= EXTI_PR_PIF10;	//clears the EXTI flag
	}
}

int main (void)
{
	//------ Clock init
	//--- GPIO Clock init
	RCC->IOPENR		|= RCC_IOPENR_GPIOAEN;
	RCC->IOPENR 	|= RCC_IOPENR_GPIOBEN;
	//--- TIM6 Clock init
	RCC->APB1ENR 	|= RCC_APB1ENR_TIM6EN;

	//------ GPIO init
	//--- LEDs config
	//Green
	GPIOA->MODER 	&= ~(GPIO_MODER_MODE7_1 | GPIO_MODER_MODE10_Msk);	//Set GPIOA pin 7 as output
	GPIOA->BSRR 	|= GPIO_BSRR_BR_7;		//Set pin to low
	//Blue (1) and Red (0)
	GPIOB->MODER	= 	GPIO_MODER_MODE1_0 | GPIO_MODER_MODE0_0;
	//GPIOB->MODER	|= (GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE0_Msk);
	GPIOB->BSRR		|= (GPIO_BSRR_BR_1 | GPIO_BSRR_BR_0);
	//Blue
//	GPIOB->MODER	= GPIO_MODER_MODE1_0;	//Set GPIOB pin 1 as output
//	GPIOB->BSRR		|= GPIO_BSRR_BR_1;		//Set pin to low
//	//Red
//	GPIOB->MODER	|= GPIO_MODER_MODE0_0;	//Set GPIOB pin 0 as outuput
//	GPIOB->BSRR		|= GPIO_BSRR_BR_0;		//Set pin to low

	//--- Buttons config
	//User button
	//GPIOA->MODER	&= ~GPIO_MODER_MODE10_Msk;	//Set GPIOA pin 10 as input
	GPIOA->PUPDR	|= GPIO_PUPDR_PUPD10_0;		//GPIO pin 10 has a pull-up

	//------ TIMER init
	//--- TIM6 config
	TIM6->DIER 		|= TIM_DIER_UIE;	//Enable interrupt
	TIM6->PSC = 209;					//timer prescaler
	TIM6->ARR = 9;						//counter counts up to (interrupt every 1 ms)
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

	while(1)
	{

	}
	return 0;
}
