
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
//NECESSARY DATA USED FOR SERIAL COMMUNICATION
#define FOSC 7372800		// Clock frequency
#define BAUD 9600              // Baud rate used by the LCD
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register


/*
serial_init  - Initialize  the  USART  port TAKEN FROM SERIAL WORKSHEET
*/
void  serial_init() {
UBRR0 = MYUBRR;             // Set  baud  rate
UCSR0B  |= (1 << TXEN0 ); // Turn on  transmitter
UCSR0B  |= (1 << RXEN0 ); // Turn on  receiver
UCSR0C = (3 << UCSZ00 ); // Set  for  async. operation , no parity ,
// one  stop bit , 8 data  bits
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

int main(void){
	//WRITTEN BY THOMAS WEAVER 2/26/2019
	serial_init();
	char c;
	//char * adc = &c;
	char strtemp[50];
	char str[] = "You entered the vowel 'a' \r\n";
	char str1[] = "You entered the vowel 'e' \r\n";
	char str2[] = "You entered the vowel 'i' \r\n";
	char str3[] = "You entered the vowel 'o' \r\n";
	char str4[] = "You entered the vowel 'u' \r\n";
	//char str5[] = "you entered the consant ";
	while(1){
		c = serial_in();
		if(c == 'a'){
			sci_outs(str);
		}
		else if(c == 'e'){
			sci_outs(str1);
		}
		else if(c == 'i'){
			sci_outs(str2);
		}
		else if(c == 'o'){
			sci_outs(str3);
		}
		else if(c == 'u'){
			sci_outs(str4);
		}
		else {
			//strcat(str5, &c);
			//strcat(strtemp, adc);
			//sprintf(buffer, "Sum of %d and %d is %d", a, b, c);
			sprintf(strtemp, "you entered teh consonant %c \r\n", c);
			sci_outs(strtemp);
			
		
		}
		
	
		
	}
	return 0;
	
}









