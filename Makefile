CC = gcc
CFLAGS = -Wall -Wextra -O2 -c -fms-extensions -Wno-microsoft-anon-tag -Wno-incompatible-pointer-types
SRC = src
BUILD = build

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c, $(BUILD)/%.o,$(SRCS))

all: $(BUILD)/main

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	bear -- $(CC) $(CFLAGS) $(SRC)/$*.c -o $(BUILD)/$*.o

$(BUILD)/main: $(OBJS)
	$(CC) $(OBJS) -o $@  

$(BUILD):
	mkdir -p $(BUILD)

run: $(BUILD)/main
	./$(BUILD)/main

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
