#include "stm32l053xx.h"

#define BUTTON_DEBOUNCE	50

uint8_t flag_EXTI 		= 0,
		user_bt_count	= 0;	//counter to debounce the button

void MCU_Init(void);

void EXTI4_15_IRQHandler (void)
{
	if(EXTI->PR & EXTI_PR_PIF10)	//if there's EXTI interrupt
	{
		flag_EXTI = 1;
		EXTI->PR |= EXTI_PR_PIF10;	//clears the EXTI flag
	}
}

int main(void)
{
	MCU_Init();

	while(1)
	{
		if(flag_EXTI)
		{
			if(user_bt_count < 250)					//limits the counting to 250
			{
				user_bt_count++;					//counts up for debouncing
			}

			if((user_bt_count >= BUTTON_DEBOUNCE) && (GPIOA->IDR & GPIO_IDR_ID10_Msk))
			{
				GPIOA->ODR &= ~GPIO_ODR_OD7_Msk;
				EXTI->PR |= EXTI_PR_PIF10;

				SCB->SCR = SCB_SCR_SLEEPDEEP_Msk; 	// low-power mode = stop mode
				PWR->CR &= ~PWR_CR_PDDS;			//making sure we're entering stop mode
				PWR->CR |= PWR_CR_CWUF;				//clears WUF after 2 system clock cycles

				PWR->CR |= 	PWR_CR_LPSDSR	|		//voltage regulator in low-power mode
							PWR_CR_ULP		;		//ultra low power mode enable
				//LPSDSR bit must be set before the LPRUN bit is set
				PWR->CR |=	PWR_CR_LPRUN;			//voltage regulator in low-power mode

				//entering stop mode procedure
//				DBGMCU->CR |= DBGMCU_CR_DBG_STOP;	//this bit needs to be set if you're going to debug this code

				PWR->CR |=	PWR_CR_DBP;				//enable write access to the RTC registers
				// --- Unlocking RTC's write protection
				RTC->WPR = 0xCA;
				RTC->WPR = 0x53;
				// --- RTC's unlocked
				RTC->CR |= RTC_CR_WUTE;				//enables RTC - starts counting

				__WFI();	//stop mode

				while(!(GPIOA->IDR & GPIO_IDR_ID10_Msk));	//holds here (for wake from stop mode)

				RTC->ISR &= ~RTC_ISR_WUTF;			//clears RTC wakeup flag

				MCU_Init();

				flag_EXTI = 0;
				EXTI->PR |= EXTI_PR_PIF10;

			}
		}
		else
		{
			GPIOA->ODR |= GPIO_ODR_OD7_Msk;
			user_bt_count=0;
		}
	}

	return 0;
}

void MCU_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;	//enable PWR clock

	/*							   *
	 * --- GPIO INITIALIZATION --- *
	 *							   */

	/* -> GPIOA
	 * PA14 -> AF mode (SWD_IO)		| Low speed	| Push pull
	 * PA13 -> AF mode (SWD_CK)		| Low speed	| Push pull
	 * PA10	-> Input (User Button)	| Low speed	| Pull-up
	 */
	RCC->IOPENR	 |=	RCC_IOPENR_GPIOAEN;

	GPIOA->MODER &=	~GPIO_MODER_MODE10_Msk;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD10_0;
	EXTI->IMR	|= EXTI_IMR_IM10;
	EXTI->FTSR	|= EXTI_FTSR_FT10;

	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn, 0);

	//Green led, debug purpose
	GPIOA->MODER &= ~GPIO_MODER_MODE7_1;
	GPIOA->BSRR |= GPIO_BSRR_BR_7;
	GPIOA->ODR |= GPIO_ODR_OD7_Msk;

	/*							   *
	 *  --- RTC INITIALIZATION --- *
	 *							   */
	RCC->CSR	|=	RCC_CSR_RTCSEL_LSI;		//sets LSI as RTC clock source (37 kHz)
	RCC->CSR	|=	RCC_CSR_RTCEN;			//enables the RTC clock

	PWR->CR |=	PWR_CR_DBP;					//enable write access to the RTC registers
	// --- Unlocking RTC's write protection
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	// --- RTC's unlocked
	RTC->CR &= ~RTC_CR_WUTE;				//disables the wakeup timer
	while(!(RTC->ISR & RTC_ISR_WUTWF));		//polling WUTWF until it is set
	RTC->WUTR 	= 	30;						//wakeup timer set to 30 seconds
	EXTI->RTSR 	|= 	EXTI_RTSR_RT20;			//EXTI line 20 sensitive to rising edges (wakeup event)
	RTC->CR 	|=	RTC_CR_WUCKSEL_2	|	//WUCKSEL = 10x: ck_spre (usually 1 Hz) clock selected
					RTC_CR_WUTIE		;	//enables periodic wakeup interrupt (to exit from stop mode)
	PWR->CR 	&= ~PWR_CR_DBP;				//disable write access to the RTC registers

	NVIC_EnableIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 0);
}
