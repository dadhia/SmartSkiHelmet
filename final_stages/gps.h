#ifndef GPS
#define GPS
#include "serial.h"
#include <stdlib.h>

char* get_GPGGA_string();
void initialize_internal_gps_map(int run_coors[3][4], char run_NSWE[3][2], char run_names[3][30]);

#endif