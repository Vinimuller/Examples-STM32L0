#include "stm32l053xx.h"
#include "macros.h"
#include "global.h"
#include "init.h"
#include "functions.h"

int main (void)
{
	GPIO_Init();
	TIM6_Init();

	BLUE_LED_ON;	//indicates we're going to increase the frequency at first

	while(1)
	{
		if(flag_TIM6)			//if there was a TIM6 interrupt
			Blink_Red_Led();

		if(flag_EXTI)			//if there was an EXTI int
			Change_Frequency();
		else
			user_bt_count=0;
	}

	return 0;
}
