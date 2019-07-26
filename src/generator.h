#ifndef GENERATOR_H
#define GENERATOR_H

#include <cstdint>
#include <utility>

#include "board.h"

namespace sudoku {

    class Generator {

        public:

        static Board generate(BoardDifficulty difficulty);
    };
}

#endif