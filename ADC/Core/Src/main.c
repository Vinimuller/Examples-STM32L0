/*
 * 	Implement a routine that reads the internal voltage reference (ADC_IN17)
 * 	and internal temperature sensor (ADC_IN18).
 * 	The internal reference voltage must be in millivolts and the temperature in Celsius.
 *
 * 	1 - Polling		[/]
 * 	2 - Interrupt	[ ]
 * 	3 - DMA			[ ]
 */

#include "stm32l053xx.h"
#include "macros.h"
#include "init.h"

uint8_t		ADC_error	= 0;	//error management flag
uint16_t	measure 	= 0,	//AD read after a conversion
			v_ref 		= 0;	//internal reference voltage measured (after calculation)
int16_t		temperature	= 0;	//temperature in Celsius degrees measured (after calculation)

void ADC_Config (uint8_t Channel);
void Wait	 (uint16_t);

int main (void)
{
	ADC_Init();

	while(1)
	{
		ADC_Config(CH_INT_TEMP);			//internal temperature reading
		ADC1->CR |= ADC_CR_ADSTART;
		while(!(ADC1->ISR & ADC_ISR_EOC));
		measure = ADC1->DR & ADC_DR_DATA;

		//temperature calculation as in RM example
		temperature = ((measure * VDD_APPLI / VDD_CALIB) - (int32_t) *TEMP30_CAL_ADDR);
		temperature = temperature * (int32_t)(130 - 30);
		temperature = temperature / (int32_t)(*TEMP130_CAL_ADDR - *TEMP30_CAL_ADDR);
		temperature = temperature + 30;			//the temperature read is now stored in temperature


		ADC_Config(CH_V_REF);				//internal reference voltage reading
		ADC1->CR |= ADC_CR_ADSTART;
		while(!(ADC1->ISR & ADC_ISR_EOC));
		measure = ADC1->DR & ADC_DR_DATA;

		//Vref calculation as in RM
		v_ref = (3000 * (int32_t) (*VREFINT_CAL)) / measure;	//*3000 so we have Vref in mV

		while(1);	//pra travar o programa aqui
	}

	return 0;
}

void ADC_Config (uint8_t Channel)
{
	ADC_error = 0;							//ADC management flag
	if(ADC1->CR & ADC_CR_ADSTART)			//we have to be sure there's no ongoing conversion
	{
		ADC1->CR |= ADC_CR_ADSTP;
		while(ADC1->CR & ADC_CR_ADSTP);
	}
	ADC1->CR |= ADC_CR_ADDIS;				//disables the ADC
	while(ADC1->CR & ADC_CR_ADEN);

	switch (Channel){
	case CH_INT_TEMP:
		ADC1->CHSELR |= ADC_CHSELR_CHSEL18;		//selecting the TSEN channel
		ADC->CCR |= ADC_CCR_TSEN; 				//enables temperature sensor
		Wait(TIME_10uSEC);						//we have to wait for the proper time for the Tsense to wake up
	break;
	case CH_V_REF:
		ADC1->CHSELR |= ADC_CHSELR_CHSEL17;		//selecting the VREF channel
		ADC->CCR |= ADC_CCR_VREFEN; 			//enables internal reference voltage
	break;
	default:
		ADC_error = 1;	//ADC error
	break;
	}

	if(!ADC_error)		//if there wasn't any error
	{
		ADC1->CR |= ADC_CR_ADEN;				//then we enable the ADC
		while(!(ADC1->ISR & ADC_ISR_ADRDY));	//and wait for it to be ready. Can be handled by interrupt
	}
}

void Wait (uint16_t time)
{
	while(time)
	{
		time--;
	}
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

