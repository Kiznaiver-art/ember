#include "alu.hpp"

// Z и N зависят только от результата.
static std::uint8_t set_zn(std::uint8_t v, Flags& f) {
    f.z = (v == 0);
    f.n = (v & 0x80) != 0;
    return v;
}

std::uint8_t alu_add(std::uint8_t a, std::uint8_t b, Flags& f) {
    unsigned wide = a + b;                 // считаем в широком типе
    f.c = wide > 0xFF;                     // не влезло в 8 бит
    return set_zn(static_cast<std::uint8_t>(wide), f);
}
std::uint8_t alu_sub(std::uint8_t a, std::uint8_t b, Flags& f) {
    f.c = a < b;                           // заём
    return set_zn(static_cast<std::uint8_t>(a - b), f);
}
std::uint8_t alu_and(std::uint8_t a, std::uint8_t b, Flags& f) {
    f.c = false;
    return set_zn(static_cast<std::uint8_t>(a & b), f);
}
std::uint8_t alu_or(std::uint8_t a, std::uint8_t b, Flags& f) {
    f.c = false;
    return set_zn(static_cast<std::uint8_t>(a | b), f);
}
std::uint8_t alu_xor(std::uint8_t a, std::uint8_t b, Flags& f) {
    f.c = false;
    return set_zn(static_cast<std::uint8_t>(a ^ b), f);
}
std::uint8_t alu_not(std::uint8_t a, Flags& f) {
    f.c = false;
    return set_zn(static_cast<std::uint8_t>(~a), f);
}
std::uint8_t alu_shl(std::uint8_t a, Flags& f) {
    f.c = (a & 0x80) != 0;                 // бит, вышедший сверху
    return set_zn(static_cast<std::uint8_t>(a << 1), f);
}
std::uint8_t alu_shr(std::uint8_t a, Flags& f) {
    f.c = (a & 0x01) != 0;                 // бит, вышедший снизу
    return set_zn(static_cast<std::uint8_t>(a >> 1), f);
}
std::uint8_t alu_inc(std::uint8_t a, Flags& f) {   // C не трогаем
    return set_zn(static_cast<std::uint8_t>(a + 1), f);
}
std::uint8_t alu_dec(std::uint8_t a, Flags& f) {   // C не трогаем
    return set_zn(static_cast<std::uint8_t>(a - 1), f);
}
