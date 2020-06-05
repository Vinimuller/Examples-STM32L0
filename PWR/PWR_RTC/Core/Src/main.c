#include "stm32l053xx.h"

//------ DEFINES
//--- INPUTS
#define USR_BT_PRESS		(!(GPIOA->IDR & GPIO_IDR_ID10_Msk))

//--- OUTPUTS
#define GREEN_LED_ON		(GPIOA->ODR |= GPIO_ODR_OD7_Msk)
#define	GREEN_LED_OFF		(GPIOA->ODR &= ~GPIO_ODR_OD7_Msk)
#define TOGGLE_GREEN_LED	(GPIOA->ODR ^=GPIO_IDR_ID7_Msk)

//--- GENERAL
#define RTC_30_SECONDS	32000	//used for RTC config

void MCU_Init(void);

int main(void)
{
	MCU_Init();

	while(1)
	{
		if(USR_BT_PRESS)
		{
			GREEN_LED_OFF;				//turn the green led off
			while(USR_BT_PRESS);		//hold here if the button is still pressed

			PWR->CR |= PWR_CR_DBP;		//enable write access to the RTC and RCC CSR registers
			RTC->CR |= RTC_CR_WUTE;		//enables RTC - starts counting

			__WFI();					//stop mode - resets the uC on wake-up (check RCC_CSR_SBF)
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
	// --- Resetting RTC registers (debug purpose)
	RCC->CSR |= RCC_CSR_RTCRST;
	while(!(RCC->CSR & RCC_CSR_RTCRST));
	RCC->CSR &= ~RCC_CSR_RTCRST;
	while((RCC->CSR & RCC_CSR_RTCRST));

	RCC->CSR |= RCC_CSR_LSION;					//LSI oscillator ON
	while(!(RCC->CSR & RCC_CSR_LSIRDY));		//poll until LSI is stable

	PWR->CR  |=	PWR_CR_DBP;						//enable write access to the RTC and RCC CSR registers
	RCC->CSR |=	RCC_CSR_RTCSEL_LSI	|			//sets LSI as RTC clock source (37 kHz)
				RCC_CSR_RTCEN		;			//enables the RTC clock (those bits are write protected!)
	// --- Unlocking RTC's write protection
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	// --- RTC's unlocked
	RTC->CR &= ~RTC_CR_WUTE;					//disables the wakeup timer
	while(!(RTC->ISR & RTC_ISR_WUTWF));			//polling WUTWF until it is set
	RTC->ISR &= ~RTC_ISR_WUTF;					//clears RTC wakeup flag
	RTC->ISR |= RTC_ISR_INIT;					//RTC enters initialization mode
	while(!(RTC->ISR & RTC_ISR_INITF));			//polling initialization mode flag
	RTC->PRER = (36 << RTC_PRER_PREDIV_S_Pos);	//set f_apre to 1 kHz
	RTC->WUTR = RTC_30_SECONDS;					//wakeup timer set to 30 seconds
	RTC->CR	 |=	RTC_CR_WUCKSEL_2	|			//10x: ck_spre (usually 1 Hz) clock is selected
				RTC_CR_WUTIE;					//enables periodic wakeup interrupt (to exit from stop mode)
	RTC->ISR &= ~RTC_ISR_INIT;					//RTC exits initialization mode
	while(RTC->ISR & RTC_ISR_INITF);			//polling initialization mode flag
	PWR->CR &= ~PWR_CR_DBP;						//disable write access to the RTC registers
//	RTC->WPR = 0xFE; /* (6) Disable write access */ //?
//	RTC->WPR = 0x64; /* (6) Disable write access */ //?
//	RTC->WPR = 0xFF; /*RTC registers can no more be modified*/	//?
}
