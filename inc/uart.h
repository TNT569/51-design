#ifndef UART_H
#define UART_H

#define UART_RX_EMPTY (-1)
#define UART_RX_OVERFLOW (-2)

void uart_init(void);
void uart_isr(void) __interrupt(4);
int uart_read(void);
void uart_write(const char *text);

#endif
