#ifndef ERROR_LOG_H
#define ERROR_LOG_H

#include <stdbool.h>
#include <stdint.h>

#define ERROR_LOG_CAPACITY 48u

void error_log_init(void);
bool error_log_get(uint8_t id);
bool error_log_set(uint8_t id, bool missed);

#endif
