#ifndef SOLVER_H
#define SOLVER_H

#include <cstdint>
#include <utility>
#include <vector>

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
    /**
     * Solve a Sudoku puzzle in a given board, if it is solvable.
     * 
     * @param board the board with the puzzle to be solved.
     * @param solvedBoard the board with the solution found for the puzzle.
     * @return a pair with 'true' in its first position and 'BoardValueError::NO_ERROR'
     * on its second position if the puzzle could be solved. Otherwise, returns a pair
     * with 'false' in its first position and the error reason code in its second
     * position.
     */
    static std::pair<bool, SolverError> solve(const Board &board, Board &solvedBoard);

    /**
     * Finds all the solutions for a Sudoku puzzle in a given board, if it is solvable.
     * 
     * @param board the board with the puzzle to be solved.
     * @param solvedBoards the boards with the solutions found for the puzzle.
     * @return a pair with 'true' in its first position and 'BoardValueError::NO_ERROR'
     * on its second position if the puzzle could be solved. Otherwise, returns a pair
     * with 'false' in its first position and the error reason code in its second
     * position.
     */
    static std::pair<bool, SolverError> solveForGood(const Board &board, std::vector<Board> &solvedBoards);
    
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