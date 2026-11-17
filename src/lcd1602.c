#include "lcd1602.h"

#include "common.h"
#include "reg52.h"
#include "timer.h"

static void write_nibble(unsigned char nibble) {
  LCD_PORT = (LCD_PORT & 0xf0) | nibble;
  LCD_E = 1;
  __asm nop __endasm;
  LCD_E = 0;
}

static void write_byte(unsigned char value) {
  write_nibble(value >> 4);
  write_nibble(value & 0x0f);
  delay_us(50);
}

void lcd_init_4bit(void) {
  LCD_E = 0;
  LCD_RW = 0;
  LCD_RS = 0;
  delay_ms(40);

  /* Synchronize even if the MCU resets while the LCD stays powered. */
  write_nibble(0x03);
  delay_ms(5);
  write_nibble(0x03);
  delay_us(150);
  write_nibble(0x03);
  delay_us(150);
  write_nibble(0x02);
  delay_us(50);

  write_byte(0x28);
  write_byte(0x08);
  write_byte(0x01);
  delay_ms(2);
  write_byte(0x06);
  write_byte(0x0c);
}

void lcd_print(unsigned char row, const char *text) {
  unsigned char column;

  LCD_RS = 0;
  write_byte(row == 1 ? 0x80 : 0xc0);
  LCD_RS = 1;
  for (column = 0; column < LCD_COLUMNS; ++column) {
    write_byte(*text ? *text++ : ' ');
  }
}
