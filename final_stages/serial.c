#include "serial.h"

#define FOSC 7372800			// Clock frequency
#define BAUD 9600              	// Baud rate used by the LCD
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register

/**
 * Initializes the USART port.
 */
void  serial_init() {
	UBRR0 = MYUBRR;             // Set  baud  rate
	UCSR0B  |= (1 << TXEN0 ); 	// Turn on  transmitter
	UCSR0B  |= (1 << RXEN0 ); 	// Turn on  receiver
	UCSR0C = (3 << UCSZ00 ); 	// Set  for  async. operation , no parity , one  stop bit , 8 data  bits
}

/**
 * Outputs a byte to the USART0 port.
 */
void serial_out(char ch)
{
while  (( UCSR0A & (1<<UDRE0 )) == 0);
UDR0 = ch;
}

/**
 * Reads a byte from the USART0 and return it.
 */
char serial_in ()
{
	while ( !( UCSR0A & (1 << RXC0)) );
	return  UDR0;
}

/**
 * Sends a string of characters terminating in the null character.
 */
void sci_outs(char *s) //this code was taken from Prof Weber's at328-6.c program file
{
    char ch;

    while ((ch = *s++) != '\0')
        serial_out(ch);
}
