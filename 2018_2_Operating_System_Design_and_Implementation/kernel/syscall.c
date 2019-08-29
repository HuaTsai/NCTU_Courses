#include <inc/stdio.h>
#include <kernel/cpu.h>
#include <kernel/mem.h>
#include <kernel/spinlock.h>
#include <kernel/syscall.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/trap.h>

// TODO: Lab6, spinlock do_puts
struct spinlock console_lock;
void do_puts(char *str, uint32_t len) {
  spin_lock(&console_lock);
  uint32_t i;
  for (i = 0; i < len; i++) {
    k_putch(str[i]);
  }
  spin_unlock(&console_lock);
}

int32_t do_getc() { return k_getc(); }

int32_t do_syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3,
                   uint32_t a4, uint32_t a5) {
  int32_t retVal = -1;
  extern void sys_settextcolor(unsigned char forecolor,
                               unsigned char backcolor);
  extern void sys_cls();
  extern void sched_yield(void);

  switch (syscallno) {
    // TODO: Lab5, syscall handler
    // TODO: Lab6, modify getpid, getcid, sleep, and kill
    case SYS_fork:
      retVal = sys_fork();
      break;
    case SYS_getc:
      retVal = do_getc();
      break;
    case SYS_puts:
      do_puts((char *)a1, a2);
      retVal = 0;
      break;
    case SYS_getpid:
      retVal = thiscpu->cpu_task->task_id;
      break;
    case SYS_getcid:
      retVal = thiscpu->cpu_id;
      break;
    case SYS_sleep:
      thiscpu->cpu_task->state = TASK_SLEEP;
      thiscpu->cpu_task->remind_ticks = a1;
      sched_yield();
      break;
    case SYS_kill:
      sys_kill(thiscpu->cpu_task->task_id);
      retVal = 0;
      break;
    case SYS_get_num_free_page:
      retVal = sys_get_num_free_page();
      break;
    case SYS_get_num_used_page:
      retVal = sys_get_num_used_page();
      break;
    case SYS_get_ticks:
      retVal = sys_get_ticks();
      break;
    case SYS_settextcolor:
      sys_settextcolor(a1, a2);
      retVal = 0;
      break;
    case SYS_cls:
      sys_cls();
      retVal = 0;
      break;
    /* TODO: Lab7, syscall handler */
    case SYS_open:
      retVal = sys_open((char *)a1, a2, a3);
      break;
    case SYS_read:
      retVal = sys_read(a1, (void *)a2, a3);
      break;
    case SYS_write:
      retVal = sys_write(a1, (void *)a2, a3);
      break;
    case SYS_close:
      retVal = sys_close(a1);
      break;
    case SYS_lseek:
      retVal = sys_lseek(a1, a2, a3);
      break;
    case SYS_unlink:
      retVal = sys_unlink((char *)a1);
      break;
    case SYS_ls:
      retVal = sys_ls((char *)a1);
      break;
    case SYS_rm:
      retVal = sys_rm((char *)a1);
      break;
    case SYS_touch:
      retVal = sys_touch((char *)a1);
      break;
  }
  return retVal;
}

static void syscall_handler(struct Trapframe *tf) {
  /* TODO: Lab5, syscall_handler
   * call do_syscall
   * Please remember to fill in the return value
   * HINT: You have to know where to put the return value
   */
  int ret =
      do_syscall(tf->tf_regs.reg_eax, tf->tf_regs.reg_edx, tf->tf_regs.reg_ecx,
                 tf->tf_regs.reg_ebx, tf->tf_regs.reg_edi, tf->tf_regs.reg_esi);
  tf->tf_regs.reg_eax = ret;
}

void syscall_init() {
  /* TODO: Lab5, syscall_init
   * Please set gate of system call into IDT
   * You can leverage the API register_handler in kernel/trap.c
   */
  extern void SYS_CALL();
  register_handler(T_SYSCALL, syscall_handler, SYS_CALL, 1, 3);
}
