/*
 * macros.h
 *
 *  Created on: Mar 6, 2020
 *      Author: rodrigocelmer
 */

#ifndef INC_MACROS_H_
#define INC_MACROS_H_

//--- Temperature sensor calibration value address
#define TEMP130_CAL_ADDR	((uint16_t*) ((uint32_t) 0x1FF8007E))
#define TEMP30_CAL_ADDR		((uint16_t*) ((uint32_t) 0x1FF8007A))
#define VDD_CALIB			((uint16_t) (300))
#define VDD_APPLI			((uint16_t) (330))

//--- Internal voltage reference value address
#define VREFINT_CAL		((uint16_t*) ((uint32_t) 0x1FF80078))

#endif /* INC_MACROS_H_ */
