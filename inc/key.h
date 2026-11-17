#ifndef KEY_H
#define KEY_H

typedef enum {
  KEY_NONE,
  KEY_K1,
  KEY_K2,
  KEY_K3,
  KEY_K1_LONG,
  KEY_K2_LONG,
  KEY_K3_LONG
} KeyEvent;

void key_init(void);
void key_scan(void);
KeyEvent key_next(void);

#endif
