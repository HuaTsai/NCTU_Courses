#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdb.h"

int main(int argc, char **argv) {
  char cmdstr[256];
  Command cmd;
  Program pg = {
    .name[0] = '\0',
    .pid = -1,
    .stat = S_UNLOADED,
    .status = -1,
    .vmtext = 0,
    .elfentry = 0,
    .elftext = 0,
    .elftextoff = 0,
    .text_size = 0,
    .ins_size = 0,
    .breakpoints = NULL,
    .instructions = NULL
  };
  int bp_id = 0;
  addr_t disasm_addr = 0;
  addr_t dump_addr = 0;
  if (argc > 1) {
    Load(&pg, argv[1]);
  }
  while (1) {
    printf("sdb> ");
    fflush(stdout);
    fgets(cmdstr, sizeof(cmdstr), stdin);
    if (cmdstr[strlen(cmdstr) - 1] == '\n') {
      cmdstr[strlen(cmdstr) - 1] = '\0';
    }
    cmd.code = NOP;
    memset(cmd.arg1, '\0', sizeof(cmd.arg1));
    memset(cmd.arg2, '\0', sizeof(cmd.arg2));
    if (ReadCommand(cmdstr, &cmd) == 0) {
      if (cmd.code == BREAK) {
        if (cmd.arg1[0] == '\0') {
          printf("** no break point address\n");
          continue;
        }
        reg_t reg;
        sscanf(cmd.arg1, "%lx", &reg);
        if (Break(&pg, bp_id, reg) == 0) {
          ++bp_id;
        }
      } else if (cmd.code == CONT) {
        Cont(&pg);
      } else if (cmd.code == DELETE) {
        if (cmd.arg1[0] == '\0') {
          printf("** no break point index\n");
          continue;
        }
        int idx;
        sscanf(cmd.arg1, "%d", &idx);
        Delete(&pg, idx);
      } else if (cmd.code == DISASM) {
        if (cmd.arg1[0] == '\0' && disasm_addr == 0) {
          printf("** no addr is given\n");
          continue;
        } else if (cmd.arg1[0] != '\0') {
          sscanf(cmd.arg1, "%lx", &disasm_addr);
        }
        int count;
        if ((count = Disasm(&pg, disasm_addr)) != -1) {
          disasm_addr += count;
        }
      } else if (cmd.code == DUMP) {
        if (cmd.arg1[0] == '\0' && dump_addr == 0) {
          printf("** no addr is given\n");
          continue;
        } else if (cmd.arg1[0] != '\0') {
          sscanf(cmd.arg1, "%lx", &dump_addr);
        }
        int count;
        if ((count = Dump(&pg, dump_addr)) != -1) {
          dump_addr += count;
        }
      } else if (cmd.code == EXIT) {
        Exit(&pg);
        break;
      } else if (cmd.code == GET) {
        if (cmd.arg1[0] == '\0') {
          printf("** no register is given\n");
          continue;
        }
        Get(&pg, cmd.arg1);
      } else if (cmd.code == GETREGS) {
        GetRegs(&pg);
      } else if (cmd.code == HELP) {
        Help();
      } else if (cmd.code == LIST) {
        List(&pg);
      } else if (cmd.code == LOAD) {
        if (cmd.arg1[0] == '\0') {
          printf("** no file is given\n");
          continue;
        }
        Load(&pg, cmd.arg1);
      } else if (cmd.code == RUN) {
        Run(&pg);
      } else if (cmd.code == VMMAP) {
        Vmmap(&pg);
      } else if (cmd.code == SET) {
        reg_t reg;
        if (cmd.arg1[0] == '\0') {
          printf("** no register is given\n");
          continue;
        }
        if (cmd.arg2[0] == '\0') {
          printf("** no value is given\n");
          continue;
        }
        if (cmd.arg2[0] == '0' && (cmd.arg2[1] == 'x' || cmd.arg2[1] == 'X')) {
          sscanf(cmd.arg2, "%lx", &reg);
        } else {
          sscanf(cmd.arg2, "%lu", &reg);
        }
        Set(&pg, cmd.arg1, reg);
      } else if (cmd.code == SI) {
        Si(&pg);
      } else if (cmd.code == START) {
        Start(&pg);
      } else if (cmd.code == NOP) {
        // NOP
      }
    }
  }
  return 0;
}
