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

//--- GENERAL
#define UP		1
#define DOWN	0

//--- Global Variables
int user_bt_count	=0,	//counter to debounce the button
	count			=0,	//counter to toggle the red led
	flag_EXTI		=0,	//flag for EXTI interrupt
	flag_TIM6		=0,	//flag for TIM6 interrupt
	flag_frequency	=1;	//flag to indicate if we are increasing or decreasing the frequency

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

int main (void)
{
	//------ GPIO config
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

	//------ TIMER config
	//--- TIM6 init
	RCC->APB1ENR 	|= RCC_APB1ENR_TIM6EN;

	TIM6->DIER 		|= TIM_DIER_UIE;	//Enable interrupt
	TIM6->PSC = 20;						//timer prescaler
	TIM6->ARR = 209;					//counter counts up to
	TIM6->EGR 		|= TIM_EGR_UG;		//Update registers values
	//Enables IRQ and IRQ priority
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	NVIC_SetPriority(TIM6_DAC_IRQn, 0);
	TIM6->CR1 		|= TIM_CR1_CEN;		//enables the counter

	//----- EXTI config
	EXTI->IMR	|= EXTI_IMR_IM10;
	EXTI->FTSR	|= EXTI_FTSR_FT10;
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn, 0);

	while(1)
	{
		if(flag_TIM6)				//if there was a TIM6 interrupt
		{
			flag_TIM6 = 0;			//clears TIM6 flag
			count++;
			if(count==100)
			{
				TOGGLE_RED_LED;		//toggle the red led every 100 ms
				count=0;
			}
		}//flag_TIM6

		if(flag_EXTI)									//if there was an EXTI int
		{
			user_bt_count++;							//counts up to 50 for debouncing
			if(user_bt_count==50)
			{
				if((TIM6->ARR > 19)&& flag_frequency == UP)
				{
					TIM6->ARR = TIM6->ARR - 10;
					TIM6->EGR |= TIM_EGR_UG;		//Update registers values
					if(TIM6->ARR <= 19)
					{
						flag_frequency = DOWN;
						GREEN_LED_ON;					//Green led indicates we're going to decrease the frequency
						BLUE_LED_OFF;
					}
				}
				else{
					if((TIM6->ARR <= 199) && flag_frequency == DOWN)
					{
						TIM6->ARR = TIM6->ARR + 10;
						TIM6->EGR |= TIM_EGR_UG;		//Update registers values
						if(TIM6->ARR >= 209)
						{
							flag_frequency = UP;
							GREEN_LED_OFF;
							BLUE_LED_ON;					//Blue led indicates we're going to increase the frequency
						}
					}
				}
			}
			if((user_bt_count>=50) && (!USR_BT_PRESS))	//if the led has been toggled and we released the button
			{
				flag_EXTI=0;							//clears flag
			}
		}//flag_EXTI
		else
		{
			user_bt_count=0;
		}//flag_EXTI
	}
	return 0;
}
