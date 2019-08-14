#include <cstdint>
#include <utility>

#include "solver.h"
#include "board.h"

using namespace sudoku;
using namespace std;

pair<bool, SolverError> Solver::solveBruteForce(const Board &board, Board &solvedBoard)
{
    auto solvable = Solver::isBoardSolvable(board);
    if (!solvable.first) {
        // Board is not solvable
        return solvable;
    }

    // TODO: add solution algorithm

    return make_pair(true, SolverError::NO_ERROR);
}

pair<bool, SolverError> Solver::solveCreekMethod(const Board &board, Board &solvedBoard)
{
    auto solvable = Solver::isBoardSolvable(board);
    if (!solvable.first) {
        // Board is not solvable
        return solvable;
    }

    // TODO: add solution algorithm
    
    return make_pair(true, SolverError::NO_ERROR);
}

pair<bool, SolverError> Solver::isBoardSolvable(const Board &board) {
    bool solvable = true;
    SolverError error = SolverError::NO_ERROR;

    if (board.isEmpty()) {
        solvable = false;
        error = SolverError::EMPTY_BOARD;
    } else if (!board.isValid()) {
        solvable = false;
        error = SolverError::INVALID_BOARD;
    } else if (board.isComplete()) {
        solvable = false;
        error = SolverError::ALREADY_SOLVED;
    }

    return make_pair(solvable, error);
}