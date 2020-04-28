#include "stm32l053xx.h"

int main(void)
{
	uint8_t	bt_debounce = 0;	//used for debounce the button

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

	//Green led, debug purpose
	GPIOA->MODER &= ~GPIO_MODER_MODE7_1;
	GPIOA->BSRR |= GPIO_BSRR_BR_7;
	GPIOA->ODR |= GPIO_ODR_OD7_Msk;

	while(1)
	{
		if(!(GPIOA->IDR & GPIO_IDR_ID10_Msk))
		{
			if(bt_debounce++==200)
			{
				while(!(GPIOA->IDR & GPIO_IDR_ID10_Msk));
				(GPIOA->ODR ^=GPIO_IDR_ID7_Msk);

				//entering stop mode procedure
				DBGMCU->CR |= DBGMCU_CR_DBG_STOP;	//this bit needs to be set if you're going to debug this code

				if(PWR->CSR & PWR_CSR_WUF)
				{
					PWR->CR |= PWR_CR_CWUF;		//clears WUF after 2 system clock cycles
				}

				PWR->CR |= 	PWR_CR_LPSDSR	|	//voltage regulator in low-power mode
							PWR_CR_ULP		;	//ultra low power mode enable
				//LPSDSR bit must be set before the LPRUN bit is set
				PWR->CR |=	PWR_CR_LPRUN;		//voltage regulator in low-power mode

				PWR->CR &= ~PWR_CR_PDDS;		//making sure we're entering stop mode

				__WFE();
			}
		}
		else
		{
			bt_debounce = 0;
		}
	}

	return 0;
}
