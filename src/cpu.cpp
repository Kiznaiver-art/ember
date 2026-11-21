#include "cpu.hpp"
#include "alu.hpp"
#include <iostream>

// Опкод — это два поля: старший полубайт (группа) и младший (индекс).
struct Decoded { Byte group; Byte index; };

static Decoded decode(Byte op) {
    Decoded d;
    d.group = (op >> 4) & 0x0F;   // сдвиг убирает младшие 4 бита
    d.index = op & 0x0F;          // маска оставляет только младшие 4 бита
    return d;
}

void step(CPU& cpu) {
    if (cpu.halted) {
        std::cout << "halted\n";
        return;
    }
    Byte op = mem_get(*cpu.mem, cpu.pc);
    Decoded d = decode(op);

    switch (d.group) {
    case 0x0:                                   // управление
        switch (d.index) {
        case 0x0: cpu.halted = true; cpu.pc += 1; break;   // HALT
        case 0x1: cpu.pc += 1; break;                      // NOP
        default:  cpu.pc += 1; break;                      // пока как NOP
        }
        break;

    case 0x1:                                   // ALU
        switch (d.index) {
        case 0x0: cpu.a = alu_add(cpu.a, cpu.b, cpu.f); break;  // ADD
        case 0x1: cpu.a = alu_sub(cpu.a, cpu.b, cpu.f); break;  // SUB
        case 0x2: cpu.a = alu_and(cpu.a, cpu.b, cpu.f); break;  // AND
        case 0x3: cpu.a = alu_or (cpu.a, cpu.b, cpu.f); break;  // OR
        case 0x4: cpu.a = alu_xor(cpu.a, cpu.b, cpu.f); break;  // XOR
        case 0x5: cpu.a = alu_not(cpu.a, cpu.f);        break;  // NOT
        case 0x6: cpu.a = alu_shl(cpu.a, cpu.f);        break;  // SHL
        case 0x7: cpu.a = alu_shr(cpu.a, cpu.f);        break;  // SHR
        case 0x8: cpu.a = alu_inc(cpu.a, cpu.f);        break;  // INC
        case 0x9: cpu.a = alu_dec(cpu.a, cpu.f);        break;  // DEC
        default: break;                                         // CMP, MUL: другие лабы
        }
        cpu.pc += 1;                            // все ALU-инструкции занимают 1 байт
        break;

    default:
        cpu.pc += 1;                            // остальные группы: пока как NOP
        break;
    }
}

void dump_regs(const CPU& cpu) {
    std::cout << "PC=" << cpu.pc
              << " A=" << (int)cpu.a << " B=" << (int)cpu.b
              << " Z=" << cpu.f.z << " N=" << cpu.f.n << " C=" << cpu.f.c
              << '\n';
}