#include "key.h"
#include "lcd1602.h"
#include "timer.h"
#include "vocab.h"

typedef enum {
  STATE_HOME,
  STATE_TEST,
  STATE_BROWSE,
  STATE_RESULT,
  STATE_REVIEW
} State;

static State state;
static unsigned char browse_position;
static unsigned char review_position;
static Word visible_word;
static char line[LCD_COLUMNS + 1];

static void clear_line(void) {
  unsigned char i;

  for (i = 0; i < LCD_COLUMNS; ++i) {
    line[i] = ' ';
  }
  line[LCD_COLUMNS] = '\0';
}

static void put_text(unsigned char col, const char *text) {
  while (col < LCD_COLUMNS && *text) {
    line[col++] = *text++;
  }
}

static void put_number(unsigned char col, unsigned char value) {
  line[col] = '0' + value / 10u;
  line[col + 1] = '0' + value % 10u;
}

static void draw_word(unsigned char position, bool show_meaning) {
  quiz_read(position, &visible_word);
  clear_line();
  put_number(0, position + 1);
  line[2] = '/';
  put_number(3, quiz_total());
  put_text(6, visible_word.spelling);
  lcd_print(1, line);

  clear_line();
  if (show_meaning) {
    put_text(0, visible_word.meaning);
    put_text(7, "[<][>][M]");
  } else {
    put_text(0, visible_word.part);
    put_text(7, "[Y][N][M]");
  }
  lcd_print(2, line);
}

static void draw_result(void) {
  unsigned char percent = quiz_accuracy();

  clear_line();
  put_text(0, "Total:");
  put_number(6, quiz_answered());
  put_text(8, " Know:");
  put_number(14, quiz_known());
  lcd_print(1, line);

  clear_line();
  put_text(0, "Acc:");
  if (percent == 100) {
    put_text(4, "100");
  } else {
    put_number(5, percent);
    if (percent < 10) {
      line[5] = ' ';
    }
  }
  line[7] = '%';
  put_text(11, "[RST]");
  lcd_print(2, line);
}

static void draw_screen(void) {
  switch (state) {
  case STATE_HOME:
    lcd_print(1, "Word Tester v1.0");
    lcd_print(2, "[ALL] [ERR] [GO]");
    break;
  case STATE_TEST:
    draw_word(quiz_answered(), false);
    break;
  case STATE_BROWSE:
    draw_word(browse_position, true);
    break;
  case STATE_RESULT:
    draw_result();
    break;
  case STATE_REVIEW:
    draw_word(review_position, true);
    break;
  }
}

static void find_miss(signed char position, signed char step) {
  signed char limit = quiz_answered();

  while (position >= 0 && position < limit) {
    if (quiz_missed(position)) {
      review_position = position;
      state = STATE_REVIEW;
      return;
    }
    position += step;
  }
}

static void handle_key(KeyEvent event) {
  switch (state) {
  case STATE_HOME:
    if (event == KEY_K1 || event == KEY_K2 || event == KEY_K3) {
      if (!quiz_start(event == KEY_K2)) {
        lcd_print(1, "No wrong words");
        return;
      }
      state = STATE_TEST;
    }
    break;
  case STATE_TEST:
    if (event == KEY_K1 || event == KEY_K2) {
      quiz_answer(event == KEY_K1);
      if (quiz_answered() == quiz_total()) {
        state = STATE_RESULT;
      }
    } else if (event == KEY_K3) {
      browse_position = quiz_answered();
      state = STATE_BROWSE;
    } else if (event == KEY_K3_LONG) {
      state = STATE_RESULT;
    }
    break;
  case STATE_BROWSE:
    if (event == KEY_K1 && browse_position > 0) {
      --browse_position;
    } else if (event == KEY_K2 && browse_position + 1 < quiz_total()) {
      ++browse_position;
    } else if (event == KEY_K3) {
      state = STATE_TEST;
    }
    break;
  case STATE_RESULT:
    if (event == KEY_K1) {
      find_miss((signed char)quiz_answered() - 1, -1);
    } else if (event == KEY_K2) {
      find_miss(0, 1);
    } else if (event == KEY_K3) {
      quiz_restart();
      state = STATE_TEST;
    } else if (event == KEY_K3_LONG) {
      state = STATE_HOME;
    }
    break;
  case STATE_REVIEW:
    if (event == KEY_K1) {
      find_miss((signed char)review_position - 1, -1);
    } else if (event == KEY_K2) {
      find_miss(review_position + 1, 1);
    } else if (event == KEY_K3) {
      state = STATE_RESULT;
    } else if (event == KEY_K3_LONG) {
      state = STATE_HOME;
    }
    break;
  }
  draw_screen();
}

void main(void) {
  KeyEvent event;

  key_init();
  timer_init();
  lcd_init_4bit();
  draw_screen();
  while (1) {
    event = key_next();
    if (event != KEY_NONE) {
      handle_key(event);
    }
  }
}
