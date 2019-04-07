#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

//NECESSARY DATA USED FOR SERIAL COMMUNICATION
#define FOSC 7372800			// Clock frequency
#define BAUD 9600              	// Baud rate used by the LCD
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register


//GPS CONSTANTS
#define MAX_NUM_CHARS 120

/*
serial_init  - Initialize  the  USART  port TAKEN FROM SERIAL WORKSHEET
*/
void  serial_init() {
UBRR0 = MYUBRR;             // Set  baud  rate
UCSR0B  |= (1 << TXEN0 ); // Turn on  transmitter
UCSR0B  |= (1 << RXEN0 ); // Turn on  receiver
UCSR0C = (3 << UCSZ00 ); // Set  for  async. operation , no parity , one  stop bit , 8 data  bits
}

/*
serial_out  - Output a byte to the  USART0  port TAKEN FROM SERIAL WORKSHEET
*/
void  serial_out(char ch)
{
while  (( UCSR0A & (1<<UDRE0 )) == 0);
UDR0 = ch;
}

/*
serial_in  - Read a byte  from  the  USART0  and  return  it TAKEN FROM SERIAL WORKSHEET
*/
char  serial_in ()
{
while ( !( UCSR0A & (1 << RXC0)) );
return  UDR0;
}


void sci_outs(char *s) //this code was taken from Prof Weber's at328-6.c program file
{
    char ch;

    while ((ch = *s++) != '\0')
        serial_out(ch);
}

char line1[MAX_NUM_CHARS];
uint8_t i;
char latitude[15], longitude[15];

uint8_t parseHex(char c) {
	if ((c >= '0') && (c <= '9')) {
		return c - '0';
	}
	if ((c >= 'A') && (c <= 'F')) {
		return (c - 'A') + 10;
	}
	return 0;
}

void get_latitude_and_longitude() {
	i = 0;
	char c;
	while (1) {
		line1[i] = c = serial_in();
		if (c == '$') {
			line1[++i] = c = serial_in();
			if (line1[i] == 'G') {
				line1[++i] = c = serial_in();
				if (line1[i] == 'P') {
					line1[++i] = c = serial_in();
					if (line1[i] == 'G') {
						line1[++i] = c = serial_in();
						if (line1[i] == 'G') {
							line1[++i] = c = serial_in();
							if (line1[i] == 'A') {
								while ( (c = serial_in()) != '\n') {
									line1[++i] = c;
								}
								line1[++i] = c;
								line1[++i] = '\0';
								//now verify checksum
								size_t length = strlen(line1);
								if (line1[length - 5] == '*') {
									uint16_t checksum = parseHex(line1[length - 4]) * 16;
									checksum += parseHex(line1[length -3]);
									for (i = 1; i < length - 5; i++) {
										checksum ^= line1[i];
									}
									if (checksum == 0) {
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		//we did not find the $GPGGA string which is desired OR checksum failed
		i = 0;
	}
	sci_outs(line1);
}


/**
 * Enables pins 4 and 5 on the Atmega 328p for output.
 */
void enable_rx_select_pins() {
	DDRD |= (1 << DDD2) | (1 << DDD3);
}

void select_GPS_for_rx() {
	PORTD |= ~(1 << PD2);
	PORTD |= ~(1 << PD3);
}

void select_RS232_for_rx() {
	PORTD &= ~(1 << PD2);
	PORTD |= (1 << PD3);
}

void select_Emic2_for_rx() {
	PORTD |= (1 << PD2);
	PORTD &= ~(1 << PD3);
}

void select_Xbee_for_rx() {
	PORTD &= ~(1 << PD2);
	PORTD &= ~(1 << PD3);
}

int main(void){
	serial_init();
	enable_rx_select_pins();
	char c;
	while(1){
		select_GPS_for_rx();
		sci_outs("I am now going to listen to GPS");
		get_latitude_and_longitude();
		select_RS232_for_rx();
		while (1) {
			c = serial_in();
			if (c == 'n') {
				sci_outs("You want the next gps string!");
				break;
			}
		}
	}
	return 0;
	
}









