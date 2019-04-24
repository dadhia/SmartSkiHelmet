#include "gps.h"

uint8_t parseHex(char c);
//GPS CONSTANTS
#define MAX_NUM_CHARS 120

//global variables for GPS info
char line1[MAX_NUM_CHARS];


/* Looks to serial line for the next GPGGA string. */
char* get_GPGGA_string() {
	int i = 0;
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

/* Converts a character to decimal hex value. */
uint8_t parseHex(char c) {
	if ((c >= '0') && (c <= '9')) {
		return c - '0';
	}
	if ((c >= 'A') && (c <= 'F')) {
		return (c - 'A') + 10;
	}
	return 0;
}
