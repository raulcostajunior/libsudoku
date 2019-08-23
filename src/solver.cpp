#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

#include "solver.h"
#include "board.h"

using namespace sudoku;
using namespace std;

SolverResult Solver::solve(const Board &board, Board &solvedBoard)
{
    auto solvable = checkBoard(board);
    if (solvable != SolverResult::NO_ERROR)
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
            if (result == SetValueResult::NO_ERROR)
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
        return SolverResult::HAS_NO_SOLUTION;
    }
    else
    {
        return SolverResult::NO_ERROR;
    }
}

SolverResult Solver::solveForGood(const Board &board, vector<Board> &solvedBoards)
{
    auto solvable = checkBoard(board);
    if (solvable != SolverResult::NO_ERROR)
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
            if (candidateBoard.setValueAt(emptyCell.first, emptyCell.second, value) == SetValueResult::NO_ERROR)
            {
                // The current empty cell with the current value is a candidate for 
                // having a solution - tries to solve it.
                Board solvedBoard;
                auto result = Solver::solve(candidateBoard, solvedBoard);
                if (result == SolverResult::NO_ERROR) 
                {
                    // The board could be solved or the insertion of the last value solved it.
                    if (find(begin(solvedBoards), end(solvedBoards), solvedBoard) == end(solvedBoards)) 
                    {
                        // Solved board is not among the current solutions; add it.
                        solvedBoards.push_back(solvedBoard);
                    }
                } else if (result == SolverResult::ALREADY_SOLVED)
                {
                    // The insertion of value solved candidateBoard
                    if (find(begin(solvedBoards), end(solvedBoards), candidateBoard) == end(solvedBoards)) 
                    {
                        solvedBoards.push_back(candidateBoard);
                    }
                }
            }
        }
    }

    if (solvedBoards.size() < 1) {
        return SolverResult::HAS_NO_SOLUTION;
    }
    else {
        return SolverResult::NO_ERROR; 
    }
}

SolverResult Solver::checkBoard(const Board &board)
{
    bool solvable = true;
    SolverResult result = SolverResult::NO_ERROR;

    if (board.isEmpty())
    {
        solvable = false;
        result = SolverResult::EMPTY_BOARD;
    }
    else if (!board.isValid())
    {
        solvable = false;
        result = SolverResult::INVALID_BOARD;
    }
    else if (board.isComplete())
    {
        solvable = false;
        result = SolverResult::ALREADY_SOLVED;
    }

    return result;
}