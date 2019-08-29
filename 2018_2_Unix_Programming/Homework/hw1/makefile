TARGET := hw1
SRC := main.c
SHELL := /bin/bash
CC := gcc
CFLAGS := -Wall

.PHONY: all
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: test
test: $(TARGET)
	@echo "test1 default: ./$<"
	@echo "test2 tdp    : ./$< -t"
	@echo "test3 udp    : ./$< -u"
	@echo "test4 tcp udp: ./$< -tu"
	@echo "test5 unknown: ./$< -tup"
	@echo "test6 string : ./$< p6"

.PHONY: clean
clean:
	rm -f $(TARGET)
