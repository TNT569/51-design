#include "common.h"
#include "lcd.h"
#include "reg52.h"
#include "timer.h"
#include <8052.h>
void lcd_write(Data_mod mode, unsigned char command, unsigned int delay) {
  if (mode == DATA) {
    lcd_RS = 1;
  } else {
    lcd_RS = 0;
  }
  lcd &= 0xF0;
  lcd |= command / 16;
  lcd_E = 1;
  delay_ms(1);
  lcd_E = 0;
  lcd &= 0xF0;
  lcd |= command % 16;
  lcd_E = 1;
  delay_ms(1);
  lcd_E = 0;
  delay_ms(delay);
}
void lcd_init_4bit(void) {
  // 1. 上电后等待 > 15ms，确保LCD内部复位完成
  delay_ms(15); // 或更大值，如20ms

  // 2. 发送0x33，然后必须等待 > 4.1ms
  lcd_write(COMMAND, 0x33, 5); // 参数5代表5ms，满足要求
  delay_ms(1);                 // 函数内部可能已延时，但额外等待更保险

  // 3. 再次发送0x33，等待 > 100us (1ms足够)
  lcd_write(COMMAND, 0x33, 1);

  // 4. 第三次发送0x33 (部分资料建议，可提高稳定性)
  lcd_write(COMMAND, 0x33, 1);

  // 5. 发送0x32，切换到4位模式
  lcd_write(COMMAND, 0x32, 1);

  // 6. 后续配置命令
  lcd_write(COMMAND, 0x28, 1); // 4位模式，2行显示，5x8点阵
  lcd_write(COMMAND, 0x0C, 1); // 显示开，光标关
  lcd_write(COMMAND, 0x01, 2); // 清屏，需要较长延时(>1.52ms)
  lcd_write(COMMAND, 0x06, 1); // 入口模式：光标右移，屏幕不移动
}
void lcd_print(unsigned char c, char *string) {
  if (c == 1)
    lcd_write(COMMAND, 0x80, 1);
  else
    lcd_write(COMMAND, 0xC0, 1);
  while (*string) {
    lcd_write(DATA, *string, 1);
    string++;
  }
}
