#ifndef EEPROM_RECORD_H
#define EEPROM_RECORD_H

#include "iap_eeprom.h"

#define EEPROM_RECORD_OVERHEAD 4u

bool eeprom_record_load(uint16_t address, uint8_t *payload, uint8_t length);
bool eeprom_record_save(uint16_t address, const uint8_t *payload, uint8_t length);

#endif
