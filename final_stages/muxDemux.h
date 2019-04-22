#ifndef MUX_DEMUX
#define MUX_DEMUX

#include <avr/io.h>
/* Enable functions. Always call these. */
void enable_rx_select_pins();
void enable_tx_select_pins();

/* Functions to select RX */
void select_GPS_for_rx();
void select_RS232_for_rx();
void select_Emic2_for_rx();
void select_Xbee_for_rx();

/* Functions to select TX */
void select_RS232_for_tx();
void select_Emic2_for_tx();
void select_Xbee_for_tx();

#endif
