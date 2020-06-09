#include "stm32l053xx.h"

//------ DEFINES
//--- INPUTS
#define USR_BT_PRESS		(!(GPIOA->IDR & GPIO_IDR_ID10_Msk))

//--- OUTPUTS
#define GREEN_LED_ON		(GPIOA->ODR |= GPIO_ODR_OD7_Msk)
#define	GREEN_LED_OFF		(GPIOA->ODR &= ~GPIO_ODR_OD7_Msk)
#define TOGGLE_GREEN_LED	(GPIOA->ODR ^=GPIO_IDR_ID7_Msk)

//------ VARIABLES
uint8_t flagEXTI = 0;	//this flag is set when there's an EXTI 10 interrupt

void EXTI4_15_IRQHandler (void)
{
	if(EXTI->PR & EXTI_PR_PIF10)	//if there's an EXTI interrupt
	{
		EXTI->PR |= EXTI_PR_PIF10;	//clear the EXTI flag
		flagEXTI = 1;				//we set the flagEXTI
	}
}

int main(void)
{
	/*							   *
	 * --- GPIO INITIALIZATION --- *
	 *							   */
	/* -> GPIOA
	 * PA14 -> AF mode (SWD_IO)		| Low speed	| Push pull
	 * PA13 -> AF mode (SWD_CK)		| Low speed	| Push pull
	 * PA10	-> Input (User Button)	| Low speed	| Pull-up
	 */
	RCC->IOPENR	 |=	RCC_IOPENR_GPIOAEN;		//enable GPIOA clock

	GPIOA->MODER &=	~GPIO_MODER_MODE10_Msk;	//set PA10 as input
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD10_0;	//enables PA10 pull-up
	EXTI->IMR	|= EXTI_IMR_IM10;			//interrupt request from Line 10 is not masked
	EXTI->FTSR	|= EXTI_FTSR_FT10;			//falling trigger enabled for input line 10

	NVIC_EnableIRQ(EXTI4_15_IRQn);			//enable EXTI interrupt on NVIC
	NVIC_SetPriority(EXTI4_15_IRQn, 0);		//set EXTI interrupt priority

	//Green led, debug purpose
	GPIOA->MODER &= ~GPIO_MODER_MODE7_1;	//set PA7 as output
	GPIOA->BSRR |= GPIO_BSRR_BR_7;			//resets the corresponding OD7 bit
	GREEN_LED_ON;

	/*									*
	 * --- STOP MODE INITIALIZATION --- *
	 *									*/
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;	//enable PWR clock
	SCB->SCR = SCB_SCR_SLEEPDEEP_Msk; 	//low-power mode = stop mode
	PWR->CR &= ~PWR_CR_PDDS;			//making sure we're entering stop mode
	PWR->CR |= 	PWR_CR_CWUF		|		//clears WUF after 2 system clock cycles
				PWR_CR_LPSDSR	|		//voltage regulator in low-power mode
				PWR_CR_ULP		;		//ultra low power mode enable

	//entering stop mode procedure
	DBGMCU->CR |= DBGMCU_CR_DBG_STOP;	//this bit needs to be set if you're going to debug this code

	while(1)
	{
		if(flagEXTI)							//if there was an EXTI interrupt
		{
			flagEXTI = 0;						//we clear flagEXTI
			GREEN_LED_OFF;						//turn the green led off
			while(USR_BT_PRESS);				//holds here if the button is still pressed
			RCC->IOPENR &= ~RCC_IOPENR_GPIOAEN;	//disable GPIOA clock
			RCC->APB1ENR &= ~RCC_APB1ENR_PWREN;	//disable PWR clock

			__WFI();							//enters stop mode

			RCC->APB1ENR |= RCC_APB1ENR_PWREN;	//enable PWR clock
			RCC->IOPENR |= RCC_IOPENR_GPIOAEN;	//enable GPIOA clock
			while(USR_BT_PRESS);				//holds here if the button is still pressed
			GREEN_LED_ON;						//turn the green led on again
			flagEXTI = 0;						//clear flagEXTI after waking up (it is set since we pressed the button to wake up)
		}
	}
	return 0;
}
