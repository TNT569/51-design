#ifndef TIMER_H
#define TIMER_H

void timer_init(void);
void timer0_isr(void) __interrupt(1);
void timer_delay_ticks(unsigned int ticks);
void delay_ms(unsigned int ms);

/* 11.0592 MHz / 12 gives 576 timer ticks per 625 microseconds. */
#define delay_us(us) timer_delay_ticks(((us) * 576ul + 624ul) / 625ul)

#endif
