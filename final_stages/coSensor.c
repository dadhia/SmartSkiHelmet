#include "coSensor.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "serial.h"
#include "emic.h"

#define NO_DATA_CO_LEVEL -1

volatile int isCountingHigh_CO;
volatile int high_count_CO;
volatile int low_count_CO;
volatile int performADC;
int current_CO_ppm;

char ADC_value[60];

int CO_Sensor_LUT[103] = {
	1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6,
	7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12,
	12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16, 16, 17,
	17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24,
	25, 26, 26, 27, 28, 29, 29, 30, 31, 32, 33, 34, 35, 36,
	38, 39, 40, 42, 43, 45, 47, 49, 51, 53, 56, 58, 61, 65,
	69, 73, 78, 84, 91, 99, 110, 124, 142, 170, 216, 313, 805
};

void initialize_CO_reading() {
	//read in relation to AVcc
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);
	
	//set prescaler to 128
	ADCSRA |= (7 << ADPS0);
	
	//use ADC4
	ADMUX |= (4 << MUX0);
	
	//enable ADC
	ADCSRA |= (1 << ADEN);
	current_CO_ppm = NO_DATA_CO_LEVEL;
	
	// Set the Timer Mode to CTC
    TCCR0A |= (1 << WGM01);
    OCR0A = 0xE0; 				//count to 1/32 of a second
    TIMSK0 |= (1 << OCIE0A);    //Set the ISR COMPA vect

    sei(); //enable interrupts
}

void start_CO_monitoring() {
	DDRC |= (1 << DDC5);
	PORTC |= (1 << PC5);
	isCountingHigh_CO = 1;
	// set prescaler to 1024 and start the timer
	TCCR0B |= (1 << CS02) | (1 << CS00);
}

ISR (TIMER0_COMPA_vect) {
	if (isCountingHigh_CO) {
		high_count_CO++;
		if (high_count_CO == 1920) {
			isCountingHigh_CO = 0;
			low_count_CO = 0;
			PORTC &= ~(1 << PC5);
		}
	} else {
		low_count_CO++;
		if (low_count_CO == 2880) {
			isCountingHigh_CO = 1;
			high_count_CO = 0;
			//should not get to perform ADC since we move back to heating
			performADC = 0;
			PORTC |= (1 << PC5);
		} else if ((low_count_CO % 160) == 0) {
			performADC = 1;
		}

	}
}

/**
 * Updates the current_CO_ppm global variable if the sensor is ready to be read.
 */
void update_CO_ppm() {
	if (performADC) {
		performADC = 0;
		ADCSRA |= (1 << ADSC ); // Start a conversion
		while ( ADCSRA & (1 << ADSC )); // wait for conversion complete
		int lower_bits = ADCL;
		int upper_bits = ADCH;
		int x = (upper_bits << 8) | lower_bits;
		current_CO_ppm = CO_Sensor_LUT[x/10];
	}
}

void speak_CO_value() {
	if (current_CO_ppm == NO_DATA_CO_LEVEL) {
		emic_CO_no_data();
	} else {
		emic_CO_speak(current_CO_ppm);
	}
}
