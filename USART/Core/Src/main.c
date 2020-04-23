#include "stm32l053xx.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"

//#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

//int __io_putchar(int ch);
void vprint(const char *fmt, va_list argp);
void serial_printf(const char *fmt, ...);

char data = 0;	//data read via USART

int main(void)
{
	//--- ENABLING GPIOB 6 AND 7 AS USART1 RX AND TX
	/*GPIOB
	 * PB7	-> Alternate function (USART1 RX)	| Low speed
	 * PB6	-> Alternate function (USART1 TX)	| Low speed
	 */
	RCC->IOPENR  |=  RCC_IOPENR_GPIOBEN;

	GPIOB->MODER &= ~GPIO_MODER_MODE6_0;
	GPIOB->MODER &= ~GPIO_MODER_MODE7_0;

	//--- USART1 CONFIG
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	USART1->BRR	=	18;				//baud rate set to 115.200 (2.1 MHz / 18,229 = 115.200)
	USART1->CR1	|= 	USART_CR1_UE |	//enables USART1
					USART_CR1_TE |	//enables TX
					USART_CR1_RE;	//enables RX

	while(1)
	{
		//--- USART1 RX
		while(!(USART1->ISR & USART_ISR_RXNE));	//we wait to receive a information
		data = USART1->RDR;						//and store it in data

		serial_printf("Data received: %c\n\rNext letter on the alphabet: %c\n\n\r", data, (data+1));

		asm("nop");	//so we can stop the program here
	}

	return 0;
}

void vprint(const char *fmt, va_list argp)
{
    char string[strlen(fmt)];
    int i = 0;

    if(0 < vsprintf(string,fmt,argp)) // build string
    {
    	for(i=0; i< strlen(string); i++)
    	{
			USART1->TDR = string[i];
			while(!(USART1->ISR & USART_ISR_TC));
    	}
    }
}

void serial_printf(const char *fmt, ...) // custom printf() function
{
    va_list argp;
    va_start(argp, fmt);
    vprint(fmt, argp);
    va_end(argp);
}

//PUTCHAR_PROTOTYPE
//{
//	USART1->TDR = ch;
//	while(!(USART1->ISR & USART_ISR_TC));
//
//	return ch;
//}

//int _write(int file, char *ptr, int len)
//{
//	int DataIdx;
//
//	for (DataIdx = 0; DataIdx < len; DataIdx++)
//	{
//		USART1->TDR = (*ptr++);
//		while(!(USART1->ISR & USART_ISR_TC));
//	}
//	return len;
//}

//int __io_putchar(int ch)
//{
//	USART1->TDR = ch;
//	while(!(USART1->ISR & USART_ISR_TC));
//
//	return 0;
//}

/*
 * --> CHARACTER TRANSMISSION PROCEDURE <--
 *
 * - configure the M bits in CR1 to define word length
 * - select the baud rate in BRR
 * - select the number of stop bits in CR2
 * - UE = 1 (enables USART)
 * - do DMA stuff (if using)
 * - TE = 1 in CR1
 * - write data to TDR
 * - wait until TC = 1 (indicates the transmission is complete)
 */

/*
 * --> CHARACTER RECEPCION PROCEDURE <--
 * - configure the M bits in CR1 to define word length
 * - selecet the baud rate in BRR
 * - select the number of stop bits in CR2
 * - UE = 1 (enables USART)
 * - do DMA stuff (if using)
 * - RE = 1 in CR1 (receiver start searching for a start bit)
 */
