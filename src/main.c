#include "key.h"
#include "lcd1602.h"
#include "timer.h"

void main(void) {
  static const __code char labels[][8] = {
    "K1", "K2", "K3", "K1 long", "K2 long", "K3 long"
  };
  KeyEvent event;

  key_init();
  timer_init();
  lcd_init_4bit();
  lcd_print(1, "Word Tester v1.0");
  while (1) {
    event = key_next();
    if (event != KEY_NONE) {
      lcd_print(2, labels[event - KEY_K1]);
    }
  }
}
