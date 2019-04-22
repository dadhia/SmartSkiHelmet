#ifndef SERIAL
#define SERIAL

#include <avr/io.h>

void serial_init();
void serial_out(char c);
void sci_outs(char * s);
char serial_in();

#endif
