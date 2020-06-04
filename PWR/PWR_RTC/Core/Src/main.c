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

//------ VARIABLES
uint8_t FlagEXTI 	= 0,		//this flag is set when there's an EXTI 10 interrupt
		UsrBtStatus	= 0,		//this var indicates the user button status (RELEASED or PRESSED)
		UsrBtCount	= 0;		//counter to debounce the button

void MCU_Init(void);

int main(void)
{
	MCU_Init();

	while(1)
	{
		if(USR_BT_PRESS)
		{
			if(UsrBtCount++ >= BUTTON_DEBOUNCE)	//we increment UsrBtCount until it reaches BUTTON_DEBOUNCE value
			{
				UsrBtStatus = PRESSED;			//then we recognize the button is definitely pressed
				FlagEXTI = 0;					//and clear the FlagEXTI
			}
			else
			{
				UsrBtStatus = RELEASED;			//or the button is not pressed
			}

			if(UsrBtStatus == PRESSED)			//when we recognized the user button was pressed
			{
				if((!USR_BT_PRESS))				//if it was then released, tha uC start taking action
				{
					GREEN_LED_OFF;				//turn the green led off
					UsrBtStatus = RELEASED;		//set user button status as RELEASED

					RTC->CR |= RTC_CR_WUTE;		//enables RTC - starts counting
					asm("nop");

					__WFI();					//stop mode

					while(USR_BT_PRESS);		//holds here to wake up from stop mode

					GREEN_LED_ON;				//turn the green led on again

					RTC->ISR &= ~RTC_ISR_WUTF;	//clears RTC wakeup flag
					FlagEXTI = 0;				//clear FlagEXTI after waking up (it is set since we pressed the button to wake up)
					EXTI->PR |= EXTI_PR_PIF10;	//clear EXTI flag after waking up (it is set since we pressed the button to wake up)
				}

			}
		}
		else
		{
			UsrBtCount = 0;
		}
	}

	return 0;
}

void MCU_Init(void)
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
//	EXTI->IMR	|= EXTI_IMR_IM10;
//	EXTI->FTSR	|= EXTI_FTSR_FT10;
//
//	NVIC_EnableIRQ(EXTI4_15_IRQn);
//	NVIC_SetPriority(EXTI4_15_IRQn, 0);

	//Green led, debug purpose
	GPIOA->MODER &= ~GPIO_MODER_MODE7_1;	//set PA7 as output
	GPIOA->BSRR |= GPIO_BSRR_BR_7;			//resets the corresponding OD7 bit
	GREEN_LED_ON;

	/*									   *
	 * --- STANDBY MODE INITIALIZATION --- *
	 *									   */
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;	//enable PWR clock
	SCB->SCR = SCB_SCR_SLEEPDEEP_Msk; 	//low-power mode = stop mode
	PWR->CR |= PWR_CR_PDDS;				//making sure we're entering standby mode
	PWR->CR |= PWR_CR_CWUF		|		//clears WUF after 2 system clock cycles
				PWR_CR_LPSDSR	|		//voltage regulator in low-power mode
				PWR_CR_ULP		;		//ultra low power mode enable

	//entering stop mode procedure
	DBGMCU->CR |= DBGMCU_CR_DBG_STOP;	//this bit needs to be set if you're going to debug this code

	/*							   *
	 *  --- RTC INITIALIZATION --- *
	 *							   */
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;			//enable PWR clock
	RCC->CSR 	 |=	RCC_CSR_RTCSEL_LSI	|		//sets LSI as RTC clock source (37 kHz)
					RCC_CSR_RTCEN		;		//enables the RTC clock

	PWR->CR |=	PWR_CR_DBP;						//enable write access to the RTC registers
	// --- Unlocking RTC's write protection
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	// --- RTC's unlocked
	RTC->CR &= ~RTC_CR_WUTE;					//disables the wakeup timer
	while(!(RTC->ISR & RTC_ISR_WUTWF));			//polling WUTWF until it is set
	RTC->PRER = (36 << RTC_PRER_PREDIV_A_Pos);	//sets asynchronous prescaler to 36 (f_apre = 1 kHz)
	RTC->WUTR = 30000;							//wakeup timer set to 30 seconds
//	EXTI->RTSR 	|= 	EXTI_RTSR_RT20;				//EXTI line 20 sensitive to rising edges (wakeup event)
	RTC->CR 	|=	RTC_CR_WUCKSEL_1	|		//WUCKSEL = 011: RTC/2
					RTC_CR_WUCKSEL_0	|		//WUCKSEL = 011: RTC/2
					RTC_CR_WUTIE		;		//enables periodic wakeup interrupt (to exit from stop mode)
	PWR->CR 	&= ~PWR_CR_DBP;					//disable write access to the RTC registers
//	RTC->WPR = 0xFE; /* (6) Disable write access */ //?
//	RTC->WPR = 0x64; /* (6) Disable write access */ //?

//	NVIC_EnableIRQ(RTC_IRQn);
//	NVIC_SetPriority(RTC_IRQn, 0);
}
