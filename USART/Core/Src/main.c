#include "stm32l053xx.h"
#include "stdio.h"

int __io_putchar(int ch);	//restructured __io_putchar() function to use USART as output

int main(void)
{
	char usart1_rx_data = 0;	//data received via USART

	//--- ENABLING GPIOB 7 AND 6 AS USART1 RX AND TX
	/*GPIOB
	 * PB7	-> Alternate function (USART1 RX)	| Low speed
	 * PB6	-> Alternate function (USART1 TX)	| Low speed
	 */
	RCC->IOPENR  |=  RCC_IOPENR_GPIOBEN;

	GPIOB->MODER &=	~(	GPIO_MODER_MODE6_0	|
						GPIO_MODER_MODE7_0	);

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
		usart1_rx_data = USART1->RDR;						//store it in data

		//and send back what we've received and the next letter in the alphabet
		printf("Data received: %c\r\nNext letter in the alphabet: %c\r\n\n", usart1_rx_data, (usart1_rx_data+1));
	}

	return 0;
}

//restructured __io_putchar() function to use USART as output
int __io_putchar(int ch)
{
	USART1->TDR = ch;
	while(!(USART1->ISR & USART_ISR_TC));

	return 0;
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
