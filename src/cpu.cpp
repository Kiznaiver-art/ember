#include "cpu.hpp"
#include "alu.hpp"
#include <iostream>
#include <iomanip>

// Опкод - это два поля: старший полубайт (группа) и младший (индекс).
struct Decoded { Byte group; Byte index; };

static Decoded decode(Byte op) {
    Decoded d;
    d.group = (op >> 4) & 0x0F;   // сдвиг убирает младшие 4 бита
    d.index = op & 0x0F;          // маска оставляет только младшие 4 бита
    return d;
}

// Размер инструкции в байтах (ISA.uk.md). 0 = такого опкода в ISA нет.
static std::size_t op_size(Byte op) {
    switch (op) {
    case 0x00: case 0x01: case 0x02: case 0x03:                    // HALT NOP OUT OUTN
    case 0x10: case 0x11: case 0x12: case 0x13: case 0x14:         // ADD SUB AND OR XOR
    case 0x15: case 0x16: case 0x17: case 0x18: case 0x19:         // NOT SHL SHR INC DEC
    case 0x1A:                                                     // CMP
    case 0x26: case 0x27:                                          // MOV
    case 0x29: case 0x2A: case 0x2B: case 0x2C:                    // [H], INCH, DECH
    case 0x2D:                                                     // HLOW
        return 1;
    case 0x20: case 0x21:                                          // LOADI
        return 2;
    case 0x22: case 0x23: case 0x24: case 0x25:                    // LOAD/STORE [addr16]
    case 0x28:                                                     // LOADH
    case 0x30: case 0x31: case 0x32: case 0x33: case 0x34:         // JMP JZ JNZ JC JNC
        return 3;
    default:
        return 0;
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
    std::size_t size = op_size(op);
    if (size == 0) {
        std::cout << "error: unknown opcode 0x" << std::hex << std::uppercase
                  << std::setw(2) << std::setfill('0') << (int)op
                  << std::dec << std::setfill(' ')
                  << " at address " << cpu.pc << "\n";
        cpu.halted = true;
        return;
    }
    if ((std::size_t)cpu.pc + size > MEM_SIZE) {
        std::cout << "error: instruction runs past the end of memory\n";
        cpu.halted = true;
        return;
    }

    Decoded d = decode(op);

    // Стрибок: сработал -> PC = адрес из операнда (размер НЕ добавляем);
    // не сработал -> обычный PC += 3.
    auto jump_if = [&](bool take) {
        if (take) cpu.pc = get16(*cpu.mem, cpu.pc + 1);
        else      cpu.pc += 3;
    };

    switch (d.group) {
    case 0x0:                                               // управление
        switch (d.index) {
        case 0x0: cpu.halted = true; cpu.pc += 1; break;                // HALT
        case 0x1: cpu.pc += 1; break;                                   // NOP
        case 0x2: std::cout << (char)cpu.a; cpu.pc += 1; break;        // OUT
        case 0x3: std::cout << (int)cpu.a << ' '; cpu.pc += 1; break;  // OUTN
        default:  break;
        }
        break;
         case 0x1:                                               // ALU
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
        case 0xA: (void)alu_sub(cpu.a, cpu.b, cpu.f);   break;  // CMP: флаги как у SUB, результат выбрасываем
        default:  break;
        }
        cpu.pc += 1;
        break;

    case 0x2:                                               // пересылка данных (флаги не трогаем)
        switch (d.index) {
        case 0x0: cpu.a = mem_get(*cpu.mem, cpu.pc + 1); cpu.pc += 2; break;                    // LOADI A, imm8
        case 0x1: cpu.b = mem_get(*cpu.mem, cpu.pc + 1); cpu.pc += 2; break;                    // LOADI B, imm8
        case 0x2: cpu.a = mem_get(*cpu.mem, get16(*cpu.mem, cpu.pc + 1)); cpu.pc += 3; break;   // LOAD A, [addr16]
        case 0x3: cpu.b = mem_get(*cpu.mem, get16(*cpu.mem, cpu.pc + 1)); cpu.pc += 3; break;   // LOAD B, [addr16]
        case 0x4: mem_set(*cpu.mem, get16(*cpu.mem, cpu.pc + 1), cpu.a); cpu.pc += 3; break;    // STORE [addr16], A
        case 0x5: mem_set(*cpu.mem, get16(*cpu.mem, cpu.pc + 1), cpu.b); cpu.pc += 3; break;    // STORE [addr16], B
        case 0x6: cpu.a = cpu.b; cpu.pc += 1; break;                                            // MOV A, B
        case 0x7: cpu.b = cpu.a; cpu.pc += 1; break;                                            // MOV B, A
        case 0x8: cpu.h = get16(*cpu.mem, cpu.pc + 1); cpu.pc += 3; break;                      // LOADH H, imm16
        case 0x9: cpu.a = mem_get(*cpu.mem, cpu.h); cpu.pc += 1; break;                         // LOAD A, [H]
        case 0xA: mem_set(*cpu.mem, cpu.h, cpu.a); cpu.pc += 1; break;                          // STORE [H], A
        case 0xB: cpu.h += 1; cpu.pc += 1; break;                                               // INCH
        case 0xC: cpu.h -= 1; cpu.pc += 1; break;                                               // DECH
        case 0xD: cpu.a = cpu.h & 0xFF; cpu.pc += 1; break;                                     // HLOW
        default:  break;
        }
        break;

    case 0x3:                                               // стрибки
        switch (d.index) {
        case 0x0: jump_if(true);      break;   // JMP
        case 0x1: jump_if(cpu.f.z);   break;   // JZ
        case 0x2: jump_if(!cpu.f.z);  break;   // JNZ
        case 0x3: jump_if(cpu.f.c);   break;   // JC  (opt)
        case 0x4: jump_if(!cpu.f.c);  break;   // JNC (opt)
        default:  break;
        }
        break;

    default:
        break;   // сюда не попасть: op_size уже отсёк неизвестные опкоды
    }
}

void dump_regs(const CPU& cpu) {
    std::cout << std::hex << std::uppercase << std::setfill('0')
              << "A=" << std::setw(2) << int(cpu.a)
              << " B=" << std::setw(2) << int(cpu.b)
              << " H=" << std::setw(4) << int(cpu.h)
              << " PC=" << std::setw(4) << int(cpu.pc)
              << std::dec << std::setfill(' ')
              << " Z=" << int(cpu.f.z)
              << " N=" << int(cpu.f.n)
              << " C=" << int(cpu.f.c) << "\n";
}