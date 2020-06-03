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

uint8_t flag_EXTI 		= 0,	//this flag is set when there's an EXTI 10 interrupt
		USR_BT_STATUS	= 0,	//this var indicates the user button status (RELEASED or PRESSED)
		user_bt_count	= 0;	//counter to debounce the button

void MCU_Init(void);

void EXTI4_15_IRQHandler (void)
{
	if(EXTI->PR & EXTI_PR_PIF10)		//if there's an EXTI interrupt
	{
		EXTI->PR |= EXTI_PR_PIF10;		//clear the EXTI flag
		if(USR_BT_STATUS == RELEASED)	//if we haven't recognized the user button was definitely pressed yet
		{
			flag_EXTI = 1;				//we set the flag_EXTI
			user_bt_count = 0;			//and reset the user_bt_count
		}
	}
}

int main(void)
{
	MCU_Init();

	while(1)
	{
		if(flag_EXTI)									//if there was an EXTI interrupt
		{
			if(user_bt_count < 250)						//button debouncing procedure
			{
				if(user_bt_count++ >= BUTTON_DEBOUNCE)	//we increment user_bt_count until it reaches BUTTON_DEBOUNCE value
				{
					USR_BT_STATUS = PRESSED;			//then we recognize the button is definitely pressed
					flag_EXTI = 0;						//and clear the flag_EXTI
				}
				else
				{
					USR_BT_STATUS = RELEASED;			//or the button is not pressed
				}
			}
		}

		if(USR_BT_STATUS == PRESSED)				//when we recognized the user button was pressed
		{
			if((!USR_BT_PRESS))						//if it was then released, tha uC start taking action
			{
				GREEN_LED_OFF;						//turn the green led off
				USR_BT_STATUS = RELEASED;			//set user button status as RELEASED

				__WFI();							//enters stop mode

				while(USR_BT_PRESS);				//holds here to wake up from stop mode

				GREEN_LED_ON;						//turn the green led on again

				flag_EXTI = 0;						//clear flag_EXTI after waking up (it is set since we pressed the button to wake up)
				EXTI->PR |= EXTI_PR_PIF10;			//clear EXTI flag after waking up (it is set since we pressed the button to wake up)
			}
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
	GREEN_LED_ON;

	/*									*
	 * --- STOP MODE INITIALIZATION --- *
	 *									*/
	SCB->SCR = SCB_SCR_SLEEPDEEP_Msk; 	//low-power mode = stop mode
	PWR->CR &= ~PWR_CR_PDDS;			//making sure we're entering stop mode
	PWR->CR |= PWR_CR_CWUF		|		//clears WUF after 2 system clock cycles
				PWR_CR_LPSDSR	|		//voltage regulator in low-power mode
				PWR_CR_ULP		;		//ultra low power mode enable

	//entering stop mode procedure
	DBGMCU->CR |= DBGMCU_CR_DBG_STOP;	//this bit needs to be set if you're going to debug this code
}

/*
 * https://www.digikey.com/eewiki/display/microcontroller/Low-Power+Modes+on+the+STM32L0+Series#Low-PowerModesontheSTM32L0Series-StopMode
 */
//int main(void)
//{
//	while(1)
//	{
//		/* Enable Clocks */
//		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
//		RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
//
//		/* Configure PA0 as External Interrupt */
////		GPIOA->MODER &= ~( GPIO_MODER_MODE0 ); // PA0 is in Input mode
////		EXTI->IMR |= EXTI_IMR_IM0; // interrupt request from line 0 not masked
////		EXTI->RTSR |= EXTI_RTSR_TR0; // rising trigger enabled for input line 0
//		GPIOA->MODER 	&= 	~(	GPIO_MODER_MODE10_Msk	);
//		GPIOA->PUPDR	|= 		GPIO_PUPDR_PUPD10_0		;
//		EXTI->IMR	|= EXTI_IMR_IM10;
//		EXTI->FTSR	|= EXTI_FTSR_FT10;
//
//		// Enable interrupt in the NVIC
////		NVIC_EnableIRQ( EXTI0_1_IRQn );
////		NVIC_SetPriority( EXTI0_1_IRQn, BTN_INT_PRIO );
//		NVIC_EnableIRQ(EXTI4_15_IRQn);
//		NVIC_SetPriority(EXTI4_15_IRQn, 0);
//
//		/* Prepare to enter stop mode */
//		PWR->CR |= PWR_CR_CWUF; // clear the WUF flag after 2 clock cycles
//		PWR->CR &= ~( PWR_CR_PDDS ); // Enter stop mode when the CPU enters deepsleep
//		RCC->CFGR |= RCC_CFGR_STOPWUCK; // HSI16 oscillator is wake-up from stop clock
//		SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; // low-power mode = stop mode
//		__WFI(); // enter low-power mode
//
//		asm("nop");
//	}
//
//	return 0;
//}

//int main(void)
//{
//	while(1)
//	{
//	    /* Enable Clocks */
//	    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
//	    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
//
//	    /* Configure PA0 as External Interrupt */
////	    GPIOA->MODER &= ~( GPIO_MODER_MODE0 ); // PA0 is in Input mode
////	    EXTI->IMR |= EXTI_IMR_IM0;   // interrupt request from line 0 not masked
////	    EXTI->RTSR |= EXTI_RTSR_TR0; // rising trigger enabled for input line 0
//	    GPIOA->MODER 	&= 	~(	GPIO_MODER_MODE10_Msk	);
//		GPIOA->PUPDR	|= 		GPIO_PUPDR_PUPD10_0		;
//		EXTI->IMR	|= EXTI_IMR_IM10;
//		EXTI->FTSR	|= EXTI_FTSR_FT10;
//
//	    // Enable interrupt in the NVIC
////	    NVIC_EnableIRQ( EXTI0_1_IRQn );
////	    NVIC_SetPriority( EXTI0_1_IRQn, BTN_INT_PRIO );
//		NVIC_EnableIRQ(EXTI4_15_IRQn);
//		NVIC_SetPriority(EXTI4_15_IRQn, 0);
//
//	    /* Prepare to enter stop mode */
//	    PWR->CR |= PWR_CR_CWUF;      // clear the WUF flag after 2 clock cycles
//	    PWR->CR &= ~( PWR_CR_PDDS ); // Enter stop mode when the CPU enters deepsleep
//	    // V_REFINT startup time ignored | V_REFINT off in LP mode | regulator in LP mode
//	    PWR->CR |= PWR_CR_FWU | PWR_CR_ULP | PWR_CR_LPSDSR;
//	    RCC->CFGR |= RCC_CFGR_STOPWUCK; // HSI16 oscillator is wake-up from stop clock
//	    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; // low-power mode = stop mode
//
//	    __disable_irq();
//
//	    //Idd_SaveContext();
//	    I2C1->CR1 &= ~I2C_CR1_PE;  // Address issue 2.5.1 in Errata
//	    __WFI(); // enter low-power mode
//
//	    I2C1->CR1 |= I2C_CR1_PE;
//	    //Idd_RestoreContext();
//
//	    __enable_irq(); // <-- go to isr
//	}
//
//	return 0;
//}
