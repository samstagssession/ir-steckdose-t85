/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 * IR-Steckdose v0.5 2012/09/16
 *
 * ATTINY85 @ 8 MHz (int. rc-osc)
 *
 * Fuses: lfuse: 0xE2 hfuse: 0xDF efuse: 0xFF
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
*/

/*=========  ToDo ===============
  - multiple addresses on same remote leads to not switching on relais
  - timeout for teachin mode
  - contact irmp author regarding
    //120908 andy - = -> |=
        TIMSK   |= 1 << OCIE1A; // OCIE1A: Interrupt by timer compare
    #endif
*/

/*======== users guide ==========
  - code will be stored in eeprom to be retained after power cycle
  - press button for relais toggle
  - hold button (5sec) to enter teachin mode (slow blinking 4 times per sec)
  - blinks 20 times per sec if any ir code is received
*/

#include "irmp.h"
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>


#ifndef F_CPU
#error F_CPU unkown
#endif

void timer1_init (void){
#if defined (__AVR_ATtiny45__) || defined (__AVR_ATtiny85__)          // ATtiny45 / ATtiny85:

#if F_CPU >= 16000000L
    OCR1C   =  (F_CPU / F_INTERRUPTS / 8) - 1;                        // compare value: 1/15000 of CPU frequency, presc = 8
    TCCR1   = (1 << CTC1) | (1 << CS12);                              // switch CTC Mode on, set prescaler to 8
#else
    OCR1C   =  (F_CPU / F_INTERRUPTS / 4) - 1;                        // compare value: 1/15000 of CPU frequency, presc = 4
    TCCR1   = (1 << CTC1) | (1 << CS11) | (1 << CS10);                // switch CTC Mode on, set prescaler to 4
#endif

#else                                                                 // ATmegaXX:
    OCR1A   =  (F_CPU / F_INTERRUPTS) - 1;                            // compare value: 1/15000 of CPU frequency
    TCCR1B  = (1 << WGM12) | (1 << CS10);                             // switch CTC Mode on, set prescaler to 1
#endif

#ifdef TIMSK1
    TIMSK1  = 1 << OCIE1A;                                            // OCIE1A: Interrupt by timer compare
#else
//120908 andy - = -> |=
    TIMSK   |= 1 << OCIE1A;                                           // OCIE1A: Interrupt by timer compare
#endif
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Timer 1 output compare A interrupt service routine, called every 1/15000 sec
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifdef TIM1_COMPA_vect                                                // ATtiny84
ISR(TIM1_COMPA_vect)
#else
ISR(TIMER1_COMPA_vect)
#endif
{
	(void) irmp_ISR();                                                // call irmp ISR
	// call other timer interrupt routines...
}

//120809 andy - PB0 for irreceiver input (see irmpconfig.h)
#define LED_PIN    PB1                     // pin 6
#define BUTTON_PIN PB2                     // pin 7
#define RELAIS_PIN PB3                     // pin 2

volatile uint32_t systemtime = 0;


ISR( TIM0_COMPA_vect ){                    // every 1ms
	systemtime++;
}

uint32_t millis(){
	return systemtime;
}

void EEPROM_write(unsigned char ucAddress, unsigned char ucData){
	while(EECR & (1<<EEPE));      /* Wait for completion of previous write */
	EECR = (0<<EEPM1)|(0<<EEPM0); /* Set Programming mode */
	EEAR = ucAddress;             /* Set up address and data registers */
	EEDR = ucData;
	EECR |= (1<<EEMPE);           /* Write logical one to EEMPE */
	EECR |= (1<<EEPE);            /* Start eeprom write by setting EEPE */
}

unsigned char EEPROM_read(unsigned char ucAddress){
	while(EECR & (1<<EEPE));      /* Wait for completion of previous write */
	EEAR = ucAddress;             /* Set up address register */
	EECR |= (1<<EERE);            /* Start eeprom read by writing EERE */
	return EEDR;                  /* Return data from data register */
}



int main (void){
	TCCR0A = (1<<WGM01);                   // enable CTC-mode (page 82)
	TCCR0B = (1<<CS01)|(1<<CS00);          // divide by 64  (page 82)
	OCR0A  = 128;                          // 128 = 3trim + 8Mhz / 64prescale / 1kHz (1kHz -> 1ms)
	TIMSK |= 1 << OCIE0A;                  // enable timer compare match interrupt (page 84)
	
	//unsigned char teached_code[5];
	int           buttonState      = 1;    // the current reading from the input pin
	int           teachin          = 0;    // flag for teachin mode
	int           oldButtonState   = 0;
	uint32_t      startTimeButton  = 0;    // starttime of button press
	
	DDRB  |=   1 << LED_PIN;
	DDRB  &=   ~(1 << BUTTON_PIN);
	PORTB |=   1 << BUTTON_PIN;            // internal pullup for button
	DDRB  |=   1 << RELAIS_PIN;
	
	IRMP_DATA irmp_data;
	
	irmp_init();                           // initialize irmp
	timer1_init();                         // initialize timer 1
	sei ();                                // enable interrupts
	
	for (;;){
		buttonState = PINB & (1 << BUTTON_PIN);                       // important: buttonState could be > 1 !
	
		if(oldButtonState != 0 && buttonState == 0){                  // falling edge button pin when button pressed
			startTimeButton = millis();
		}
		if(buttonState == 0 && (millis() - startTimeButton > 5000)){  // hold 5s for teachin mode
			teachin = 1;
		}
		if(oldButtonState == 0 && buttonState != 0 && (millis() - startTimeButton > 50) && teachin == 0){  //hold 50ms for relais toggle
			PORTB ^= 1 << RELAIS_PIN;
			PORTB ^= 1 << LED_PIN;
		}
		if(teachin == 1){
			PORTB ^= 1 << LED_PIN;
			_delay_ms(250);
			PORTB ^= 1 << LED_PIN;
			_delay_ms(250);
		}
		
		oldButtonState = buttonState;
	
		if (irmp_get_data (&irmp_data)){
			PORTB ^= 1 << LED_PIN;                                    //indicate any received ir code
			_delay_ms(50);
			PORTB ^= 1 << LED_PIN;
			_delay_ms(50);

			if (teachin == 1){                                        //store received code into eeprom
				EEPROM_write(0, irmp_data.protocol);
				EEPROM_write(1, irmp_data.address & 0xFF);
				EEPROM_write(2, irmp_data.address >> 8);
				EEPROM_write(3, irmp_data.command & 0xFF);
				EEPROM_write(4, irmp_data.command >> 8);
				teachin = 0;
			}else{
				//relais off if code in EEPROM / switch on if any button
				if( irmp_data.protocol == EEPROM_read(0) && (irmp_data.address & 0xFF) == EEPROM_read(1) && (irmp_data.address >> 8) == EEPROM_read(2) ){
					if( (irmp_data.command & 0xFF) == EEPROM_read(3) && (irmp_data.command >> 8) == EEPROM_read(4) ){
						PORTB &= ~(1 << RELAIS_PIN);                  //relais off / pin low
						PORTB &= ~(1 << LED_PIN);                     //led    off / pin low
					}else{
						PORTB |= 1 << RELAIS_PIN;                     //relais on  / pin high
						PORTB |= 1 << LED_PIN;                        //led    on  / pin high
					}
				}
			}
		}
	}
}
