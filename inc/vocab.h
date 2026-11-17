#ifndef VOCAB_H
#define VOCAB_H

#include <stdbool.h>

#define VOCAB_WORD_LEN 10
#define VOCAB_PART_LEN 3
#define VOCAB_MEANING_LEN 7

typedef struct {
  char spelling[VOCAB_WORD_LEN + 1];
  char part[VOCAB_PART_LEN + 1];
  char meaning[VOCAB_MEANING_LEN + 1];
} Word;

bool quiz_start(bool errors_only);
void quiz_restart(void);
void quiz_read(unsigned char position, Word *word);
bool quiz_answer(bool known);
bool quiz_missed(unsigned char position);
unsigned char quiz_total(void);
unsigned char quiz_answered(void);
unsigned char quiz_known(void);
unsigned char quiz_accuracy(void);

#endif
