#ifndef LCD1602_H
#define LCD1602_H

#define LCD_COLUMNS 16

void lcd_init_4bit(void);
void lcd_print(unsigned char row, const char *text);

#endif
