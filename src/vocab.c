#include "vocab.h"

#include <string.h>

#include "eeprom_record.h"
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
#define WORD_STORE_BASE (EEPROM_BASE + 2u * EEPROM_SECTOR_SIZE)
#define WORD_SLOT_SIZE 32u
#define WORD_SLOTS (2u * EEPROM_SECTOR_SIZE / WORD_SLOT_SIZE)
#define VOCAB_CAPACITY (BUILTIN_COUNT + WORD_SLOTS)
#define BITMAP_BYTES ((VOCAB_CAPACITY + 7u) / 8u)

typedef char WordFitsSlot[(sizeof(Word) + EEPROM_RECORD_OVERHEAD <= WORD_SLOT_SIZE) ? 1 : -1];
typedef char ErrorsFitBitmap[(VOCAB_CAPACITY <= ERROR_LOG_CAPACITY) ? 1 : -1];

static __xdata Word existing_word;
static __idata unsigned char valid_slots[WORD_SLOTS / 8u];
static unsigned char next_slot;
static __idata unsigned char misses[BITMAP_BYTES];
static __xdata unsigned char selection[VOCAB_CAPACITY];
static unsigned char total;
static unsigned char answered;
static unsigned char correct;

static unsigned char word_mask(unsigned char index) {
  return 1u << (index & 7u);
}

static uint16_t word_address(unsigned char slot) {
  return WORD_STORE_BASE + slot * WORD_SLOT_SIZE;
}

static bool read_slot(unsigned char slot, Word *word) {
  return eeprom_record_load(word_address(slot), (uint8_t *)word, sizeof(*word)) &&
         word->spelling[VOCAB_WORD_LEN] == '\0' && word->part[VOCAB_PART_LEN] == '\0' &&
         word->meaning[VOCAB_MEANING_LEN] == '\0';
}

void vocab_init(void) {
  unsigned char slot;

  memset(valid_slots, 0, sizeof(valid_slots));
  next_slot = 0;
  for (slot = 0; slot < WORD_SLOTS; ++slot) {
    if (eeprom_read(word_address(slot)) != EEPROM_ERASED) {
      next_slot = slot + 1;
      if (read_slot(slot, &existing_word)) {
        valid_slots[slot >> 3] |= word_mask(slot);
      }
    }
  }
}

static bool has_word(unsigned char id) {
  unsigned char slot;

  if (id < BUILTIN_COUNT) {
    return true;
  }
  slot = id - BUILTIN_COUNT;
  return valid_slots[slot >> 3] & word_mask(slot);
}

static bool read_word(unsigned char id, Word *word) {
  if (id < BUILTIN_COUNT) {
    *word = builtins[id];
    return true;
  }
  return read_slot(id - BUILTIN_COUNT, word);
}

static char lower_ascii(char letter) {
  return letter >= 'A' && letter <= 'Z' ? letter + ('a' - 'A') : letter;
}

static bool same_spelling(const char *left, const char *right) {
  while (lower_ascii(*left) == lower_ascii(*right)) {
    if (!*left++) {
      return true;
    }
    ++right;
  }
  return false;
}

VocabAddResult vocab_add(const Word *word) {
  unsigned char id;
  unsigned char slot = next_slot;
  uint16_t address;

  for (id = 0; id < BUILTIN_COUNT + next_slot; ++id) {
    if (!has_word(id)) {
      continue;
    }
    if (!read_word(id, &existing_word)) {
      return VOCAB_SAVE_FAILED;
    }
    if (same_spelling(word->spelling, existing_word.spelling)) {
      if (!strcmp(word->part, existing_word.part) && !strcmp(word->meaning, existing_word.meaning)) {
        return VOCAB_ADDED;
      }
      return VOCAB_DUPLICATE;
    }
  }

  if (slot == WORD_SLOTS) {
    return VOCAB_FULL;
  }
  address = word_address(slot);
  if (!eeprom_record_save(address, (const uint8_t *)word, sizeof(*word))) {
    if (eeprom_read(address) != EEPROM_ERASED) {
      ++next_slot;
    }
    return VOCAB_SAVE_FAILED;
  }
  valid_slots[slot >> 3] |= word_mask(slot);
  ++next_slot;
  return VOCAB_ADDED;
}

bool quiz_start(bool errors_only) {
  unsigned char id;

  total = 0;
  for (id = 0; id < BUILTIN_COUNT + next_slot; ++id) {
    if (has_word(id) && (!errors_only || error_log_get(id))) {
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

bool quiz_read(unsigned char position, Word *word) {
  return position < total && read_word(selection[position], word);
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
