SDCC ?= sdcc
BUILD_DIR := build
TARGET := $(BUILD_DIR)/main
SOURCES := $(wildcard src/*.c)
XRAM_SIZE := 256
ifeq ($(SIMULATOR),1)
XRAM_SIZE := 0
CPPFLAGS += -DWORD_TESTER_SIMULATOR
SOURCES := $(filter-out src/eeprom_record.c src/iap_eeprom.c src/serial_vocab.c src/uart.c,$(SOURCES))
endif
DEVICE_FLAGS := -mmcs51 --model-small --iram-size 256 --xram-loc 0 --xram-size $(XRAM_SIZE) --code-size 8192
HEADERS := $(wildcard inc/*.h)
OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/%.rel,$(SOURCES))

.PHONY: all sim clean
all: $(TARGET).hex

sim:
	$(MAKE) BUILD_DIR=$(BUILD_DIR)/sim SIMULATOR=1 all

$(TARGET).hex: $(OBJECTS)
	$(SDCC) $(DEVICE_FLAGS) --out-fmt-ihx -o $@ $^

$(BUILD_DIR)/%.rel: src/%.c $(HEADERS) Makefile | $(BUILD_DIR)
	$(SDCC) $(DEVICE_FLAGS) $(CPPFLAGS) --std-c99 --opt-code-size -Iinc -c -o $@ $<

ifeq ($(OS),Windows_NT)
WINDOWS_BUILD_DIR := $(subst /,\,$(BUILD_DIR))

$(BUILD_DIR):
	@if not exist "$(WINDOWS_BUILD_DIR)" mkdir "$(WINDOWS_BUILD_DIR)"

clean:
	@if exist "$(WINDOWS_BUILD_DIR)" rmdir /s /q "$(WINDOWS_BUILD_DIR)"
else
$(BUILD_DIR):
	mkdir -p -- "$(BUILD_DIR)"

clean:
	rm -rf -- "$(BUILD_DIR)"
endif
