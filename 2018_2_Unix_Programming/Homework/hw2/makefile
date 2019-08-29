TARGET := fsmon.so
SRC := hw2.c
SHELL := /bin/bash
CC := gcc
CFLAGS_LIB := -Wall -shared -fPIC -ldl
CFLAGS_EXEC := -Wall
TESTCASES := dir file fscanf getline stdio

.PHONY: all
all: $(TARGET)

%: %.c
	$(CC) $< $(CFLAGS_EXEC) -o $@

$(TARGET):$(SRC)
	$(CC) $< $(CFLAGS_LIB) -o $@

.PHONY: test
test: $(TARGET) $(TESTCASES)
	@echo "test with log: MONITOR_OUTPUT=xxx.log"
	@for t in $(TESTCASES); do echo "test $$t: LD_PRELOAD=./$(TARGET) ./$$t"; done
	@echo "test wget: wget -O /dev/null google.com > /dev/null"
	@echo "test cat: LD_PRELOAD=./$(TARGET) cat /etc/passwd > /dev/null"
	@echo "test ls: LD_PRELOAD=./$(TARGET) ls -la /dev > /dev/null"

.PHONY: clean
clean:
	rm -f $(TARGET) $(TESTCASES) *.log
