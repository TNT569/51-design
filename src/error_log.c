#include "error_log.h"

#include <string.h>

#include "eeprom_record.h"

#define BITMAP_BYTES (ERROR_LOG_CAPACITY / 8u)
#define RECORD_SIZE 16u
#define SLOTS_PER_SECTOR (EEPROM_SECTOR_SIZE / RECORD_SIZE)
#define PAYLOAD_BYTES (BITMAP_BYTES + 2u)

static __idata uint8_t errors[BITMAP_BYTES];
static __idata uint8_t payload[PAYLOAD_BYTES];
static uint8_t used[2];
static uint8_t active_sector;
static uint16_t generation;

static uint16_t sector_address(uint8_t sector) {
  return EEPROM_BASE + sector * EEPROM_SECTOR_SIZE;
}

static uint16_t slot_address(uint8_t sector, uint8_t slot) {
  return sector_address(sector) + slot * RECORD_SIZE;
}

void error_log_init(void) {
  uint8_t sector, slot;
  bool found = false;

  memset(errors, 0, sizeof(errors));
  memset(used, 0, sizeof(used));
  active_sector = 0;
  generation = 0;
  for (sector = 0; sector < 2; ++sector) {
    for (slot = 0; slot < SLOTS_PER_SECTOR; ++slot) {
      uint16_t address = slot_address(sector, slot);
      uint16_t sequence, delta;

      if (eeprom_read(address) == EEPROM_ERASED) {
        continue;
      }
      used[sector] = slot + 1;
      if (!eeprom_record_load(address, payload, sizeof(payload))) {
        continue;
      }
      sequence = payload[0] | ((uint16_t)payload[1] << 8);
      delta = sequence - generation;
      if (!found || (delta != 0 && delta < 0x8000u)) {
        generation = sequence;
        active_sector = sector;
        memcpy(errors, payload + 2, sizeof(errors));
        found = true;
      }
    }
  }
}

bool error_log_get(uint8_t id) {
  return id < ERROR_LOG_CAPACITY && (errors[id >> 3] & (1u << (id & 7u)));
}

bool error_log_set(uint8_t id, bool missed) {
  uint8_t sector = active_sector;
  uint8_t slot = used[sector];
  uint8_t mask = 1u << (id & 7u);
  uint16_t next = generation + 1u;

  if (id >= ERROR_LOG_CAPACITY) {
    return false;
  }
  if (error_log_get(id) == missed) {
    return true;
  }
  if (slot == SLOTS_PER_SECTOR) {
    sector ^= 1;
    /* Keep the newest committed snapshot while recycling the other sector. */
    if (!eeprom_erase(sector_address(sector))) {
      return false;
    }
    slot = 0;
  }
  payload[0] = next;
  payload[1] = next >> 8;
  memcpy(payload + 2, errors, sizeof(errors));
  payload[2 + (id >> 3)] ^= mask;
  used[sector] = slot + 1;
  if (!eeprom_record_save(slot_address(sector, slot), payload, sizeof(payload))) {
    return false;
  }
  memcpy(errors, payload + 2, sizeof(errors));
  active_sector = sector;
  generation = next;
  return true;
}
