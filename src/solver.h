#ifndef SOLVER_H
#define SOLVER_H

#include <cstdint>
#include <utility>

// TODO: Properly document public methods

namespace sudoku
{

class Board;

enum class SolverError : uint8_t
{
    NO_ERROR,
    INVALID_BOARD,
    BOARD_HAS_NO_SOLUTION
};

class Solver
{

public:
    static std::pair<bool, SolverError> solveBruteForce(const Board &board, Board &solvedBoard);
    
    // James Creek's algorithm is described at
    // http:://www.ams.org/notices/200904/rtx090400460p.pdf
    static std::pair<bool, SolverError> solveCreekMethod(const Board &board, Board &solvedBoard);

};

} // namespace sudoku

#endif