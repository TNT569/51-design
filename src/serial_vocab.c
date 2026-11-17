#include "serial_vocab.h"

#include <stdint.h>
#include <string.h>

#include "uart.h"
#include "vocab.h"

#define MAX_FRAME (4 + VOCAB_WORD_LEN + 1 + VOCAB_PART_LEN + 1 + VOCAB_MEANING_LEN)
static __xdata char frame[MAX_FRAME + 2]; /* Optional CR and terminating NUL. */
static __xdata Word incoming_word;
static uint8_t length;
static bool invalid_frame;

static const char *read_field(const char *input, char *output, uint8_t limit,
                              char separator) {
  uint8_t count = 0;
  char first_printable = separator ? '!' : ' ';

  while (*input != separator) {
    if (*input < first_printable || *input > '~' || *input == '|' || count == limit) {
      return NULL;
    }
    output[count++] = *input++;
  }
  return count ? input + 1 : NULL;
}

static void handle_frame(void) {
  const char *next;

  if (invalid_frame || strncmp(frame, "ADD ", 4) != 0) {
    uart_write("ERR FORMAT\r\n");
    return;
  }
  memset(&incoming_word, 0, sizeof(incoming_word));
  next = read_field(frame + 4, incoming_word.spelling, VOCAB_WORD_LEN, '|');
  if (next) {
    next = read_field(next, incoming_word.part, VOCAB_PART_LEN, '|');
  }
  if (next) {
    next = read_field(next, incoming_word.meaning, VOCAB_MEANING_LEN, '\0');
  }
  if (!next) {
    uart_write("ERR FORMAT\r\n");
    return;
  }
  switch (vocab_add(&incoming_word)) {
  case VOCAB_ADDED:
    uart_write("OK\r\n");
    break;
  case VOCAB_DUPLICATE:
    uart_write("ERR DUP\r\n");
    break;
  case VOCAB_FULL:
    uart_write("ERR FULL\r\n");
    break;
  case VOCAB_SAVE_FAILED:
    uart_write("ERR SAVE\r\n");
    break;
  }
}

void serial_vocab_poll(void) {
  int byte = uart_read();

  if (byte == UART_RX_EMPTY) {
    return;
  }
  if (byte == UART_RX_OVERFLOW) {
    length = 0;
    invalid_frame = false;
    uart_write("ERR RX\r\n");
  } else if (byte == '\n') {
    if (length && frame[length - 1u] == '\r') {
      --length;
    }
    frame[length] = '\0';
    handle_frame();
    length = 0;
    invalid_frame = false;
  } else if (byte == '\0' || length == sizeof(frame) - 1u) {
    invalid_frame = true;
  } else if (!invalid_frame) {
    frame[length++] = byte;
  }
}
