#include "sdb.h"
#include <gtest/gtest.h>
#include <string>

TEST(ReadCommand, General) {
  Command cmd;
  char cmdstr[5][256] = {"load hello", "l", "run", "m", "unknown"};
  ReadCommand(cmdstr[0], &cmd);
  EXPECT_EQ(cmd.code, LOAD);
  EXPECT_STREQ(cmd.arg1, "hello");
  ReadCommand(cmdstr[1], &cmd);
  EXPECT_EQ(cmd.code, LIST);
  ReadCommand(cmdstr[2], &cmd);
  EXPECT_EQ(cmd.code, RUN);
  ReadCommand(cmdstr[3], &cmd);
  EXPECT_EQ(cmd.code, VMMAP);
  int ret = ReadCommand(cmdstr[4], &cmd);
  EXPECT_EQ(ret, -1);
}

TEST(Load, General) {
  Command cmd;
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  char str[13] = "load nofile";
  ReadCommand(str, &cmd);
  int ret = Load(&pg, cmd.arg1);
  EXPECT_EQ(ret, -1);
  snprintf(str, sizeof(str), "%s", "load hello64");
  ReadCommand(str, &cmd);
  ret = Load(&pg, cmd.arg1);
  EXPECT_EQ(ret, 0);
  Exit(&pg);
}

TEST(ReadElf, General) {
  Program pg;
  memcpy(pg.name, "hello64", 8);
  EXPECT_EQ(ReadElf(&pg), 0);
  EXPECT_EQ(pg.elfentry, 0x4000b0);
  EXPECT_EQ(pg.elftext, 0x4000b0);
  EXPECT_EQ(pg.elftextoff, 0xb0);
  EXPECT_EQ(pg.text_size, 0x23);
  memcpy(pg.name, "guess", 6);
  EXPECT_EQ(ReadElf(&pg), 0);
  EXPECT_EQ(pg.elfentry, 0x820);
  EXPECT_EQ(pg.elftext, 0x820);
  EXPECT_EQ(pg.elftextoff, 0x820);
  EXPECT_EQ(pg.text_size, 0x262);
}

TEST(BreakPoints, General) {
  BreakPoints *head = NULL;
  ListBreakPoints(head);
  AddBreakPoint(&head, 0, 0x01, 0x90);
  AddBreakPoint(&head, 1, 0x02, 0x90);
  AddBreakPoint(&head, 2, 0x03, 0x90);
  AddBreakPoint(&head, 3, 0x04, 0x90);
  AddBreakPoint(&head, 4, 0x05, 0x90);
  ListBreakPoints(head);
  addr_t addr;
  uint8_t code;
  DeleteBreakPoint(&head, 3, &addr, &code);
  DeleteBreakPoint(&head, 4, &addr, &code);
  AddBreakPoint(&head, 5, 0x06, 0x90);
  DeleteBreakPoint(&head, 2, &addr, &code);
  DeleteBreakPoint(&head, 5, &addr, &code);
  DeleteBreakPoint(&head, 5, &addr, &code);
  DeleteBreakPoint(&head, 0, &addr, &code);
  DeleteBreakPoint(&head, 1, &addr, &code);
  DeleteBreakPoint(&head, 1, &addr, &code);
  ListBreakPoints(head);
  AddBreakPoint(&head, 6, 0x07, 0x90);
  FreeBreakPoints(head);
}

TEST(BreakPoints, Repeat) {
  BreakPoints *head = NULL;
  ListBreakPoints(head);
  AddBreakPoint(&head, 0, 0x01, 0x90);
  AddBreakPoint(&head, 1, 0x02, 0x90);
  EXPECT_EQ(AddBreakPoint(&head, 0, 0x03, 0x90), 1);
  EXPECT_EQ(AddBreakPoint(&head, 0, 0x01, 0x90), 1);
  EXPECT_EQ(AddBreakPoint(&head, 2, 0x02, 0x90), 2);
  FreeBreakPoints(head);
}

TEST(Vmmap, General) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "hello64");
  Vmmap(&pg);
  pg.stat = S_RUNNING;
  Vmmap(&pg);
  Exit(&pg);
}

TEST(Vmmap, Guess) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "guess");
  std::cerr << "[  loaded  ]" << std::endl;
  Vmmap(&pg);
  pg.stat = S_RUNNING;
  std::cerr << "[  running ]" << std::endl;
  Vmmap(&pg);
  Exit(&pg);
}

TEST(Instructions, General) {
  Program pg;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  pg.ins_size = 5;
  uint8_t codes[22] = {0xb8, 0x04, 0x00, 0x00, 0x00, 0xbb, 0x01, 0x00,
                       0x00, 0x00, 0xb9, 0xd4, 0x00, 0x60, 0x00, 0xba,
                       0x0e, 0x00, 0x00, 0x00, 0xcd, 0x80};
  csh handle;
  cs_insn *insn;
  cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
  size_t count = cs_disasm(handle, codes, 22, 0x4000b0, 0, &insn);
  AddInstructions(&pg.instructions, count, insn);
  cs_free(insn, count);
  cs_close(&handle);
  std::cerr << "[ 0x4000b0 ]" << std::endl;
  ListMaxTenInstructions(&pg, 0x4000b0);
  std::cerr << "[ 0x4000bb ]" << std::endl;
  ListMaxTenInstructions(&pg, 0x4000bb);
  Exit(&pg);
}

TEST(Instructions, Boundary) {
  Program pg;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  pg.ins_size = 2;
  uint8_t codes[10] = {0xb8, 0x04, 0x00, 0x00, 0x00,
                       0xbb, 0x01, 0x00, 0x00, 0x00};
  csh handle;
  cs_insn *insn;
  cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
  size_t count = cs_disasm(handle, codes, 10, 0x4000b0, 0, &insn);
  AddInstructions(&pg.instructions, count, insn);
  cs_free(insn, count);
  cs_close(&handle);
  std::cerr << "[ 0x4000af ]" << std::endl;
  ListMaxTenInstructions(&pg, 0x4000af);
  std::cerr << "[ 0x4000b0 ]" << std::endl;
  ListMaxTenInstructions(&pg, 0x4000b0);
  std::cerr << "[ 0x4000b9 ]" << std::endl;
  ListMaxTenInstructions(&pg, 0x4000b9);
  std::cerr << "[ 0x4000ba ]" << std::endl;
  ListMaxTenInstructions(&pg, 0x4000ba);
  Exit(&pg);
}

TEST(Instructions, MoreThanTenInstructions) {
  Program pg;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  pg.ins_size = 15;
  uint8_t codes[15] = {0x90, 0x90, 0x90, 0x90, 0x90,
                       0x90, 0x90, 0x90, 0x90, 0x90,
                       0x90, 0x90, 0x90, 0x90, 0x90};
  csh handle;
  cs_insn *insn;
  cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
  size_t count = cs_disasm(handle, codes, 10, 0x4000b0, 0, &insn);
  AddInstructions(&pg.instructions, count, insn);
  cs_free(insn, count);
  cs_close(&handle);
  ListMaxTenInstructions(&pg, 0x4000b0);
  Exit(&pg);
}

TEST(PrintDumpedData, General) {
  uint8_t data[30] = {0xcc, 0x01, 0x00, 0x00, 0x00,
                      0xbb, 0x00, 0x00, 0x00, 0x00,
                      0xcd, 0x80, 0xc3, 0x00, 0x68,
                      0x65, 0x6c, 0x6c, 0x6f, 0x2c,
                      0x20, 0x77, 0x6f, 0x72, 0x6c,
                      0x64, 0x21, 0x0a, 0x00, 0x00};
  PrintDumpedData(data, 30, 0x400c6);
}

TEST(PrintDumpedData, Boundary) {
  uint8_t data[40] = {0x44, 0x6f, 0x5e, 0x2a, 0x05,
                      0x6b, 0x00, 0x00, 0x00, 0xfe,
                      0xff, 0x6f, 0x00, 0x22, 0x23,
                      0x75, 0x66, 0x40, 0x40, 0x47,
                      0x4c, 0x49, 0x42, 0x43, 0x5f,
                      0x32, 0x2e, 0x32, 0x2e, 0x35,
                      0x00, 0x5f, 0x5f, 0x54, 0x4d,
                      0x43, 0x5f, 0x45, 0x4e, 0x6e};
  std::cerr << "[    15    ]" << std::endl;
  PrintDumpedData(data, 15, 0x55555);
  std::cerr << "[    16    ]" << std::endl;
  PrintDumpedData(data, 16, 0x55555);
  std::cerr << "[    17    ]" << std::endl;
  PrintDumpedData(data, 17, 0x55555);
  std::cerr << "[    31    ]" << std::endl;
  PrintDumpedData(data, 31, 0x55555);
  std::cerr << "[    32    ]" << std::endl;
  PrintDumpedData(data, 32, 0x55555);
  std::cerr << "[    33    ]" << std::endl;
  PrintDumpedData(data, 33, 0x55555);
}

TEST(Dump, General) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "hello64");
  Start(&pg);
  Dump(&pg, 0x4000b0);
  Exit(&pg);
}

TEST(Dump, Boundary) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "hello64");
  Start(&pg);
  std::cerr << "[ 0x3fffff ] 0 bytes" << std::endl;
  Dump(&pg, 0x3fffff);
  std::cerr << "[ 0x400000 ] 80 bytes" << std::endl;
  Dump(&pg, 0x400000);
  std::cerr << "[ 0x401000 ] 0 bytes" << std::endl;
  Dump(&pg, 0x401000);
  std::cerr << "[ 0x400fff ] 0 bytes" << std::endl;
  Dump(&pg, 0x400fff);
  std::cerr << "[ 0x400ff9 ] 0 bytes" << std::endl;
  Dump(&pg, 0x400ff9);
  std::cerr << "[ 0x400ff8 ] 8 bytes" << std::endl;
  Dump(&pg, 0x400ff8);
  std::cerr << "[ 0x400ff7 ] 8 bytes" << std::endl;
  Dump(&pg, 0x400ff7);
  std::cerr << "[ 0x400ff6 ] 8 bytes" << std::endl;
  Dump(&pg, 0x400ff6);
  std::cerr << "[ 0x400ff1 ] 8 bytes" << std::endl;
  Dump(&pg, 0x400ff1);
  std::cerr << "[ 0x400ff0 ] 16 bytes" << std::endl;
  Dump(&pg, 0x400ff0);
  std::cerr << "[ 0x400fe9 ] 16 bytes" << std::endl;
  Dump(&pg, 0x400fe9);
  std::cerr << "[ 0x400fe8 ] 24 bytes" << std::endl;
  Dump(&pg, 0x400fe8);
  Exit(&pg);
}

TEST(Dump, ContinueDump) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "hello64");
  Start(&pg);
  int count = -1;
  addr_t dump_addr = 0x4000b0;
  while (count != 0) {
    std::cerr << "[   dump   ]" << std::endl;
    count = Dump(&pg, dump_addr);
    dump_addr += count;
  }
  Exit(&pg);
}

TEST(Disasm, General) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "hello64");
  std::cerr << "[ 0x4000b0 ]" << std::endl;
  Disasm(&pg, 0x4000b0);
  std::cerr << "[ 0x4000c0 ]" << std::endl;
  Disasm(&pg, 0x4000c0);
  Exit(&pg);
}

TEST(Disasm, PIEProgram) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "guess");
  std::cerr << "[  0x820   ]" << std::endl;
  Disasm(&pg, 0x820);
  std::cerr << "[  0xa70   ]" << std::endl;
  Disasm(&pg, 0xa70);
  Exit(&pg);
}

TEST(Disasm, ContinueDisasm) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "guess");
  Start(&pg);
  int count = -1;
  addr_t dump_addr = 0xa10;
  while (count != 0) {
    std::cerr << "[  disasm  ]" << std::endl;
    if ((count = Disasm(&pg, dump_addr)) != -1) {
      dump_addr += count;
    }
  }
  Exit(&pg);
}

TEST(Run, NoBreakPoint) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  EXPECT_EQ(Load(&pg, "hello64"), 0);
  EXPECT_EQ(::Run(&pg), 0);
  EXPECT_EQ(pg.stat, S_INTERMEDIATE);
  Exit(&pg);
}

TEST(Run, BreakPoint) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "hello64");
  Start(&pg);
  Break(&pg, 0, 0x4000c6);
  Dump(&pg, 0x4000c6);
  ::Run(&pg);
  Cont(&pg);
  EXPECT_EQ(pg.stat, S_INTERMEDIATE);
  Exit(&pg);
}

TEST(Break, Boundary1) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "hello64");
  EXPECT_EQ(Break(&pg, 0, 0x4000af), -1);
  Break(&pg, 0, 0x4000b0);
  Break(&pg, 1, 0x4000d2);
  EXPECT_EQ(Break(&pg, 2, 0x4000d3), -1);
  Exit(&pg);
}

TEST(Break, Boundary2) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "guess");
  EXPECT_EQ(Break(&pg, 0, 0x81f), -1);
  Break(&pg, 0, 0x820);
  Break(&pg, 1, 0xa81);
  EXPECT_EQ(Break(&pg, 2, 0xa82), -1);
  Exit(&pg);
}

TEST(Break, Repeat) {
  Program pg;
  pg.stat = S_UNLOADED;
  pg.breakpoints = NULL;
  pg.instructions = NULL;
  Load(&pg, "hello64");
  Break(&pg, 0, 0x4000b0);
  EXPECT_EQ(Break(&pg, 0, 0x4000b1), -1);
  EXPECT_EQ(Break(&pg, 1, 0x4000b0), -1);
  EXPECT_EQ(Break(&pg, 0, 0x4000b0), -1);
  Exit(&pg);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  // TODO: use source file of hello64, guess, not executable file
  if (access("hello64", F_OK | X_OK) != 0) {
    printf("Executable 'hello64' is needed\n");
    exit(1);
  }
  if (access("guess", F_OK | X_OK) != 0) {
    printf("Executable 'guess' is needed\n");
    exit(1);
  }
  return RUN_ALL_TESTS();
}
