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

// Размер инструкции в байтах (из ISA.uk.md).
static std::size_t op_size(Byte op) {
    switch (op) {
    case 0x20: case 0x21:                        return 2;  // LOADI
    case 0x22: case 0x23: case 0x24: case 0x25:             // LOAD/STORE [addr16]
    case 0x28:                                   return 3;  // LOADH
    default:                                     return 1;
    }
}

void step(CPU& cpu) {
    if (cpu.halted) {
        std::cout << "halted\n";
        return;
    }
    if ((std::size_t)cpu.pc >= MEM_SIZE) {
        std::cout << "error: PC is outside memory\n";
        cpu.halted = true;
        return;
    }
    Byte op = mem_get(*cpu.mem, cpu.pc);
    if ((std::size_t)cpu.pc + op_size(op) > MEM_SIZE) {
        std::cout << "error: instruction runs past the end of memory\n";
        cpu.halted = true;
        return;
    }
    Decoded d = decode(op);

    switch (d.group) {
    case 0x0:                                   // управление
        switch (d.index) {
        case 0x0: cpu.halted = true; cpu.pc += 1; break;                  // HALT
        case 0x1: cpu.pc += 1; break;                                     // NOP
        case 0x2: std::cout << (char)cpu.a; cpu.pc += 1; break;           // OUT
        case 0x3: std::cout << (int)cpu.a << ' '; cpu.pc += 1; break;     // OUTN
        default:  cpu.pc += 1; break;
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
        cpu.pc += 1;
        break;

    case 0x2:                                   // пересылка данных (флаги не трогаем)
        switch (d.index) {
        case 0x0: cpu.a = mem_get(*cpu.mem, cpu.pc + 1); cpu.pc += 2; break;                        // LOADI A, imm8
        case 0x1: cpu.b = mem_get(*cpu.mem, cpu.pc + 1); cpu.pc += 2; break;                        // LOADI B, imm8
        case 0x2: cpu.a = mem_get(*cpu.mem, get16(*cpu.mem, cpu.pc + 1)); cpu.pc += 3; break;       // LOAD A, [addr16]
        case 0x3: cpu.b = mem_get(*cpu.mem, get16(*cpu.mem, cpu.pc + 1)); cpu.pc += 3; break;       // LOAD B, [addr16]
        case 0x4: mem_set(*cpu.mem, get16(*cpu.mem, cpu.pc + 1), cpu.a); cpu.pc += 3; break;        // STORE [addr16], A
        case 0x5: mem_set(*cpu.mem, get16(*cpu.mem, cpu.pc + 1), cpu.b); cpu.pc += 3; break;        // STORE [addr16], B
        case 0x6: cpu.a = cpu.b; cpu.pc += 1; break;                                                // MOV A, B
        case 0x7: cpu.b = cpu.a; cpu.pc += 1; break;                                                // MOV B, A
        case 0x8: cpu.h = get16(*cpu.mem, cpu.pc + 1); cpu.pc += 3; break;                          // LOADH H, imm16
        case 0x9: cpu.a = mem_get(*cpu.mem, cpu.h); cpu.pc += 1; break;                             // LOAD A, [H]
        case 0xA: mem_set(*cpu.mem, cpu.h, cpu.a); cpu.pc += 1; break;                              // STORE [H], A
        case 0xB: cpu.h += 1; cpu.pc += 1; break;                                                   // INCH
        case 0xC: cpu.h -= 1; cpu.pc += 1; break;                                                   // DECH
        default:  cpu.pc += 1; break;
        }
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
              << " H=" << cpu.h
              << '\n';
}
