#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "muxDemux.h"
#include "gps.h"
#include "serial.h"


void send_message ( char * text);
void emic_speak(char * run);

char * gps_get_info(char * info_array , char gps);

void init_timer0(unsigned short int m);
//GPS CONSTANTS
#define MAX_NUM_CHARS 120
char info[20];
//**********************

uint8_t i;
//char latitude[15], longitude[15];
//***********************

//uint8_t parseHex(char c);
//GPS CONSTANTS
//#define MAX_NUM_CHARS 120

//global variables for GPS info
//volatile char line1[110];

/* Converts a character to decimal hex value. 
uint8_t parseHex(char c) {
	if ((c >= '0') && (c <= '9')) {
		return c - '0';
	}
	if ((c >= 'A') && (c <= 'F')) {
		return (c - 'A') + 10;
	}
	return 0;
}*/

char run_names[3][30];
char run_NSWE[3][2];


int run_coor[3][4]; //each entry has lattitude then longitude




char friend_NS;
char friend_EW;
long longitude_i, latitude_i; //of my friend converted to integer value
volatile char longitude[15]; //of my friend
volatile char latitude[15]; //of my friend
volatile char  rx_data[22];
volatile char rx_input, z, cc;
volatile short count,count_fif, gps_want, gps_flag, j, friend_tx, friend_rx, friend_rx_stop, lat_flag, long_flag, long_match, lat_match;
			//sprintf(strtemp, "you entered the consonant %c \r\n", c);
char strtemp[20];

int main(void){

	strcpy(run_names[0],"Dave's Run"); //the engineering quad
	run_coor[0][0] = 34011862; //lat_min
	run_coor[0][1] = 34012641; //lat_max
	run_coor[0][2] = 118172746; //long_min
	run_coor[0][3] = 118173958; //long_max

	run_NSWE[0][0] = 'N';
	run_NSWE[0][1] = 'W';
	strcpy(run_names[1] ,"Hangman's Hollow"); //SGM
	run_coor[1][0] = 34012323;
	run_coor[1][1] = 34013118;
	run_coor[1][2] = 118172414;
	run_coor[1][3] = 118173647;
	run_NSWE[1][0] = 'N';
	run_NSWE[1][1] = 'W';
	strcpy(run_names[2], "Drop Out Chutes");//Campus Center to alumni
	run_coor[2][0] = 34011808;
	run_coor[2][1] = 34012688;
	run_coor[2][2] = 118171224;
	run_coor[2][3] = 118171543;
	run_NSWE[0][0] = 'N';
	run_NSWE[0][1] = 'W';
	/*
	strcpy(run_names[3],"Wipe Out Chutes"); //cromwell Area
	run_coor[3][0] = ;
	run_coor[3][1] = ;
	run_coor[3][2] = ;
	run_coor[3][3] = ;
	run_NSWE[0][0] = 'N';
	run_NSWE[0][1] = 'W';*/
	

	friend_rx = 0;
	friend_rx_stop = 0;
	friend_tx = 0;
	count = 0;
	count_fif = 0;
	long_flag = 0;
	lat_flag = 0;
	long_match = 0;
	lat_match = 0;

	gps_flag = 0;
	gps_want = 0;
	j = 0;

	//enable serial stuff and timer stuff
	serial_init();
	init_timer0(28800);
	UCSR0B |= (1<<RXCIE0); //enable the interrupts on recieving data
	sei(); //global interrupts;
	
	//enable serial pins for muxes
	enable_rx_select_pins();
	enable_tx_select_pins();

	//select_RS232_for_tx();
	//serial_out('H');

	//select_Emic2_for_rx();
	char run [MAX_NUM_CHARS];
	sprintf(run, "Black Diamond");
	emic_speak(run);
	//
	//all other pin declarations and code

	DDRC |= (1<<DDC2) | (1<<DDC0); //PC0 is an output
	PORTC |= (1<< PC3); //PC1 has pull up enabled
	PORTB |= (1<<PB0); //PULL UP RESISITOR FOR PB0
	DDRD |= (1<<DDD7); //this is now an output
	PORTD |= (1<<PD6);
	PORTC &= ~(1<<PC0);
	char * gps_info;
	char * tx_test;
	char c;
	char ack;
	char * ptr, ptr_; //for strtod

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
		if( (PIND & (1<<PD6)) == 0){
			PORTD &= ~(1<<PD7);
			cli();
			serial_out('L');
			select_GPS_for_rx();
			select_RS232_for_tx();
			_delay_ms(1000);
			//sci_outs(gps_get_info(get_GPGGA_string(), 'F'));
			char gps_array[100];
			strcpy(gps_array, "O34012231NA118173111W");//gps_get_info(get_GPGGA_string(), 'F'));
			//sci_outs(gps_array);
			int x = 0;
			count = 0;
			short n_flag = 0;
			for(x = 1; x < 21; x++)
			{
				if((gps_array[x] != 'N') & (n_flag == 0))
				{
					
					latitude[count] = gps_array[x];
					//serial_out(latitude[count]);
					count++;
				}
				else if(gps_array[x] == 'N')
				{
					n_flag = 1;
					latitude[count] = '\n';
					count = 0;
				}
				else if((n_flag == 1) & (gps_array[x] != 'A') & (gps_array[x] != 'W'))
				{
					longitude[count] = gps_array[x];
					count++;
				}
				longitude[count] = '\n';
			}
			sci_outs(latitude);
			sci_outs(longitude);
			latitude_i = atoi(latitude);
			longitude_i = atoi(longitude);
			for(x = 0; x < 3; x++)
			{
				if((run_coor[x][0] < latitude_i) & (run_coor[x][1] > latitude_i))
				{
					lat_match = 1;
				}
				if((run_coor[x][2] < longitude_i) & (run_coor[x][3] > longitude_i))
				{
					long_match = 1;
				}
				if((long_match & lat_match) == 1)
				{
					PORTD &= ~(1<<PD7);
					_delay_ms(500);
					PORTD |= (1<<PD7);
					_delay_ms(500);
					PORTD &= ~(1<<PD7);
					_delay_ms(500);
					PORTD |= (1<<PD7);
					_delay_ms(500);
					sprintf(strtemp, run_names[x]);
					//sci_outs(strtemp);
					emic_speak(strtemp);
					_delay_ms(2000);
					PORTD &= ~(1<<PD7);
					break;
					
				}
			
			}
			sei();
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
	//serial_out(rx_input);
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
					if(count_fif == 15) {
						count_fif = 0;
					}
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
					if(count_fif == 15) {
						count_fif = 0;
					}
					count++;
				}
			}
			
			/*RX_FIFO[count] = rx_input;
			count++;
			if(count == 10) count = 0;*/
		}
	}

	
	
	
}

ISR(TIMER1_COMPA_vect){
	if((friend_tx == 1) & (friend_rx == 0))

	{
		//we sent a T and didn't get a response, so we send another T;
		serial_out('T');
	}
}

void init_timer0 (unsigned short int m)
{
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
	sci_outs("V15\n");
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
