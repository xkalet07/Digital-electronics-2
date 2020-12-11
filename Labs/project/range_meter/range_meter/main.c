/***********************************************************************
 * range_meter.c - Paring assistant
 *
 * Measuring the range of obstacles behind car using 3 sensors HC-SR04
 *
 * Created: 18.11.2020 12:37:41
 * Author : Lukas Kaleta
 * 
 ***********************************************************************/ 


/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <stdlib.h>
#include "timer.h"          // Timer library for AVR-GCC
#include "gpio.h"			// Library for using I/O ports
#include "lcd.h"            // Peter Fleury's LCD library
#include "lcd_definitions.h"// Library defining LCD pin conection
#include "uart.h"           // Peter Fleury's UART library

#ifndef F_CPU
#define F_CPU 16000000		// CPU freq definition
#endif

#include <util/delay.h>		// Delay C library

/* Port definitions --------------------------------------------------*/
#define trig_right	PB2		// I/O pins definition for sensors
#define trig_middle	PB3
#define trig_left	PB4

#define echo_common	PD2		// INT0 pin
 
#define sound_sig	PB5		// output pin for sound signalization
#define sound_gnd	PD1	

#define LED0		PC0		// LEDbar
#define LED1		PC1
#define LED2		PC2
#define LED3		PC3
#define LED4		PC4
#define LED5		PC5
#define LED6		PC6
#define LED7		PD0

/* Variables definitions ---------------------------------------------*/
uint8_t sensor_select = 0;

uint16_t L_range = 500;			
uint16_t M_range = 500;
uint16_t R_range = 500;
	
/* Function definitions ----------------------------------------------*/
int main(void)
{
	// GPIO setup for trigger sensor outputs
	GPIO_config_output(&DDRB, trig_right);
	GPIO_config_output(&DDRB, trig_middle);
	GPIO_config_output(&DDRB, trig_left);
	
	// GPIO setup for echo input
	GPIO_config_input_pullup(&DDRD, echo_common);
	
	// GPIO setup for sound output
	GPIO_config_output(&DDRB, sound_sig);
	GPIO_write_low(&PORTB,sound_sig);
	GPIO_config_output(&DDRD, sound_gnd);
	GPIO_write_high(&PORTB,sound_gnd);
	
	// set trigger sensor pins to 0
	GPIO_write_low(&PORTB,trig_left);
	GPIO_write_low(&PORTB,trig_middle);
	GPIO_write_low(&PORTB,trig_right);

	// GPIO setup for LED pins
	GPIO_config_output(&DDRC,LED0);
	GPIO_config_output(&DDRC,LED1);
	GPIO_config_output(&DDRC,LED2);
	GPIO_config_output(&DDRC,LED3);
	GPIO_config_output(&DDRC,LED4);
	GPIO_config_output(&DDRC,LED5);
	GPIO_config_output(&DDRC,LED6);
	GPIO_config_output(&DDRD,LED7);
	
	// Set all LEDbar pins to 0
	GPIO_write_low(&PORTC,LED0);
	GPIO_write_low(&PORTC,LED1);
	GPIO_write_low(&PORTC,LED2);
	GPIO_write_low(&PORTC,LED3);
	GPIO_write_low(&PORTC,LED4);
	GPIO_write_low(&PORTC,LED5);
	GPIO_write_low(&PORTC,LED6);
	GPIO_write_low(&PORTD,LED7);
	
	_delay_ms(100);
	
	// Initialize LCD display
	lcd_init(LCD_DISP_ON);
	
	// directions of range measuring
	lcd_gotoxy(2, 0);			// 3 directions on the Back of the car
	lcd_puts("Left: ");		// Every string ends on position 7
	lcd_gotoxy(3, 1);			// Positions 8 - 15 remains clear
	lcd_puts("Mid: ");
	lcd_gotoxy(1, 2);
	lcd_puts("Right: ");
	
    // Enables interrupts by setting the global interrupt mask
    sei();	

	// Configure 16-bit Timer/Counter1 
	// Enable interrupt and set the overflow prescaler to 1 second
	TIM1_overflow_262ms();
	TIM1_overflow_interrupt_enable();
	
	// Configure 8-bit Timer/Counter2 used for generating sound
	// Enable interrupt and set the overflow prescaler to 512 us
	TIM2_overflow_512us();
	TIM2_overflow_interrupt_enable();

    // Initialize UART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
	// Write app initial text 
	uart_puts("\r\n-----RANGEMETER------\n--Parking-assistant--\r\n");
	
	// Enable interrupt ports INT0, INT1  (PD2, PD3)
	EIMSK |= (1<<INT0);
	// Config interrupt ports INT0, INT1 for rising edge (1 1)
	EICRA |= ((1<<ISC00) | (1<<ISC01));

		
	// Infinite loop
    while (1) 
    {
		// Empty loop
    }
	// Will never reach this
	return 0;
}



/**********************************************************************************
 * ISR starts when Timer/Counter1 overflows.
 * Selecting one of the sensors and sending 10us trigger pulse
 */
ISR(TIMER1_OVF_vect)
{
	sensor_select++;
	
	GPIO_toggle(&PORTD,sound_gnd);		// enables or disables sound
	
	if (sensor_select == 1)
	{
		GPIO_write_low(&PORTB,trig_left);
		_delay_us(10);	
		GPIO_write_high(&PORTB,trig_left);
		_delay_us(10);
		GPIO_write_low(&PORTB,trig_left);	
	}
	else if (sensor_select == 2)
	{
		GPIO_write_low(&PORTB,trig_middle);
		_delay_us(10);
		GPIO_write_high(&PORTB,trig_middle);
		_delay_us(10);
		GPIO_write_low(&PORTB,trig_middle);
	}
	else if (sensor_select == 3)
	{
		GPIO_write_low(&PORTB,trig_right);
		_delay_us(10);
		GPIO_write_high(&PORTB,trig_right);
		_delay_us(10);
		GPIO_write_low(&PORTB,trig_right);
	}
}


/**********************************************************************************
 * ISR starts when INT0 interrupt port is set 1.
 * Starts with rising edge of echo pulse. Then, measuring width of echo pulse,
 *  counting distance and writing to LCD & UART
 */
ISR(INT0_vect)
{
	static uint16_t us_counter = 0;
	char string[8];
	
	// Incrementing us_counter in every loop
	while(GPIO_read(&PIND, echo_common))
	{
		us_counter++;
	}
	
	// Calculating range [cm] from echo pulse width [us]  
	us_counter = us_counter/21.61;	// 21.61 is calibrated
	
	if (sensor_select == 1)			// Left sensor
	{	
		// Clear LCD for Left positions
		lcd_gotoxy(8, 0);
		lcd_puts("        ");
		
		L_range = us_counter;		// Saving left distance to var.
		
		// Writing to LCD and UART
		if (us_counter > 400)			// Maximal range 400 cm
		{
			uart_puts("Left: no obstacle \n");
			lcd_gotoxy(9, 0);
			lcd_puts("clear");			
		}
		else if (us_counter <= 2)		// Minimal range 2 cm
		{
			uart_puts("Left: to close! \n");
			lcd_gotoxy(8, 0);
			lcd_puts("close!");
		}
		else
		{
			itoa(us_counter, string, 10);		// convert range to string
			// Write to UART
			uart_puts("Left: ");
			uart_puts(string);
			uart_puts(" cm");
			uart_puts("\n");
			// Write to LCD display
			if (us_counter < 10)
			{
				lcd_gotoxy(10, 0);
				lcd_puts(string);
				lcd_gotoxy(12, 0);
				lcd_puts("cm");
			}
			else if ((us_counter >= 10) & (us_counter < 100))
			{
				lcd_gotoxy(9, 0);
				lcd_puts(string);
				lcd_gotoxy(12, 0);
				lcd_puts("cm");
			}
			else if (us_counter >= 100)
			{
				lcd_gotoxy(8, 0);
				lcd_puts(string);
				lcd_gotoxy(12, 0);
				lcd_puts("cm");
			}
		}
	}
	else if (sensor_select == 2)		// Middle sensor
	{
		// Clear LCD for Middle positions
		lcd_gotoxy(8, 1);
		lcd_puts("        ");
		
		M_range = us_counter;		// Saving middle distance to var.
		
		// wtriting to LCD and UART
		if (us_counter > 400)			// Maximal range 400 cm
		{
			uart_puts("Middle: no obstacle \n");
			lcd_gotoxy(9, 1);
			lcd_puts("clear");
		}
		else if (us_counter <= 2)		// Minimal range 2 cm
		{
			uart_puts("Middle: to close! \n");
			lcd_gotoxy(8, 1);
			lcd_puts("close!");
		}
		else
		{
			itoa(us_counter,string,10);		// convert range to string
			// Write to UART
			uart_puts("Middle: ");
			uart_puts(string);
			uart_puts(" cm");
			uart_puts("\n");
			// Write to LCD display
			if (us_counter < 10)
			{
				lcd_gotoxy(10, 1);
				lcd_puts(string);
				lcd_gotoxy(12, 1);
				lcd_puts("cm");
			}
			else if ((us_counter >= 10) & (us_counter < 100))
			{
				lcd_gotoxy(9, 1);
				lcd_puts(string);
				lcd_gotoxy(12, 1);
				lcd_puts("cm");
			}
			else if (us_counter >= 100)
			{
				lcd_gotoxy(8, 1);
				lcd_puts(string);
				lcd_gotoxy(12, 1);
				lcd_puts("cm");
			}
		}
	
		/*****************************************
		 * signalizing LEDs for back sensor
		 */
		// Set LEDbar to 0 = turn LEDbar off
		GPIO_write_low(&PORTC,LED0);
		GPIO_write_low(&PORTC,LED1);
		GPIO_write_low(&PORTC,LED2);
		GPIO_write_low(&PORTC,LED3);
		GPIO_write_low(&PORTC,LED4);
		GPIO_write_low(&PORTC,LED5);
		GPIO_write_low(&PORTC,LED6);
		GPIO_write_low(&PORTD,LED7);
		// Set to 1 LEDs for adequate range
		if ((us_counter <= 400) & (us_counter > 300))
		{
			GPIO_write_high(&PORTC,LED0);
		}
		else if ((us_counter <= 300) & (us_counter > 225))
		{
			GPIO_write_high(&PORTC,LED0);
			GPIO_write_high(&PORTC,LED1);
		}
		else if ((us_counter <= 225) & (us_counter > 160))
		{
			GPIO_write_high(&PORTC,LED0);
			GPIO_write_high(&PORTC,LED1);
			GPIO_write_high(&PORTC,LED2);
		}
		else if ((us_counter <= 160) & (us_counter > 110))
		{
			GPIO_write_high(&PORTC,LED0);
			GPIO_write_high(&PORTC,LED1);
			GPIO_write_high(&PORTC,LED2);
			GPIO_write_high(&PORTC,LED3);
		}
		else if ((us_counter <= 110) & (us_counter > 70))
		{
			GPIO_write_high(&PORTC,LED0);
			GPIO_write_high(&PORTC,LED1);
			GPIO_write_high(&PORTC,LED2);
			GPIO_write_high(&PORTC,LED3);
			GPIO_write_high(&PORTC,LED4);
		}
		else if ((us_counter <= 70) & (us_counter > 40))
		{
			GPIO_write_high(&PORTC,LED0);
			GPIO_write_high(&PORTC,LED1);
			GPIO_write_high(&PORTC,LED2);
			GPIO_write_high(&PORTC,LED3);
			GPIO_write_high(&PORTC,LED4);
			GPIO_write_high(&PORTC,LED5);
		}
		else if ((us_counter <= 40) & (us_counter > 15))
		{
			GPIO_write_high(&PORTC,LED0);
			GPIO_write_high(&PORTC,LED1);
			GPIO_write_high(&PORTC,LED2);
			GPIO_write_high(&PORTC,LED3);
			GPIO_write_high(&PORTC,LED4);
			GPIO_write_high(&PORTC,LED5);
			GPIO_write_high(&PORTC,LED6);
		}
		else if (us_counter <= 15)
		{
			GPIO_write_high(&PORTC,LED0);
			GPIO_write_high(&PORTC,LED1);
			GPIO_write_high(&PORTC,LED2);
			GPIO_write_high(&PORTC,LED3);
			GPIO_write_high(&PORTC,LED4);
			GPIO_write_high(&PORTC,LED5);
			GPIO_write_high(&PORTC,LED6);
			GPIO_write_high(&PORTD,LED7);	
		}
	}
	else if (sensor_select == 3)				// Right sensor
	{
		sensor_select = 0;
		// Clear LCD for Right positions
		lcd_gotoxy(8, 2);
		lcd_puts("        ");
		
		R_range = us_counter;		// Saving right distance to var.
		
		// wtriting to LCD and UART
		if (us_counter > 400)			// Maximal range 400 cm
		{
			uart_puts("Right: no obstacle \n");
			lcd_gotoxy(9, 2);
			lcd_puts("clear");
		}
		else if (us_counter <= 2)		// Minimal range 2 cm
		{
			uart_puts("Right: to close! \n");
			lcd_gotoxy(8, 2);
			lcd_puts("close!");
		}
		else
		{
			itoa(us_counter,string,10);		// convert range to string
			// Write to UART
			uart_puts("Right: ");
			uart_puts(string);
			uart_puts(" cm");
			uart_puts("\n");
			// Write to LCD display
			if (us_counter < 10)
			{
				lcd_gotoxy(10, 2);
				lcd_puts(string);
				lcd_gotoxy(12, 2);
				lcd_puts("cm");
			}
			else if ((us_counter >= 10) & (us_counter < 100))
			{
				lcd_gotoxy(9, 2);
				lcd_puts(string);
				lcd_gotoxy(12, 2);
				lcd_puts("cm");
			}
			else if (us_counter >= 100)
			{
				lcd_gotoxy(8, 2);
				lcd_puts(string);
				lcd_gotoxy(12, 2);
				lcd_puts("cm");
			}
		}
	}
}



/**********************************************************************************
 * ISR starts when Timer/Counter2 overflows.
 * Generating signalizing sound on pin B5
 */
ISR(TIMER2_OVF_vect)
{
	if ((L_range <= 30) | (M_range <= 30) | (R_range <= 30))
	{
		TIM2_overflow_512us();
		GPIO_toggle(&PORTB,sound_sig);
	}
	else if ((L_range <= 70) | (M_range <= 70) | (R_range <= 70))
	{
		TIM2_overflow_1ms();
		GPIO_toggle(&PORTB,sound_sig);
	}
	else if ((L_range <= 200) | (M_range <= 200) | (R_range <= 200))
	{
		TIM2_overflow_2ms();
		GPIO_toggle(&PORTB,sound_sig);
	}	
}
	
