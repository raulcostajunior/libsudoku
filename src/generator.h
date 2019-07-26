#ifndef GENERATOR_H
#define GENERATOR_H

#include <cstdint>
#include <utility>

#include "board.h"

namespace sudoku
{

enum class PuzzleDifficulty : uint8_t
{
    EASY,
    MEDIUM,
    HARD,
    UNKNOWN
};

class Generator
{

public:
    static Board generate(PuzzleDifficulty difficulty);

};

} // namespace sudoku

#endif