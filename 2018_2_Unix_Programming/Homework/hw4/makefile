TARGET_MAIN := sdb
TARGET_TEST := sdb_test
OBJ_MAIN := sdb_main.o
OBJ_TEST := sdb_test.o
OBJS := sdb.o elftool.o
SHELL := /bin/bash
CC := gcc
CXX := g++
CFLAGS := -c -Wall
LDFLAGS := -lelf -lcapstone
TESTLDFLAGS := -lgtest -pthread

.PHONY: all
all: $(TARGET_MAIN)

%.o: %.c
	$(CC) $< $(CFLAGS) -o $@

%.o: %.cc
	$(CC) $< $(CFLAGS) -o $@

$(TARGET_MAIN): $(OBJ_MAIN) $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

$(TARGET_TEST): $(OBJ_TEST) $(OBJS)
	$(CXX) $^ $(LDFLAGS) $(TESTLDFLAGS) -o $@

elftool.o: elftool.h
sdb.o: sdb.h elftool.h
$(OBJ_MAIN): sdb.h
$(OBJ_TEST): sdb.h

.PHONY: test
test: $(TARGET_TEST)
	@./$(TARGET_TEST)

.PHONY: clean
clean:
	rm -f $(TARGET_MAIN) $(TARGET_TEST) *.o
