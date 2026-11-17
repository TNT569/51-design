/**
 * CPU: 89C52
 * Freq: 12MHz
 */

#include "lcd.h"
#include <reg52.h>
#include <stdio.h>

void main(void) {
  lcd_init_4bit();
  lcd_print(1, "Hello, World!");
  while (1) {
  }
}
