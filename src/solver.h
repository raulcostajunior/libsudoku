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
    EMPTY_BOARD,
    ALREADY_SOLVED,
    HAS_NO_SOLUTION
};

class Solver
{

public:
    static std::pair<bool, SolverError> solveBruteForce(const Board &board, Board &solvedBoard);
    
    // James Creek's algorithm is described at
    // http:://www.ams.org/notices/200904/rtx090400460p.pdf
    static std::pair<bool, SolverError> solveCreekMethod(const Board &board, Board &solvedBoard);

private:
    /**
     * Returns whether a given board is solvable (true) or not.
     * If the board is not solvable, the reason for its insolvability
     * is also returned.
     * 
     * @param board the board to be validated
     * @return a pair with 'true' in its first position and 'BoardValueError::NO_ERROR'
     * on its second position if the value could be set. Otherwise returns a pair
     * with 'false' in its first position and the error reason code in its second
     * position.
     */
    static std::pair<bool, SolverError> isBoardSolvable(const Board &board);

    

};

} // namespace sudoku

#endif