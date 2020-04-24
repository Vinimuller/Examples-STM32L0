#include "stm32l053xx.h"

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

	//Green led, debug purpose
	GPIOA->MODER &= ~GPIO_MODER_MODE7_1;
	GPIOA->BSRR |= GPIO_BSRR_BR_7;

	while(1)
	{
		if(!(GPIOA->IDR & GPIO_IDR_ID10_Msk))
		{
			(GPIOA->ODR ^=GPIO_IDR_ID7_Msk);
		}
	}

	return 0;
}
