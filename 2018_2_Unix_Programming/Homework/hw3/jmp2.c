#ifdef USEGCC
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>
#else
#include "libmini.h"
#endif

void addmask(int sig) {
  sigset_t act;
  if (sigprocmask(0, NULL, &act) < 0) write(1, "err", 3);
  sigaddset(&act, sig);
  if (sigprocmask(0, &act, NULL) < 0) write(1, "err", 3);
}

void pr_mask() {
  sigset_t sigset;
  int errno_save;
  errno_save = errno;
  if (sigprocmask(0, NULL, &sigset) < 0) write(1, "err", 3);
  if (sigismember(&sigset, SIGINT)) write(1, "SIGINT ", 7);
  if (sigismember(&sigset, SIGQUIT)) write(1, "SIGQUIT ", 8);
  if (sigismember(&sigset, SIGUSR1)) write(1, "SIGUSR1 ", 8);
  if (sigismember(&sigset, SIGALRM)) write(1, "SIGALRM ", 8);
  write(1, "\n", 1);
  errno = errno_save;
}

typedef void (*proc_t)();
static jmp_buf jb;

#define FUNBODY(m, from)    \
  {                         \
    write(1, m, strlen(m)); \
    pr_mask();        \
    longjmp(jb, from);      \
  }

void a() FUNBODY("function a(): ", 1);
void b() FUNBODY("function b(): ", 2);
void c() FUNBODY("function c(): ", 3);
void d() FUNBODY("function d(): ", 4);
void e() FUNBODY("function e(): ", 5);
void f() FUNBODY("function f(): ", 6);

proc_t funs[] = {a, b, c, d, e, f};

int main() {
  volatile int i = 0;
  if (setjmp(jb) != 0) {
    i++;
    write(1, "back to main: ", 14);
    pr_mask();
  }
  if (i == 1) {
    addmask(SIGINT);
    write(1, "add SIGINT mask\n", 16);
    funs[i]();
  } else if (i == 2) {
    addmask(SIGQUIT);
    write(1, "add SIGQUIT mask\n", 17);
    funs[i]();
  } else if (i == 3) {
    addmask(SIGUSR1);
    write(1, "add SIGUSR1 mask\n", 17);
    funs[i]();
  } else if (i == 4) {
    addmask(SIGALRM);
    write(1, "add SIGALRM mask\n", 17);
    funs[i]();
  } else if (i < 6) {
    funs[i]();
  }
  return 0;
}
