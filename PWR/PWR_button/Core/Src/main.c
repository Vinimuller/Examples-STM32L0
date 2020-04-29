#include "stm32l053xx.h"

int main(void)
{
	while(1)
	{
		/* Enable Clocks */
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

		/* Configure PA0 as External Interrupt */
//		GPIOA->MODER &= ~( GPIO_MODER_MODE0 ); // PA0 is in Input mode
//		EXTI->IMR |= EXTI_IMR_IM0; // interrupt request from line 0 not masked
//		EXTI->RTSR |= EXTI_RTSR_TR0; // rising trigger enabled for input line 0
		GPIOA->MODER 	&= 	~(	GPIO_MODER_MODE10_Msk	);
		GPIOA->PUPDR	|= 		GPIO_PUPDR_PUPD10_0		;
		EXTI->IMR	|= EXTI_IMR_IM10;
		EXTI->FTSR	|= EXTI_FTSR_FT10;

		// Enable interrupt in the NVIC
//		NVIC_EnableIRQ( EXTI0_1_IRQn );
//		NVIC_SetPriority( EXTI0_1_IRQn, BTN_INT_PRIO );
		NVIC_EnableIRQ(EXTI4_15_IRQn);
		NVIC_SetPriority(EXTI4_15_IRQn, 0);

		/* Prepare to enter stop mode */
		PWR->CR |= PWR_CR_CWUF; // clear the WUF flag after 2 clock cycles
		PWR->CR &= ~( PWR_CR_PDDS ); // Enter stop mode when the CPU enters deepsleep
		RCC->CFGR |= RCC_CFGR_STOPWUCK; // HSI16 oscillator is wake-up from stop clock
		SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; // low-power mode = stop mode
		__WFI(); // enter low-power mode

		asm("nop");
	}

	return 0;
}

//int main(void)
//{
//	uint8_t	bt_debounce = 0;	//used for debounce the button
//
//	RCC->APB1ENR |= RCC_APB1ENR_PWREN;	//enable PWR clock
//
//	/*							   *
//	 * --- GPIO INITIALIZATION --- *
//	 *							   */
//
//	/* -> GPIOA
//	 * PA14 -> AF mode (SWD_IO)		| Low speed	| Push pull
//	 * PA13 -> AF mode (SWD_CK)		| Low speed	| Push pull
//	 * PA10	-> Input (User Button)	| Low speed	| Pull-up
//	 */
//	RCC->IOPENR	 |=	RCC_IOPENR_GPIOAEN;
//
//	GPIOA->MODER &=	~GPIO_MODER_MODE10_Msk;
//	GPIOA->PUPDR |= GPIO_PUPDR_PUPD10_0;
//
//	//Green led, debug purpose
//	GPIOA->MODER &= ~GPIO_MODER_MODE7_1;
//	GPIOA->BSRR |= GPIO_BSRR_BR_7;
//	GPIOA->ODR |= GPIO_ODR_OD7_Msk;
//
//	while(1)
//	{
//		if(!(GPIOA->IDR & GPIO_IDR_ID10_Msk))
//		{
//			if(bt_debounce++==200)
//			{
//				while(!(GPIOA->IDR & GPIO_IDR_ID10_Msk));
//				(GPIOA->ODR ^=GPIO_IDR_ID7_Msk);
//
//				//entering stop mode procedure
//				DBGMCU->CR |= DBGMCU_CR_DBG_STOP;	//this bit needs to be set if you're going to debug this code
//
//				if(PWR->CSR & PWR_CSR_WUF)
//				{
//					PWR->CR |= PWR_CR_CWUF;		//clears WUF after 2 system clock cycles
//				}
//
//				PWR->CR |= 	PWR_CR_LPSDSR	|	//voltage regulator in low-power mode
//							PWR_CR_ULP		;	//ultra low power mode enable
//				//LPSDSR bit must be set before the LPRUN bit is set
//				PWR->CR |=	PWR_CR_LPRUN;		//voltage regulator in low-power mode
//
//				PWR->CR &= ~PWR_CR_PDDS;		//making sure we're entering stop mode
//
//				__WFE();
//			}
//		}
//		else
//		{
//			bt_debounce = 0;
//		}
//	}
//
//	return 0;
//}
