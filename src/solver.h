#ifndef SOLVER_H
#define SOLVER_H

#include <cstdint>
#include <utility>

namespace sudoku {

    class Board;

    enum class SolverError: uint8_t {
        NO_ERROR,
        INVALID_BOARD,
        BOARD_HAS_NO_SOLUTION
    };

    enum class SolverAlgorithm: uint8_t {
        BRUTE_FORCE,
        // James Creek's algorithm is described at
        // http:://www.ams.org/notices/200904/rtx090400460p.pdf 
        CREEK_PENCIL_AND_PAPER 
    };

    class Solver {

        public:

        static std::pair<bool, SolverError> solve(const Board& board, SolverAlgorithm algorithm, Board& solvedBoard);
    };
}

#endif