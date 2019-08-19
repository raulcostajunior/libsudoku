#include <cstdint>
#include <utility>
#include <vector>

#include "solver.h"
#include "board.h"

using namespace sudoku;
using namespace std;

pair<bool, SolverError> Solver::solveBruteForce(const Board &board, Board &solvedBoard)
{
    auto solvable = Solver::isBoardSolvable(board);
    if (!solvable.first) {
        // Board is not solvable.
        return solvable;
    }

    // Gather empty cells.
    vector<pair<uint8_t, uint8_t>> emptyCells;

    for (uint8_t lin = 0; lin < 9; lin++) {
        for (uint8_t col = 0; col < 9; col++) {
            if (board.valueAt(lin, col) == 0) {
                emptyCells.push_back(make_pair(lin, col));
            }   
        }
    }

    solvedBoard = board; // board is the starting point for solvedBoard.
    size_t currCellPos = 0;
    bool boardUnsolvable = false;
    while (currCellPos < emptyCells.size() && !boardUnsolvable) {
        auto currCell = emptyCells[currCellPos];
        uint8_t currCellVal = solvedBoard.valueAt(currCell.first, currCell.second) + 1;
        bool currCellSolved = false;
        while (!currCellSolved && currCellVal <= 9) {
            auto result = solvedBoard.setValueAt(currCell.first, currCell.second, currCellVal);
            if (result.first) {
                currCellSolved = true;
            } else {
                // Try the next value in the cell.
                currCellVal++;
            }
        }
        if (currCellSolved) {
            currCellPos++;
        } else {
            // currCellVal > 9 - have to rollback to the previous cell, if possible.
            if (currCellPos > 0) {
                // Resets the current cell before rolling back.
                solvedBoard.setValueAt(currCell.first, currCell.second, 0);
                currCellPos--;
            } else {
                boardUnsolvable = true;
            }
        }
    }
    if (boardUnsolvable) {
        return make_pair(false, SolverError::HAS_NO_SOLUTION);
    } else {
        return make_pair(true, SolverError::NO_ERROR);
    }
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