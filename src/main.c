#include "../include/utils.h"

uint16_t memory[MEMORY_MAX] = { 0 };
uint16_t reg[R_COUNT] = { 0 };

int main(int argc, const char **argv) {
  // Load args
  if (argc < 2) {
    printf("[pLC-3] Usage: %s <image-file1> ...\n", argv[0]);
    exit(2);
  }

  for (int i = 1; i < argc; ++i) {
    if (!read_image(argv[i])) {
      printf("[pLC-3] Failed to load image: %s\n", argv[i]);
      exit(1);
    }
  }

  // Setup
  signal(SIGINT, handle_interrupt);
  disable_input_buffering();

  reg[R_COND] = FL_ZRO;

  // set PC to start position
  // default being 0x3000
  enum { PC_START = 0x3000 };
  reg[R_PC] = PC_START;

  int running = 1;
  while (running) {
    uint16_t instr = mem_read(reg[R_PC]++);
    uint16_t op = instr >> 12; // ??
    uint16_t r0 = 0;
    uint16_t r1 = 0;
    uint16_t r2 = 0;
    uint16_t imm_flag = 0;
    uint16_t imm5 = 0;
    uint16_t pc_offset = 0;
    uint16_t offset = 0;
    uint16_t cond_flag = 0;
    uint16_t long_flag = 0;
    uint16_t long_pc_offset = 0;

    switch (op) {
    case OP_ADD: {
      // destination reg
      r0 = (instr >> 9) & 0x7;
      // first operand
      r1 = (instr >> 6) & 0x7;
      // immediate?
      imm_flag = (instr >> 5) & 0x1;

      if (imm_flag) {
        imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] + imm5;
      } else {
        r2 = instr & 0x7;
        reg[r0] = reg[r1] + reg[r2];
      }
      update_flags(r0);
    } break;
    case OP_AND: {
      r0 = (instr >> 9) & 7; // destination
      r1 = (instr >> 6) & 7; // sr1
      imm_flag = (instr >> 5) & 1;
      if (!imm_flag) {
        r2 = instr & 7;
        reg[r0] = reg[r1] & reg[r2];
      } else {
        imm5 = sign_extend(instr & 0x1F, 5);
        reg[r0] = reg[r1] & imm5;
      }
      update_flags(r0);
    } break;
    case OP_NOT: {
      r0 = (instr >> 9) & 7; // dest
      r1 = (instr >> 6) & 7; // src

      reg[r0] = ~reg[r1];
      update_flags(r0);
    } break;
    case OP_BR: {
      pc_offset = sign_extend(instr & 0x1FF, 9);
      cond_flag = (instr >> 9) & 7;
      if (cond_flag & reg[R_COND]) {
        reg[R_PC] += pc_offset;
      }
    } break;
    case OP_JMP: {
      r1 = (instr >> 6) & 7;
      reg[R_PC] = reg[r1];
    } break;
    case OP_JSR: {
      long_flag = (instr >> 11) & 1;
      reg[R_R7] = reg[R_PC];
      if (long_flag) {
        long_pc_offset = sign_extend(instr & 0x7FF, 11);
        reg[R_PC] += long_pc_offset;
      } else {
        r1 = (instr >> 6) & 0x7;
        reg[R_PC] = reg[r1];
      }
    } break;
    case OP_LD: {
      pc_offset = sign_extend(instr & 0x1ff, 9);
      r0 = (instr >> 9) & 7;
      reg[r0] = mem_read(reg[R_PC] + pc_offset);
      update_flags(r0);
    } break;
    case OP_LDI: {
      r0 = (instr >> 9) & 7;
      pc_offset = sign_extend(instr & 0x1FF, 9);
      reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
      update_flags(r0);
    } break;
    case OP_LDR: {
      r0 = (instr >> 9) & 7;
      r1 = (instr >> 6) & 7;
      offset = sign_extend(instr & 0x3F, 6);
      reg[r0] = mem_read(reg[r1] + offset);
      update_flags(r0);
    } break;
    case OP_LEA: {
      r0 = (instr >> 9) & 7;
      pc_offset = sign_extend(instr & 0x1FF, 9);
      reg[r0] = reg[R_PC] + pc_offset;
      update_flags(r0);
    } break;
    case OP_ST: {
      r0 = (instr >> 9) & 7;
      pc_offset = sign_extend(instr & 0x1ff, 9);
      mem_write(reg[R_PC] + pc_offset, reg[r0]);
    } break;
    case OP_STI: {
      r0 = (instr >> 9) & 7;
      pc_offset = sign_extend(instr & 0x1ff, 9);
      mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
    } break;
    case OP_STR: {
      r0 = (instr >> 9) & 7;
      r1 = (instr >> 6) & 7;
      offset = sign_extend(instr & 0x3f, 6);
      mem_write(reg[r1] + offset, reg[r0]);
    } break;
    case OP_TRAP:
      reg[R_R7] = reg[R_PC];

      switch (instr & 0xFF) {
      case TRAP_GETC:
        reg[R_R0] = (uint16_t)getchar();
        update_flags(R_R0);
        break;
      case TRAP_OUT:
        putc((char)reg[R_R0], stdout);
        fflush(stdout);
        break;
      case TRAP_PUTS: {
        uint16_t *c = memory + reg[R_R0];
        while (*c) {
          putc((char)*c, stdout);
          ++c;
        }
        fflush(stdout);
      } break;
      case TRAP_IN: {
        printf("Enter a character: ");
        char c = getchar();
        putc(c, stdout);
        fflush(stdout);
        reg[R_R0] = (uint16_t)c;
        update_flags(R_R0);
      } break;
      case TRAP_PUTSP: {
        uint16_t *c = memory + reg[R_R0];
        while (*c) {
          char char1 = (*c) & 0xFF;
          putc(char1, stdout);
          char char2 = (*c) >> 8;
          if (char2)
            putc(char2, stdout);
          ++c;
        }
        fflush(stdout);
      } break;
      case TRAP_HALT:
        puts("HALT");
        fflush(stdout);
        running = 0;
        break;
      }
      break;
    case OP_RES:
    case OP_RTI:
    default:
      abort();
      break;
    }
  }
  restore_input_buffering();
}

