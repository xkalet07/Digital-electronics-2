/***********************************************************************
 * 
 * Stopwatch with LCD display output.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2017-2020 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "lcd.h"            // Peter Fleury's LCD library
#include <stdlib.h>         // C library. Needed for conversion function

/* Variables ---------------------------------------------------------*/
// Custom characters definition using https://omerk.github.io/lcdchargen/
uint8_t customChar[] = {
    // addr 0: .....
    0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000,
    // addr 1: |....
    0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000,
    // addr 1: ||...
	0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000,
	// addr 1: |||..
	0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100,
	// addr 1: ||||.
	0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110,
	// addr 1: |||||
	0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111,
};

/* Function definitions ----------------------------------------------*/
/**
 * Main function where the program execution begins. Update stopwatch
 * value on LCD display when 8-bit Timer/Counter2 overflows.
 */
int main(void)
{
    // Initialize LCD display
    lcd_init(LCD_DISP_ON);
    
	
	/* loading custom characters */
	// Set pointer to beginning of CGRAM memory
	lcd_command(1 << LCD_CGRAM);
	for (uint8_t i = 0; i < 8*6; i++)
	{
		// Store all new chars to memory line by line
		lcd_data(customChar[i]);
	}
	// Set DDRAM address
	lcd_command(1 << LCD_DDRAM);


			
    // Put string(s) at LCD display
    lcd_gotoxy(1, 0);
    lcd_puts("00:00.0");
/*  lcd_gotoxy(11, 0);
    lcd_puts("a");
	lcd_gotoxy(1, 1);
	lcd_puts("b");
*/
	lcd_gotoxy(11, 1);
	lcd_puts("c");
		
	
 
    // Configure 8-bit Timer/Counter2 for Stopwatch
    // Set prescaler and enable overflow interrupt every 16 ms
	TIM2_overflow_16ms();
	TIM2_overflow_interrupt_enable();

	// Configure 8-bit Timer/Counter0 for loading bar
	// Set prescaler and enable overflow interrupt every 16 ms
	TIM0_overflow_4ms();
	TIM0_overflow_interrupt_enable();

    // Enables interrupts by setting the global interrupt mask
    sei();

    // Infinite loop
    while (1)
    {
        /* Empty loop. All subsequent operations are performed exclusively 
         * inside interrupt service routines ISRs */
    }

    // Will never reach this
    return 0;
}

/* Interrupt service routines ----------------------------------------*/
/**
 * ISR starts when Timer/Counter2 overflows. Update the stopwatch on
 * LCD display every sixth overflow, ie approximately every 100 ms
 * (6 x 16 ms = 100 ms).
 */
ISR(TIMER2_OVF_vect)
{
    static uint8_t number_of_overflows = 0;
    static uint8_t tens = 0;        // Tenths of a second
    static uint8_t secs = 0;        // Seconds
    static uint8_t mins = 0;        // Minutes
	
    static uint16_t secs_square = 0; // squared number of secs on position A
	
    char lcd_string[] = "  ";      // String for converting numbers by itoa()

    number_of_overflows++;
    if (number_of_overflows >= 5)
    {
        // Do this every 6 x 16 ms = cca 100 ms
        number_of_overflows = 0;
		
        // WRITE YOUR CODE HERE
		tens++;
		if(tens > 9)
		{
			tens = 0;
			secs++;
			if(secs > 9)
			{
				if(secs > 59)
				{
					secs = 0;
					lcd_gotoxy(5, 0);
					lcd_puts("0");
					
					mins++;
					if(mins > 9)
					{
						if(mins > 59)
						{
							mins = 0;
							lcd_gotoxy(2, 0);
							lcd_puts("0");
						}
						itoa(mins, lcd_string, 10);	// displaying > 9 minutes
						lcd_gotoxy(1, 0);
						lcd_puts(lcd_string);
					}
					
					else
					{
						itoa(mins, lcd_string, 10);	// displaying < 10 minutes
						lcd_gotoxy(2, 0);
						lcd_puts(lcd_string);
					}
					
					// clearing square of secs
					lcd_gotoxy(11, 0);
					lcd_puts("    ");
					
				}
				itoa(secs, lcd_string, 10);			// displaying > 9 seconds
				lcd_gotoxy(4, 0);
				lcd_puts(lcd_string);

			}			
			else
			{
				itoa(secs, lcd_string, 10);			// displaying < 10 seconds
				lcd_gotoxy(5, 0);
				lcd_puts(lcd_string);
			}

			// computing square of secs
			secs_square = secs * secs;				
			itoa(secs_square, lcd_string, 10);
			lcd_gotoxy(11, 0);
			lcd_puts(lcd_string);
									
		}
		itoa(tens, lcd_string, 10);					// displaying tenths of seconds
		lcd_gotoxy(7, 0);
		lcd_puts(lcd_string);
    }
}


/*--------------------------------------------------------------------*/
/**
 * ISR starts when Timer/Counter0 overflows. Update the progress bar on
 * LCD display every 16 ms.
 */
ISR(TIMER0_OVF_vect)
{	
	static uint8_t number_of_overflows = 0;
	static uint8_t symbol = 0;
    static uint8_t position = 1;

	
	number_of_overflows++;
	
	if (number_of_overflows >= 6)
	{
		number_of_overflows = 0;
		symbol++;
		if (symbol > 5)							// one bar is full
		{
			symbol = 1;
			position++;
			if (position > 9)
			{
				position = 1;					// clear position
				lcd_gotoxy(1, 1);				// clear all bars
				lcd_puts("          ");
			}
		}

		// display bar
		lcd_gotoxy(position, 1);
		lcd_putc(symbol);
	}

}