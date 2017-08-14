#include "./emu.h"

bool intel8080::zero(uint8_t b) { return b == 0; }
bool intel8080::sign(uint8_t b) { return ((b & 0x80) == 0x80); }
bool intel8080::carry(uint32_t b) { return b > 0xFFFF; }
// https://graphics.stanford.edu/~seander/bithacks.html#ParityMultiply
bool intel8080::parity(uint8_t b) {
  return !((((b * 0x0101010101010101ULL) & 0x8040201008040201ULL) % 0x1FF) & 1);
}

void intel8080::NOP() {
  pc += 1;
  cycles += 4;
}

void intel8080::LXI(uint8_t *reg1, uint8_t *reg2) {
  *reg1 = memory.at(pc + 2);
  *reg2 = memory.at(pc + 1);
  pc += 3;
  cycles += 10;
}

void intel8080::LXI(uint16_t *reg) {
  *reg = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
  pc += 3;
  cycles += 10;
}

void intel8080::DCR(uint8_t *reg, uint8_t opCycles) {
  (*reg)--;

  f.Z = zero(*reg);
  f.S = sign(*reg);
  f.P = parity(*reg);
  // Auxiliary Carry - NOT IMPLEMENTED

  pc += 1;
  cycles += opCycles;
}

void intel8080::STAX(const uint8_t *reg1, const uint8_t *reg2) {
  memory.at((*reg1 << 8) | *reg2) = A;
  pc += 1;
  cycles += 7;
}

void intel8080::INX(uint16_t *reg) {
  (*reg)++;
  pc += 1;
  cycles += 5;
}

void intel8080::INX(uint8_t *reg1, uint8_t *reg2) {
  uint16_t res = ((*reg1 << 8) | *reg2) + 1;
  *reg1 = res >> 8;
  *reg2 = res & 0x00FF;

  pc += 1;
  cycles += 5;
}

void intel8080::MOV(uint8_t *reg1, const uint8_t *reg2, uint8_t opCycles) {
  *reg1 = *reg2;
  pc += 1;
  cycles += opCycles;
}

void intel8080::INR(uint8_t *reg, uint8_t opCycles) {
  (*reg)++;

  f.Z = zero(*reg);
  f.S = sign(*reg);
  f.P = parity(*reg);
  // Auxiliary flag - NOT IMPLEMENTED

  pc += 1;
  cycles += opCycles;
}

void intel8080::DCX(uint16_t *reg) {
  (*reg)--;

  pc += 1;
  cycles += 5;
}

void intel8080::DCX(uint8_t *reg1, uint8_t *reg2) {
  uint16_t res = ((*reg1 << 8) | L) - 1;
  *reg1 = res >> 8;
  *reg2 = res & 0x00FF;

  pc += 1;
  cycles += 5;
}

void intel8080::MVI(uint8_t *reg, uint8_t opCycles) {
  *reg = memory.at(pc + 1);

  pc += 2;
  cycles += opCycles;
}

void intel8080::DAD(const uint8_t *reg1, const uint8_t *reg2) {
  uint16_t res = ((H << 8) | L) + ((*reg1 << 8) | *reg2);
  H = res >> 8;
  L = res & 0x00FF;

  pc += 1;
  cycles += 10;
}

void intel8080::DAD(const uint16_t *reg) {
  uint16_t res = ((H << 8) | L) + *reg;
  H = res >> 8;
  L = res & 0x00FF;

  pc += 1;
  cycles += 10;
}

void intel8080::LDAX(const uint8_t *reg1, const uint8_t *reg2) {
  A = memory.at((*reg1 << 8) | *reg2);
  pc += 1;
  cycles += 7;
}

void intel8080::ADD(const uint8_t *reg, uint8_t opCycles) {
  f.CY = A > (0xFF - *reg);

  A += *reg;

  f.Z = zero(*reg);
  f.S = sign(*reg);
  f.P = parity(*reg);
  // Auxiliary Carry - NOT IMPLEMENTED

  pc += 1;
  cycles += opCycles;
}

void intel8080::ADC(const uint8_t *reg, uint8_t opCycles) {
  f.CY = A > (0xFF - *reg);

  uint8_t CYValue = f.CY ? 1 : 0;
  A += *reg + CYValue;

  f.Z = zero(*reg);
  f.S = sign(*reg);
  f.P = parity(*reg);
  // Auxiliary Carry - NOT IMPLEMENTED

  pc += 1;
  cycles += opCycles;
}

void intel8080::SUB(const uint8_t *reg, uint8_t opCycles) {
  A -= *reg;

  f.Z = true;
  f.CY = false;
  // AC = false;
  f.P = true;
  f.S = false;

  cycles += opCycles;
  pc += 1;
}

// TODO: check CY
void intel8080::SBB(const uint8_t *reg, uint8_t opCycles) {
  auto CYValue = static_cast<uint8_t>(f.CY);

  uint16_t res = A - *reg - CYValue;

  A = res & 0x00ff;

  f.Z = zero(A);
  f.S = sign(A);
  f.P = parity(A);
  f.CY = res > 0xff;

  pc += 1;
  cycles += opCycles;
}

void intel8080::ANA(const uint8_t *reg, uint8_t opCycles) {
  A &= *reg;

  f.Z = zero(A);
  f.S = sign(A);
  f.P = parity(A);
  f.CY = false; // Carry bit is reset to zero
  // Auxiliary flag - NOT IMPLEMENTED

  pc += 1;
  cycles += opCycles;
}

void intel8080::XRA(const uint8_t *reg, uint8_t opCycles) {
  A ^= *reg;

  f.Z = zero(A);
  f.S = sign(A);
  f.P = parity(A);
  f.CY = false; // Carry bit is reset to zero
  // Auxiliary flag - NOT IMPLEMENTED

  pc += 1;
  cycles += opCycles;
}

void intel8080::ORA(const uint8_t *reg, uint8_t opCycles) {
  A |= *reg;

  f.Z = zero(A);
  f.S = sign(A);
  f.P = parity(A);
  // Auxiliary Carry - NOT IMPLEMENTED
  f.CY = false; // Carry bit reset

  pc += 1;
  cycles += opCycles;
}

void intel8080::CMP(const uint8_t *reg, uint8_t opCycles) {
  f.CY = A < *reg;

  uint8_t result = A - *reg;

  f.Z = zero(result);
  f.S = sign(result);
  f.P = parity(result);
  // Auxiliary Carry - NOT IMPLEMENTED

  pc += 1;
  cycles += opCycles;
}

void intel8080::ret(bool condition) {
  if (condition) {
    pc = (memory.at((sp + 1)) << 8) | memory.at(sp);
    sp += 2;
    cycles += 10;
  } else {
    pc += 1;
    cycles += 5;
  }
}

void intel8080::RST(const uint8_t num) {
  memory.at(sp - 1) = (pc >> 8) & 0xff;
  memory.at(sp - 2) = pc & 0xff;
  sp -= 2;

  pc = num;
  cycles += 11;
}

void intel8080::call(bool condition) {
  if (condition) {
    memory.at(sp - 1) = ((pc + 3) >> 8) & 0xFF;
    memory.at(sp - 2) = ((pc + 3) & 0xFF);
    sp -= 2;

    pc = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
    cycles += 17;
  } else {
    pc += 3;
    cycles += 11;
  }
}

void intel8080::enableDisableCY(bool operation) {
  f.CY = operation;

  pc += 1;
  cycles += 4;
}

void intel8080::enableDisableInterrupts(bool operation) {
  interrupts = operation;

  pc += 1;
  cycles += 4;
}

void intel8080::jump(bool condition) {
  if (condition) {
    pc = (memory.at(pc + 1) | memory.at(pc + 2) << 8);
  } else {
    pc += 3;
  }

  cycles += 10;
}

void intel8080::exchange(uint8_t *a1, uint8_t *a2, uint8_t *b1, uint8_t *b2,
                         uint8_t opCycles) {
  std::swap(*a1, *b1);
  std::swap(*a2, *b2);

  pc += 1;
  cycles += opCycles;
}

void intel8080::storeLoadHL(bool storing) {
  uint16_t adr = (memory.at(pc + 2) << 8) | memory.at(pc + 1);
  if (storing) { // Store
    memory.at(adr) = L;
    memory.at(adr + 1) = H;
  } else { // Load
    L = memory.at(adr);
    H = memory.at(adr + 1);
  }
  pc += 3;
  cycles += 16;
}

void intel8080::putHL(uint16_t *r) {
  /* if r is PC pc += 1 doesn't matter
   * if r is SP pc will advance */
  pc += 1;
  *r = (H << 8) | L;
  cycles += 5;
}
