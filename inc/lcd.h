#ifndef LCD_H
#define LCD_H
typedef enum { DATA, COMMAND } Data_mod;

void lcd_write(Data_mod mode, unsigned char command, unsigned int delay);
void lcd_init_4bit(void);
void lcd_print(unsigned char c, char *string);
#endif
