/********************************************
*
*  Name:
*  Lab section:
*  Assignment:
*
********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//#define DOT_LENGTH  250         /* Define the length of a "dot" time in msec */

/*void dot(void);
void dash(void);
void makeOutput(char);
char checkInput(char);*/
//#define FOSC 7372800			// Clock frequency
#define FOSC 16000000 
#define BAUD 9600              	// Baud rate used 
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
void  serial_init() {
UBRR0 = MYUBRR;             // Set  baud  rate
UCSR0B  |= (1 << TXEN0 ); // Turn on  transmitter
UCSR0B  |= (1 << RXEN0 ); // Turn on  receiver
UCSR0C = (3 << UCSZ00 ); // Set  for  async. operation , no parity , one  stop bit , 8 data  bits
//UCSR0B |= (1<<RXCIE0); //enable the interrupts on recieving data
//sei();	
}
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
//PORTB |= (1<<PB1);
//_delay_us(10);

while ( !( UCSR0A & (1 << RXC0)) );

//PORTB &= ~(1<<PB1);
//_delay_us(10);
return  UDR0;
}

char  serial_in_handshake ()
{
while( !( UCSR0A & (1 << RXC0)) & (UDR0 != 'R'))
{
	serial_out('R');//retransmit the T
	_delay_us(5);
}
return  UDR0;
}
void sci_outs(char *s) //this code was taken from Prof Weber's at328-6.c program file
{
    char ch;

    while ((ch = *s++) != '\0')
        serial_out(ch);
}


volatile char RX_FIFO[10];
volatile char rx_input;
volatile int count;
int main(void)
{
	count = 0;
    // Initialize appropriate DDR and PORT registers
	//DDRB &= ~(1<<DDB0); //PIN I need is already configured as input
	PORTB |= (1<<PB0);
	DDRB |= (1<<DDB1);
	//char gps;
	//PORTB |= (1<<PB1);
	serial_init();
	UCSR0B |= (1<<RXCIE0); //enable the interrupts on recieving data
	sei(); //global interrupts;
	
	char rx;
	int count = 1;
	char string[74] = "$GPGGA,201937.000,3401.2181,N,11817.3458,W,1,06,2.34,129.0,M,-33.8,M,,*54@";
	char longitude[22] = "O34011501NA118172008W@";
	char latitude[5] = "A1817";
	char stopper = '@';
	//DDRD = 0;
    while (1) {                 // Loop forever
		rx = rx_input;
		if(rx == 'T')//DID WE GET A REQUEST 
		{
			/*serial_out('R');   //WE DID, LET'S SEND AN ACKNOWLEDGE
			PORTB |= (1<<PB1);
			rx = serial_in();
			_delay_ms(1000);
			PORTB &= ~(1<<PB1);
			_delay_ms(1000);
			if(rx == 'X')
			{*/

			sci_outs(longitude);
			//sci_outs(latitude);
			//serial_out(stopper);
			PORTB |= (1<<PB1);
			_delay_ms(1000);
			PORTB &= ~(1<<PB1);
			rx_input = 'x';
			
			//}	
		}
	}
    return 0;   /* never reached */
}
ISR(USART_RX_vect){
	rx_input = UDR0;
}
/*ISR(USART_RX_vect){
gps = serial_in(); //wait for response
		if(gps == 'x') 
		{
			PORTB |= (1<<PB1);
			_delay_ms(1000);
			PORTB &= ~(1<<PB1);
		}
		else
		{
			PORTB &= ~(1<<PB1);
		}

}*/
/*
  dot() - Makes the output LED blink a "dot".

void dot()
{
makeOutput(1);
_delay_ms(250);
makeOutput(0);
_delay_ms(250);
}
*/
/*
  dash() - Makes the output LED blink a "dash".

void dash()
{
makeOutput(1);
_delay_ms(750);
makeOutput(0);
_delay_ms(250);
}
*/
/*
  makeOutput() - Changes the output bit to either a zero or one.
  If the argument is zero, the bit is cleared to zero,
  otherwise it is set to a one.

void makeOutput(char value)
{
    if (value == 0)
        PORTB &= ~(1 << PB0);
    else
        PORTB |= (1 << PB0);
}


char checkInput(char bit)
{
    if ((PIND & (1 << bit)) != 0)
        return(1);
    else
        return(0);
}
*/