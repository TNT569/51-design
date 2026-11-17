#include "timer.h"

#include "key.h"
#include "reg52.h"

/* 10 ms at 11.0592 MHz in 12T mode is 9216 timer ticks. */
#define TIMER0_RELOAD (65536u - 9216u)

static void timer0_reload(void) {
  TH0 = TIMER0_RELOAD >> 8;
  TL0 = TIMER0_RELOAD & 0xff;
}

void timer_init(void) {
  TR0 = 0;
  TR1 = 0;
  TMOD = 0x11;
  timer0_reload();
  TF0 = 0;
  ET0 = 1;
  TR0 = 1;
  EA = 1;
}

void timer0_isr(void) __interrupt(1) {
  timer0_reload();
  key_scan();
}

void timer_delay_ticks(unsigned int ticks) {
  unsigned int reload = 0u - ticks;

  TH1 = reload >> 8;
  TL1 = reload;
  TF1 = 0;
  TR1 = 1;
  while (!TF1) {
  }
  TR1 = 0;
}

void delay_ms(unsigned int ms) {
  while (ms--) {
    delay_us(1000);
  }
}
