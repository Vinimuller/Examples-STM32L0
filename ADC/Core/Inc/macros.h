/*
 * macros.h
 *
 *  Created on: Mar 6, 2020
 *      Author: rodrigocelmer
 */

#ifndef INC_MACROS_H_
#define INC_MACROS_H_

//--- TEMPERATURE SENSOR CALIBRATION VALUE ADDRESS - FROM REFERENCE MANUAL
//we use them to calculate the temperature read by the ADC
#define TEMP130_CAL_ADDR	((uint16_t*) ((uint32_t) 0x1FF8007E))
#define TEMP30_CAL_ADDR		((uint16_t*) ((uint32_t) 0x1FF8007A))
#define VDD_CALIB			((uint16_t) (300))
#define VDD_APPLI			((uint16_t) (330))

//--- INTERNAL VOLTAGE REFERENCE VALUE ADDRESS - FROM REFERENCE MANUAL
//we use it to calculate the Vref read by the ADC
#define VREFINT_CAL		((uint16_t*) ((uint32_t) 0x1FF80078))

//--- TIME MANAGEMENT DEFINES
/*
 * Here we define constant values for Wait(uint16_t) function
 * System clock is running at 2.1 MHz
 * Ex.: 10 uS / (1 / 2.1 MHz) = 21 system clock cycles to wait for 10 uS
 */
#define TIME_10uSEC		21	//21 system clock cycles to wait for 10uS - time for the temperature sensor to wake up

#endif /* INC_MACROS_H_ */
