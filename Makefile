SDCC ?= sdcc
BUILD_DIR := build
TARGET := $(BUILD_DIR)/main
DEVICE_FLAGS := -mmcs51 --model-small --iram-size 256 --xram-size 0 --code-size 8192
SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard inc/*.h)
OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/%.rel,$(SOURCES))

.PHONY: all clean
all: $(TARGET).hex

$(TARGET).hex: $(OBJECTS)
	$(SDCC) $(DEVICE_FLAGS) --out-fmt-ihx -o $@ $^

$(BUILD_DIR)/%.rel: src/%.c $(HEADERS) Makefile | $(BUILD_DIR)
	$(SDCC) $(DEVICE_FLAGS) --std-c99 --opt-code-size -Iinc -c -o $@ $<

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
else
	rm -rf -- $(BUILD_DIR)
endif
