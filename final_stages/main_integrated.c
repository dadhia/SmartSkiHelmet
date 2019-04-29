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
#define DEBUG 1
#define RS232_DELAY 500
#define NUM_RUNS 3

void init_timer0(unsigned short int m);
void start_CO_monitoring();

char info[20];

uint8_t i;


//global variables for internal gps map
long run_coor[NUM_RUNS][4]; 	//each entry has lattitude then longitude
char run_NSWE[NUM_RUNS][2];
char run_names[NUM_RUNS][30];

char friend_NS;
char friend_EW;
long longitude_i, latitude_i; //of my friend converted to integer value
volatile char longitude[15]; //of my friend
volatile char latitude[15]; //of my friend
volatile char  rx_data[22];
volatile char rx_input, z, cc;
volatile short count,count_fif, gps_want, gps_flag, j, friend_tx, friend_rx, friend_rx_stop, lat_flag, long_flag;
char strtemp[20];

char white_debug[100];

volatile int failed_requests_count;
volatile int request_aborted;


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
	friend_tx = 0;
	count = 0;
	count_fif = 0;
	long_flag = 0;
	lat_flag = 0;

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
		// RED BUTTON PRESSED
		if ( (PINC & (1 << PC3)) == 0) {
			if (DEBUG) {
				select_RS232_for_tx();
				_delay_ms(RS232_DELAY);
				sci_outs("Red button pressed!\r\n");
			}
			speak_CO_value();
		}
		
		// BLUE BUTTON PRESSED
		if( (PIND & (1<<PD6)) == 0){
			if (DEBUG) {
				select_RS232_for_tx();
				_delay_ms(RS232_DELAY);
				sci_outs("Blue button pressed!\r\n");
			}
			
			cli();
			select_GPS_for_rx();
			select_RS232_for_tx();
			_delay_ms(1000);
			
			char gps_array[100];
			char* gpgga_string;
			char debug[200];
			char* lat_long_info_string;
			gpgga_string = get_GPGGA_string();
			if (DEBUG) {
				select_RS232_for_tx();
				_delay_ms(RS232_DELAY);
				strcpy(debug, gpgga_string);
				sci_outs(debug);
				_delay_ms(RS232_DELAY);
			}
			lat_long_info_string = gps_get_info(gpgga_string);
			
			if (lat_long_info_string == NULL) {
				emic_no_gps_fix();
				if (DEBUG) {
					select_RS232_for_tx();
					_delay_ms(RS232_DELAY);
					sci_outs("There is no gps fix!\r\n");
					_delay_ms(RS232_DELAY);
				}
			} else {
				strcpy(gps_array, lat_long_info_string);
				int x = 0;
				count = 0;
				short n_flag = 0;
				
				for(x = 1; x < 21; x++)
				{
					if((gps_array[x] != 'N') & (n_flag == 0)) {
						latitude[count] = gps_array[x];
						count++;
					}
					else if(gps_array[x] == 'N') {
						n_flag = 1;
						latitude[count] = '\n';
						count = 0;
					}
					else if((n_flag == 1) & (gps_array[x] != 'A') & (gps_array[x] != 'W')) {
						longitude[count] = gps_array[x];
						count++;
					}
				}
				longitude[count] = '\n';
				
				if (DEBUG) {
					sprintf(white_debug, "Latitude is: %s\r\n", latitude);
					select_RS232_for_tx();
					_delay_ms(RS232_DELAY);
					sci_outs(white_debug);
					
					sprintf(white_debug, "Longitude is: %s\r\n", longitude);
					select_RS232_for_tx();
					_delay_ms(RS232_DELAY);
					sci_outs(white_debug);
				}

				latitude_i = atol(latitude);
				longitude_i = atol(longitude);
				
				if (DEBUG) {
					sprintf(white_debug, "Latitude is: %ld\r\n", latitude_i);
					select_RS232_for_tx();
					_delay_ms(RS232_DELAY);
					sci_outs(white_debug);
				
					sprintf(white_debug, "Longitude is: %ld\r\n", longitude_i);
					select_RS232_for_tx();
					_delay_ms(RS232_DELAY);
					sci_outs(white_debug);
				}
				for(x = 0; x < NUM_RUNS; x++)
				{
					int lat_match = 0;
					int long_match = 0;
					
					if((run_coor[x][0] <= latitude_i) && (run_coor[x][1] >= latitude_i))
					{
						lat_match = 1;
					}
					if((run_coor[x][2] <= longitude_i) && (run_coor[x][3] >= longitude_i))
					{
						long_match = 1;
					}
					if(long_match && lat_match)
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
						emic_my_slope_speak(strtemp);
						PORTD &= ~(1<<PD7);
						break;
					}
				}
			}
			sei();
		}
		
		// WHITE BUTTON PRESSED
		if ( (PINB & (1 << PB0)) == 0) {
			if (DEBUG) {
				select_RS232_for_tx();
				_delay_ms(RS232_DELAY);
				sci_outs("White button pressed!\r\n");
				_delay_ms(RS232_DELAY);
			}
			emic_contacting_your_friend();
			failed_requests_count = 0;
			request_aborted = 0;
			friend_rx_stop = 0;
			select_Xbee_for_rx();
			select_Xbee_for_tx();
			serial_out('T');
			friend_tx = 1;
			
			while((friend_rx_stop == 0) && (request_aborted == 0));
			if (DEBUG) {
				select_RS232_for_tx();
				_delay_ms(RS232_DELAY);
				sci_outs("Transmission complete\r\n");
				_delay_ms(RS232_DELAY);
			}
			if (request_aborted) {
				if (DEBUG) {
					select_RS232_for_tx();
					_delay_ms(RS232_DELAY);
					sci_outs("Transmission aborted\r\n");
					_delay_ms(RS232_DELAY);
				}
				emic_no_friend();
			} else {
				if (DEBUG) {
					sprintf(white_debug, "Latitude is: %s\r\n", latitude);
					select_RS232_for_tx();
					_delay_ms(RS232_DELAY);
					sci_outs(white_debug);
					
					sprintf(white_debug, "Longitude is: %s\r\n", longitude);
					select_RS232_for_tx();
					_delay_ms(RS232_DELAY);
					sci_outs(white_debug);
				}

				latitude_i = atol(latitude);
				longitude_i = atol(longitude);
				
				if (DEBUG) {
					sprintf(white_debug, "Latitude is: %ld\r\n", latitude_i);
					select_RS232_for_tx();
					_delay_ms(RS232_DELAY);
					sci_outs(white_debug);
					
					sprintf(white_debug, "Longitude is: %ld\r\n", longitude_i);
					select_RS232_for_tx();
					_delay_ms(RS232_DELAY);
					sci_outs(white_debug);
				}

				int i = 0;
				for(i = 0; i < NUM_RUNS; i++) {
					int lat_match = 0;
					int long_match = 0;
					if ((run_coor[i][0] <= latitude_i) && (run_coor[i][1] >= latitude_i)) {
						lat_match = 1;
					}
					if (( run_coor[i][2] <= longitude_i) && (run_coor[i][3] >= longitude_i)) {
						long_match = 1;
					}
					if (long_match && lat_match) {
						PORTD |= (1<<PD7);
						sprintf(strtemp, run_names[i]);
						emic_slope_speak(strtemp);
						PORTD &= ~(1<<PD7);
						break;
					}
				}	
			}
			
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
		failed_requests_count++;
		if (failed_requests_count == 40) {
			friend_tx = 0;
			request_aborted = 1;
		}
	}
}

void init_timer0 (unsigned short int m)
{
	TCCR1B |= (1<<WGM12);
	TIMSK1 |= (1<<OCIE1A);
	OCR1A = m; //m=2880
	TCCR1B |= (1<<CS12);	
}