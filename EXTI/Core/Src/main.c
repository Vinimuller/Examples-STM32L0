#include "stm32l053xx.h"
#include "macros.h"
#include "init.h"

uint8_t	user_bt_count	=0,	//counter to debounce the button
		flag_EXTI		=0,	//flag for EXTI interrupt
		flag_TIM6		=0,	//flag for TIM6 interrupt
		flag_frequency	=1;	//flag to indicate if we are increasing or decreasing the frequency

int main (void)
{
	GPIO_Init();
	TIM6_Init();

	BLUE_LED_ON;	//indicates we're going to increase the frequency at first

	while(1)
	{
		if(flag_TIM6)			//if there was a TIM6 interrupt
		{
			flag_TIM6 = 0;		//clears TIM6 flag
			TOGGLE_RED_LED;		//toggle the red led
		}//flag_TIM6

		if(flag_EXTI)								//if there was an EXTI int
		{
			if(user_bt_count < 250)					//limits the counting to 250
			{
				user_bt_count++;					//counts up for debouncing
			}

			if(user_bt_count == BUTTON_DEBOUNCE)
			{
				if((TIM6->ARR > 19) && flag_frequency == UP)
				{
					TIM6->ARR -= 10;
					TIM6->EGR |= TIM_EGR_UG;		//Update registers values
					if(TIM6->ARR <= 19)
					{
						flag_frequency = DOWN;
						GREEN_LED_ON;				//Green led indicates we're going to decrease the frequency
						BLUE_LED_OFF;
					}
				}
				else
				{
					if((TIM6->ARR <= 199) && flag_frequency == DOWN)
					{
						TIM6->ARR += 10;
						TIM6->EGR |= TIM_EGR_UG;	//Update registers values
						if(TIM6->ARR >= 209)
						{
							flag_frequency = UP;
							GREEN_LED_OFF;
							BLUE_LED_ON;			//Blue led indicates we're going to increase the frequency
						}
					}
				}
			}
			if((user_bt_count >= BUTTON_DEBOUNCE) && (!USR_BT_PRESS))	//if the led has been toggled and we released the button
			{
				flag_EXTI=0;											//clears flag
			}
		}//flag_EXTI
		else
		{
			user_bt_count=0;
		}//flag_EXTI
	}

	return 0;
}
