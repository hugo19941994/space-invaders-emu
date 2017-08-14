#include "emu.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

// Macro to avoid verbose switch syntax
// https://gitlab.com/higan/higan/blob/master/higan/processor/mos6502/disassembler.cpp
#define op(id, oper)                                                           \
  case id:                                                                     \
    oper;                                                                      \
    break;

void intel8080::emulateCycle() {
  uint8_t *opcode = &memory.at(pc);

  switch (*opcode) {
    op(0x00, NOP());
    op(0x08, NOP());
    op(0x10, NOP());
    op(0x18, NOP());
    op(0x20, NOP());
    op(0x28, NOP());
    op(0x30, NOP());
    op(0x38, NOP());

    op(0x01, LXI(&B, &C));
    op(0x11, LXI(&D, &E));
    op(0x21, LXI(&H, &L));
    op(0x31, LXI(&sp));

    op(0x05, DCR(&B, 5));
    op(0x0d, DCR(&C, 5));
    op(0x15, DCR(&D, 5));
    op(0x1D, DCR(&E, 5));
    op(0x25, DCR(&H, 5));
    op(0x2D, DCR(&L, 5));
    op(0x35, DCR(memHL(), 10));
    op(0x3d, DCR(&A, 5));

    op(0x02, STAX(&B, &C));
    op(0x12, STAX(&D, &E));

    op(0x03, INX(&B, &C));
    op(0x13, INX(&D, &E));
    op(0x23, INX(&H, &L));
    op(0x33, INX(&sp));

    op(0x40, MOV(&B, &B, 5));
    op(0x41, MOV(&B, &C, 5));
    op(0x42, MOV(&B, &D, 5));
    op(0x43, MOV(&B, &E, 5));
    op(0x44, MOV(&B, &H, 5));
    op(0x45, MOV(&B, &L, 5));
    op(0x46, MOV(&B, memHL(), 7));
    op(0x47, MOV(&B, &A, 5));

    op(0x48, MOV(&C, &B, 5));
    op(0x49, MOV(&C, &C, 5));
    op(0x4A, MOV(&C, &D, 5));
    op(0x4B, MOV(&C, &E, 5));
    op(0x4C, MOV(&C, &H, 5));
    op(0x4D, MOV(&C, &L, 5));
    op(0x4E, MOV(&C, memHL(), 7));
    op(0x4F, MOV(&C, &A, 5));

    op(0x50, MOV(&D, &B, 5));
    op(0x51, MOV(&D, &C, 5));
    op(0x52, MOV(&D, &D, 5));
    op(0x53, MOV(&D, &E, 5));
    op(0x54, MOV(&D, &H, 5));
    op(0x55, MOV(&D, &L, 5));
    op(0x56, MOV(&D, memHL(), 7));
    op(0x57, MOV(&D, &A, 5));

    op(0x58, MOV(&E, &B, 5));
    op(0x59, MOV(&E, &C, 5));
    op(0x5A, MOV(&E, &D, 5));
    op(0x5B, MOV(&E, &E, 5));
    op(0x5C, MOV(&E, &H, 5));
    op(0x5D, MOV(&E, &L, 5));
    op(0x5E, MOV(&E, memHL(), 7));
    op(0x5F, MOV(&E, &A, 5));

    op(0x60, MOV(&H, &B, 5));
    op(0x61, MOV(&H, &C, 5));
    op(0x62, MOV(&H, &D, 5));
    op(0x63, MOV(&H, &E, 5));
    op(0x64, MOV(&H, &H, 5));
    op(0x65, MOV(&H, &L, 5));
    op(0x66, MOV(&H, memHL(), 7));
    op(0x67, MOV(&H, &A, 5));

    op(0x68, MOV(&L, &B, 5));
    op(0x69, MOV(&L, &C, 5));
    op(0x6A, MOV(&L, &D, 5));
    op(0x6B, MOV(&L, &E, 5));
    op(0x6C, MOV(&L, &H, 5));
    op(0x6D, MOV(&L, &L, 5));
    op(0x6E, MOV(&L, memHL(), 7));
    op(0x6F, MOV(&L, &A, 5));

    op(0x70, MOV(memHL(), &B, 7));
    op(0x71, MOV(memHL(), &C, 7));
    op(0x72, MOV(memHL(), &D, 7));
    op(0x73, MOV(memHL(), &E, 7));
    op(0x74, MOV(memHL(), &H, 7));
    op(0x75, MOV(memHL(), &L, 7));
    op(0x77, MOV(memHL(), &A, 7));

    op(0x78, MOV(&A, &B, 5));
    op(0x79, MOV(&A, &C, 5));
    op(0x7A, MOV(&A, &D, 5));
    op(0x7B, MOV(&A, &E, 5));
    op(0x7C, MOV(&A, &H, 5));
    op(0x7D, MOV(&A, &L, 5));
    op(0x7E, MOV(&A, memHL(), 7));
    op(0x7F, MOV(&A, &A, 5));

    op(0x04, INR(&B, 5));
    op(0x0C, INR(&C, 5));
    op(0x14, INR(&D, 5));
    op(0x1C, INR(&E, 5));
    op(0x24, INR(&H, 5));
    op(0x2C, INR(&L, 5));
    op(0x34, INR(memHL(), 10));
    op(0x3C, INR(&A, 5));

    op(0x0B, DCX(&B, &C));
    op(0x1B, DCX(&D, &E));
    op(0x2B, DCX(&H, &L));
    op(0x3B, DCX(&sp));

    op(0x06, MVI(&B, 7));
    op(0x0E, MVI(&C, 7));
    op(0x16, MVI(&D, 7));
    op(0x1E, MVI(&E, 7));
    op(0x26, MVI(&H, 7));
    op(0x2E, MVI(&L, 7));
    op(0x36, MVI(memHL(), 10));
    op(0x3E, MVI(&A, 7));

    op(0x09, DAD(&B, &C));
    op(0x19, DAD(&D, &E));
    op(0x29, DAD(&H, &L));
    op(0x39, DAD(&sp));

    op(0x0A, LDAX(&B, &C));
    op(0x1A, LDAX(&D, &E));

    op(0x80, ADD(&B, 4));
    op(0x81, ADD(&C, 4));
    op(0x82, ADD(&D, 4));
    op(0x83, ADD(&E, 4));
    op(0x84, ADD(&H, 4));
    op(0x85, ADD(&L, 4));
    op(0x86, ADD(memHL(), 7));
    op(0x87, ADD(&A, 4));

    op(0x88, ADC(&B, 4));
    op(0x89, ADC(&C, 4));
    op(0x8A, ADC(&D, 4));
    op(0x8B, ADC(&E, 4));
    op(0x8C, ADC(&H, 4));
    op(0x8D, ADC(&L, 4));
    op(0x8E, ADC(memHL(), 7));
    op(0x8F, ADC(&A, 4));

    op(0x90, SUB(&B, 4));
    op(0x91, SUB(&C, 4));
    op(0x92, SUB(&D, 4));
    op(0x93, SUB(&E, 4));
    op(0x94, SUB(&H, 4));
    op(0x95, SUB(&L, 4));
    op(0x96, SUB(memHL(), 7));
    op(0x97, SUB(&A, 4));

    op(0x98, SBB(&B, 4));
    op(0x99, SBB(&C, 4));
    op(0x9A, SBB(&D, 4));
    op(0x9B, SBB(&E, 4));
    op(0x9C, SBB(&H, 4));
    op(0x9D, SBB(&L, 4));
    op(0x9E, SBB(memHL(), 7));
    op(0x9F, SBB(&A, 4));

    op(0xA0, ANA(&B, 4));
    op(0xA1, ANA(&C, 4));
    op(0xA2, ANA(&D, 4));
    op(0xA3, ANA(&E, 4));
    op(0xA4, ANA(&H, 4));
    op(0xA5, ANA(&L, 4));
    op(0xA6, ANA(memHL(), 7));
    op(0xA7, ANA(&A, 4));

    op(0xA8, XRA(&B, 4));
    op(0xA9, XRA(&C, 4));
    op(0xAA, XRA(&D, 4));
    op(0xAB, XRA(&E, 4));
    op(0xAC, XRA(&H, 4));
    op(0xAD, XRA(&L, 4));
    op(0xAE, XRA(memHL(), 7));
    op(0xAF, XRA(&A, 4));

    op(0xB0, ORA(&B, 4));
    op(0xB1, ORA(&C, 4));
    op(0xB2, ORA(&D, 4));
    op(0xB3, ORA(&E, 4));
    op(0xB4, ORA(&H, 4));
    op(0xB5, ORA(&L, 4));
    op(0xB6, ORA(memHL(), 7));
    op(0xB7, ORA(&A, 4));

    op(0xB8, CMP(&B, 4));
    op(0xB9, CMP(&C, 4));
    op(0xBA, CMP(&D, 4));
    op(0xBB, CMP(&E, 4));
    op(0xBC, CMP(&H, 4));
    op(0xBD, CMP(&L, 4));
    op(0xBE, CMP(memHL(), 7));
    op(0xBF, CMP(&A, 4));

    op(0xC2, jump(!f.Z));   // JNZ
    op(0xC3, jump(true)); // JMP
    op(0xCA, jump(f.Z));    // JZ
    op(0xCB, jump(true)); // JZ
    op(0xD2, jump(!f.CY));  // JNC
    op(0xDA, jump(f.CY));   // JC
    op(0xE2, jump(!f.P));   // JNC
    op(0xEA, jump(f.P));    // JNC
    op(0xF2, jump(!f.S));   // JM
    op(0xFA, jump(f.S));    // JM

    op(0xC0, ret(!f.Z));
    op(0xC8, ret(f.Z));
    op(0xC9, ret(true));
    op(0xD0, ret(!f.CY));
    op(0xD8, ret(f.CY));
    op(0xD9, ret(true));
    op(0xE0, ret(!f.P));
    op(0xE8, ret(f.P));
    op(0xF0, ret(!f.S));
    op(0xFF, ret(f.S));

    op(0xC4, call(!f.Z));
    op(0xCC, call(f.Z));
    op(0xCD, call(true));
    op(0xD4, call(!f.CY));
    op(0xDC, call(f.CY));
    op(0xDD, call(true));
    op(0xE4, call(!f.P));
    op(0xEC, call(f.P));
    op(0xED, call(true));
    op(0xF4, call(!f.S));
    op(0xFC, call(f.S));
    op(0xFD, call(true));

    op(0xC1, POP(&B, &C));
    op(0xD1, POP(&D, &E));
    op(0xE1, POP(&H, &L));
    op(0xF1, POP(&A, &f));

    op(0xC5, PUSH(&B, C));
    op(0xD5, PUSH(&D, E));
    op(0xE5, PUSH(&H, L));
    op(0xF5, PUSH(&A, f.psw()));

    op(0xEB, exchange(&H, &L, &D, &E, 5));                              // XCHG
    op(0xE3, exchange(&H, &L, &memory.at(sp + 1), &memory.at(sp), 18)); // XTHL

    op(0x22, storeLoadHL(true));  // SHLD
    op(0x2A, storeLoadHL(false)); // LHLD

    op(0xE9, putHL(&pc));
    op(0xF9, putHL(&sp));

    op(0x37, enableDisableCY(true));  // STC
    op(0x3F, enableDisableCY(false)); // CMC

    op(0xFB, enableDisableInterrupts(true));  // EI
    op(0xF3, enableDisableInterrupts(false)); // DI

  case (0x2f): // CMA
    A = ~A;

    cycles += 4;
    pc += 1;
    break;

  case (0x1f): { // RAR
    auto CYValue = static_cast<uint8_t>(f.CY);
    uint16_t result = (CYValue << 7) | (A >> 1);

    f.CY = static_cast<bool>(A & 0x1);

    A = result & 0x00FF;

    pc += 1;
    cycles += 4;
    break;
  }

  case (0x0f): { // RRC
    uint16_t result = ((A & 0x1) << 7) | (A >> 1);

    f.CY = static_cast<bool>(A & 0x1);

    A = result & 0x00FF;

    pc += 1;
    cycles += 4;
    break;
  }

  case (0x32): { // STA adr
    // (adr) <- A
    uint16_t adr = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
    memory.at(adr) = A;

    pc += 3;
    cycles += 13;
    break;
  }

  case (0x3a): { // LDA adr
    // A <- (adr)
    uint16_t adr = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
    A = memory.at(adr);

    pc += 3;
    cycles += 13;
    break;
  }

  case (0xc6): // ADI D8
    // A <- A + byte
    f.CY = A > (0xFF - memory.at(pc + 1));

    A += memory.at(pc + 1);

    f.Z = zero(A);
    f.S = sign(A);
    f.P = parity(A);
    // Auxiliary flag - NOT IMPLEMENTED

    pc += 2;
    cycles += 7;
    break;

  case (0xdb): { // IN para input
    if (memory.at(pc + 1) == 0x01) {
      A = Read0;
    } else if (memory.at(pc + 1) == 0x02) {
      A = Read1;
    } else if (memory.at(pc + 1) == 0x03) {
      int dwval = (shift1 << 8) | shift0;
      A = dwval >> (8 - noOfBitsToShift);
    }

    pc += 2;
    cycles += 10;
    break;
  }

  case (0xd3): // OUT D8
    if (memory.at(pc + 1) == 0x02) {
      noOfBitsToShift = A & 0x7;
    } else if (memory.at(pc + 1) == 0x04) {
      shift0 = shift1;
      shift1 = A;
    }

    pc += 2;
    cycles += 10;
    break;

  case (0xe6): // ANI D8
    // A <-A & data
    A &= memory.at(pc + 1);

    f.Z = zero(A);
    f.S = sign(A);
    f.P = parity(A);
    f.CY = false; // Carry bit is reset to zero
    // Auxiliary flag - NOT IMPLEMENTED

    pc += 2;
    cycles += 7;
    break;

  case (0xfe): { // CPI D8
    uint8_t res = A - memory.at(pc + 1);

    f.CY = A < memory.at(pc + 1);
    f.Z = zero(res);
    f.S = sign(res);
    f.P = parity(res);

    pc += 2;
    cycles += 7;
    break;
  }

  case (0x27): { // DAA
    uint8_t ls = A & 0xf;
    if (ls > 9) { // Or AC
      A += 6;
    }

    uint8_t ms = (A & 0xf0) >> 4;
    if (ms > 9 || f.CY) {
      ms += 6;

      f.CY = ms > 0xf;

      ms &= 0xf;
      A &= 0xf;
      A |= (ms << 4);
    }

    f.Z = zero(A);
    f.S = sign(A);
    f.P = parity(A);

    pc += 1;
    cycles += 4;
    break;
  }

  case (0x17): { // RAL
    auto CYValue = static_cast<uint8_t>(f.CY);
    uint16_t result = (A << 1) | CYValue;

    f.CY = static_cast<bool>((A & 0x80) >> 7);

    A = result & 0x00FF;

    pc += 1;
    cycles += 4;
    break;
  }

  case (0x07): { // RLC
    uint16_t result = (A << 1) | ((A & 0x08) >> 7);

    f.CY = static_cast<bool>((A & 0x80) >> 7);

    A = result & 0x00FF;

    pc += 1;
    cycles += 4;
    break;
  }

  case (0xf6): // ORI d8
    f.CY = A > (0xFF - memory.at(pc + 1));

    A |= memory.at(pc + 1);

    f.Z = zero(A);
    f.S = sign(A);
    f.P = parity(A);
    // Auxiliary flag - NOT IMPLEMENTED

    pc += 2;
    cycles += 7;
    break;

  case (0xd6): // SUI d8
    // Carry flag
    f.CY = A < memory.at(pc + 1);

    A -= memory.at(pc + 1);

    f.Z = zero(A);
    f.S = sign(A);
    f.P = parity(A);
    // Auxiliary flag - NOT IMPLEMENTED

    pc += 2;
    cycles += 7;
    break;

  case (0xde): { // SBI d8
    auto CYValue = static_cast<uint8_t>(f.CY);
    uint16_t result = memory.at(pc + 1) + CYValue;

    f.CY = A < result;

    A -= result;

    f.Z = zero(A);
    f.S = sign(A);
    f.P = parity(A);
    // Auxiliary flag - NOT IMPLEMENTED

    pc += 2;
    cycles += 7;
    break;
  }

  default:
    std::cout << "ERROR " << std::bitset<8>(*opcode) << std::endl;
    cycles += 4;
    break;
  }
}
#undef op
