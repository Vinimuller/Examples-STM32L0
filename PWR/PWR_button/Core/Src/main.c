#include "stm32l053xx.h"

//------ DEFINES
//--- INPUTS
#define USR_BT_PRESS		(!(GPIOA->IDR & GPIO_IDR_ID10_Msk))

//--- OUTPUTS
#define GREEN_LED_ON		(GPIOA->ODR |= GPIO_ODR_OD7_Msk)
#define	GREEN_LED_OFF		(GPIOA->ODR &= ~GPIO_ODR_OD7_Msk)
#define TOGGLE_GREEN_LED	(GPIOA->ODR ^=GPIO_IDR_ID7_Msk)

//--- GENERAL
#define BUTTON_DEBOUNCE	50		//constant for button debounce
#define RELEASED		0		//user button status define
#define	PRESSED			1		//user button status define

uint8_t FlagEXTI 	= 0,	//this flag is set when there's an EXTI 10 interrupt
		UsrBtStatus	= 0,	//this var indicates the user button status (RELEASED or PRESSED)
		UsrBtCount	= 0;	//counter to debounce the button

void EXTI4_15_IRQHandler (void)
{
	if(EXTI->PR & EXTI_PR_PIF10)		//if there's an EXTI interrupt
	{
		EXTI->PR |= EXTI_PR_PIF10;		//clear the EXTI flag
		if(UsrBtStatus == RELEASED)	//if we haven't recognized the user button was definitely pressed yet
		{
			FlagEXTI = 1;				//we set the FlagEXTI
			UsrBtCount = 0;			//and reset the UsrBtCount
		}
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
	GREEN_LED_ON;

	/*									*
	 * --- STOP MODE INITIALIZATION --- *
	 *									*/
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;	//enable PWR clock
	SCB->SCR = SCB_SCR_SLEEPDEEP_Msk; 	//low-power mode = stop mode
	PWR->CR &= ~PWR_CR_PDDS;			//making sure we're entering stop mode
	PWR->CR |= PWR_CR_CWUF		|		//clears WUF after 2 system clock cycles
				PWR_CR_LPSDSR	|		//voltage regulator in low-power mode
				PWR_CR_ULP		;		//ultra low power mode enable

	//entering stop mode procedure
	DBGMCU->CR |= DBGMCU_CR_DBG_STOP;	//this bit needs to be set if you're going to debug this code

	while(1)
	{
		if(FlagEXTI)									//if there was an EXTI interrupt
		{
			if(UsrBtCount < 250)						//button debouncing procedure
			{
				if(UsrBtCount++ >= BUTTON_DEBOUNCE)	//we increment UsrBtCount until it reaches BUTTON_DEBOUNCE value
				{
					UsrBtStatus = PRESSED;			//then we recognize the button is definitely pressed
					FlagEXTI = 0;						//and clear the FlagEXTI
				}
				else
				{
					UsrBtStatus = RELEASED;			//or the button is not pressed
				}
			}
		}

		if(UsrBtStatus == PRESSED)				//when we recognized the user button was pressed
		{
			if((!USR_BT_PRESS))						//if it was then released, tha uC start taking action
			{
				GREEN_LED_OFF;						//turn the green led off
				UsrBtStatus = RELEASED;			//set user button status as RELEASED

				__WFI();							//enters stop mode

				while(USR_BT_PRESS);				//holds here to wake up from stop mode

				GREEN_LED_ON;						//turn the green led on again

				FlagEXTI = 0;						//clear FlagEXTI after waking up (it is set since we pressed the button to wake up)
				EXTI->PR |= EXTI_PR_PIF10;			//clear EXTI flag after waking up (it is set since we pressed the button to wake up)
			}
		}
	}

	return 0;
}
