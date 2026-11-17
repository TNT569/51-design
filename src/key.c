#include "key.h"

#include "common.h"
#include "reg52.h"

#define DEBOUNCE_TICKS 2u
#define LONG_PRESS_TICKS 80u
#define EVENT_QUEUE_SIZE 8u
#define EVENT_QUEUE_MASK (EVENT_QUEUE_SIZE - 1u)

static unsigned char stable;
static unsigned char changes[3];
static unsigned char held[3];
static volatile unsigned char events[EVENT_QUEUE_SIZE];
static volatile unsigned char head;
static volatile unsigned char tail;

void key_init(void) {
  KEY_PORT |= KEY_MASK;
}

static void enqueue(KeyEvent event) {
  unsigned char next = (head + 1u) & EVENT_QUEUE_MASK;

  if (next != tail) {
    events[head] = event;
    head = next;
  }
}

void key_scan(void) {
  unsigned char pressed = ~KEY_PORT & KEY_MASK;
  unsigned char mask = 1;
  unsigned char i;

  for (i = 0; i < 3; ++i, mask <<= 1) {
    if ((pressed ^ stable) & mask) {
      if (++changes[i] == DEBOUNCE_TICKS) {
        changes[i] = 0;
        stable ^= mask;
        if (pressed & mask) {
          held[i] = 0;
        } else if (held[i] < LONG_PRESS_TICKS) {
          enqueue(KEY_K1 + i);
        }
      }
    } else {
      changes[i] = 0;
      if ((stable & mask) && held[i] < LONG_PRESS_TICKS) {
        if (++held[i] == LONG_PRESS_TICKS) {
          enqueue(KEY_K1_LONG + i);
        }
      }
    }
  }
}

KeyEvent key_next(void) {
  KeyEvent event;

  if (tail == head) {
    return KEY_NONE;
  }
  event = events[tail];
  tail = (tail + 1u) & EVENT_QUEUE_MASK;
  return event;
}
