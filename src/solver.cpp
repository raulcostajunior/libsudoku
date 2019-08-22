#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

#include "solver.h"
#include "board.h"

using namespace sudoku;
using namespace std;

pair<bool, SolverError> Solver::solve(const Board &board, Board &solvedBoard)
{
    auto solvable = Solver::isBoardSolvable(board);
    if (!solvable.first)
    {
        // Board is not solvable.
        return solvable;
    }

    // Gather empty cells.
    vector<pair<uint8_t, uint8_t>> emptyCells;

    for (uint8_t lin = 0; lin < 9; lin++)
    {
        for (uint8_t col = 0; col < 9; col++)
        {
            if (board.valueAt(lin, col) == 0)
            {
                emptyCells.push_back(make_pair(lin, col));
            }
        }
    }

    solvedBoard = board; // board is the starting point for solvedBoard.
    size_t currCellPos = 0;
    bool boardUnsolvable = false;
    while (currCellPos < emptyCells.size() && !boardUnsolvable)
    {
        auto currCell = emptyCells[currCellPos];
        uint8_t currCellVal = solvedBoard.valueAt(currCell.first, currCell.second) + 1;
        bool currCellSolved = false;
        while (!currCellSolved && currCellVal <= 9)
        {
            auto result = solvedBoard.setValueAt(currCell.first, currCell.second, currCellVal);
            if (result.first)
            {
                currCellSolved = true;
            }
            else
            {
                // Try the next value in the cell.
                currCellVal++;
            }
        }
        if (currCellSolved)
        {
            currCellPos++;
        }
        else
        {
            // currCellVal > 9 - have to rollback to the previous cell, if possible.
            if (currCellPos > 0)
            {
                // Resets the current cell before rolling back.
                solvedBoard.setValueAt(currCell.first, currCell.second, 0);
                currCellPos--;
            }
            else
            {
                boardUnsolvable = true;
            }
        }
    }
    if (boardUnsolvable)
    {
        return make_pair(false, SolverError::HAS_NO_SOLUTION);
    }
    else
    {
        return make_pair(true, SolverError::NO_ERROR);
    }
}

pair<bool, SolverError> Solver::solveForGood(const Board &board, vector<Board> &solvedBoards)
{
    auto solvable = Solver::isBoardSolvable(board);
    if (!solvable.first)
    {
        // Board is not solvable.
        return solvable;
    }

    // Gather empty cells
    vector<pair<uint8_t, uint8_t>> emptyCells;

    for (uint8_t lin = 0; lin < 9; lin++)
    {
        for (uint8_t col = 0; col < 9; col++)
        {
            if (board.valueAt(lin, col) == 0)
            {
                emptyCells.push_back(make_pair(lin, col));
            }
        }
    }

    for (const auto &emptyCell : emptyCells)
    {
        for (uint8_t value = 1; value < 10; value++) 
        {
            Board candidateBoard = board;
            if (candidateBoard.setValueAt(emptyCell.first, emptyCell.second, value).first)
            {
                // The current empty cell with the current val is a candidate for 
                // having a solution - tries to solve it.
                Board solvedBoard;
                auto result = Solver::solve(candidateBoard, solvedBoard);
                if (result.first) 
                {
                    if (find(begin(solvedBoards), end(solvedBoards), solvedBoard) == end(solvedBoards)) 
                    {
                        // Solved board is not among the current solutions; add it.
                        solvedBoards.push_back(solvedBoard);
                    }
                }
            }
        }
    }

    if (solvedBoards.size() < 1) {
        return make_pair(false, SolverError::HAS_NO_SOLUTION);
    }
    else {
        return make_pair(true, SolverError::NO_ERROR); 
    }
}

pair<bool, SolverError> Solver::isBoardSolvable(const Board &board)
{
    bool solvable = true;
    SolverError error = SolverError::NO_ERROR;

    if (board.isEmpty())
    {
        solvable = false;
        error = SolverError::EMPTY_BOARD;
    }
    else if (!board.isValid())
    {
        solvable = false;
        error = SolverError::INVALID_BOARD;
    }
    else if (board.isComplete())
    {
        solvable = false;
        error = SolverError::ALREADY_SOLVED;
    }

    return make_pair(solvable, error);
}