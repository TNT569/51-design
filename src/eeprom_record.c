#include "eeprom_record.h"

#define RESERVED 0x00u
#define COMMITTED 0xa5u

/* Layout: reservation, payload, little-endian CRC-16, commit marker. */
static uint16_t record_crc(const uint8_t *payload, uint8_t length) {
  uint16_t crc = 0xffffu;
  uint8_t i;

  while (length--) {
    crc ^= *payload++;
    for (i = 0; i < 8; ++i) {
      if (crc & 1u) {
        crc = (crc >> 1) ^ 0xa001u;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

bool eeprom_record_load(uint16_t address, uint8_t *payload, uint8_t length) {
  uint16_t tail = address + 1u + length;
  uint16_t crc;
  uint8_t i;

  if (eeprom_read(address) != RESERVED || eeprom_read(tail + 2u) != COMMITTED) {
    return false;
  }
  for (i = 0; i < length; ++i) {
    payload[i] = eeprom_read(address + 1u + i);
  }
  crc = record_crc(payload, length);
  return eeprom_read(tail) == (uint8_t)crc &&
         eeprom_read(tail + 1u) == (uint8_t)(crc >> 8);
}

bool eeprom_record_save(uint16_t address, const uint8_t *payload, uint8_t length) {
  uint16_t tail = address + 1u + length;
  uint16_t crc = record_crc(payload, length);
  uint8_t i;

  /* A failed write occupies its slot until its sector can be erased. */
  if (!eeprom_program(address, RESERVED)) {
    return false;
  }
  for (i = 0; i < length; ++i) {
    if (!eeprom_program(address + 1u + i, payload[i])) {
      return false;
    }
  }
  return eeprom_program(tail, (uint8_t)crc) &&
         eeprom_program(tail + 1u, (uint8_t)(crc >> 8)) &&
         eeprom_program(tail + 2u, COMMITTED);
}
