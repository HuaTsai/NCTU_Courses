# Homework #2

## Environment

* Ubuntu 18.04
* gcc version: 7.3.0
* library: `libc.so.6`, `libdl.so.2`

## Functions Can be Monitored

* same as minimum requirements: i.e. 20 functions

* `fcntl.h`
  * `open(2)`: `creat`
  * `open(2)`: `open`
* `sys/stat.h`
  * `stat(2)`: `stat`
  * `stat(2)`: `lstat`
  * `mkdir(2)`: `mkdir`
  * `chmod(2)`: `chmod`
* `unistd.h`
  * `read(2)`: `read`
  * `write(2)`: `write`
  * `close(2)`: `close`
  * `dup(2)`: `dup`
  * `dup(2)`: `dup2`
  * `chdir(2)`: `chdir`
  * `chown(2)`: `chown`
  * `rmdir(2)`: `rmdir`
  * `pread(2)`: `pwrite`
  * `link(2)`: `link`
  * `unlink(2)`: `unlink`
  * `readlink(2)`: `readlink`
  * `symlink(2)`: `symlink`
* `dirent.h`
  * `opendir(3)`: `opendir`
  * `readdir(3)`: `readdir`
  * `closedir(3)`: `closedir`
* `stdio.h`
  * `fopen(3)`: `fopen`
  * `fclose(3)`: `fclose`
  * `remove(3)`: `remove`
  * `rename(2)`: `rename`
  * `fgetc(3)`: `fgetc`
  * `fgetc(3)`: `fgets`
  * `fread(3)`: `fread`
  * `fread(3)`: `fwrite`
  * `scanf(3)`: `fscanf`
  * `printf(3)`: `fprintf`

## Input and Output

* input file: `hw2.c`
* output file: `fsmon.so`
* build by simply run `make`
* target `test` is for myself to test the result, please ignore it

## Please Note

1. In Ubuntu 18.04, it seems that if `fopen(3)` or `fclose(3)` is hijacked, then there will be a segmentation fault after issue `LD_PRELOAD=./fsmon ls -la`. After a lot of research, I still cannot solve this problem so far.
2. Note that the function declaration in `hw2.c` is the same with your system.
