#pragma once
#include "memory.hpp"

// Очистить видеопамять (256 байт с 0xA00).
void clear_screen(Memory& mem);

// Засветить пиксель (x, y). Возвращает false, если координаты вне экрана
// (тогда память не меняется).
bool plot(Memory& mem, int x, int y);

// Нарисовать видеопамять в терминале: '#' = горит, '.' = не горит.
void show(const Memory& mem);