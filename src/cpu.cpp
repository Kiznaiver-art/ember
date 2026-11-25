#include "cpu.hpp"
#include "alu.hpp"
#include <iostream>
#include <iomanip>
#include "screen.hpp"

// Опкоды ember (ISA.uk.md). enum class: имена не протекают наружу,
// а в число он сам не превращается - нужен явный static_cast.
enum class Op : Byte {
    HALT = 0x00, NOP = 0x01, OUT = 0x02, OUTN = 0x03,

    ADD = 0x10, SUB = 0x11, AND = 0x12, OR = 0x13, XOR = 0x14,
    NOT = 0x15, SHL = 0x16, SHR = 0x17, INC = 0x18, DEC = 0x19,
    CMP = 0x1A,

    LOADI_A = 0x20, LOADI_B = 0x21,
    LOAD_A = 0x22, LOAD_B = 0x23,
    STORE_A = 0x24, STORE_B = 0x25,
    MOV_A_B = 0x26, MOV_B_A = 0x27,
    LOADH = 0x28, LOAD_A_H = 0x29, STORE_H_A = 0x2A,
    INCH = 0x2B, DECH = 0x2C, HLOW = 0x2D,

    JMP = 0x30, JZ = 0x31, JNZ = 0x32, JC = 0x33, JNC = 0x34,

    CLS = 0x40, PLOT = 0x41, SHOW = 0x43
};

// Размер инструкции в байтах (ISA.uk.md). 0 = такого опкода в ISA нет.
static std::size_t op_size(Byte raw) {
    switch (static_cast<Op>(raw)) {
    case Op::HALT: case Op::NOP: case Op::OUT: case Op::OUTN:
    case Op::ADD: case Op::SUB: case Op::AND: case Op::OR: case Op::XOR:
    case Op::NOT: case Op::SHL: case Op::SHR: case Op::INC: case Op::DEC:
    case Op::CMP:
    case Op::MOV_A_B: case Op::MOV_B_A:
    case Op::LOAD_A_H: case Op::STORE_H_A: case Op::INCH: case Op::DECH:
    case Op::HLOW:
    case Op::CLS: case Op::PLOT: case Op::SHOW:
        return 1;
    case Op::LOADI_A: case Op::LOADI_B:
        return 2;
    case Op::LOAD_A: case Op::LOAD_B: case Op::STORE_A: case Op::STORE_B:
    case Op::LOADH:
    case Op::JMP: case Op::JZ: case Op::JNZ: case Op::JC: case Op::JNC:
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

    Byte raw = mem_get(*cpu.mem, cpu.pc);
    std::size_t size = op_size(raw);
    if (size == 0) {
        std::cout << "error: unknown opcode 0x" << std::hex << std::uppercase
                  << std::setw(2) << std::setfill('0') << (int)raw
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

    // Стрибок: сработал -> PC = адрес из операнда (размер НЕ добавляем);
    // не сработал -> обычный PC += 3.
    auto jump_if = [&](bool take) {
        if (take) cpu.pc = get16(*cpu.mem, cpu.pc + 1);
        else      cpu.pc += 3;
    };

    switch (static_cast<Op>(raw)) {
    // --- управление и вывод ---
    case Op::HALT: cpu.halted = true; cpu.pc += 1; break;
    case Op::NOP:  cpu.pc += 1; break;
    case Op::OUT:  std::cout << (char)cpu.a; cpu.pc += 1; break;
    case Op::OUTN: std::cout << (int)cpu.a << ' '; cpu.pc += 1; break;

    // --- ALU ---
    case Op::ADD: cpu.a = alu_add(cpu.a, cpu.b, cpu.f); cpu.pc += 1; break;
    case Op::SUB: cpu.a = alu_sub(cpu.a, cpu.b, cpu.f); cpu.pc += 1; break;
    case Op::AND: cpu.a = alu_and(cpu.a, cpu.b, cpu.f); cpu.pc += 1; break;
    case Op::OR:  cpu.a = alu_or (cpu.a, cpu.b, cpu.f); cpu.pc += 1; break;
    case Op::XOR: cpu.a = alu_xor(cpu.a, cpu.b, cpu.f); cpu.pc += 1; break;
    case Op::NOT: cpu.a = alu_not(cpu.a, cpu.f);        cpu.pc += 1; break;
    case Op::SHL: cpu.a = alu_shl(cpu.a, cpu.f);        cpu.pc += 1; break;
    case Op::SHR: cpu.a = alu_shr(cpu.a, cpu.f);        cpu.pc += 1; break;
    case Op::INC: cpu.a = alu_inc(cpu.a, cpu.f);        cpu.pc += 1; break;
    case Op::DEC: cpu.a = alu_dec(cpu.a, cpu.f);        cpu.pc += 1; break;
    case Op::CMP: (void)alu_sub(cpu.a, cpu.b, cpu.f);   cpu.pc += 1; break;  // флаги как у SUB, результат выбрасываем

    // --- пересылка данных (флаги не трогаем) ---
    case Op::LOADI_A: cpu.a = mem_get(*cpu.mem, cpu.pc + 1); cpu.pc += 2; break;
    case Op::LOADI_B: cpu.b = mem_get(*cpu.mem, cpu.pc + 1); cpu.pc += 2; break;
    case Op::LOAD_A:  cpu.a = mem_get(*cpu.mem, get16(*cpu.mem, cpu.pc + 1)); cpu.pc += 3; break;
    case Op::LOAD_B:  cpu.b = mem_get(*cpu.mem, get16(*cpu.mem, cpu.pc + 1)); cpu.pc += 3; break;
    case Op::STORE_A: mem_set(*cpu.mem, get16(*cpu.mem, cpu.pc + 1), cpu.a); cpu.pc += 3; break;
    case Op::STORE_B: mem_set(*cpu.mem, get16(*cpu.mem, cpu.pc + 1), cpu.b); cpu.pc += 3; break;
    case Op::MOV_A_B: cpu.a = cpu.b; cpu.pc += 1; break;
    case Op::MOV_B_A: cpu.b = cpu.a; cpu.pc += 1; break;
    case Op::LOADH:   cpu.h = get16(*cpu.mem, cpu.pc + 1); cpu.pc += 3; break;
    case Op::LOAD_A_H:  cpu.a = mem_get(*cpu.mem, cpu.h); cpu.pc += 1; break;
    case Op::STORE_H_A: mem_set(*cpu.mem, cpu.h, cpu.a); cpu.pc += 1; break;
    case Op::INCH: cpu.h += 1; cpu.pc += 1; break;
    case Op::DECH: cpu.h -= 1; cpu.pc += 1; break;
    case Op::HLOW: cpu.a = cpu.h & 0xFF; cpu.pc += 1; break;

    // --- стрибки ---
    case Op::JMP: jump_if(true);     break;
    case Op::JZ:  jump_if(cpu.f.z);  break;
    case Op::JNZ: jump_if(!cpu.f.z); break;
    case Op::JC:  jump_if(cpu.f.c);  break;   // opt
    case Op::JNC: jump_if(!cpu.f.c); break;   // opt

    // --- экран ---
    case Op::CLS:  clear_screen(*cpu.mem); cpu.pc += 1; break;
    case Op::PLOT: cpu.f.c = !plot(*cpu.mem, cpu.a, cpu.b); cpu.pc += 1; break;  // x=A, y=B; вне экрана -> C=1
    case Op::SHOW: show(*cpu.mem); cpu.pc += 1; break;

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