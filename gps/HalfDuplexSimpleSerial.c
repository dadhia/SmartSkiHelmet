/*
Files     	: HalfDuplexSimpleSerial.c
Version 	: 1.0
Date     	: 2013/01/16
Description	: This is a Half Duplex Serial Driver, The Driver only tested on SimplyTronics GPS Smart Module @ 9600 bps and UNO
                  The driver needs a pull-up resistor, please add a 4.7k ~ 100 k resistor to the SIO
                  (The GPS Smart Module already has Pull-up resistor)
                  The driver is basic on the original SoftwareSerial object from the library. 
                  Re-write with C. Did some changes to the low level, inline assembly language, pins configure etc...
                  Because the serial drive need accurate timing, so no big change to the original code to avoid creating the look up table.

                 Note: The driver will block the program while waiting for the data. This global interrupt has been disable, and will be resume before exit the function.
                 
History		:

    1. Date		: 2013/01/16
       Author		: John Zhong@SimplyTronics
       Modification	: Create
   
    2. Date		: 2013/06/18
       Author		: John Zhong@SimplyTronics
       Modification	: Fixed the timeout bug, add the Error check.
   
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "HalfDuplexSimpleSerial.h"
#define LOW 0
#define HIGH 1
# define cli()  __asm__ __volatile__ ("cli" ::)

// Lookup table
typedef struct _DELAY_TABLE
{
  long baud;
  unsigned short rx_delay_centering;
  unsigned short rx_delay_intrabit;
  unsigned short rx_delay_stopbit;
  unsigned short tx_delay;
} DELAY_TABLE;

static const DELAY_TABLE table[] PROGMEM = 
{
  //  baud    rxcenter    rxintra    rxstop  tx
  { 115200,   1,          5,         5,      3,      },
  { 57600,    1,          15,        15,     13,     },
  { 38400,    2,          25,        26,     23,     },
  { 31250,    7,          32,        33,     29,     },
  { 28800,    11,         35,        35,     32,     },
  { 19200,    20,         55,        55,     52,     },
  { 14400,    30,         75,        75,     72,     },
  { 9600,     50,         114,       114,    112,    },
  { 4800,     110,        233,       233,    230,    },
  { 2400,     229,        472,       472,    469,    },
  { 1200,     467,        948,       948,    945,    },
  { 300,      1895,       3805,      3805,   3802,   },
};
const int XMIT_START_ADJUSTMENT = 4;


/* Global variables for the serial port */
uint8_t _rxtxPin;
uint8_t _rxtxBitMask;
// io input and output registers(different registers) 
volatile uint8_t *_rxtxPortRegister;

// rx the center of a bit
uint16_t _rx_delay_centering;
uint16_t _rx_delay_intrabit;
// rx stop bit
uint16_t _rx_delay_stopbit;
uint16_t _tx_delay;

uint8_t _inverse_logic;

volatile unsigned long timer0_millis = 0;

uint8_t HDSS_rxtx_pin_read();
void HDSS_rxtx_pin_write(uint8_t pin_state);
void HDSS_set_to_tx(void);
void HDSS_set_to_rx(void);

inline void HDSS_tunedDelay(uint16_t delay);


/* delay written with assembly to get accurate timing */
inline void HDSS_tunedDelay(uint16_t delay) 
{ 
  uint8_t tmp=0;  
  asm volatile("sbiw    %0, 0x01 \n\t"
    "ldi %1, 0xFF \n\t"
    "cpi %A0, 0xFF \n\t"
    "cpc %B0, %1 \n\t"
    "brne .-10 \n\t"
    : "+r" (delay), "+a" (tmp)
    : "0" (delay)
    );
}

/* set the io to tx (output) to tx data */
void HDSS_set_to_tx(void)
{
    DDRD |= 1 << DDD7;
    PORTD |= 1 << PD7;
    _rxtxPortRegister = &PORTD;
}

/* set the io to rx(input), ready for receiving */
void HDSS_set_to_rx(void)
{
    DDRD &= ~(1 << DDD7);
    if (!_inverse_logic)
    {
        PORTD |= 1 << PD7;
    }
    _rxtxPortRegister = &PIND;
}

/* write a bit to sio */
void HDSS_rxtx_pin_write(uint8_t pin_state)
{
  if (pin_state == LOW)
  {
    *_rxtxPortRegister &= ~_rxtxBitMask;
  }
  else
  {
    *_rxtxPortRegister |= _rxtxBitMask;
  }
}

/* read a bit from sio*/
uint8_t HDSS_rxtx_pin_read()
{
  return *_rxtxPortRegister & _rxtxBitMask;
}

/* release the io */
void HDSS_stop(void)
{
    PCMSK0 &= ~(1 << 4);
}

/* start and initalize the io, baudrate */
void HDSS_start(uint8_t inverse_logic, long speed)
{
  unsigned char i = 0;
  //uint8_t port;
  _rx_delay_centering = 0;
  _rx_delay_intrabit = 0;
  _rx_delay_stopbit = 0;
  _tx_delay = 0;

  _rxtxBitMask = 1 << PD7;

  for (i=0; i<sizeof(table)/sizeof(table[0]); ++i)
  {
    long baudrate = pgm_read_dword(&table[i].baud);
    if (baudrate == speed)
    {
      _rx_delay_centering = pgm_read_word(&table[i].rx_delay_centering);
      _rx_delay_intrabit = pgm_read_word(&table[i].rx_delay_intrabit);
      _rx_delay_stopbit = pgm_read_word(&table[i].rx_delay_stopbit);
      _tx_delay = pgm_read_word(&table[i].tx_delay);
      break;
    }
  }
  HDSS_tunedDelay(_tx_delay); // if we were low this establishes the end
}


/* send out a bit */
void HDSS_write(uint8_t b)
{
  char mask = 0x01;
  if (_tx_delay != 0) 
  {  
    uint8_t oldSREG = SREG;
    cli();  // turn off interrupts for a clean txmit
    
    HDSS_set_to_tx();
    
    // Write the start bit
    HDSS_rxtx_pin_write(_inverse_logic ? HIGH : LOW);
    HDSS_tunedDelay(_tx_delay + XMIT_START_ADJUSTMENT);
  
    // Write each of the 8 bits
    if (_inverse_logic)
    {
      for (; mask; mask <<= 1)
      {
        if (b & mask) // choose bit
        {
          HDSS_rxtx_pin_write(LOW); // send 1
        }
        else
        {
          HDSS_rxtx_pin_write(HIGH); // send 0
        }
        HDSS_tunedDelay(_tx_delay);
      }
  
      HDSS_rxtx_pin_write(LOW); // restore pin to natural state
    }
    else
    {
      for (; mask; mask <<= 1)
      {
        if (b & mask) // choose bit
        {
          HDSS_rxtx_pin_write(HIGH); // send 1
        }
        else
        {
          HDSS_rxtx_pin_write(LOW); // send 0
        }      
        HDSS_tunedDelay(_tx_delay);
      }  
      HDSS_rxtx_pin_write(HIGH); // restore pin to natural state
    }
    
    SREG = oldSREG; // turn interrupts back on
    HDSS_tunedDelay(_tx_delay);
  
    HDSS_set_to_rx();
  }
}

unsigned long millis()
{
        unsigned long m;
        uint8_t oldSREG = SREG;

        // disable interrupts while we read timer0_millis or we might get an
        // inconsistent value (e.g. in the middle of a write to timer0_millis)
        cli();
        m = timer0_millis;
        SREG = oldSREG;

        return m;
}


/* receive a bit, block until received the data */
uint8_t HDSS_read(void)
{
  unsigned char i = 0x01;
  HDSS_set_to_rx();
  uint8_t oldSREG = SREG;
  cli();  // turn off interrupts for a clean receive
  uint8_t d = 0;

  // If RX line is high, then we don't see any start bit
  if (_inverse_logic)
  {
	while(!HDSS_rxtx_pin_read());
  }
  else
  {
	while(HDSS_rxtx_pin_read());
  }
  
  // Wait approximately 1/2 of a bit width to "center" the sample
  HDSS_tunedDelay(_rx_delay_centering);

  // Read each of the 8 bits
  for (; i; i <<= 1)
  {
    HDSS_tunedDelay(_rx_delay_intrabit);
    uint8_t noti = ~i;
    if (HDSS_rxtx_pin_read())
    {
      d |= i;
    }
    else // else clause added to ensure function timing is ~balanced
    {
      d &= noti;
    }
  }

  // skip the stop bit
  HDSS_tunedDelay(_rx_delay_stopbit);

  if (_inverse_logic)
  {
    d = ~d;
  }
  SREG = oldSREG;
  return d;
}


/* receive a byte with a simple implement of timeout */
uint16_t HDSS_read_time(uint32_t timeout)
{
  unsigned char i = 0x01;
  HDSS_set_to_rx();
  uint8_t oldSREG = SREG;
  uint32_t time = 0;
  uint16_t d = 0;
  time = millis();
  timeout += time;
  
  // If RX line is high, then we don't see any start bit
  if (_inverse_logic)
  {
	while((!HDSS_rxtx_pin_read()) && (time < timeout))
        {
          time = millis();
        }
  }
  else
  {
	while((HDSS_rxtx_pin_read()) && (time < timeout))
        {
          time = millis();
        }
  }
  if (time >= timeout)
  {
    return ERROR_RX_TIMEOUT;
  }
  
  cli();// turn off interrupts for a clean receive
  // Wait approximately 1/2 of a bit width to "center" the sample
  HDSS_tunedDelay(_rx_delay_centering);

  // Read each of the 8 bits
  for (; i; i <<= 1)
  {
    HDSS_tunedDelay(_rx_delay_intrabit);
    uint8_t noti = ~i;
    if (HDSS_rxtx_pin_read())
    {
      d |= i;
    }
    else // else clause added to ensure function timing is ~balanced
    {
      d &= noti;
    }
  }

  // skip the stop bit
  HDSS_tunedDelay(_rx_delay_stopbit);

  if (_inverse_logic)
  {
    d = ~d;
  }
  SREG = oldSREG;
  return d & 0x00FF;
}

