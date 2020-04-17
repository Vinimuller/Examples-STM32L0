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
#include "init.h"
#include "struct.h"

uint8_t	flag_EOC = 0,	//flag for End Of Conversion (ADC converted one single channel)
		flag_EOS = 0;	//flag for End Of conversion Sequence (ADC converted all channels enabled)

ADC_Status	ADC_Config();	//we call this function with the ADC channel we want to read
void 		wait(uint16_t);					//delay function, system clock based

int main(void)
{
	uint16_t	measure 	= 0,	//ADC data read is stored here after a conversion
				v_ref 		= 0;	//internal reference voltage measured (after calculation)
	int16_t		temperature	= 0;	//temperature in Celsius degrees measured (after calculation)

	ADC_Init();	//ADC initialization function

	while(1)
	{
		if((flag_EOC==0) && (!(ADC1->CR & ADC_CR_ADSTART)) && (!(ADC1->ISR & ADC_ISR_EOSEQ)))	//if there was NOT an ADC interrupt and we haven't started it already
		{
			ADC_Config();	//we'll configure the ADC for internal temperature reading
			ADC1->CR |= ADC_CR_ADSTART;		//and start the ADC
		}
		else
		{
			if(flag_EOC!=0)		//if THERE WAS an ADC interrupt
			{
				switch (flag_EOC)
				{
					case 1:	//Internal reference voltage
							measure = ADC1->DR;					//storing the data read in measure

							//Vref calculation as in RM
							v_ref = (3 * (int32_t) (*VREFINT_CAL));
							v_ref = (v_ref * 1000) / measure;		//*1000 so we have Vref in mV
																	//the calculated internal reference voltage is now stored in v_ref
					break;
					case 2:	//Internal temperature
							measure = ADC1->DR;		//we'll store the data read in measure

							//temperature calculation as in RM example
							temperature = ((measure * VDD_APPLI / VDD_CALIB) - (int32_t) *TEMP30_CAL_ADDR);
							temperature = temperature * (int32_t)(130 - 30);
							temperature = temperature / (int32_t)(*TEMP130_CAL_ADDR - *TEMP30_CAL_ADDR);
							temperature = temperature + 30;			//the calculated temperature is now stored in temperature
					break;
					default:
							//error management
					break;

				}

				if(ADC1->ISR & ADC_ISR_EOSEQ)		//if there was and End Of Sequence
				{
					flag_EOC = 0;					//clears flag_EOC
					ADC1->ISR |= ADC_ISR_EOSEQ;		//clears EOS interrupt flag by writing 1 to it
				}
			}
		}

		asm("nop");		//so we can hold the uC here after reading the temperature and Vref
	}

	return 0;
}

ADC_Status ADC_Config()
{
	if(ADC1->CR & ADC_CR_ADSTART)			//we have to be sure there's no ongoing conversion
	{
		ADC1->CR |= ADC_CR_ADSTP;			//if so, we have to set ADSTP bit
		while(ADC1->CR & ADC_CR_ADSTP);		//and wait for it to clear
	}
	ADC1->CR |= ADC_CR_ADDIS;				//disables the ADC
	while(ADC1->CR & ADC_CR_ADEN);

	ADC1->CHSELR|=	ADC_CHSELR_CHSEL18	|	//selecting the TSEN channel
					ADC_CHSELR_CHSEL17	;	//and the VREF channel
	ADC->CCR 	|=	ADC_CCR_TSEN	|		//enables temperature sensor
					ADC_CCR_VREFEN	; 		//and the internal reference voltage
	wait(TIME_10uSEC);						//we have to wait for the proper time for the Tsense to wake up

	ADC1->ISR|= ADC_ISR_ADRDY;				//clear the ADRDY bit by programming it to 1
	ADC1->CR |= ADC_CR_ADEN;				//then we enable the ADC
	while(!(ADC1->ISR & ADC_ISR_ADRDY));	//and wait for it to be ready. (Can be handled by interrupt)

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
