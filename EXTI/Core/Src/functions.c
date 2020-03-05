/*
 * functions.c
 *
 *  Created on: Mar 5, 2020
 *      Author: rodrigocelmer
 */
#include "stm32l053xx.h"
#include "macros.h"

extern int	user_bt_count,	//counter to debounce the button
			count,			//counter to toggle the red led
			flag_EXTI,		//flag for EXTI interrupt
			flag_TIM6,		//flag for TIM6 interrupt
			flag_frequency;	//flag to indicate if we are increasing or decreasing the frequency

void Blink_Red_Led (void)
{
	flag_TIM6 = 0;		//clears TIM6 flag
	TOGGLE_RED_LED;		//toggle the red led every 100 ms
}

void Change_Frequency (void)
{
	user_bt_count++;							//counts up to 50 for debouncing
	if(user_bt_count==50)
	{
		if((TIM6->ARR > 19)&& flag_frequency == UP)
		{
			TIM6->ARR = TIM6->ARR - 10;
			TIM6->EGR |= TIM_EGR_UG;			//Update registers values
			if(TIM6->ARR <= 19)
			{
				flag_frequency = DOWN;
				GREEN_LED_ON;					//Green led indicates we're going to decrease the frequency
				BLUE_LED_OFF;
			}
		}
		else{
			if((TIM6->ARR <= 199) && flag_frequency == DOWN)
			{
				TIM6->ARR = TIM6->ARR + 10;
				TIM6->EGR |= TIM_EGR_UG;		//Update registers values
				if(TIM6->ARR >= 209)
				{
					flag_frequency = UP;
					GREEN_LED_OFF;
					BLUE_LED_ON;				//Blue led indicates we're going to increase the frequency
				}
			}
		}
	}
	if((user_bt_count>=50) && (!USR_BT_PRESS))	//if the led has been toggled and we released the button
	{
		flag_EXTI=0;							//clears flag
	}
}
