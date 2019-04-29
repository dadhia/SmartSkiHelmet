#ifndef CO_SENSOR
#define CO_SENSOR

/**
 * Library built in reference to: https://github.com/swatish17/MQ7-Library
 * and https://www.sparkfun.com/datasheets/Sensors/Biometric/MQ-7.pdf
 */
 
void initialize_CO_reading();
void start_CO_monitoring();
void update_CO_ppm();

#endif