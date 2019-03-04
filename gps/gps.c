#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include "HalfDuplexSimpleSerial.h"
#include "GPSSmartModule.h"
#include "gps_type_define.h"

#define GPS_BAUD_RATE 9600
#define FOSC 7372800		    // Clock frequency
#define BAUD 9600               // Baud rate used by the LCD
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register

/*
serial_init  - Initialize  the  USART  port TAKEN FROM SERIAL WORKSHEET
*/
void serial_init() {
    UBRR0 = MYUBRR;             // Set  baud  rate
    UCSR0B  |= (1 << TXEN0 );   // Turn on  transmitter
    UCSR0B  |= (1 << RXEN0 );   // Turn on  receiver
    UCSR0C = (3 << UCSZ00 );    // Set  for  async. operation, no parity, one stop bit, 8 data  bits
}

/*
serial_out  - Output a byte to the  USART0  port TAKEN FROM SERIAL WORKSHEET
*/
void serial_out(char ch)
{
    while  (( UCSR0A & (1<<UDRE0 )) == 0);
    UDR0 = ch;
}

/*
serial_in  - Read a byte  from  the  USART0  and  return  it TAKEN FROM SERIAL WORKSHEET
*/
char serial_in ()
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

void gps_send_cmd(uint8_t command_class, uint8_t command) {
    HDSS_write(0xA5);
    HDSS_write(command_class);
    HDSS_write(command);
    HDSS_write(0x5A);
}

int main(void) {
	serial_init();
    HDSS_start(0, GPS_BAUD_RATE);
	char c;
    uint8_t i;
    uint8_t rxdata = 0;
    char buf[50];
    version_t version;
    char debug[] = "Sending request...\r\n";
    char error[] = "Error RX Timeout\r\n";
	while(1){
        sci_outs(debug);
        gps_send_cmd(CMD_INFO_BASIC, CMD_GET_INFO);
        for (i = 0; i < 2; i++) {
            rxdata = HDSS_read_time(8);
            if (rxdata == ERROR_RX_TIMEOUT) {
                sci_outs(error);
            }
            version.buff[i] = rxdata;
        }
        sprintf(buf, "Hardware: %c Software %c\r\n", version.hardware_ver, version.firmware_ver);
        sci_outs(debug);
        _delay_ms(2000);
	}
	return 0;
}
