SDCC ?= sdcc
BUILD_DIR := build
TARGET := $(BUILD_DIR)/main
DEVICE_FLAGS := -mmcs51 --model-small --iram-size 256 --xram-loc 0 --xram-size 256 --code-size 8192
SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard inc/*.h)
OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/%.rel,$(SOURCES))

.PHONY: all clean
all: $(TARGET).hex

$(TARGET).hex: $(OBJECTS)
	$(SDCC) $(DEVICE_FLAGS) --out-fmt-ihx -o $@ $^

$(BUILD_DIR)/%.rel: src/%.c $(HEADERS) Makefile | $(BUILD_DIR)
	$(SDCC) $(DEVICE_FLAGS) --std-c99 --opt-code-size -Iinc -c -o $@ $<

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
