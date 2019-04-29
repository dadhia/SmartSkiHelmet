#include "gps.h"
#include "muxDemux.h"
#include <string.h>
#include <stdlib.h>

//GPS CONSTANTS
#define MAX_NUM_CHARS 120

uint8_t parseHex(char c);

char NMEA_line[MAX_NUM_CHARS];
char info[MAX_NUM_CHARS];
/**
 * Initialize the run_coors and run_names arrays.
 */
void initialize_internal_gps_map(long run_coor[3][4], char run_NSWE[3][2], char run_names[3][30]) {
	//engineering quad
	strcpy(run_names[0],"Dave's Run");
	run_coor[0][0] = 34011700; 
	run_coor[0][1] = 34012641; 	//lat_max
	run_coor[0][2] = 118172746; //long_min
	run_coor[0][3] = 118173958; //long_max
	run_NSWE[0][0] = 'N';
	run_NSWE[0][1] = 'W';
	
	//SGM
	strcpy(run_names[1] ,"Hangman's Hollow");
	run_coor[1][0] = 34012323;
	run_coor[1][1] = 34013118;
	run_coor[1][2] = 118172414;
	run_coor[1][3] = 118173647;
	run_NSWE[1][0] = 'N';
	run_NSWE[1][1] = 'W';
	
	//Campus Center to alumni
	strcpy(run_names[2], "Drop Out Chutes");
	run_coor[2][0] = 34011808;
	run_coor[2][1] = 34012688;
	run_coor[2][2] = 118171224;
	run_coor[2][3] = 118171543;
	run_NSWE[0][0] = 'N';
	run_NSWE[0][1] = 'W';
}

/* Looks to serial line for the next GPGGA string. */
char* get_GPGGA_string() {
	select_GPS_for_rx();
	int i = 0;
	char c;
	while (1) {
		NMEA_line[i] = c = serial_in();
		if (c == '$') {
			NMEA_line[++i] = c = serial_in();
			if (NMEA_line[i] == 'G') {
				NMEA_line[++i] = c = serial_in();
				if (NMEA_line[i] == 'P') {
					NMEA_line[++i] = c = serial_in();
					if (NMEA_line[i] == 'G') {
						NMEA_line[++i] = c = serial_in();
						if (NMEA_line[i] == 'G') {
							NMEA_line[++i] = c = serial_in();
							if (NMEA_line[i] == 'A') {
								while ( (c = serial_in()) != '\n') {
									NMEA_line[++i] = c;
								}
								NMEA_line[++i] = c;
								NMEA_line[++i] = '\0';
								//now verify checksum
								size_t length = strlen(NMEA_line);
								if (NMEA_line[length - 5] == '*') {
									uint16_t checksum = parseHex(NMEA_line[length - 4]) * 16;
									checksum += parseHex(NMEA_line[length -3]);
									for (i = 1; i < length - 5; i++) {
										checksum ^= NMEA_line[i];
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
	return NMEA_line;
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

char * gps_get_info(char * info_array) {
	/*
		$--GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx
		0123456789012345678901234567890123456789012345678901234567890123456
			1         2         3         4         5         6  
		$GPGGA,201937.000,3401.2181,N,11817.3458,W,1,06,2.34,129.0,M,-33.8,M,,*54
	*/
	short count = 0;
	int i = 0;
	info[0] = '\0';//to clear string each time, since it is global
	if( (info_array[18] == ',') || (info_array[19] == ',') || (info_array[20] == ',')) {
		return NULL;
	}

	info[count] = 'O';
	count++;
	for (i = 18; i < 42; i++) {
		if((info_array[i] != ',') & (info_array[i] != '.'))
		{
			info[count] = info_array[i];
			count++;
			if((info_array[i] == 'N') | (info_array[i] == 'S'))
			{
				info[count] = 'A';
				count++;
			}
		}
	}
	info[count] = '@';
	return info;
}
