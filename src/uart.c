#include "uart.h"

#include <stdint.h>

#include "reg52.h"

__sfr __at(0xc9) T2MOD;

#define RX_CAPACITY 32u
#define RX_MASK (RX_CAPACITY - 1u)
#define BAUD_RELOAD 0xffdcu

static volatile __xdata uint8_t rx[RX_CAPACITY];
static volatile uint8_t head;
static volatile uint8_t tail;
static volatile __bit overflow;
static volatile __bit discard_line;
static volatile __bit tx_ready;

void uart_init(void) {
  ES = 0;
  RXD = 1;
  TXD = 1;
  PCON &= 0x3f;
  SCON = 0x50;
  T2CON = 0;
  T2MOD = 0;

  /* 11.0592 MHz / (32 * (65536 - 0xFFDC)) = 9600 in 12T mode. */
  RCAP2H = BAUD_RELOAD >> 8;
  RCAP2L = BAUD_RELOAD & 0xff;
  TH2 = RCAP2H;
  TL2 = RCAP2L;
  RCLK = 1;
  TCLK = 1;
  TR2 = 1;
  tx_ready = 1;
  ES = 1;
}

void uart_isr(void) __interrupt(4) {
  if (RI) {
    uint8_t byte = SBUF;
    uint8_t next = (head + 1u) & RX_MASK;

    RI = 0;
    if (discard_line) {
      discard_line = byte != '\n';
    } else if (next == tail) {
      /* Drop the partial frame and synchronize on the next LF. */
      tail = head;
      overflow = 1;
      discard_line = byte != '\n';
    } else {
      rx[head] = byte;
      head = next;
    }
  }
  if (TI) {
    TI = 0;
    tx_ready = 1;
  }
}

int uart_read(void) {
  int byte = UART_RX_EMPTY;

  ES = 0;
  if (overflow) {
    overflow = 0;
    byte = UART_RX_OVERFLOW;
  } else if (head != tail) {
    byte = rx[tail];
    tail = (tail + 1u) & RX_MASK;
  }
  ES = 1;
  return byte;
}

void uart_write(const char *text) {
  while (*text) {
    while (!tx_ready) {
    }
    tx_ready = 0;
    SBUF = *text++;
  }
}
