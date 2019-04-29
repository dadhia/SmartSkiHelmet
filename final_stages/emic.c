#include "emic.h"
#include "serial.h"
#include "muxDemux.h"
#include <util/delay.h>
#include <stdio.h>

#define SPEAK_TEXT_LENGTH 120
#define EMIC_DELAY 1000
#define EMIC_DEBUG 1

char emic_speak_text[SPEAK_TEXT_LENGTH];


void initialize_emic() {
	char c;
	select_Emic2_for_rx();
	while ( (c = serial_in()) != ':') ;
	_delay_ms(EMIC_DELAY);
	select_Emic2_for_tx();
	sci_outs("V15\n");
	while ( (c = serial_in()) != ':');
	_delay_ms(EMIC_DELAY);
	sci_outs("SWelcome to your smart ski helmet.\n");
	while ( (c = serial_in()) != ':');
}

void emic_slope_speak(char *run) {
	select_Emic2_for_tx();
	sprintf(emic_speak_text, "SYour friend is on %s\n", run);
	_delay_ms(EMIC_DELAY);
	sci_outs(emic_speak_text);
	_delay_ms(EMIC_DELAY);
}

void emic_my_slope_speak(char *run) {
	select_Emic2_for_tx();
	sprintf(emic_speak_text, "SYou are on %s\n", run);
	_delay_ms(EMIC_DELAY);
	sci_outs(emic_speak_text);
	_delay_ms(EMIC_DELAY);
}

void emic_no_gps_fix() {
	select_Emic2_for_tx();
	sprintf(emic_speak_text, "SNo GPS fix. Please try later.\n");
	_delay_ms(EMIC_DELAY);
	sci_outs(emic_speak_text);
	_delay_ms(EMIC_DELAY);
	if (EMIC_DEBUG) {
		select_RS232_for_tx();
		_delay_ms(500);
		sci_outs("Finished sending no gps fix data.\r\n");
	}
}

void emic_CO_speak(int current_CO_level) {
	select_Emic2_for_tx();
	sprintf(emic_speak_text, "SThe current carbon monoxide level is %d parts per million.\n", current_CO_level);
	
	_delay_ms(EMIC_DELAY);
	sci_outs(emic_speak_text);
	_delay_ms(EMIC_DELAY);
	if (current_CO_level >= 200) {
		sprintf(emic_speak_text, "SCarbon monoxide levels are high. Please leave the area.\n");
		_delay_ms(EMIC_DELAY);
		sci_outs(emic_speak_text);
		_delay_ms(EMIC_DELAY);
	}
}

void emic_CO_no_data() {
	select_Emic2_for_tx();
	sprintf(emic_speak_text, "SNo carbon manoxide data yet. Wait a minute.\n");
	_delay_ms(EMIC_DELAY);
	sci_outs(emic_speak_text);
	_delay_ms(EMIC_DELAY);
}

void emic_no_friend() {
	select_Emic2_for_tx();
	sprintf(emic_speak_text, "SSorry. Cannot connect to your friend. Try later.\n");
	_delay_ms(EMIC_DELAY);
	sci_outs(emic_speak_text);
	_delay_ms(EMIC_DELAY);
}

void emic_contacting_your_friend() {
	select_Emic2_for_tx();
	sprintf(emic_speak_text, "SConnecting to your friend's helmet.\n");
	_delay_ms(EMIC_DELAY);
	sci_outs(emic_speak_text);
	_delay_ms(EMIC_DELAY);
}
