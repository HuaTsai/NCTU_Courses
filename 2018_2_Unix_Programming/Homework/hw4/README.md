# Homework 4

* Install elf and capstone library
  * Ubuntu: `sudo apt install libelf-dev libcapstone-dev`
* Implement all of required commands
  * `break`
    * in my implementation, break point should be set on first byte of an instruction, otherwise it won't work
    * accept address specified in elf file in loaded state
    * accept memory layout address in running state
  * `cont`
  * `delete`
  * `disasm`
    * **accept address specified in elf file**
    * **accept address in .text segment**
    * in my implementation, I store all instructions in .text of target file
    * this prevents from being unable to disassemble when break point `0xcc` is set
    * however, it takes more efforts to implement disassembling on other segments
    * I decided to remain current implementation in the end
  * `dump`
    * **accept memory layout address**
  * `exit`
  * `get`
  * `getregs`
  * `help`
  * `list`
  * `load`
  * `run`
  * `vmmap`
  * `set`
    * accept hexdecimal input (need start from `0x` or `0X`)
    * accept decimal input (unsigned 8 bytes integer)
  * `si`
  * `start`
* I added reload program feature whenever the program finishes
* changes made in `elftool.c`
  * add `free(e->dsym->symbol);` in line 85 and line 90
  * prevent from memory leaking
* `makefile`
  * `sdb_test.cc` is my unit test file, not provided in the submission
  * please ignore related tasks, it does not interfere `make` command
* `hello64` and `guess` are provided
* executable name: `sdb`

## Example Tests

### Test1

``` bash
$ ./sdb
sdb> load hello64
** program 'hello64' loaded. entry point 0x4000b0, vaddr 0x4000b0, offset 0xb0, size 0x23
sdb> vmmap
00000000004000b0-00000000004000d3 r-x b0       hello64
sdb> start
** pid 17131
sdb> vmmap
0000000000400000-0000000000401000 r-x 0        /home/ee904/Desktop/HuaTsai/Unix/Homework/hw4/hello64
0000000000600000-0000000000601000 rwx 0        /home/ee904/Desktop/HuaTsai/Unix/Homework/hw4/hello64
00007fff632e3000-00007fff63305000 rwx 0        [stack]
00007fff633db000-00007fff633de000 r-- 0        [vvar]
00007fff633de000-00007fff633e0000 r-x 0        [vdso]
ffffffffff600000-ffffffffff601000 r-x 0        [vsyscall]
sdb> get rip
rip = 4194480 (0x4000b0)
sdb> run
** program 'hello64' is already running.
hello, world!
** child process 17131 terminated normally (code 0)
** reload program
** program 'hello64' loaded. entry point 0x4000b0, vaddr 0x4000b0, offset 0xb0, size 0x23
sdb> exit
```

### Test2

``` bash
$ ./sdb hello64
** program 'hello64' loaded. entry point 0x4000b0, vaddr 0x4000b0, offset 0xb0, size 0x23
sdb> start
** pid 17424
sdb> getregs
RAX 0                 RBX 0                 RCX 0                 RDX 0
R8  0                 R9  0                 R10 0                 R11 0
R12 0                 R13 0                 R14 0                 R15 0
RDI 0                 RSI 0                 RBP 0                 RSP 7ffdc4ed2320
RIP 4000b0            FLAGS 0000000000000200
sdb> exit
```

### Test3

``` bash
$ ./sdb hello64
** program 'hello64' loaded. entry point 0x4000b0, vaddr 0x4000b0, offset 0xb0, size 0x23
sdb> disasm
** no addr is given
sdb> disasm 0x4000b0
     4000b0: b8 04 00 00 00                    mov       eax, 4
     4000b5: bb 01 00 00 00                    mov       ebx, 1
     4000ba: b9 d4 00 60 00                    mov       ecx, 0x6000d4
     4000bf: ba 0e 00 00 00                    mov       edx, 0xe
     4000c4: cd 80                             int       0x80
     4000c6: b8 01 00 00 00                    mov       eax, 1
     4000cb: bb 00 00 00 00                    mov       ebx, 0
     4000d0: cd 80                             int       0x80
     4000d2: c3                                ret
sdb> start
** pid 17649
sdb> b 0x4000c6
sdb> disasm 0x4000c6
     4000c6: b8 01 00 00 00                    mov       eax, 1
     4000cb: bb 00 00 00 00                    mov       ebx, 0
     4000d0: cd 80                             int       0x80
     4000d2: c3
sdb> dump 0x4000c6
     4000c6: cc 01 00 00 00 bb 00 00 00 00 cd 80 c3 00 68 65  |..............he|
     4000d6: 6c 6c 6f 2c 20 77 6f 72 6c 64 21 0a 00 00 00 00  |llo, world!.....|
     4000e6: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
     4000f6: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 00  |................|
     400106: 01 00 b0 00 40 00 00 00 00 00 00 00 00 00 00 00  |....@...........|
sdb> exit
```

### Test4

``` bash
$ ./sdb hello64
** program 'hello64' loaded. entry point 0x4000b0, vaddr 0x4000b0, offset 0xb0, size 0x23
sdb> disasm 0x4000b0
     4000b0: b8 04 00 00 00                    mov       eax, 4
     4000b5: bb 01 00 00 00                    mov       ebx, 1
     4000ba: b9 d4 00 60 00                    mov       ecx, 0x6000d4
     4000bf: ba 0e 00 00 00                    mov       edx, 0xe
     4000c4: cd 80                             int       0x80
     4000c6: b8 01 00 00 00                    mov       eax, 1
     4000cb: bb 00 00 00 00                    mov       ebx, 0
     4000d0: cd 80                             int       0x80
     4000d2: c3                                ret
sdb> b 0x4000c6
sdb> l
 0: 0x4000c6
sdb> run
** pid 18083
hello, world!
** breakpoint @       4000c6: b8 01 00 00 00                    mov       eax, 1
sdb> set rip 0x4000b0
sdb> cont
hello, world!
** breakpoint @       4000c6: b8 01 00 00 00                    mov       eax, 1
sdb> delete 0
** breakpoint 0 deleted.
sdb> set rip 0x4000b0
sdb> cont
hello, world!
** child process 18083 terminated normally (code 0)
** reload program
** program 'hello64' loaded. entry point 0x4000b0, vaddr 0x4000b0, offset 0xb0, size 0x23
sdb> exit
```

### Test5

``` bash
$ ./sdb guess
** program 'guess' loaded. entry point 0x820, vaddr 0x820, offset 0x820, size 0x262
sdb> vmmap
0000000000000820-0000000000000a82 r-x 820      guess
sdb> disasm 0x985
        985: 48 8d 3d 08 01 00 00              lea       rdi, qword ptr [rip + 0x108]
        98c: b8 00 00 00 00                    mov       eax, 0
        991: e8 0a fe ff ff                    call      0x7a0
        996: 48 8b 15 73 06 20 00              mov       rdx, qword ptr [rip + 0x200673]
        99d: 48 8d 45 d0                       lea       rax, qword ptr [rbp - 0x30]
        9a1: be 10 00 00 00                    mov       esi, 0x10
        9a6: 48 89 c7                          mov       rdi, rax
        9a9: e8 12 fe ff ff                    call      0x7c0
        9ae: 48 8d 45 d0                       lea       rax, qword ptr [rbp - 0x30]
        9b2: ba 00 00 00 00                    mov       edx, 0
sdb> disasm
        9b7: be 00 00 00 00                    mov       esi, 0
        9bc: 48 89 c7                          mov       rdi, rax
        9bf: e8 0c fe ff ff                    call      0x7d0
        9c4: 8b 15 52 06 20 00                 mov       edx, dword ptr [rip + 0x200652]
        9ca: 89 d2                             mov       edx, edx
        9cc: 48 39 d0                          cmp       rax, rdx
        9cf: 75 0e                             jne       0x9df
        9d1: 48 8d 3d ce 00 00 00              lea       rdi, qword ptr [rip + 0xce]
        9d8: e8 93 fd ff ff                    call      0x770
        9dd: eb 0c                             jmp       0x9eb
sdb> b 0x9cc
sdb> start
** pid 18449
sdb> vmmap
000055db8602d000-000055db8602e000 r-x 0        /home/ee904/Desktop/HuaTsai/Unix/Homework/hw4/guess
000055db8622d000-000055db8622f000 rw- 0        /home/ee904/Desktop/HuaTsai/Unix/Homework/hw4/guess
00007f7f7a6b9000-00007f7f7a6e0000 r-x 0        /lib/x86_64-linux-gnu/ld-2.27.so
00007f7f7a8e0000-00007f7f7a8e2000 rw- 27000    /lib/x86_64-linux-gnu/ld-2.27.so
00007f7f7a8e2000-00007f7f7a8e3000 rw- 0
00007ffd546c1000-00007ffd546e3000 rw- 0        [stack]
00007ffd547db000-00007ffd547de000 r-- 0        [vvar]
00007ffd547de000-00007ffd547e0000 r-x 0        [vdso]
ffffffffff600000-ffffffffff601000 r-x 0        [vsyscall]
sdb> cont
Show me the key: 123
** breakpoint @          9cc: 48 39 d0                          cmp       rax, rdx
sdb> get rax
rax = 123 (0x7b)
sdb> get rdx
rdx = 2083991430 (0x7c372f86)
sdb> set rax 2083991430
sdb> cont
Bingo!
** child process 18449 terminated normally (code 0)
** reload program
** program 'guess' loaded. entry point 0x820, vaddr 0x820, offset 0x820, size 0x262
sdb> exit
```
