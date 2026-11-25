#include "screen.hpp"

#include <iostream>
#include <string>

static const int VRAM_LO = 0xA00;
static const int VRAM_SIZE = 256;
static const int SCREEN_W = 64;
static const int SCREEN_H = 32;

void clear_screen(Memory& mem) {
    for (int a = VRAM_LO; a < VRAM_LO + VRAM_SIZE; ++a) {
        mem.data[a] = 0;
    }
}

bool plot(Memory& mem, int x, int y) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) {
        return false;
    }
    int addr = VRAM_LO + y * 8 + x / 8;
    int bit = 7 - (x % 8);
    mem.data[addr] |= (1 << bit);
    return true;
}

void show(const Memory& mem) {
    std::cout << '+' << std::string(SCREEN_W, '-') << "+\n";
    for (int y = 0; y < SCREEN_H; ++y) {
        std::cout << '|';
        for (int x = 0; x < SCREEN_W; ++x) {
            int b = mem.data[VRAM_LO + y * 8 + x / 8];
            int bit = 7 - (x % 8);
            std::cout << (((b >> bit) & 1) ? '#' : '.');
        }
        std::cout << "|\n";
    }
    std::cout << '+' << std::string(SCREEN_W, '-') << "+\n";
}