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
		$--GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx
		0123456789012345678901234567890123456789012345678901234567890123456
			1         2         3         4         5         6  
		$GPGGA,201937.000,3401.2181,N,11817.3458,W,1,06,2.34,129.0,M,-33.8,M,,*54
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
