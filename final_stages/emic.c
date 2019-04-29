#include "emic.h"
#include "serial.h"
#include "muxDemux.h"
#include <util/delay.h>
#include <stdio.h>

#define SPEAK_TEXT_LENGTH 120

char emic_speak_text[SPEAK_TEXT_LENGTH];

void initialize_emic() {
	char c;
	select_Emic2_for_rx();
	while ( (c = serial_in()) != ':') ;
	_delay_ms(500);
	select_Emic2_for_tx();
	sci_outs("V15\n");
	while ( (c = serial_in()) != ':');
	_delay_ms(500);
	sci_outs("SWelcome to your smart ski helmet.\n");
	while ( (c = serial_in()) != ':');
}

void emic_slope_speak(char *run) {
	char c;
	select_Emic2_for_tx();
	select_Emic2_for_rx();
	sprintf(emic_speak_text, "SYour friend is on %s\n", run);
	sci_outs(emic_speak_text);
	while ( (c = serial_in()) != ':');
}

void emic_CO_speak(int current_CO_level) {
	char c;
	select_Emic2_for_tx();
	select_Emic2_for_rx();
	sprintf(emic_speak_text, "SThe current carbon monoxide level is %d parts per million.\n", current_CO_level);
	_delay_ms(500);
	sci_outs(emic_speak_text);

	if (current_CO_level >= 200) {
		select_Emic2_for_tx();
		sprintf(emic_speak_text, "SCarbon monoxide levels are high. Please leave the area.\n");
		_delay_ms(500);
		sci_outs(emic_speak_text);
		while ( (c = serial_in()) != ':');
	}
}
