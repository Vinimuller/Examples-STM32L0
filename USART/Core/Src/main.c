#include "stm32l053xx.h"
#include "stdio.h"
#include "stdarg.h"

int serial_printf(const char *format, ...);
int serial_write(int file, char ptr, int len);
int __io_putchar(int ch);

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

		//--- USART1 TX
//		USART1->TDR = data + 1;					//then we send a message as a response
//		while(!(USART1->ISR & USART_ISR_TC));	//and wait for TC flag to set, knowing that all the data was sent

		serial_write(0, (data+1), 20);	//that's for later
		//printf("data: %d", data);

		asm("nop");	//so we can stop the program here
	}

	return 0;
}

int serial_write(int file, char ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		__io_putchar(ptr++);
	}
	return len;
}

int __io_putchar(int ch)
{
	USART1->TDR = ch;
	while(!(USART1->ISR & USART_ISR_TC));

	return 0;
}

int serial_printf(const char *format, ...)
{
	va_list arg;
	int done;

	va_start (arg, format);
	done = vfprintf (stdout, format, arg);
	va_end (arg);

	return done;
}

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
