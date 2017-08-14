#ifndef emu_h
#define emu_h
#include <array>
#include <cstdint>

struct intel8080 {
  uint16_t pc, sp;
  uint32_t cycles;
  uint8_t A, B, C, D, E, H, L;
  bool interrupts;
  std::array<uint8_t, 65536> memory;

  // Access memory[HL]
  uint8_t* memHL() {
    return &memory.at((H << 8) | L);
  }

  struct Flags {
    bool Z, S, P, CY, AC;

    uint8_t psw() {
      uint8_t res = 0b00000010;
      if (CY) {
        res |= 0b00000001;
      }
      if (P) {
        res |= 0b00000100;
      }
      if (AC) {
        res |= 0b00010000;
      }
      if (Z) {
        res |= 0b01000000;
      }
      if (S) {
        res |= 0b10000000;
      }
      return res;
    }

    void operator=(uint8_t a) {
      CY = static_cast<bool>(a & 0x01);
      P = static_cast<bool>((a >> 2) & 0x01);
      AC = static_cast<bool>((a >> 4) & 0x01);
      Z = static_cast<bool>((a >> 6) & 0x01);
      S = static_cast<bool>((a >> 7) & 0x01);
    }
  } f;

  /* Interrupts: $cf (RST 0x08) at the start of vblank
   * $d7 (RST 0x10) at the end of vblank.
   */

  // Ports
  uint8_t Read0 = 0x00;
  uint8_t Read1 = 0b10000011;

  // Hardware shifter
  uint16_t noOfBitsToShift = 0x00;
  uint32_t shift0 = 0;
  uint32_t shift1 = 0;

  // dispatcher.cpp
  void emulateCycle();

  // cpu.cpp
  bool parity(uint8_t b);
  bool zero(uint8_t b);
  bool sign(uint8_t b);
  bool carry(uint32_t b);

  auto NOP() -> void;
  void LXI(uint8_t *reg1, uint8_t *reg2);
  void LXI(uint16_t *reg);
  void DCR(uint8_t *reg, uint8_t opCycles);
  void STAX(const uint8_t *reg1, const uint8_t *reg2);
  void INX(uint16_t *reg);
  void INX(uint8_t *reg1, uint8_t *reg2);
  void MOV(uint8_t *reg1, const uint8_t *reg2, uint8_t opCycles);
  void INR(uint8_t *reg, uint8_t opCycles);
  void DCX(uint16_t *reg);
  void DCX(uint8_t *reg1, uint8_t *reg2);
  void MVI(uint8_t *reg, uint8_t opCycles);
  void DAD(const uint8_t *reg1, const uint8_t *reg2);
  void DAD(const uint16_t *reg);
  void LDAX(const uint8_t *reg1, const uint8_t *reg2);
  void ADD(const uint8_t *reg, uint8_t opCycles);
  void ADC(const uint8_t *reg, uint8_t opCycles);
  void SUB(const uint8_t *reg, uint8_t opCycles);
  void SBB(const uint8_t *reg, uint8_t opCycles);
  void ANA(const uint8_t *reg, uint8_t opCycles);
  void XRA(const uint8_t *reg, uint8_t opCycles);
  void ORA(const uint8_t *reg, uint8_t opCycles);
  void CMP(const uint8_t *reg, uint8_t opCycles);

  void ret(bool condition);
  void RST(const uint8_t num);
  void call(bool condition);
  void enableDisableCY(bool operation);
  void enableDisableInterrupts(bool operation);
  void jump(bool condition);
  void exchange(uint8_t *a1, uint8_t *a2, uint8_t *b1, uint8_t *b2,
                uint8_t opCycles);
  void storeLoadHL(bool storing);
  void putHL(uint16_t *r);

  template <typename T> void PUSH(const uint8_t *reg1, T reg2) {
    memory.at(sp - 1) = *reg1;
    memory.at(sp - 2) = reg2;
    sp = sp - 2;

    pc += 1;
    cycles += 11;
  }

  template <typename T> void POP(uint8_t *reg1, T *reg2) {
    *reg1 = memory.at(sp + 1);
    *reg2 = memory.at(sp);
    sp += 2;

    pc += 1;
    cycles += 10;
  }
};

#endif /* emu_h */
