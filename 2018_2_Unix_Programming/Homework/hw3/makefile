TARGET := libmini.so
LIB_C := libmini.o
LIB_ASM := libmini64.o
LIB_ENTRY := start.o
SHELL := /bin/bash
CC := gcc
ASM := yasm
LD := ld
CFLAGS_LIB := -c -g -Wall -fno-stack-protector -fPIC -nostdlib
ASMFLAGS_LIB := -f elf64 -DYASM -D__x86_64__ -DPIC
LDFLAGS_LIB := -shared
CFLAGS_EXEC := -c -g -Wall -fno-stack-protector -nostdlib -I. -DUSEMINI
LDFLAGS_EXEC := -m elf_x86_64 --dynamic-linker /lib64/ld-linux-x86-64.so.2 -L. -lmini
TESTCASES := alarm1 alarm2 alarm3 jmp1 jmp2

.PHONY: all
all: $(TARGET) $(LIB_ENTRY)

%.o: %.asm
	$(ASM) $(ASMFLAGS_LIB) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS_LIB) $< -o $@

%: %.c all
	$(CC) $(CFLAGS_EXEC) $< -o $(basename $<).o
	$(LD) $(LDFLAGS_EXEC) $(basename $<).o $(LIB_ENTRY) -o $@

$(TARGET): $(LIB_C) $(LIB_ASM)
	$(LD) $(LDFLAGS_LIB) $^ -o $@

.PHONY: test
test: $(TESTCASES) all
	@for t in $(TESTCASES); do echo "test $$t: LD_LIBRARY_PATH=. ./$$t"; done

.PHONY: clean
clean:
	rm -f *.o *.so $(TESTCASES)
