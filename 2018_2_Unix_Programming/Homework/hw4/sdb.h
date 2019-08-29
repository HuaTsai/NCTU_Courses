#ifndef HW4_SDB_H_
#define HW4_SDB_H_

#include <stdint.h>
#include <sys/types.h>
#include <capstone/capstone.h>

typedef uint64_t reg_t;
typedef uint64_t addr_t;

typedef enum CommandType_e {
  BREAK,
  CONT,
  DELETE,
  DISASM,
  DUMP,
  EXIT,
  GET,
  GETREGS,
  HELP,
  LIST,
  LOAD,
  RUN,
  VMMAP,
  SET,
  SI,
  START,
  NOP
} CommandType;

/* S_INTERMEDIATE: state of loaded without running */
typedef enum State_e { S_UNLOADED, S_INTERMEDIATE, S_RUNNING } State;

typedef struct Command_s {
  CommandType code;
  char arg1[256];
  char arg2[256];
} Command;

typedef struct BreakPoints_s {
  int id;
  /* not actually breakpoint address, it's address of elf file */
  addr_t addr;
  uint8_t orig_code;
  struct BreakPoints_s *next;
} BreakPoints;

typedef struct Instructions_s {
  /* not actually instruction address, it's address of elf file */
  reg_t rip;
  uint8_t bytes[16];
  size_t size;
  char opr[32];
  char opnd[160];
} Instructions;

typedef struct Program_s {
  char name[256];
  pid_t pid;
  State stat;
  int status;
  /* text start address of the program in memory layout */
  addr_t vmtext;
  /* following four fields are read from elf file */
  addr_t elfentry;
  addr_t elftext;
  size_t elftextoff;
  size_t text_size;
  size_t ins_size;
  BreakPoints *breakpoints;
  Instructions *instructions;
} Program;

#ifdef __cplusplus
extern "C" {
#endif

int ReadCommand(char *buf, Command *cmd);

int Break(Program *pg, int id, addr_t addr);
int Cont(Program *pg);
int Delete(Program *pg, int id);
int Disasm(Program *pg, addr_t addr);
int Dump(const Program *pg, addr_t addr);
void Exit(Program *pg);
int Get(const Program *pg, const char *regstr);
int GetRegs(const Program *pg);
void Help();
void List(const Program *pg);
int Load(Program *pg, const char *path);
int Run(Program *pg);
int Vmmap(const Program *pg);
int Set(Program *pg, const char *regstr, reg_t reg);
int Si(Program *pg);
int Start(Program *pg);

void ErrQuit(const char *msg);
int IsLoaded(const Program *pg);
int IsRunning(const Program *pg);
int ReadElf(Program *pg);
int BreakPointExist(const BreakPoints *head, int id, addr_t addr);
int SetBreakPoints(Program *pg);
int AddBreakPoint(BreakPoints **head, int id, addr_t addr, uint8_t code);
int DeleteBreakPoint(BreakPoints **head, int id, addr_t *addr, uint8_t *code);
void ListBreakPoints(const BreakPoints *head);
void FreeBreakPoints(BreakPoints *head);
int AddInstructions(Instructions **ins, size_t size, cs_insn *insn);
int ListInstructions(Program *pg, addr_t addr, int mode);
int ListOneInstruction(Program *pg, addr_t addr);
int ListMaxTenInstructions(Program *pg, addr_t addr);
void FreeInstructions(Instructions *ins);
int IsChildExit(Program *pg);
int GetTextStartInMemory(Program *pg);
void PrintDumpedData(const uint8_t *data, int size, addr_t addr);
int RestoreRIPAndCode(Program *pg, addr_t *rip);
int PatchCode(Program *pg, addr_t rip, uint8_t value);
int PatchBreakPoint(Program *pg, addr_t rip);
int CheckIfHit(Program *pg, int printbp);
addr_t VMToELF(addr_t addr, addr_t elfentry, addr_t elfoffset, addr_t vmtext);
addr_t ELFToVM(addr_t addr, addr_t elfentry, addr_t elfoffset, addr_t vmtext);

#ifdef __cplusplus
}
#endif

#endif  // HW4_SDB_H_
