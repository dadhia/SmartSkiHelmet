//#include <math.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

#include <avr/interrupt.h>
//NECESSARY DATA USED FOR SERIAL COMMUNICATION
#define FOSC 7372800			// Clock frequency
#define BAUD 9600              	// Baud rate used by the LCD
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
void send_message ( char * text);
void emic_speak(char * run);
void serial_init();
void serial_out(char c);
void sci_outs(char * s);
char serial_in();
uint8_t parseHex(char c);
char  serial_in_handshake ();
char * serial_in_string();
char* get_GPGGA_string();
char * gps_get_info(char * info_array , char gps);
void enable_rx_select_pins();
void enable_tx_select_pins();
void select_Emic2_for_rx();
void select_Emic2_for_tx();
void select_GPS_for_rx();
void select_RS232_for_rx();
void select_RS232_for_tx();
void select_Xbee_for_rx();
void select_Xbee_for_tx();

void init_timer0(unsigned short int m);
//GPS CONSTANTS
#define MAX_NUM_CHARS 120
char info[20];
//**********************
//global variables for GPS info
char line1[MAX_NUM_CHARS];
uint8_t i;
//char latitude[15], longitude[15];
//***********************


char run_names[5][30];
char run_NSWE[5][2];
int run_coor[5][4]; //each entry has lattitude then longitude
char friend_NS;
char friend_EW;
int longitude_i, latitude_i; //of my friend converted to integer value
volatile char longitude[15]; //of my friend
volatile char latitude[15]; //of my friend
volatile char  rx_data[22];
volatile char rx_input, z;
volatile short count,count_fif, friend_tx, friend_rx, friend_rx_stop, lat_flag, long_flag, long_match, lat_match;
			//sprintf(strtemp, "you entered the consonant %c \r\n", c);
char strtemp[20];
int main(void){
	strcpy(run_names[0],"Dave's Run"); //the engineering quad
	run_coor[0][0] = 34011080; //lat_min
	run_coor[0][1] = 34011610; //lat_max
	run_coor[0][2] = 118171610; //long_min
	run_coor[0][3] = 118172390; //long_max
	//run_NSWE[0] = {'N', 'W'}
	/*strcpy(run_names[1] ,"Hangman's Hollow"); //Dedeaux Field
	run_coor[1][0] = 3401258; 
	run_coor[1][1] = -11817222;
	strcpy(run_names[2], "Drop Out Chutes");//MUDD Hall
	run_coor[2][0] = 3401081;
	run_coor[2][1] = -11817115;
//	run_NSWE[0] = {'N', 'W'}
	strcpy(run_names[3],"Wipe Out Chutes");
	run_coor[3][0] = 3401153;
	run_coor[3][1] = -11817118; //Bookstore
	//run_NSWE[0] = {'N', 'W'}
	strcpy(run_names[4], "Unbound");//Seaver science library
	run_coor[4][0] = 34011050;
	run_coor[4][0] = -118171880;*/
	//run_NSWE[0] = {'N', 'W'}
	/*run_names[5] = "Dragon's Back";
	run_coor[5] =
	run_names[6] = "Cornice Bowl";
	run_coor[6] =
	run_names[7] = "Scotty's";
	run_coor[7] =
	run_names[8] = "Phileppe's";
	run_coor[8] = 
	run_names[9] = "Kiwi Flat";
	run_coor[9] = */
	friend_rx = 0;
	friend_rx_stop = 0;
	friend_tx = 0;
	count = 0;
	count_fif = 0;
	long_flag = 0;
	lat_flag = 0;
	long_match = 0;
	lat_match = 0;
	//enable serial stuff and timer stuff
	serial_init();
	init_timer0(28800);
	UCSR0B |= (1<<RXCIE0); //enable the interrupts on recieving data
	sei(); //global interrupts;
	
	//enable serial pins for muxes
	enable_rx_select_pins();
	enable_tx_select_pins();
	//select_Emic2_for_rx();
	char run [MAX_NUM_CHARS];
	sprintf(run, "Black Diamond");
	emic_speak(run);
	//
	//all other pin declarations and code

	DDRC |= (1<<DDC2); //PC0 is an output
	PORTC |= (1<< PC3); //PC1 has pull up enabled
	PORTB |= (1<<PB0); //PULL UP RESISITOR FOR PB0
	DDRD |= (1<<DDD7); //this is now an output
	char * gps_info;
	char * tx_test;
	char c;
	char ack;
	char * ptr, ptr_; //for strtod
	//select_RS232_for_rx();
	int index = 0;
	int i = 0;
	int magnitude = 0;
	/*
		char longitude[5] = "O3401";
	char latitude[5] = "A1817";
	*/
	while(1){
		select_Xbee_for_tx();
		select_Xbee_for_rx();
		//select_RS232_for_tx();
		//select_RS232_for_rx();
		if((PINB & (1<<PB0)) == 0)
		{
			serial_out('T');
			friend_tx = 1;
			int i = 0;
			while(friend_rx_stop == 0)
			{
				i++;
			}
			friend_rx_stop = 0;
			//friend_tx = 0;
			//sci_outs(RX_FIFO);
	//			sprintf(strtemp, "you entered the consonant %c \r\n", c);
			
			//if(RX_FIFO[9] == '@') PORTD |= (1<<PD7);
			if(rx_data[22] == '@') rx_data[22] = '\0';
			//latitude_d = strtod(latitude, &ptr); 
			//longitude_d = strtod(longitude, &ptr_);
			//latitude_d = atof(latitude);
			
			latitude_i = atoi(latitude);
			longitude_i = atoi(longitude);
			/*select_RS232_for_tx();
			select_RS232_for_rx();
			_delay_ms(1000);
			sci_outs("\r\n");
			sci_outs(latitude);
			sci_outs("\r\n");
			sci_outs(longitude);
			sci_outs("\r\n");*/
			//serial_out_num(latitude_i);
			for(i = 0; i < 1; i++)
			{
				if((run_coor[i][0] < latitude_i) & (run_coor[i][1] > latitude_i))
				{
					lat_match = 1;
				}
				if((run_coor[i][2] < longitude_i) & (run_coor[i][3] > longitude_i))
				{
					long_match = 1;
				}
				if((long_match & lat_match) == 1)
				{
					
					PORTD |= (1<<PD7);
					sprintf(strtemp, run_names[i]);
					//sci_outs(strtemp);
					emic_speak(strtemp);
					_delay_ms(2000);
					PORTD &= ~(1<<PD7);
					
				}
			
			}
			//sprintf(strtemp, "friend is at %s \r\n", RX_FIFO);
			//select_RS232_for_tx();
			//select_RS232_for_rx();
			//_delay_ms(1000);
			//sci_outs(strtemp);
			
			//_delay_ms(5000);
			//PORTD &= ~(1<<PD7);
			
		}
	}
	return 0;
	
}







ISR(USART_RX_vect){
	//flags for synchronizing the radio transmissions:
	//friend_rx => we have begun receiving the friends gps data
	//friend_rx_stop => we have all of his data
	//'R'
	rx_input = UDR0;
	if(rx_input == ':') z = rx_input;
	if(rx_input == 'O') {
		friend_rx = 1;
		friend_tx = 0;
		lat_flag = 1;
	}
	if((friend_rx == 1) & (friend_rx_stop == 0))
	{
		if(rx_input == '@')
		{	
			friend_rx_stop = 1;// we got the stop byte
			friend_rx = 0;
			long_flag = 0;
			lat_flag = 0;
			count = 0;
			count_fif = 0;
		}
		else if(rx_input != '@') 
		{
			if((lat_flag == 1) & (rx_input != 'A') & (rx_input != 'O'))
			{
				if((rx_input == 'N') | (rx_input == 'S'))
				{
					friend_NS = rx_input;
				}
				else
				{
					rx_data[count] = rx_input;
					latitude[count_fif] = rx_input;
					count_fif++;
					if(count_fif == 15) count_fif == 0;
					count++;
				}
			}
			else if(rx_input == 'A')
			{
				long_flag =1;
				lat_flag = 0;
				count_fif = 0;
			}
			else if((long_flag == 1) & (rx_input != '@' ))
			{
				if((rx_input == 'E') | (rx_input == 'W'))
				{
					friend_EW = rx_input;
				}
				else
				{
					rx_data[count] = rx_input;
					longitude[count_fif] = rx_input;
					count_fif++;
					if(count_fif == 15) count_fif == 0;
					count++;
				}
			}
			
			/*RX_FIFO[count] = rx_input;
			count++;
			if(count == 10) count = 0;*/
		}
	}

	//if we receive an R then we need to set a global flag that "hey we are getting ready to receive GPS data";
	//we can't miss this, we need to set up a timer to keep sending a T
	
	
	
}

ISR(TIMER1_COMPA_vect){
	if(friend_tx == 1 & friend_rx == 0)
	{
		//we sent a T and didn't get a response, so we send another T;
		serial_out('T');
		
	}
	
}

void init_timer0 (unsigned short int m)
{
	//int prescale = 256;
	//int cyclecnt = 16000000/prescale;
	TCCR1B |= (1<<WGM12);
	TIMSK1 |= (1<<OCIE1A);
	OCR1A = m; //m=2880
	TCCR1B |= (1<<CS12);	
}

void send_message ( char * text)
{
	char c;
	char out[MAX_NUM_CHARS];
	
	while (1)
	{
		c = serial_in();
		while (c == ':')
		{
			strcpy(out, "S");
			strcat(out, text);
			sci_outs(out);
		}
	}	
}

int initializer_count = 0;
void emic_speak(char * run)
{
	//char z;
	if(initializer_count ==0){
	select_Emic2_for_rx();
	while ( z != ':') ;
	z = 'x';
	select_Emic2_for_tx();
	sci_outs("V10\n");
	while ( z != ':') ;
	z = 'x';
	_delay_ms(500);
	//parsing string
	char final_run[MAX_NUM_CHARS];
	sprintf(final_run, "SYour friend is on  %s run\n", run);
	
	sci_outs(final_run);

	while ( z  != ':') ;
	z = 'x';
	initializer_count++;
	}
	else{
		select_Emic2_for_tx();
		char final_run[MAX_NUM_CHARS];
		sprintf(final_run, "SYour friend is on  %s run\n", run);
		sci_outs(final_run);
	}
}
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


void  serial_out_num(int x)
{
while  (( UCSR0A & (1<<UDRE0 )) == 0);
UDR0 = x;
}
/*
serial_in  - Read a byte  from  the  USART0  and  return  it TAKEN FROM SERIAL WORKSHEET
*/
char  serial_in ()
{
	while ( !( UCSR0A & (1 << RXC0)) );
	return  UDR0;
}

//for handshake protocol
char  serial_in_handshake ()
{
while( !( UCSR0A & (1 << RXC0)) & (UDR0 != 'R'))
{
	serial_out('T');//retransmit the T
	_delay_us(5);
}
return  UDR0;
}


//char array[74];
char *  serial_in_string ()
{
	//array[0] = '\0';
	//int i;
	while( !( UCSR0A & (1 << RXC0)) );
	/*for(i = 0; i < MAX_NUM_CHARS; ++i)
	{
		if((UDR0 == '\0') | (UDR0 == '@')) break;
		array[i] = UDR0;
	}*/
	return UDR0;
}

void sci_outs(char *s) //this code was taken from Prof Weber's at328-6.c program file
{
    char ch;

    while ((ch = *s++) != '\0')
        serial_out(ch);
}

uint8_t parseHex(char c) {
	if ((c >= '0') && (c <= '9')) {
		return c - '0';
	}
	if ((c >= 'A') && (c <= 'F')) {
		return (c - 'A') + 10;
	}
	return 0;
}

char* get_GPGGA_string() {
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
	return line1;
}

/*
The function below will get any piece of the GPS information
here are the commands to feed it and what it will produce for each one:

T => hhmmss.ss = UTC of position
N => llll.ll = latitude of position; a = N or S
W => yyyyy.yy = Longitude of position; a = E or W
Q => x = GPS Quality indicator (0=no fix, 1=GPS fix, 2=Dif. GPS fix)
S => xx = number of satellites in use
P => x.x = horizontal dilution of precision
A => x.x = Antenna altitude above mean-sea-level
M => M = units of antenna altitude, meters
I => x.x = Geoidal separation
G => M = units of geoidal separation, meters
Z => x.x = Age of Differential GPS data (seconds)
D => xxxx = Differential reference station ID 

*/

char * gps_get_info(char * info_array , char gps)
{
	/*
eg2. $--GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx
	 0123456789012345678901234567890123456789012345678901234567890123456
	           1         2         3         4         5         6  
   //$GPGGA,201937.000,3401.2181,N,11817.3458,W,1,06,2.34,129.0,M,-33.8,M,,*54
	*/
	
	short count = 0;
	int i = 0;
	info[0] = '\0';//to clear string each time, since it is global
	if( (info_array[43] != '1') | (info_array[18] == ',') | (info_array[19] == ',') | (info_array[20] == ','))
	{
		return "GPS NOT VALID";
	}
	switch(gps)
	{
		case 'T':
			for(i = 7; i < 13; ++i)
			{
				info[count] = info_array[i];
				++count;
			}
			info[count] = '\r\n';
			count = 0;
			return info;
			break;
		
		case 'N':
			for(i = 18; i < 27; ++i)
			{
				info[count] = info_array[i];
				++count;
			}
			info[count] = '\r\n';
			count = 0;
			return info;
			break;			
		
		case 'W':
			for(i = 30; i < 39; ++i)
			{
				info[count] = info_array[i];
				++count;
			}
			info[count] = '\r\n';
			count = 0;
			return info;
		break;
		
		case 'Q':
			for(i = 43; i < 44; ++i)
			{
				info[count] = info_array[i];
				++count;
			}
			info[count] = '\r\n';
			count = 0;
			return info;
			break;

		default:
		break;
	}
	return info;
	
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

/**
 * Enables pins 15 and 16 on the Atmega328p for output.
 */
void enable_tx_select_pins() {
	DDRB |= (1 << DDB1) | (1 << DDB2);
}

void select_RS232_for_tx() {
	PORTB &= ~(1 << PB1);
	PORTB &= ~(1 << PB2);
}

void select_Emic2_for_tx() {
	PORTB |= (1 << PB1);
	PORTB &= ~(1 << PB2);
}

void select_Xbee_for_tx() {
	PORTB &= ~(1 << PB1);
	PORTB |= (1 << PB2);
}





















/*

1.)send gps data over radio and receive it.
2.)initiate travel of gps data with a button.
3.)use interrupts to make use of asynchronous requests of gps data by multiple helmets
4.)data structure for GPS data, must be small to fit inside of Arduino
5.)maybe find a way to upload different maps depending on the mountain you're at328-6
6.)put different map files on a website


GOAL OF TONIGHT:
GET A BUTTON PRESS ON ONE MICROCONTROLLER TO RECEIVE GPS DATA FROM THE OTHER







			
			gps_info = gps_get_info(get_GPGGA_string(), 'T');
			sci_outs(a1);
			sci_outs(gps_info);
			//sci_outs(get_GPGGA_string());//$GPGGA,201937.000,3401.2181,N,11817.3458,W,1,06,2.34,129.0,M,-33.8,M,,*54
			
			gps_info = gps_get_info(get_GPGGA_string(), 'N');
			sci_outs(a2);
			sci_outs(gps_info);
			
			gps_info = gps_get_info(get_GPGGA_string(), 'W');
			sci_outs(a3);
			sci_outs(gps_info);
			
			gps_info = gps_get_info(get_GPGGA_string(), 'Q');
			sci_outs(a4);
			sci_outs(gps_info);
			
			_delay_ms(2000);
		
			serial_out('H');
			_delay_ms(50);
			if(serial_in() == 'H'){ 001029
				PORTC |= (1 << PC2);
				_delay_ms(1000);
				PORTC &= ~(1 << PC2);
			}
			*/
		//
		





















