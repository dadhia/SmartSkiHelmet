/*************************************************************
* Lab 3
* Loops through each I/O pin in order, turning each one on
* and off.
* Order of pins: 2, 3, 4, 5, 6, 10, 11, 12, 14, 14, 15, 16,
* 17, 18, 19, 23, 24, 25, 26, 27, 28.
* This means first trigger pin is pin 2, but of course you can
* use another pin.  For instance, setting probes on 11, 12,
* 13, and 14 will require trigger on 11 and can show you that
* all four pins are working correctly.
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>


void clearPins();
void enablePinsForOutput();

int main(void)
{
    clearPins();
    enablePinsForOutput();

    while (1) {
      PORTD |= 1 << PD0;
      PORTD &= ~(1 << PD0);
      
      PORTD |= 1 << PD1;
      PORTD &= ~(1 << PD1);
      PORTD |= 1 << PD2;
      PORTD &= ~(1 << PD2);
      PORTD |= 1 << PD3;
      PORTD &= ~(1 << PD3);
      PORTD |= 1 << PD4;
      PORTD &= ~(1 << PD4);
      
      PORTB |= 1 << PB7;
      PORTB &= ~(1 << PB7);
      PORTD |= 1 << PD5;
      PORTD &= ~(1 << PD5);
      PORTD |= 1 << PD6;
      PORTD &= ~(1 << PD6);
      PORTD |= 1 << PD7;
      PORTD &= ~(1 << PD7);
      PORTB |= 1 << PB0;
      PORTB &= ~(1 << PB0);
      
      PORTB |= 1 << PB1;
      PORTB &= ~(1 << PB1);
      PORTB |= 1 << PB2;
      PORTB &= ~(1 << PB2);
      PORTB |= 1 << PB3;
      PORTB &= ~(1 << PB3);
      PORTB |= 1 << PB4;
      PORTB &= ~(1 << PB4);
      PORTB |= 1 << PB5;
      PORTB &= ~(1 << PB5);
      
      PORTC |= 1 << PC0;
      PORTC &= ~(1 << PC0);
      PORTC |= 1 << PC1;
      PORTC &= ~(1 << PC1);
      PORTC |= 1 << PC2;
      PORTC &= ~(1 << PC2);
      PORTC |= 1 << PC3;
      PORTC &= ~(1 << PC3);
      PORTC |= 1 << PC4;
      PORTC &= ~(1 << PC4);
      PORTC |= 1 << PC5;
      PORTC &= ~(1 << PC5);
    }

    return 0;
}

/**
 * Clears the value set for PORT B, C, and D.
 */
void clearPins() {
    PORTB &= ~(1 << PB0) & ~(1 << PB1) & ~(1 << PB2);
    PORTB &= ~(1 << PB3) & ~(1 << PB4) & ~(1 << PB5) & ~(1 << PB7);

    PORTC &= ~(1 << PC0) & ~(1 << PC1) & ~(1 << PC2);
    PORTC &= ~(1 << PC3) & ~(1 << PC4) & ~(1 << PC5);
    
    PORTD &= ~(1 << PD0) & ~(1 << PD1) & ~(1 << PD2) & ~(1 << PD3);
    PORTD &= ~(1 << PD4) & ~(1 << PD5) & ~(1 << PD6) & ~(1 << PD7);
}

/**
 * Sets DDR bits for B, C, and D to 1.  1 indicates output mode.
 */
void enablePinsForOutput() {
    DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2);
    DDRB |= (1 << DDB3) | (1 << DDB4) | (1 << DDB5) | (1 << DDB7);
    
    DDRC |= (1 << DDC0) | (1 << DDC1) | (1 << DDC2);
    DDRC |= (1 << DDC3) | (1 << DDC4) | (1 << DDC5);
    
    DDRD |= (1 << DDD0) | (1 << DDD1) | (1 << DDD2) | (1 << DDD3);
    DDRD |= (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
}

