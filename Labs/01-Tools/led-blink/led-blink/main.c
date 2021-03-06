/***********************************************************************
 * 
 * Blink a LED and use the function from the delay library.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2018-2020 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/

/* Defines -----------------------------------------------------------*/
#define LED_GREEN   PB5     // AVR pin where green LED is connected
#define SHORT_DELAY 300     // Delay in miliseconds
#define	LONG_DELAY 1000		// 
#define	END_DELAY 3000		//
#ifndef F_CPU
#define F_CPU 16000000      // CPU frequency in Hz required for delay func
#endif

/* Includes ----------------------------------------------------------*/	// knihovny
#include <util/delay.h>     // Functions for busy-wait delay loops
#include <avr/io.h>         // AVR device-specific IO definitions			// knihovna popisuj�c� v�stupy

/* Variables ---------------------------------------------------------*/

/* Function prototypes -----------------------------------------------*/

/* Functions ---------------------------------------------------------*/
/**
 * Toggle one LED and use the function from the delay library.
 */
int main(void)
{
    /* // Set pin as output in Data Direction Register
    // DDRB = DDRB or 0010 0000
    DDRB = DDRB | (1<<LED_GREEN);											// << je bin�rn� shift doleva, | je logick� sou�et OR 

    // Set pin LOW in Data Register (LED off)
    // PORTB = PORTB and 1101 1111
    PORTB = PORTB & ~(1<<LED_GREEN);										// & je logick� sou�in AND, ~ je negace

    // Infinite loop
    while (1)
    {
        // Pause 50 miliseconds
        _delay_ms(SHORT_DELAY);

        // Invert LED in Data Register
        // PORTB = PORTB xor 0010 0000
        PORTB = PORTB ^ (1<<LED_GREEN);										// ^ je XOR
    }

    // Will never reach this */
    PORTB = PORTB & ~(1<<LED_GREEN);		// led off
	while (1)								// DE2 morse code
	{
		// D
		PORTB = PORTB ^ (1<<LED_GREEN);		// led on
		_delay_ms(LONG_DELAY);				// _
		PORTB = PORTB ^ (1<<LED_GREEN);		
		_delay_ms(SHORT_DELAY);
		
		PORTB = PORTB ^ (1<<LED_GREEN);		// .
		_delay_ms(SHORT_DELAY);				
		PORTB = PORTB ^ (1<<LED_GREEN);
		_delay_ms(SHORT_DELAY);
		
		PORTB = PORTB ^ (1<<LED_GREEN);		// .
		_delay_ms(SHORT_DELAY);
		PORTB = PORTB ^ (1<<LED_GREEN);
		
		_delay_ms(LONG_DELAY);
		
		// E
		PORTB = PORTB ^ (1<<LED_GREEN);		// .
		_delay_ms(SHORT_DELAY);
		PORTB = PORTB ^ (1<<LED_GREEN);
		
		_delay_ms(LONG_DELAY);
		
		// 2
		PORTB = PORTB ^ (1<<LED_GREEN);		// .
		_delay_ms(SHORT_DELAY);
		PORTB = PORTB ^ (1<<LED_GREEN);
		_delay_ms(SHORT_DELAY);
		
		PORTB = PORTB ^ (1<<LED_GREEN);		// .
		_delay_ms(SHORT_DELAY);
		PORTB = PORTB ^ (1<<LED_GREEN);
		_delay_ms(SHORT_DELAY);
		
		PORTB = PORTB ^ (1<<LED_GREEN);		// _
		_delay_ms(LONG_DELAY);				
		PORTB = PORTB ^ (1<<LED_GREEN);
		_delay_ms(SHORT_DELAY);
		
		PORTB = PORTB ^ (1<<LED_GREEN);		// _
		_delay_ms(LONG_DELAY);
		PORTB = PORTB ^ (1<<LED_GREEN);
		_delay_ms(SHORT_DELAY);
		
		PORTB = PORTB ^ (1<<LED_GREEN);		// _
		_delay_ms(LONG_DELAY);
		PORTB = PORTB ^ (1<<LED_GREEN);
		_delay_ms(SHORT_DELAY);
		
		_delay_ms(END_DELAY);
	}
	
	
	return 0;
}

/* Interrupt routines ------------------------------------------------*/
