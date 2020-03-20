#include "sdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <libgen.h>
#include <unistd.h>
#include <capstone/capstone.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include "elftool.h"

int regs_size = 27;
int rippos = 16;
char regs_name[27][9] = {"r15", "r14", "r13", "r12", "rbp",
                         "rbx", "r11", "r10", "r9", "r8",
                         "rax", "rcx", "rdx", "rsi", "rdi",
                         "orig_rax", "rip", "cs", "eflags", "rsp",
                         "ss", "fs_base", "gs_base", "ds", "es",
                         "fs", "gs"};

int ReadCommand(char *buf, Command *cmd) {
  char *arg1 = NULL;
  char *arg2 = NULL;
  char *saveptr = NULL;
  strtok_r(buf, " ", &saveptr);
  arg1 = strtok_r(NULL, " ", &saveptr);
  arg2 = strtok_r(NULL, " ", &saveptr);
  if (arg1) {
    memcpy(cmd->arg1, arg1, strlen(arg1) + 1);
  }
  if (arg2) {
    memcpy(cmd->arg2, arg2, strlen(arg2) + 1);
  }
  if (buf[0] == '\0') {
    cmd->code = NOP;
  } else if (strcmp(buf, "break") == 0 || strcmp(buf, "b") == 0) {
    cmd->code = BREAK;
  } else if (strcmp(buf, "cont") == 0 || strcmp(buf, "c") == 0) {
    cmd->code = CONT;
  } else if (strcmp(buf, "delete") == 0) {
    cmd->code = DELETE;
  } else if (strcmp(buf, "disasm") == 0 || strcmp(buf, "d") == 0) {
    cmd->code = DISASM;
  } else if (strcmp(buf, "dump") == 0 || strcmp(buf, "x") == 0) {
    cmd->code = DUMP;
  } else if (strcmp(buf, "exit") == 0 || strcmp(buf, "q") == 0) {
    cmd->code = EXIT;
  } else if (strcmp(buf, "get") == 0 || strcmp(buf, "g") == 0) {
    cmd->code = GET;
  } else if (strcmp(buf, "getregs") == 0) {
    cmd->code = GETREGS;
  } else if (strcmp(buf, "help") == 0 || strcmp(buf, "h") == 0) {
    cmd->code = HELP;
  } else if (strcmp(buf, "list") == 0 || strcmp(buf, "l") == 0) {
    cmd->code = LIST;
  } else if (strcmp(buf, "load") == 0) {
    cmd->code = LOAD;
  } else if (strcmp(buf, "run") == 0 || strcmp(buf, "r") == 0) {
    cmd->code = RUN;
  } else if (strcmp(buf, "vmmap") == 0 || strcmp(buf, "m") == 0) {
    cmd->code = VMMAP;
  } else if (strcmp(buf, "set") == 0 || strcmp(buf, "s") == 0) {
    cmd->code = SET;
  } else if (strcmp(buf, "si") == 0) {
    cmd->code = SI;
  } else if (strcmp(buf, "start") == 0) {
    cmd->code = START;
  } else {
    printf("** unknown command.\n");
    return -1;
  }
  return 0;
}

/**
 * Break Command
 * \param pg program
 * \param id breakpoint id
 * \param addr elf file when loaded or memory layout address when running
 */
int Break(Program *pg, int id, addr_t addr) {
  if (!IsLoaded(pg)) {
    printf("** no program is loaded.\n");
    return -1;
  }

  addr_t vm_addr;
  addr_t elf_addr;
  if (IsRunning(pg)) {
    vm_addr = addr;
    elf_addr = VMToELF(addr, pg->elftext, pg->elftextoff, pg->vmtext);
  } else {
    elf_addr = addr;
    vm_addr = ELFToVM(addr, pg->elftext, pg->elftextoff, pg->vmtext);
  }

  if (elf_addr < pg->elftext || elf_addr >= pg->elftext + pg->text_size) {
    printf("** address %#lx is not in text segment\n", addr);
    return -1;
  }

  int ins_exist = 0;
  for (int i = 0; i < pg->ins_size; ++i) {
    if (pg->instructions[i].rip == elf_addr) {
      ins_exist = 1;
      break;
    }
  }
  if (!ins_exist) {
    printf("** break point should be set on first byte of an instruction\n");
    return -1;
  }


  uint8_t codes[8];
  reg_t *code = (reg_t *)codes;
  errno = 0;
  *code = ptrace(PTRACE_PEEKTEXT, pg->pid, vm_addr, 0);
  if (errno != 0) {
    printf("** PEEKTEXT @ %#lx fail\n", addr);
    return -1;
  }
  int ret;
  if ((ret = AddBreakPoint(&pg->breakpoints, id, elf_addr, codes[0])) != 0) {
    if (ret == 1) {
      printf("** breakpoint id %d exists\n", id);
    } else if (ret == 2) {
      printf("** breakpoint address %#lx exists\n", addr);
    }
    return -1;
  }

  if (IsRunning(pg)) {
    codes[0] = 0xcc;
    if (ptrace(PTRACE_POKETEXT, pg->pid, vm_addr, *code) == -1) {
      ErrQuit("ptrace @ POKETEXT");
    }
  }
  return 0;
}

/**
 * Cont Command
 * \param pg program
 */
int Cont(Program *pg) {
  if (!IsRunning(pg)) {
    printf("** program is not running.\n");
    return -1;
  }
  addr_t rip;
  int hit = CheckIfHit(pg, 0);
  if (hit) {
    RestoreRIPAndCode(pg, &rip);
  }
  if (ptrace(PTRACE_CONT, pg->pid, 0, 0) == -1) {
    ErrQuit("ptrace @ CONT");
  }
  if (waitpid(pg->pid, &pg->status, 0) < 0) {
    ErrQuit("waitpid");
  }
  if (IsChildExit(pg)) {
    return 0;
  }
  if (hit) {
    PatchBreakPoint(pg, rip);
  }
  CheckIfHit(pg, 1);
  return 0;
}

/**
 * Delete Command
 * \param pg program
 * \param id breakpoint id
 */
int Delete(Program *pg, int id) {
  addr_t addr;
  uint8_t orig_code;
  if (DeleteBreakPoint(&pg->breakpoints, id, &addr, &orig_code) == -1) {
    return -1;
  }
  if (IsRunning(pg)) {
    PatchCode(pg, addr, orig_code);
  }
  return 0;
}

/**
 * Disasm Command
 * \param pg program
 * \param addr address from elf file
 */
int Disasm(Program *pg, addr_t addr) {
  if (!IsLoaded(pg)) {
    printf("** no program is loaded.\n");
    return -1;
  }
  return ListMaxTenInstructions(pg, addr);
}

/**
 * Dump Command
 * \param pg program
 * \param addr address in memory layout
 */
int Dump(const Program *pg, addr_t addr) {
  if (!IsRunning(pg)) {
    printf("** program is not running.\n");
    return -1;
  }
  int count;
  uint8_t codes[80];
  for (count = 0; count < 10; ++count) {
    reg_t *code = (reg_t *)&codes[count * 8];
    errno = 0;
    *code = ptrace(PTRACE_PEEKTEXT, pg->pid, addr + count * 8, 0);
    if (errno != 0) {
      printf("** PEEKTEXT @ %#lx fail\n", addr + count * 8);
      break;
    }
  }
  PrintDumpedData(codes, count * 8, addr);
  return count * 8;
}

/**
 * Exit Command
 * \param pg program
 */
void Exit(Program *pg) {
  FreeBreakPoints(pg->breakpoints);
  FreeInstructions(pg->instructions);
}

/**
 * Get Command
 * \param pg program
 * \param regstr register string
 */
int Get(const Program *pg, const char *regstr) {
  if (!IsRunning(pg)) {
    printf("** program is not running.\n");
    return -1;
  }
  for (int i = 0; i < 27; ++i) {
    if (strcmp(regstr, regs_name[i]) == 0) {
      errno = 0;
      reg_t reg = ptrace(PTRACE_PEEKUSER, pg->pid, sizeof(reg_t) * i, 0);
      if (errno != 0) {
        ErrQuit("ptrace @ PEEKUSER");
      }
      printf("%s = %lu (%#lx)\n", regs_name[i], reg, reg);
      return 0;
    }
  }
  printf("** register %s doesn't exist\n", regstr);
  return 1;
}

/**
 * Getregs Command
 * \param pg program
 */
int GetRegs(const Program *pg) {
  if (!IsRunning(pg)) {
    printf("** program is not running.\n");
    return -1;
  }
  struct user_regs_struct regs;
  if (ptrace(PTRACE_GETREGS, pg->pid, 0, &regs) == -1) {
    ErrQuit("ptrace @ GETREGS");
  }
  printf("RAX %-16llx  RBX %-16llx  ", regs.rax, regs.rbx);
  printf("RCX %-16llx  RDX %-16llx\n", regs.rcx, regs.rdx);
  printf("R8  %-16llx  R9  %-16llx  ", regs.r8, regs.r9);
  printf("R10 %-16llx  R11 %-16llx\n", regs.r10, regs.r11);
  printf("R12 %-16llx  R13 %-16llx  ", regs.r12, regs.r13);
  printf("R14 %-16llx  R15 %-16llx\n", regs.r14, regs.r15);
  printf("RDI %-16llx  RSI %-16llx  ", regs.rdi, regs.rsi);
  printf("RBP %-16llx  RSP %-16llx\n", regs.rbp, regs.rsp);
  printf("RIP %-16llx  FLAGS %.16llx\n", regs.rip, regs.eflags);
  return 0;
}

/**
 * Help Command
 */
void Help() {
  printf("- break {instruction-address}: add a break point\n");
  printf("- cont: continue execution\n");
  printf("- delete {break-point-id}: remove a break point\n");
  printf("- disasm addr: disassemble instructions in a file or a memory region\n");
  printf("- dump addr [length]: dump memory content\n");
  printf("- exit: terminate the debugger\n");
  printf("- get reg: get a single value from a register\n");
  printf("- getregs: show registers\n");
  printf("- help: show this message\n");
  printf("- list: list break points\n");
  printf("- load {path/to/a/program}: load a program\n");
  printf("- run: run the program\n");
  printf("- vmmap: show memory layout\n");
  printf("- set reg val: get a single value to a register\n");
  printf("- si: step into instruction\n");
  printf("- start: start the program and stop at the \n");
}

/**
 * List Command
 * \param pg program
 */
void List(const Program *pg) {
  ListBreakPoints(pg->breakpoints);
}

/**
 * Load Command
 * \param pg program
 * \param path program path
 */
int Load(Program *pg, const char *path) {
  if (IsLoaded(pg)) {
    printf("** a program '%s' was already loaded.\n", pg->name);
    return -1;
  }
  if (access(path, F_OK | X_OK) != 0) {
    printf("** file '%s' does not exist or is not executable.\n", path);
    return -1;
  }

  pid_t child;
  if ((child = fork()) < 0) {
    ErrQuit("fork");
  }
  if (child == 0) {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1) {
      ErrQuit("ptrace @ TRACEME");
    }
    char **argv = NULL;
    execv(path, argv);
    ErrQuit("execv");
  }
  if (waitpid(child, &pg->status, 0) < 0) {
    ErrQuit("waitpid");
  }
  ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL);

  memcpy(pg->name, path, strlen(path) + 1);
  pg->stat = S_INTERMEDIATE;
  pg->pid = child;
  if (GetTextStartInMemory(pg) == -1) {
    printf("** not found .text memory layout\n");
    exit(1);
  }
  if (ReadElf(pg) != 0) {
    printf("** fail to read elf file\n");
    exit(1);
  }

  /* get all instructions */
  if (pg->instructions == NULL) {
    int counts = (pg->text_size >> 3) + ((pg->text_size & 0b111) ? 1 : 0);
    uint8_t *codes = malloc(counts * sizeof(reg_t));
    if (codes == NULL) {
      ErrQuit("malloc");
    }
    for (int i = 0; i < counts; ++i) {
      reg_t *code = (reg_t *)&codes[i * 8];
      addr_t addr = pg->vmtext + pg->elftextoff + i * 8;
      errno = 0;
      *code = ptrace(PTRACE_PEEKTEXT, child, addr, 0);
      if (errno != 0) {
        ErrQuit("ptrace @ PEEKTEXT");
      }
    }

    csh handle;
    cs_insn *ins;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
      printf("** fail to call cs_open\n");
      exit(1);
    }
    size_t size = cs_disasm(handle, codes, pg->text_size, pg->elftext, 0, &ins);
    free(codes);
    if (size > 0) {
      pg->ins_size = size;
      AddInstructions(&pg->instructions, size, ins);
      cs_free(ins, size);
    }
    cs_close(&handle);
  }

  printf("** program '%s' loaded. ", path);
  printf("entry point %#lx, ", pg->elfentry);
  printf("vaddr %#lx, ", pg->elftext);
  printf("offset %#lx, ", pg->elftextoff);
  printf("size %#lx\n", pg->text_size);
  return 0;
}

/**
 * Run Command
 * \param pg program
 */
int Run(Program *pg) {
  if (!IsLoaded(pg)) {
    printf("** no program is loaded.\n");
    return -1;
  } else if (IsRunning(pg)) {
    printf("** program '%s' is already running.\n", pg->name);
    return Cont(pg);
  } else {
    printf("** pid %d\n", pg->pid);
    pg->stat = S_RUNNING;
  }
  SetBreakPoints(pg);
  if (ptrace(PTRACE_CONT, pg->pid, 0, 0) == -1) {
    ErrQuit("ptrace @ CONT");
  }
  if (waitpid(pg->pid, &pg->status, 0) < 0) {
    ErrQuit("waitpid");
  }
  if (IsChildExit(pg)) {
    return 0;
  }
  CheckIfHit(pg, 1);
  return 0;
}

/**
 * Vmmap Command
 * \param pg program
 */
int Vmmap(const Program *pg) {
  if (!IsLoaded(pg)) {
    printf("** no program is loaded.\n");
    return -1;
  }
  if (!IsRunning(pg)) {
    printf("%016lx-%016lx r-x %-8lx %s\n", pg->elftext,
           pg->elftext + pg->text_size, pg->elftextoff, pg->name);
    return 0;
  }
  char mapstr[30];
  snprintf(mapstr, sizeof(mapstr), "/proc/%d/maps", pg->pid);
  FILE *mapfp = NULL;
  if ((mapfp = fopen(mapstr, "r")) == NULL) {
    ErrQuit("fopen");
  }
  addr_t adrs, adre, offset;
  char s[256], perm[5], pgm[256];
  int ret = 0;
  while (fgets(s, sizeof(s), mapfp)) {
    ret = sscanf(s, "%lx-%lx%s%lx%*s%*s%s", &adrs, &adre, perm, &offset, pgm);
    if (ret == 5) {
      printf("%016lx-%016lx %.3s %-8lx %s\n", adrs, adre, perm, offset, pgm);
      continue;
    } else if (ret == 0) {
      ret = sscanf(s, "%lx-%lx%s%lx%*s%*s", &adrs, &adre, perm, &offset);
    }
    if (ret == 4) {
      printf("%016lx-%016lx %.3s %-8lx\n", adrs, adre, perm, offset);
      continue;
    }
  }
  fclose(mapfp);
  return 0;
}

/**
 * Set Command
 * \param pg program
 * \param regstr register string
 * \param reg register value
 */
int Set(Program *pg, const char *regstr, reg_t reg) {
  if (!IsRunning(pg)) {
    printf("** program is not running.\n");
    return -1;
  }
  for (int i = 0; i < 27; ++i) {
    if (strcmp(regstr, regs_name[i]) == 0) {
      if (ptrace(PTRACE_POKEUSER, pg->pid, sizeof(reg_t) * i, reg) == -1) {
        ErrQuit("ptrace @ POKEUSER");
      }
      return 0;
    }
  }
  printf("** register %s doesn't exist\n", regstr);
  return 1;
}

/**
 * Si Command
 * \param pg program
 */
int Si(Program *pg) {
  if (!IsRunning(pg)) {
    printf("** program is not running.\n");
    return -1;
  }
  addr_t rip;
  int hit = CheckIfHit(pg, 0);
  if (hit) {
    RestoreRIPAndCode(pg, &rip);
  }
  if (ptrace(PTRACE_SINGLESTEP, pg->pid, 0, 0) == -1) {
    ErrQuit("ptrace @ SINGLESTEP");
  }
  if (waitpid(pg->pid, &pg->status, 0) < 0) {
    ErrQuit("waitpid");
  }
  if (IsChildExit(pg)) {
    return 0;
  }
  if (hit) {
    PatchBreakPoint(pg, rip);
  }
  CheckIfHit(pg, 1);
  return 0;
}

/**
 * Start Command
 * \param pg program
 */
int Start(Program *pg) {
  if (!IsLoaded(pg)) {
    printf("** no program is loaded.\n");
    return -1;
  } else if (IsRunning(pg)) {
    printf("** program '%s' is already running.\n", pg->name);
    return -1;
  }
  printf("** pid %d\n", pg->pid);
  pg->stat = S_RUNNING;
  SetBreakPoints(pg);
  return 0;
}

void ErrQuit(const char *msg) {
  perror(msg);
  exit(-1);
}

int IsLoaded(const Program *pg) {
  return pg->stat != S_UNLOADED;
}

int IsRunning(const Program *pg) {
  return pg->stat == S_RUNNING;
}

int ReadElf(Program *pg) {
  elf_handle_t *eh = NULL;
  elf_strtab_t *tab = NULL;
  elf_init();
  if ((eh = elf_open(pg->name)) == NULL) {
    printf("** unable to open '%s'.\n", pg->name);
    return -1;
  }
  if (elf_load_all(eh) < 0) {
    printf("** unable to load '%s.\n", pg->name);
    if (eh) {
      elf_close(eh);
      eh = NULL;
    }
    return -1;
  }
  for (tab = eh->strtab; tab != NULL; tab = tab->next) {
    if (tab->id == eh->shstrndx) {
      break;
    }
  }
  if (tab == NULL) {
    printf("** section header string table not found.\n");
    if (eh) {
      elf_close(eh);
      eh = NULL;
    }
    return -1;
  }
  pg->elfentry = eh->entrypoint;
  // start from 1, because eh->shdr[0].name sometimes not valid
  for (int i = 1; i < eh->shnum; i++) {
    if (strcmp(&tab->data[eh->shdr[i].name], ".text") == 0) {
      pg->elftext = eh->shdr[i].addr;
      pg->elftextoff = eh->shdr[i].offset;
      pg->text_size = eh->shdr[i].size;
      break;
    }
  }
  if (eh) {
    elf_close(eh);
    eh = NULL;
  }
  return 0;
}

/**
 * Check if breakpoint exists
 * id exists, return 1
 * addr exists, return 2
 */
int BreakPointExist(const BreakPoints *head, int id, addr_t addr) {
  const BreakPoints *cur = head;
  while (cur) {
    if (cur->id == id) {
      return 1;
    }
    if (cur->addr == addr) {
      return 2;
    }
    cur = cur->next;
  }
  return 0;
}

int SetBreakPoints(Program *pg) {
  BreakPoints *cur = pg->breakpoints;
  while (cur) {
    addr_t addr = ELFToVM(cur->addr, pg->elftext, pg->elftextoff, pg->vmtext);
    PatchBreakPoint(pg, addr);
    cur = cur->next;
  }
  return 0;
}

int AddBreakPoint(BreakPoints **head, int id, addr_t addr, uint8_t code) {
  int ret;
  if ((ret = BreakPointExist(*head, id, addr)) != 0) {
    return ret;
  }
  BreakPoints *cur = *head;
  if (!cur) {
    if ((*head = (BreakPoints *)malloc(sizeof(BreakPoints))) == NULL) {
      ErrQuit("malloc");
    }
    (*head)->id = id;
    (*head)->addr = addr;
    (*head)->next = NULL;
    (*head)->orig_code = code;
    return 0;
  }
  while (cur->next != NULL) {
    cur = cur->next;
  }
  if ((cur->next = (BreakPoints *)malloc(sizeof(BreakPoints))) == NULL) {
    ErrQuit("malloc");
  }
  cur->next->id = id;
  cur->next->addr = addr;
  cur->next->orig_code = code;
  cur->next->next = NULL;
  return 0;
}

int DeleteBreakPoint(BreakPoints **head, int id, addr_t *addr, uint8_t *code) {
  BreakPoints *cur = *head;
  if (!cur) {
    printf("** no breakpoint number %d.\n", id);
    return -1;
  }
  if (cur->id == id) {
    BreakPoints *del = cur;
    *addr = cur->addr;
    *code = cur->orig_code;
    *head = cur->next;
    printf("** breakpoint %d deleted.\n", del->id);
    free(del);
    return 0;
  }
  while (cur->next != NULL) {
    if (cur->next->id == id) {
      BreakPoints *del = cur->next;
      *addr = cur->next->addr;
      *code = cur->next->orig_code;
      cur->next = cur->next->next;
      printf("** breakpoint %d deleted.\n", del->id);
      free(del);
      return 0;
    }
    cur = cur->next;
  }
  printf("** no breakpoint number %d.\n", id);
  return -1;
}

void ListBreakPoints(const BreakPoints *head) {
  if (!head) {
    printf("** no breakpoints.\n");
    return;
  }
  while (head) {
    printf("%3d: %#.6lx\n", head->id, head->addr);
    head = head->next;
  }
}

void FreeBreakPoints(BreakPoints *head) {
  BreakPoints *cur = head;
  while (cur) {
    BreakPoints *del = cur;
    cur = cur->next;
    free(del);
  }
}

int AddInstructions(Instructions **ins, size_t size, cs_insn *insn) {
  *ins = (Instructions *)malloc(size * sizeof(Instructions));
  if (*ins == NULL) {
    ErrQuit("malloc");
  }
  for (size_t i = 0; i < size; ++i) {
    (*ins)[i].rip = insn[i].address;
    (*ins)[i].size = insn[i].size;

    memcpy((*ins)[i].opr, insn[i].mnemonic, strlen(insn[i].mnemonic) + 1);
    memcpy((*ins)[i].opnd, insn[i].op_str, strlen(insn[i].op_str) + 1);
    for (size_t j = 0; j < insn[i].size; ++j) {
      (*ins)[i].bytes[j] = insn[i].bytes[j];
    }
  }
  return 0;
}

/**
 * List instructions
 * \param mode 0 used in disasm command, 1 used if break point hit
 */
int ListInstructions(Program *pg, addr_t addr, int mode) {
  Instructions *ins = pg->instructions;
  int lower_bound = -1;
  for (size_t i = 0; i < pg->ins_size; ++i) {
    if (addr >= ins[i].rip && addr < ins[i].rip + ins[i].size) {
      lower_bound = i;
      break;
    }
  }
  if (lower_bound == -1) {
    printf("** no assembly at %lx\n", addr);
    return 0;
  }
  char bytes[128];
  int count = 0;
  int max = (mode == 0) ? 10 : 1;
  for (int i = lower_bound; i < lower_bound + max && i < pg->ins_size; ++i) {
    for (size_t j = 0; j < ins[i].size; ++j) {
      snprintf(&bytes[j * 3], 4, "%.2x ", ins[i].bytes[j]);
    }
    reg_t rip;
    if (mode == 0) {
      rip = ins[i].rip;
    } else {
      rip = ELFToVM(ins[i].rip, pg->elfentry, pg->elftextoff, pg->vmtext);
    }
    printf("%12lx: %-32s\t%-10s%s\n", rip, bytes, ins[i].opr, ins[i].opnd);
    count += ins[i].size;
  }
  return count;
}

int ListOneInstruction(Program *pg, addr_t addr) {
  return ListInstructions(pg, addr, 1);
}

int ListMaxTenInstructions(Program *pg, addr_t addr) {
  return ListInstructions(pg, addr, 0);
}

void FreeInstructions(Instructions *ins) {
  free(ins);
}

int IsChildExit(Program *pg) {
  if (WIFEXITED(pg->status)) {
    int ret = WEXITSTATUS(pg->status);
    if (ret == 0) {
      printf("** child process %d terminated normally (code 0)\n", pg->pid);
    } else {
      printf("** child process %d terminated with code %d\n", pg->pid, ret);
    }
    pg->stat = S_UNLOADED;
    printf("** reload program\n");
    Load(pg, pg->name);
    return 1;
  }
  return 0;
}

int GetTextStartInMemory(Program *pg) {
  char mapstr[30];
  snprintf(mapstr, sizeof(mapstr), "/proc/%d/maps", pg->pid);
  FILE *mapfp = NULL;
  if ((mapfp = fopen(mapstr, "r")) == NULL) {
    ErrQuit("fopen");
  }
  addr_t adrs;
  char s[256], perm[5], pgm[256];
  while (fgets(s, sizeof(s), mapfp)) {
    if (sscanf(s, "%lx-%*s%s%*s%*s%*s%s", &adrs, perm, pgm) != 0) {
      if (strcmp(perm, "r-xp") == 0 &&
          strcmp(basename(pgm), basename(pg->name)) == 0) {
        pg->vmtext = adrs;
        fclose(mapfp);
        return 0;
      }
    }
  }
  fclose(mapfp);
  return -1;
}

void PrintDumpedData(const uint8_t *data, int size, addr_t addr) {
  int remainder = size & 0b1111;
  int line = (size >> 4) + (remainder ? 1 : 0);
  for (int i = 0; i < line; ++i) {
    printf("%12lx: ", addr + 16 * i);
    for (int j = 0; j < 16; ++j) {
      if (i == line - 1 && remainder != 0 && j >= remainder) {
        printf("   ");
      } else {
        printf("%.2x ", data[i * 16 + j]);
      }
    }
    printf(" |");
    for (int j = 0; j < 16; ++j) {
      char c = data[i * 16 + j];
      if (i == line - 1 && remainder != 0 && j >= remainder) {
        printf(" ");
      } else if (isprint(c)) {
        printf("%c", c);
      } else {
        printf(".");
      }
    }
    printf("|\n");
  }
}

int RestoreRIPAndCode(Program *pg, addr_t *rip) {
  errno = 0;
  *rip = ptrace(PTRACE_PEEKUSER, pg->pid, sizeof(reg_t) * rippos, 0) - 1;
  if (errno != 0) {
    ErrQuit("ptrace @ PEEKUSER");
  }
  if (ptrace(PTRACE_POKEUSER, pg->pid, sizeof(reg_t) * rippos, *rip) == -1) {
    ErrQuit("ptrace @ POKEUSER");
  }
  BreakPoints *cur = pg->breakpoints;
  while (cur) {
    if (cur->addr == VMToELF(*rip, pg->elfentry, pg->elftextoff, pg->vmtext)) {
      PatchCode(pg, *rip, cur->orig_code);
      return 0;
    }
    cur = cur->next;
  }
  return -1;
}

int PatchCode(Program *pg, addr_t rip, uint8_t value) {
  uint8_t codes[8];
  reg_t *code = (reg_t *)codes;
  errno = 0;
  *code = ptrace(PTRACE_PEEKTEXT, pg->pid, rip, 0);
  if (errno != 0) {
    ErrQuit("ptrace @ PEEKTEXT");
  }
  codes[0] = value;
  if (ptrace(PTRACE_POKETEXT, pg->pid, rip, *code) == -1) {
    ErrQuit("ptrace @ POKETEXT");
  }
  return 0;
}

int PatchBreakPoint(Program *pg, addr_t rip) {
  PatchCode(pg, rip, 0xcc);
  return 0;
}

int CheckIfHit(Program *pg, int printbp) {
  errno = 0;
  reg_t rip = ptrace(PTRACE_PEEKUSER, pg->pid, sizeof(reg_t) * rippos, 0) - 1;
  if (errno != 0) {
    ErrQuit("ptrace @ PEEKUSER");
  }
  BreakPoints *cur = pg->breakpoints;
  while (cur) {
    addr_t elfrip = VMToELF(rip, pg->elftext, pg->elftextoff, pg->vmtext);
    if (cur->addr == elfrip) {
      if (printbp) {
        printf("** breakpoint @ ");
        ListOneInstruction(pg, cur->addr);
      }
      return 1;
    }
    cur = cur->next;
  }
  return 0;
}

addr_t VMToELF(addr_t addr, addr_t elfentry, addr_t elfoffset, addr_t vmtext) {
  if (elfentry - elfoffset == vmtext) {
    return addr;
  } else {
    return addr - vmtext;
  }
}

addr_t ELFToVM(addr_t addr, addr_t elfentry, addr_t elfoffset, addr_t vmtext) {
  if (elfentry - elfoffset == vmtext) {
    return addr;
  } else {
    return addr + vmtext;
  }
}
