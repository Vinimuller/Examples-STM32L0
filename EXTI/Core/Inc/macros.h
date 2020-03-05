/*
 * macros.h
 *
 *  Created on: Mar 5, 2020
 *      Author: rodrigocelmer
 */

#ifndef INC_MACROS_H_
#define INC_MACROS_H_

//------ DEFINES
//--- INPUTS
#define USR_BT_PRESS		(!(GPIOA->IDR & GPIO_IDR_ID10_Msk))

//--- OUTPUTS
#define RED_LED_ON			(GPIOB->ODR |= GPIO_ODR_OD0_Msk)
#define RED_LED_OFF			(GPIOB->ODR &= ~GPIO_ODR_OD0_Msk)
#define TOGGLE_RED_LED		(GPIOB->ODR ^= GPIO_IDR_ID0_Msk)
#define GREEN_LED_ON		(GPIOA->ODR |= GPIO_ODR_OD7_Msk)
#define	GREEN_LED_OFF		(GPIOA->ODR &= ~GPIO_ODR_OD7_Msk)
#define TOGGLE_GREEN_LED	(GPIOA->ODR ^=GPIO_IDR_ID7_Msk)
#define BLUE_LED_ON			(GPIOB->ODR |= GPIO_ODR_OD1_Msk)
#define BLUE_LED_OFF		(GPIOB->ODR &= ~GPIO_ODR_OD1_Msk)
#define TOGGLE_BLUE_LED		(GPIOB->ODR ^= GPIO_IDR_ID1_Msk)

//--- GENERAL
#define UP		1
#define DOWN	0

#endif /* INC_MACROS_H_ */
