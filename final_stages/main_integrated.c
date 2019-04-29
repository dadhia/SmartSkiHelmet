#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "muxDemux.h"
#include "gps.h"
#include "serial.h"
#include "coSensor.h"
#include "emic.h"

#define MAX_NUM_CHARS_MAIN_INTEGRATED 120

char * gps_get_info(char * info_array , char gps);

void init_timer0(unsigned short int m);
void start_CO_monitoring();

char info[20];

uint8_t i;


//global variables for internal gps map
int run_coor[3][4]; 	//each entry has lattitude then longitude
char run_NSWE[3][2];
char run_names[3][30];

char friend_NS;
char friend_EW;
long longitude_i, latitude_i; //of my friend converted to integer value
volatile char longitude[15]; //of my friend
volatile char latitude[15]; //of my friend
volatile char  rx_data[22];
volatile char rx_input, z, cc;
volatile short count,count_fif, gps_want, gps_flag, j, friend_tx, friend_rx, friend_rx_stop, lat_flag, long_flag, long_match, lat_match;
char strtemp[20];

int main(void){
	serial_init();
	enable_serial_mux_demux();
	initialize_emic();
	
	initialize_CO_reading();
	initialize_internal_gps_map(run_coor, run_NSWE, run_names);
	
	//enable serial stuff and timer stuff
	init_timer0(28800);
	UCSR0B |= (1<<RXCIE0); //enable the interrupts on recieving data
	sei(); //global interrupts;
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

	DDRC |= (1<<DDC2) | (1<<DDC0); //PC0 is an output
	PORTC |= (1<< PC3); //PC1 has pull up enabled
	PORTB |= (1<<PB0); //PULL UP RESISITOR FOR PB0
	DDRD |= (1<<DDD7); //this is now an output
	PORTD |= (1<<PD6);
	PORTC &= ~(1<<PC0);
	
	
	select_RS232_for_tx();
	
	start_CO_monitoring();
	while (1) {
		update_CO_ppm();
	}
	

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

			if(rx_data[22] == '@') rx_data[22] = '\0';
			
			latitude_i = atoi(latitude);
			longitude_i = atoi(longitude);

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
					emic_speak(strtemp);
					_delay_ms(2000);
					PORTD &= ~(1<<PD7);
					
				}
			
			}
		}
		if( (PIND & (1<<PD6)) == 0){
			PORTD &= ~(1<<PD7);
			cli();
			serial_out('L');
			select_GPS_for_rx();
			select_RS232_for_tx();
			_delay_ms(1000);
			char gps_array[100];
			strcpy(gps_array, "O34012231NA118173111W");
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
	if((friend_tx == 1) & (friend_rx == 0))	{
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
