//
// Zwei Byte werden übertragen, Z.B.: erst  0x00 und dann 0x90
// 0000 0000  1001 0000
//    |____|  |   |____|
//      |   Wert    |
//  Haus-Adresse  Gerät-Adresse
// MSBF muss bei der Anwendung nicht beachtet werden !

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include "433mhz.h"

#define F_CPU 8000000UL
extern volatile uint8_t timer = 0;

void init433mhz(void)
{
  DDRA = 0x01;
// Timer 0
  TCCR0A |= (1 << CTC0); // CTC Modus
  TCCR0B |= (1 << CS01); // Prescaler 8
  OCR0A = 75 - 1; // (1/(8000000/8)) = 1us --> 75us / 1us = 75
  TIMSK |= (1 << OCIE0A);  // Compare Interrupt erlauben
  sei();  // Global Interrupts aktivieren
}

ISR (TIMER0_COMPA_vect)
{
  TCNT0L = 0;
  timer = timer <= 255 ? timer + 1 :  0;
}

void wait (uint8_t n)
{
  timer = 1;
  while (timer <= (5 * n)) {};
}

void bit0(void)
{
  PORTA = 0x01;
  wait(1);
  PORTA = 0x00;
  wait(3);
  PORTA = 0x01;
  wait(1);
  PORTA = 0x00;
  wait(3);
}

void bit1(void)
{
  PORTA = 0x01;
  wait(3);
  PORTA = 0x00;
  wait(1);
  PORTA = 0x01;
  wait(3);
  PORTA = 0x00;
  wait(1);
}

void bitf(void)
{
  PORTA = 0x01;
  wait(1);
  PORTA = 0x00;
  wait(3);
  PORTA = 0x01;
  wait(3);
  PORTA = 0x00;
  wait(1);
}

void sync(void)
{
  PORTA = 0x01;
  wait(1);
  PORTA = 0x00;
  wait(31);
}

void send(uint8_t addr, uint8_t dev)
{
  for (int n = 0; n < 8; ++n)
  {
    sync();
    for (int i = 4; i >= 0; --i)
    {
      if (addr & (1 << i))
      {
        bit1();
      }
      else
      {
        bit0();
      }
    }

    for (int i = 4; i >= 0; --i)
    {
      if (dev & (1 << i))
      {
        bit0();
      }
      else
      {
        bitf();
      }
    }

    if (dev & (1 << 7))
    {
      bit0();
      bitf();
    }
    else
    {
      bitf();
      bit0();
    }
  }
}
