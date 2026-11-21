#pragma once
#include <cstdint>
#include "cpu.hpp"   // отсюда берём Flags

std::uint8_t alu_add(std::uint8_t a, std::uint8_t b, Flags& f);
std::uint8_t alu_sub(std::uint8_t a, std::uint8_t b, Flags& f);
std::uint8_t alu_and(std::uint8_t a, std::uint8_t b, Flags& f);
std::uint8_t alu_or (std::uint8_t a, std::uint8_t b, Flags& f);
std::uint8_t alu_xor(std::uint8_t a, std::uint8_t b, Flags& f);
std::uint8_t alu_not(std::uint8_t a, Flags& f);
std::uint8_t alu_shl(std::uint8_t a, Flags& f);
std::uint8_t alu_shr(std::uint8_t a, Flags& f);
std::uint8_t alu_inc(std::uint8_t a, Flags& f);
std::uint8_t alu_dec(std::uint8_t a, Flags& f);