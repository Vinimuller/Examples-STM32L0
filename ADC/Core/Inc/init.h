/*
 * init.h
 *
 *  Created on: Mar 10, 2020
 *      Author: rodrigocelmer
 */

#ifndef INC_INIT_H_
#define INC_INIT_H_

void GPIO_Init (void);
void TIM6_Init (void);	//Interrupt every 100 kHz/10 us
void ADC_Init (void);

#endif /* INC_INIT_H_ */
