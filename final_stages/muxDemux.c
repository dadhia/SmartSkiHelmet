#include "muxDemux.h"

void enable_serial_mux_demux() {
	enable_rx_select_pins();
	enable_tx_select_pins();
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