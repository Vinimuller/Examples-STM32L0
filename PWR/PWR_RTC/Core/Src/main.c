#include "stm32l053xx.h"

//------ DEFINES
//--- INPUTS
#define USR_BT_PRESS		(!(GPIOA->IDR & GPIO_IDR_ID10_Msk))

//--- OUTPUTS
#define GREEN_LED_ON		(GPIOA->ODR |= GPIO_ODR_OD7_Msk)
#define	GREEN_LED_OFF		(GPIOA->ODR &= ~GPIO_ODR_OD7_Msk)
#define TOGGLE_GREEN_LED	(GPIOA->ODR ^=GPIO_IDR_ID7_Msk)

//--- GENERAL
#define RTC_30_SECONDS	(30 + 1)//used for RTC config, equals to 30 seconds (time spent in standy mode)
#define RTC_KEY1		0xCA	//1st key to unlock RTC's registers (write to RTC_WPR)
#define RTC_KEY2		0x53	//2nd key to unlock RTC's registers (write to RTC_WPR)

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

	//Green led, debug purpose
	GPIOA->MODER &= ~GPIO_MODER_MODE7_1;	//set PA7 as output
	GPIOA->BSRR |= GPIO_BSRR_BR_7;			//resets the corresponding OD7 bit
	GREEN_LED_ON;

	/*									   *
	 * --- STANDBY MODE INITIALIZATION --- *
	 *									   */
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;	//enable PWR clock
	SCB->SCR = SCB_SCR_SLEEPDEEP_Msk; 	//low-power mode = deep sleep
	PWR->CR |= 	PWR_CR_PDDS;			//making sure we're entering standby mode
	PWR->CR |= 	PWR_CR_CWUF		|		//clears WUF after 2 system clock cycles
				PWR_CR_ULP		;		//ultra low power mode enable

	//entering standy mode procedure
	DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY;	//this bit needs to be set if you're going to debug this code

	/*							   *
	 *  --- RTC INITIALIZATION --- *
	 *							   */
	// --- Resetting RTC registers (debug purpose)
	RCC->CSR |= RCC_CSR_RTCRST;
	while(!(RCC->CSR & RCC_CSR_RTCRST));
	RCC->CSR &= ~RCC_CSR_RTCRST;
	while((RCC->CSR & RCC_CSR_RTCRST));

	PWR->CR  |=	PWR_CR_DBP;						//enable write access to the RTC and RCC CSR registers
	RCC->CSR |=	RCC_CSR_RTCSEL_LSI	|			//sets LSI as RTC clock source (37 kHz)
				RCC_CSR_RTCEN		|			//enables the RTC clock (those bits are write protected!)
				RCC_CSR_LSION;					//LSI oscillator ON
	while(!(RCC->CSR & RCC_CSR_LSIRDY));		//poll until LSI is stable
	// --- Unlocking RTC's write protection
	RTC->WPR = RTC_KEY1;
	RTC->WPR = RTC_KEY2;
	// --- RTC's unlocked
	RTC->CR &= ~RTC_CR_WUTE;						//disables the wakeup timer
	while(!(RTC->ISR & RTC_ISR_WUTWF));				//polling WUTWF until it is set
	RTC->ISR &= ~RTC_ISR_WUTF;						//clears RTC wakeup flag
	RTC->ISR |= RTC_ISR_INIT;						//RTC enters initialization mode
	while(!(RTC->ISR & RTC_ISR_INITF));				//polling initialization mode flag
	RTC->PRER = (( 36 << RTC_PRER_PREDIV_A_Pos) |	//set f_apre to 1 kHz
				 (999 << RTC_PRER_PREDIV_S_Pos));	//set f_spre to 1 Hz - RTC clock source
	RTC->WUTR = RTC_30_SECONDS;						//wakeup timer set to 30 seconds
	RTC->CR	 |=	RTC_CR_WUCKSEL_2	|				//10x: ck_spre as RTC clock input (1 Hz)
				RTC_CR_WUTIE;						//enables periodic wakeup interrupt (to exit from standy mode)
	RTC->ISR &= ~RTC_ISR_INIT;						//RTC exits initialization mode
	while(RTC->ISR & RTC_ISR_INITF);				//polling initialization mode flag
	PWR->CR &= ~PWR_CR_DBP;							//disable write access to the RTC registers

	while(1)
	{
		if(USR_BT_PRESS)
		{
			GREEN_LED_OFF;				//turn the green led off
			while(USR_BT_PRESS);		//hold here if the button is still pressed (debounce)

			PWR->CR |= PWR_CR_DBP;		//enable write access to the RTC and RCC CSR registers
			RTC->CR |= RTC_CR_WUTE;		//enables RTC - starts counting

			__WFI();					//standy mode - resets the uC on wake-up (check RCC_CSR_SBF)
		}
	}

	return 0;
}
