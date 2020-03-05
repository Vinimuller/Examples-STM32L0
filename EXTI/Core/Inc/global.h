/*
 * global.h
 *
 *  Created on: Mar 5, 2020
 *      Author: rodrigocelmer
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

//--- Global Variables
int user_bt_count	=0,	//counter to debounce the button
	flag_EXTI		=0,	//flag for EXTI interrupt
	flag_TIM6		=0,	//flag for TIM6 interrupt
	flag_frequency	=1;	//flag to indicate if we are increasing or decreasing the frequency

#endif /* INC_GLOBAL_H_ */
