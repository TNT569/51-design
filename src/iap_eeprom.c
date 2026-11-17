#include "iap_eeprom.h"

#include "reg52.h"

__sfr __at(0xe2) IAP_DATA;
__sfr __at(0xe3) IAP_ADDRH;
__sfr __at(0xe4) IAP_ADDRL;
__sfr __at(0xe5) IAP_CMD;
__sfr __at(0xe6) IAP_TRIG;
__sfr __at(0xe7) IAP_CONTR;

#define IAP_READ 1u
#define IAP_PROGRAM 2u
#define IAP_ERASE 3u
/* STC89C51RC/RD+ datasheet, chapter 9: enable with WT=1 below 20 MHz. */
#define IAP_ENABLE 0x81u

static uint8_t iap_command(uint8_t command, uint16_t address, uint8_t value) {
  uint8_t result;

  __critical {
    IAP_CONTR = IAP_ENABLE;
    IAP_CMD = command;
    IAP_ADDRH = address >> 8;
    IAP_ADDRL = address;
    IAP_DATA = value;
    IAP_TRIG = 0x46;
    IAP_TRIG = 0xb9;
    __asm nop __endasm;
    result = IAP_DATA;
    IAP_CONTR = 0;
    IAP_CMD = 0;
    IAP_TRIG = 0;
    IAP_ADDRH = 0;
    IAP_ADDRL = 0;
  }
  return result;
}

uint8_t eeprom_read(uint16_t address) {
  if (address < EEPROM_BASE || address >= EEPROM_END) {
    return EEPROM_ERASED;
  }
  return iap_command(IAP_READ, address, EEPROM_ERASED);
}

bool eeprom_program(uint16_t address, uint8_t value) {
  if (address < EEPROM_BASE || address >= EEPROM_END ||
      eeprom_read(address) != EEPROM_ERASED) {
    return false;
  }
  if (value != EEPROM_ERASED) {
    iap_command(IAP_PROGRAM, address, value);
  }
  return eeprom_read(address) == value;
}

bool eeprom_erase(uint16_t address) {
  uint16_t offset;

  if (address < EEPROM_BASE || address >= EEPROM_END ||
      (address & (EEPROM_SECTOR_SIZE - 1u))) {
    return false;
  }
  iap_command(IAP_ERASE, address, EEPROM_ERASED);
  for (offset = 0; offset < EEPROM_SECTOR_SIZE; ++offset) {
    if (eeprom_read(address + offset) != EEPROM_ERASED) {
      return false;
    }
  }
  return true;
}
