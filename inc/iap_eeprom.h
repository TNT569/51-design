#ifndef IAP_EEPROM_H
#define IAP_EEPROM_H

#include <stdbool.h>
#include <stdint.h>

#define EEPROM_BASE 0x2000u
#define EEPROM_END 0x3000u
#define EEPROM_SECTOR_SIZE 512u
#define EEPROM_ERASED 0xffu

uint8_t eeprom_read(uint16_t address);
bool eeprom_program(uint16_t address, uint8_t value);
bool eeprom_erase(uint16_t address);

#endif
