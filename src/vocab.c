#include "vocab.h"

#include "error_log.h"

static const __code Word builtins[] = {
  {"abandon", "v.", "give up"},
  {"accurate", "a.", "correct"},
  {"benefit", "n.", "gain"},
  {"brief", "a.", "short"},
  {"contrast", "v.", "compare"},
  {"eager", "a.", "keen"},
  {"fragile", "a.", "brittle"},
  {"habit", "n.", "routine"},
  {"journey", "n.", "trip"},
  {"observe", "v.", "watch"},
  {"retain", "v.", "keep"},
  {"vivid", "a.", "bright"}
};

#define BUILTIN_COUNT (sizeof(builtins) / sizeof(builtins[0]))
#define BITMAP_BYTES ((BUILTIN_COUNT + 7) / 8)

static unsigned char misses[BITMAP_BYTES];
static __idata unsigned char selection[BUILTIN_COUNT];
static unsigned char total;
static unsigned char answered;
static unsigned char correct;

static unsigned char word_mask(unsigned char index) {
  return 1u << (index & 7u);
}

bool quiz_start(bool errors_only) {
  unsigned char id;

  total = 0;
  for (id = 0; id < BUILTIN_COUNT; ++id) {
    if (!errors_only || error_log_get(id)) {
      selection[total++] = id;
    }
  }
  quiz_restart();
  return total != 0;
}

void quiz_restart(void) {
  unsigned char i;

  answered = 0;
  correct = 0;
  for (i = 0; i < BITMAP_BYTES; ++i) {
    misses[i] = 0;
  }
}

void quiz_read(unsigned char position, Word *word) {
  *word = builtins[selection[position]];
}

bool quiz_answer(bool known) {
  unsigned char id;

  if (answered >= total) {
    return false;
  }
  id = selection[answered];
  if (!error_log_set(id, !known)) {
    return false;
  }
  if (known) {
    ++correct;
  } else {
    misses[answered >> 3] |= word_mask(answered);
  }
  ++answered;
  return true;
}

bool quiz_missed(unsigned char position) {
  return position < answered && (misses[position >> 3] & word_mask(position));
}

unsigned char quiz_total(void) {
  return total;
}

unsigned char quiz_answered(void) {
  return answered;
}

unsigned char quiz_known(void) {
  return correct;
}

unsigned char quiz_accuracy(void) {
  return answered ? (correct * 100u) / answered : 0;
}
