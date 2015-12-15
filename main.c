/****************************************************************************
 * I2C Slave using USI.
 *
 * (C) Andreas Kaiser 2005.
 ****************************************************************************/

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include "i2c-slave.h"
#include "433mhz.h"

#define I2C_ADDRESS 0x1a

#define DEBUG	0
#define SLEEP	1

#if DEBUG
#define SetD(x) PORTD &= ~(1<<(x))
#define ClrD(x) PORTD |=  (1<<(x))
#else
#define SetD(x)
#define ClrD(x)
#endif

void
initialize(void)
{

	i2c_initialize(I2C_ADDRESS);

#if SLEEP
	MCUCR |= 1 << SE | 1 << SM1 | 1 << SM0;	// power-down when sleeping
#endif
}

int
main(void)
{
	
	init433mhz();
	initialize();
	uint8_t buf = 0x77;
	for (;;) {
#if SLEEP
		wdt_disable();

		/*
		 * Cannot use the avr-libc code, as there exists a race condition when the
		 * interrupt occurs between the idle test and the sleep.  Since the instruction
		 * after the SEI instruction is always executed before an interrupt, this code
		 * is safe however.
		 */
		cli();
		if (i2c_idle()) {
			asm volatile ("sei\n\tsleep");
		}
		sei();

		wdt_enable(WDTO_2S);
#endif
		if (i2c_reply_ready()) { //SENDE
			i2c_rdbuf[0] = buf; //PINA & 0x03;		// respond with data on port A
			i2c_reply_done(1);
		}

		if (i2c_message_ready()) {//EMPFANGE
			buf = i2c_wrbuf[0];		// output message data to port D
			i2c_message_done();
			//i2c_rdbuf[0] = buf; //PINA & 0x03;		// respond with data on port A
			//i2c_reply_done(1);
			send(i2c_wrbuf[0], i2c_wrbuf[1]);
		}
	}

}


