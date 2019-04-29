#ifndef GPS
#define GPS
#include "serial.h"

char* get_GPGGA_string();
void initialize_internal_gps_map(long run_coors[3][4], char run_NSWE[3][2], char run_names[3][30]);
char * gps_get_info(char * info_array); 

#endif