// main.cpp — the prompt.
//
// GIVEN, in full. This file is scaffolding: it reads a line, splits it into
// words, and calls one of your functions. It uses `while`, `if` and functions,
// which the course only explains properly in Labs 4 and 7. That is on purpose —
// in Lab 1 you read this file, you do not write it.
//
// What you add in later labs is one more `else if` branch per command.
#include <iostream>
#include <sstream>
#include <string>
#include "cpu.hpp"
#include "dump.hpp"
#include "memory.hpp"
#include "alu.hpp"

// Turn a word into a number. Accepts decimal (65) and hex (0x41).
// Returns false if the word is not a number at all.
static bool parse_number(const std::string& word, long& out) {
    try {
        std::size_t used = 0;
        // base 0 means: look at the prefix. "0x41" is hex, "65" is decimal.
        out = std::stol(word, &used, 0);
        return used == word.size();  // reject things like "12abc"
    } catch (...) {
        return false;
    }
}

static void print_help() {
    std::cout << "commands:\n"
              << "  dump              print all " << MEM_SIZE << " bytes\n"
              << "  get <addr>        show one byte four ways\n"
              << "  set <addr> <val>  write one byte (dec or 0x hex)\n"
              << "  help              this list\n"
              << "  find <lo> <hi> <byte>  first address in [lo,hi) holding byte\n"
              << "  quit              leave\n";
}
// Линейный поиск: первый i в [lo, hi), где mem[i] == needle; промах -> hi.
static std::uint16_t find(const Memory& mem, std::uint16_t lo, std::uint16_t hi, Byte needle) {
    for (std::uint16_t i = lo; i < hi; ++i) {
        if (mem_get(mem, i) == needle) return i;
    }
    return hi;
}
int main() {
    Memory mem;  // 4096 bytes, on the stack, zeroed by the {} in memory.hpp
CPU cpu;
cpu.mem = &mem;
    std::cout << "ember 0.1 - 4096 bytes of memory you can see. Type `help`.\n";

    std::string line;
    while (true) {
        std::cout << "ember> ";

        // getline reads one whole line. It returns false at end of input
        // (Ctrl-D), which is how the loop ends if you never type `quit`.
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }

        // Split the line into words: "set 0 65" -> cmd="set", args "0" and "65".
        std::istringstream words(line);
        std::string cmd;
        words >> cmd;

        if (cmd.empty()) {
            continue;  // the user just pressed Enter
        } else if (cmd == "quit" || cmd == "exit") {
            break;
        } else if (cmd == "help") {
            print_help();
        } else if (cmd == "dump") {
            dump(mem);
        } else if (cmd == "get") {
            std::string a;
            long addr = 0;
            if (!(words >> a) || !parse_number(a, addr)) {
                std::cout << "usage: get <addr>\n";
            } else if (addr < 0) {
                std::cout << "address must not be negative\n";
            } else {
                show_byte(mem_get(mem, (std::size_t)addr));
            }
        } else if (cmd == "set") {
            std::string a, v;
            long addr = 0, value = 0;
            if (!(words >> a) || !(words >> v) || !parse_number(a, addr) ||
                !parse_number(v, value)) {
                std::cout << "usage: set <addr> <value>\n";
            } else if (addr < 0) {
                std::cout << "address must not be negative\n";
            } else if (value < 0 || value > 255) {
                // A cell holds ONE byte. 256 does not fit. Lab 1, theory 3.
                std::cout << "a byte is 0..255, got " << value << '\n';
            } else if (!mem_set(mem, (std::size_t)addr, (Byte)value)) {
                std::cout << "address " << addr << " is outside 0.." << MEM_SIZE - 1
                          << '\n';
            }} else if (cmd == "reg") {
    std::string r, v;
    long value = 0;
    if (!(words >> r) || !(words >> v) || !parse_number(v, value) || value < 0 ||
        value > 255) {
        std::cout << "usage: reg <a|b> <0..255>\n";
    } else if (r == "a") {
        cpu.a = (Byte)value;
    } else if (r == "b") {
        cpu.b = (Byte)value;
    } else {
        std::cout << "unknown register: " << r << '\n';
    }
} else if (cmd == "regs") {
    dump_regs(cpu);
} else if (cmd == "step") {
            step(cpu);
    } else if (cmd == "run") {
        int steps = 0;
        while (!cpu.halted && steps < 100000) {
            step(cpu);
            ++steps;
        }
        if (!cpu.halted) {
            std::cout << "\nrun stopped after " << steps << " steps (limit)\n";
        } else {
            std::cout << "\n[" << steps << " steps]\n";
        }
    } else if (cmd == "set16") {
        std::string a, v;
        long addr = 0, value = 0;
        if (!(words >> a) || !(words >> v) || !parse_number(a, addr) || !parse_number(v, value)) {
            std::cout << "usage: set16 <addr> <value>\n";
        } else if (addr < 0) {
            std::cout << "address must not be negative\n";
        } else if (value < 0 || value > 65535) {
            std::cout << "a 16-bit value is 0..65535, got " << value << '\n';
        } else if (!set16(mem, (std::size_t)addr, (std::uint16_t)value)) {
            std::cout << "set16 needs two cells inside 0.." << MEM_SIZE - 1 << '\n';
        }
    } else if (cmd == "get16") {
        std::string a;
        long addr = 0;
        if (!(words >> a) || !parse_number(a, addr)) {
            std::cout << "usage: get16 <addr>\n";
        } else if (addr < 0 || (std::size_t)addr + 1 >= MEM_SIZE) {
            std::cout << "get16 needs two cells inside 0.." << MEM_SIZE - 1 << '\n';
        } else {
            std::uint16_t v = get16(mem, (std::size_t)addr);
            std::cout << v << "  0x" << std::hex << v << std::dec << '\n';
        }
            } else if (cmd == "find") {
        std::string a, b, n;
        long lo = 0, hi = 0, needle = 0;
        if (!(words >> a) || !(words >> b) || !(words >> n) ||
            !parse_number(a, lo) || !parse_number(b, hi) || !parse_number(n, needle)) {
            std::cout << "usage: find <lo> <hi> <byte>\n";
        } else if (lo < 0 || hi < lo || (std::size_t)hi > MEM_SIZE) {
            std::cout << "range must satisfy 0 <= lo <= hi <= " << MEM_SIZE << '\n';
        } else if (needle < 0 || needle > 255) {
            std::cout << "a byte is 0..255, got " << needle << '\n';
        } else {
            std::uint16_t r = find(mem, (std::uint16_t)lo, (std::uint16_t)hi, (Byte)needle);
            if (r == hi) {
                std::cout << "not found\n";
            } else {
                std::cout << "found at " << r << "  0x" << std::hex << r << std::dec << '\n';
            }
        }
    } else if (cmd == "alu") {
    std::string op, sa, sb;
    long x = 0, y = 0;
    if (!(words >> op) || !(words >> sa) || !parse_number(sa, x) || x < 0 || x > 255) {
        std::cout << "usage: alu <op> <a> [b]\n";
    } else if ((op == "add" || op == "sub" || op == "and" || op == "or" || op == "xor") &&
               (!(words >> sb) || !parse_number(sb, y) || y < 0 || y > 255)) {
        std::cout << "usage: alu " << op << " <a> <b>\n";
    } else {
        Flags f;
        Byte r = 0, ua = (Byte)x, ub = (Byte)y;
        bool known = true;
        if      (op == "add") r = alu_add(ua, ub, f);
        else if (op == "sub") r = alu_sub(ua, ub, f);
        else if (op == "and") r = alu_and(ua, ub, f);
        else if (op == "or")  r = alu_or (ua, ub, f);
        else if (op == "xor") r = alu_xor(ua, ub, f);
        else if (op == "not") r = alu_not(ua, f);
        else if (op == "shl") r = alu_shl(ua, f);
        else if (op == "shr") r = alu_shr(ua, f);
        else if (op == "inc") r = alu_inc(ua, f);
        else if (op == "dec") r = alu_dec(ua, f);
        else known = false;
        if (!known) {
            std::cout << "unknown alu op: " << op << '\n';
        } else {
            std::cout << "result=" << (int)r << "  Z=" << f.z << " N=" << f.n
                      << " C=" << f.c << '\n';
        }
    }
        } else {
            std::cout << "unknown command: " << cmd << " (try `help`)\n";
        }
    }

    return 0;  // 0 means "success" to the operating system
}




