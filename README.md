# `ember` — starter skeleton (Lab 1)

Copy this folder to a repository of your own, `git init`, and start from **M1** of
[Lab 01](../lab-01-a-box-of-bytes.md). It builds and runs as-is.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/ember
```

## Why a skeleton exists

Lab 1 is about **bytes and types**. It is not about `while` loops, splitting a
string into words, or `std::setw` — you meet those properly in Labs 4, 5 and 7.
So the parts that need them are given to you, fully written and commented. You
read those. You write the four small things that *are* Lab 1.

## Given — read it, don't rewrite it

| File | What it does |
|---|---|
| `CMakeLists.txt` | C++17, `-Wall -Wextra -Werror`, ASan + UBSan on Debug |
| `src/main.cpp` | the prompt: read a line, split it into words, call your functions |
| `src/memory.hpp` | `Byte`, `MEM_SIZE`, `struct Memory` — the box |
| `src/dump.hpp` | the two declarations |
| `src/dump.cpp` → `dump()` | the hex dump loop |

## Yours — four `TODO(lab-01)` markers

```bash
grep -rn "TODO(lab-01)" src/
```

| # | Where | The job |
|---|---|---|
| 1 | `memory.cpp` → `mem_get` | return the byte, or 0 if the address is outside the box |
| 2 | `memory.cpp` → `mem_set` | write the byte, or return `false` if the address is outside |
| 3 | `dump.cpp` → the ASCII gutter | print the character when the byte is printable |
| 4 | `dump.cpp` → `show_byte` | one byte, four views |

When all four are done:

```txt
ember> set 0 65
ember> set 1 66
ember> get 0
65  0x41  0b01000001  'A'
ember> dump
0000  41 42 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |AB..............|
```

That is M2 and M3 of Lab 1. M4 (the three deliberate breakages) is in the lab.

Until you implement `mem_set`, `set` accepts everything and stores nothing, and
`get` prints `show_byte: not implemented yet`. That is the starting state, not a
bug.

## Later labs

You keep this repository for all eight labs. Every lab adds one `else if` branch
to the dispatcher in `main.cpp` and one or two new files next to these.

---

## Українською

Скопіюйте цю теку у свій репозиторій — вона вже збирається й запускається.

Lab 1 — про **байти й типи**, а не про цикли, розбір рядка на слова чи
форматування виводу (це Labs 4, 5, 7). Тому все, що потребує ще не пройденого,
вам **дано** — з коментарями, щоб читати. Ви пишете чотири маленькі речі, які й
є Lab 1: `mem_get`, `mem_set`, ASCII-колонку в дампі та `show_byte`.

Знайти свою роботу: `grep -rn "TODO(lab-01)" src/`.

Якщо C++ бачите вперше — спочатку
[C++ за годину](../cpp-survival-kit.notes.md), потім
[інструменти й git](../setup.notes.md).
## Результаты

### sizeof

| Тип | Байт |
|---|---|
| char | 1 |
| short | 2 |
| int | 4 |
| long | 8 |
| long long | 8 |
| float | 4 |
| double | 8 |
| void* | 8 |
| size_t | 8 |

### Поломка 1: байт заворачивается

```
255 + 1 -> 0, 0 - 1 -> 255
```

Байт хранит 8 бит, то есть значения 0..255. После 255 счёт идёт по кругу: `255 + 1` даёт `0`, а `0 - 1` даёт `255`.

### Поломка 2: 0.1 + 0.2 != 0.3

```
0.1 + 0.2 == 0.3: 0, value 0.30000000000000004
```

Числа 0.1 и 0.2 нельзя точно записать в двоичной дроби, поэтому сумма чуть отличается от 0.3, а сравнение `==` даёт ложь.

### Поломка 3: строка "AB\0" в дампе

```
0000  41 42 00 0a ff 00 00 00 00 00 00 00 00 00 00 00  |AB.............|
```

Нулевой байт (`00`) в ASCII-колонке рисуется точкой, как любой непечатный байт. Поэтому по дампу не видно, что строка "AB" на нём кончилась.

### Поломка 4: память без {}

Убрал `{}` у `Byte data[MEM_SIZE]`, пересобрал и сделал `dump`:

```
0000  90 db 20 00 00 00 00 00 90 db 20 00 00 00 00 00
0010  90 db 20 00 00 00 00 00 20 00 00 00 00 00 00 00
0020  20 00 00 00 00 00 00 00 04 00 00 00 00 00 00 00
0030  07 00 00 00 04 00 00 00 98 eb 20 00 00 00 00 00
```

Без `{}` массив не обнуляется: память на стеке достаётся такой, какой её оставил предыдущий код, поэтому в дампе мусор. Чтение такой памяти — неопределённое поведение, поэтому `{}` возвращён.
## Lab 02

### Ловушка приоритета операций

```
flags & 4 == 4        -> 0
(flags & 4) == 4      -> 1
op & 0xF0 >> 4        -> 6
(op >> 4) & 0x0F      -> 1
```

При `flags = 6` (0b110) бит 2 установлен, поэтому правильный ответ 1. В C++ оператор `==` связывает сильнее, чем `&`, и `flags & 4 == 4` читается как `flags & (4 == 4)`, то есть `6 & 1 = 0`. Аналогично `>>` сильнее `&`: `op & 0xF0 >> 4` читается как `op & 0x0F`, и вместо группы (1) получается индекс (6). Поэтому в `decode()` скобки обязательны: `(op >> 4) & 0x0F`.

### Примеры ALU: перенос при сложении

Пример 1: ADD 200 + 100

```
  11001000   (200)
+ 01100100   (100)
----------
1 00101100   -> в 8 бит остаётся 00101100 = 44
```

Результат 44, Z = 0, N = 0 (старший бит 0), C = 1 (был перенос из 8-го разряда). Настоящий результат 300 не влезает в байт, поэтому он «заворачивается»: 300 - 256 = 44.

Пример 2: ADD 255 + 1

```
  11111111   (255)
+ 00000001   (1)
----------
1 00000000   -> в 8 бит остаётся 00000000 = 0
```

Результат 0, Z = 1, N = 0, C = 1. Это самый наглядный случай: Z и C стоят одновременно.
