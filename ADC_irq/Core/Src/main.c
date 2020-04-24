/*
 * 	Implement a routine that reads the internal voltage reference (ADC_IN17)
 * 	and internal temperature sensor (ADC_IN18).
 * 	The internal reference voltage must be in millivolts and the temperature in Celsius.
 *
 * 	1 - Polling		[X]
 * 	2 - Interrupt	[/]
 * 	3 - DMA			[ ]
 */

#include "stm32l053xx.h"
#include "macros.h"
#include "struct.h"

ADC_Status	ADC_Config(uint32_t Channel);	//we call this function with the ADC channel we want to read
void 		wait(uint16_t time);			//delay function, system clock based

uint8_t	flag_EOC = 0;						//flag for End Of Conversion (ADC converted one single channel)

void ADC1_COMP_IRQHandler(void)
{
	if(ADC1->ISR & ADC_ISR_EOC)			//if there was an End Of Conversion
	{
		flag_EOC = 1;					//sets flag_EOC
		ADC1->ISR |= ADC_ISR_EOC;		//clears EOC interrupt flag by writing 1 to it

		if(ADC1->ISR & ADC_ISR_OVR)
		{
			while(1);	//so we can hold the program at fault
		}
	}
}

int main(void)
{
	uint16_t	v_ref 		= 0;	//internal reference voltage measured (after calculation)
	int16_t		temperature	= 0;	//temperature in Celsius degrees measured (after calculation)

	/*										*
	 * --- ADC INITIALIZATION PROCEDURE --- *
	 *										*/
	RCC->APB2ENR 	|= 	RCC_APB2ENR_ADC1EN;		//enables ADC clock
	ADC1->CR 		|=	ADC_CR_ADVREGEN;		//enables the voltage regulator
	ADC1->CFGR2 	|= 	ADC_CFGR2_CKMODE_0;		//sets ADC clock as PCLK/2
	ADC->CCR 		|= 	ADC_CCR_LFMEN;			//low frequency mode enable
	ADC1->SMPR 		|= 	ADC_SMPR_SMPR_1	|		//sets sample time for 12.5 ADC clock cycles
						ADC_SMPR_SMPR_0	;		//because we need 10.5 ADC clock cycles at least
	ADC1->CFGR1		|=	ADC_CFGR1_WAIT;			//enables wait mode to prevent overrun

	ADC1->CR |= ADC_CR_ADCAL;					//starting the calibration
	while(ADC1->CR & ADC_CR_ADCAL);				//we have to wait until ADCAL = 0 (Can be handled by interrupt)

	ADC1->IER		|=	ADC_IER_EOCIE;			//enables End Of Conversion interrupt

	//Enables IRQ and IRQ priority
	NVIC_EnableIRQ(ADC1_COMP_IRQn);
	NVIC_SetPriority(ADC1_COMP_IRQn, 0);

	ADC1->ISR|= ADC_ISR_ADRDY;					//clear the ADRDY bit by programming it to 1
	ADC1->CR |= ADC_CR_ADEN;					//then we enable the ADC
	while(!(ADC1->ISR & ADC_ISR_ADRDY));		//and wait for it to be ready (Can be handled by interrupt)

	/*												*
	 * --- ADC CHANNEL CONFIG FOR V_REF READING --- *
	 *												*/
	if(ADC1->CR & ADC_CR_ADSTART)			//we have to be sure there's no ongoing conversion
	{
		ADC1->CR |= ADC_CR_ADSTP;			//if so, we have to set ADSTP bit
		while(ADC1->CR & ADC_CR_ADSTP);		//and wait for it to clear
	}
	ADC1->CR |= ADC_CR_ADDIS;				//disables the ADC
	while(ADC1->CR & ADC_CR_ADEN);

	ADC1->CHSELR = ADC_CHSELR_CHSEL17;		//selecting the VREF channel
	ADC->CCR |= ADC_CCR_VREFEN; 			//enables internal reference voltage

	ADC1->CR |= ADC_CR_ADEN;				//then we enable the ADC
	while(!(ADC1->ISR & ADC_ISR_ADRDY));	//and wait for it to be ready. (Can be handled by interrupt)

	/*									*
	 * --- STARTING THE ADC READING --- *
	 *									*/
	ADC1->CR |= ADC_CR_ADSTART;				//starts the ADC

	while(1)
	{
		if(flag_EOC)									//if THERE WAS an ADC interrupt
		{
			flag_EOC = 0;

			switch (ADC1->CHSELR)
			{
				case ADC_CHSELR_CHSEL17:				//Internal reference voltage
						v_ref = ADC1->DR;				//storing the data read in v_ref

						//configuring the next channel to be read
						ADC1->CHSELR = ADC_CHSELR_CHSEL18;
						ADC->CCR 	|= ADC_CCR_TSEN; 	//enables temperature sensor
						wait(TIME_10uSEC);				//we have to wait for the proper time for the Tsense to wake up
						ADC1->CR 	|= ADC_CR_ADSTART;	//starts the ADC
				break;
				case ADC_CHSELR_CHSEL18:				//Internal temperature
						temperature = ADC1->DR;			//we'll store the data read in temperature
				break;
				default:
						//error management
				break;
			}

			if(ADC1->CHSELR & ADC_CHSELR_CHSEL18)		//if we've finished reading all channels (could be done with a #define)
			{											//we're going to calculate what we need
				uint16_t aux = v_ref;					//we only use aux to calculate v_ref

				//Vref calculation as in RM
				v_ref = (3 * (int32_t) (*VREFINT_CAL));
				v_ref = (v_ref * 1000) / aux;			//*1000 so we have Vref in mV
														//the calculated internal reference voltage is now stored in v_ref

				//temperature calculation as in RM example
				temperature = ((temperature * VDD_APPLI / VDD_CALIB) - (int32_t) *TEMP30_CAL_ADDR);
				temperature = temperature * (int32_t)(130 - 30);
				temperature = temperature / (int32_t)(*TEMP130_CAL_ADDR - *TEMP30_CAL_ADDR);
				temperature = temperature + 30;			//the calculated temperature is now stored in temperature

				asm("nop");		//so we can hold the uC here after reading the temperature and Vref
			}

		}
	}

	return 0;
}

void wait(uint16_t time)
{
	while(time--);
}

/*
 * --- CALIBRATION
 * > ADC calculates a calibration factor which is internally applied to the ADC until ADC power-off
 * > Wait until calibration is complete and should be performed BEFORE any AD conversion
 * > initiated with ADCAL = 1 (must ADEN = 0) and will stay =1 until calibration is done
 * > Calibration factor on ADC_DR [6:0]
 * > the calibration is kept with ADEN = 0 or ADVREGEN = 0
 *
 * -> Procedure:
 * ADEN = 0; DMAEN = 0;
 * ADCAL = 1;
 * wait until ADCAL = 0 or EOCAL = 1
 * calibration factor will be in bits 6:0 in ADC_DR or ADC_CALFACT
 * pg. 979 example
 */

/*
 * --- ON-OFF Control
 * > ADEN = 1 enables the adc. ADRDY flag is set when ADC is ready for operation
 * ADVREGEN is automatically set when ADEN = 1 (tSTAB is longer)
 * > ADDIS = 1 disables the ADC and puts in power down mode. ADEN and ADDIS go to 0
 * > ADSTART = 1 starts the conversion or with an external event
 *
 * -> Procedure to enable:
 * ADRDY = 1 in ADC_ISR to clear it
 * ADEN = 1
 * wait until ADRDY = 1 (can be handled by interrupt)
 * pg. 980 example
 *
 * -> Procedure to disable
 * check if ADSTART = 0 (ADSTP = 1 stops any conversion in ADC_CR)
 * ADDIS = 1
 * wait until ADEN = 0 (ADC_CR) and ADDIS = 0
 * clear ADRDY writing 1 to it
 */

/*
 * --- Clock
 * >LFMEN = 1 (ADC_CCR) low frequency mode enable
 */

/*
 * --- channel select
 * > single channel or sequence (ADC_CHSELR)
 * > SCANDIR = 0 ch0 -> ch18 (ADC_CFGR1) - VREF (ADC_IN17), TEMP (ADC_IN18)
 */

/*
 * --- Sampling Time
 * tCONV = Sampling time + 12.5 x ADC clock cycles
 * Ex.:
 * ADC_CLK = 16 MHz, sampling time = 1.5 ADC clock cycles
 * tCONV = 1.5 + 12.5 = 14 ADC clock cycles = 0.875 us (?)
 */
