# Makefile for the simple kernel.
CC	=gcc
AS	=as
LD	=ld
OBJCOPY = objcopy
OBJDUMP = objdump
NM = nm

LDFLAGS = -m elf_i386

CFLAGS = -m32 -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -fno-stack-protector

# Add debug symbol
CFLAGS += -g

CFLAGS += -I.

OBJDIR = .


CPUS ?= 1

all: boot/boot kernel/system
	dd if=/dev/zero of=$(OBJDIR)/kernel.img count=10000 2>/dev/null
	dd if=$(OBJDIR)/boot/boot of=$(OBJDIR)/kernel.img conv=notrunc 2>/dev/null
	dd if=$(OBJDIR)/kernel/system of=$(OBJDIR)/kernel.img seek=1 conv=notrunc 2>/dev/null

include boot/Makefile
include kernel/Makefile

clean:
	rm -rf $(OBJDIR)/boot/*.o $(OBJDIR)/boot/boot.out $(OBJDIR)/boot/boot $(OBJDIR)/boot/boot.asm
	rm -rf $(OBJDIR)/kernel/*.o $(OBJDIR)/kernel/system* kernel.*
	rm -rf $(OBJDIR)/lib/*.o
	rm -rf $(OBJDIR)/user/*.o
	rm -rf $(OBJDIR)/user/*.asm
	rm -rf $(OBJDIR)/kernel/fs/*.o $(OBJDIR)/kernel/fs/fat/*.o
	rm -rf $(OBJDIR)/kernel/drv/*.o

lab7img:
	rm -rf lab7.img
	qemu-img create -f raw lab7.img 32M

qemu:
	qemu-system-i386 -hda kernel.img -hdb lab7.img -smp $(CPUS)

debug:
	qemu-system-i386 -hda kernel.img -hdb lab7.img -s -S -smp $(CPUS)
