CC = gcc
CFLAGS = -Wall -Wextra -O0 -c -I./include/ -std=gnu23 -MMD -MP -ggdb3

SRC = src
BUILD = build

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

all: $(BUILD)/main

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	bear -- $(CC) $(CFLAGS) $< -o $@

$(BUILD)/main: $(OBJS)
	$(CC) $(OBJS) -o $@

$(BUILD):
	mkdir -p $@

-include $(DEPS)

run: $(BUILD)/main
	./$(BUILD)/main $(ARGS)

run-gf2: $(BUILD)/main
	gf2 ./$(BUILD)/main $(ARGS)

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
