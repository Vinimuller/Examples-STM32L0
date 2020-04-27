#include "stm32l053xx.h"

int main(void)
{
	uint8_t	bt_debounce = 0;	//used for debounce the button
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
				(GPIOA->ODR ^=GPIO_IDR_ID7_Msk);
				while(!(GPIOA->IDR & GPIO_IDR_ID10_Msk));
			}
		}
		else
		{
			bt_debounce = 0;
		}
	}

	return 0;
}
