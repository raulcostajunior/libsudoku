#include "solver.h"

#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <utility>
#include <vector>

#include "board.h"

using namespace sudoku;
using namespace std;

Solver::Solver() : _asyncSolvingCancelled(false), _asyncSolvingActive(false) {}

Solver::~Solver() {
    if (_asyncSolvingActive) {
        cancelAsyncSolving();
    }
    if (_solveForGoodWorker.joinable()) {
        _solveForGoodWorker.join();
    }
}

SolverResult Solver::asyncSolveForGood(const Board &board,
                                       const SolverProgressCallback &fnProgress,
                                       const SolverFinishedCallback &fnFinished,
                                       int maxSolutions) {
    if (_asyncSolvingActive) {
        // Only one solving process can be active at once.
        return SolverResult::AsyncSolvingBusy;
    }

    _asyncSolvingActive = true;
    _asyncSolvingCancelled = false;

    _solveForGoodWorker = std::thread(&Solver::searchSolutions, this, board,
                                      fnProgress, fnFinished, maxSolutions);

    return SolverResult::AsyncSolvingSubmitted;
}

SolverResult Solver::solve(const Board &board, Board &solvedBoard) {
    // const vector<uint8_t> candidates({1, 2, 3, 4, 5, 6, 7, 8, 9});

    auto solvable = checkBoard(board);
    if (solvable != SolverResult::NoError) {
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

    solvedBoard = board;  // board is the starting point for solvedBoard.
    size_t currCellPos = 0;
    bool boardUnsolvable = false;
    while (currCellPos < emptyCells.size() && !boardUnsolvable) {
        auto currCell = emptyCells[currCellPos];
        auto currCellValue =
            solvedBoard.valueAt(currCell.first, currCell.second);
        uint8_t candidateValue = 1;
        if (currCellValue != 0) {
            // We're backtracking - must start with the next candidate value.
            candidateValue = currCellValue + 1;
        }

        bool currCellSolved = false;
        while (!currCellSolved && candidateValue <= 9) {
            auto result = solvedBoard.setValueAt(
                currCell.first, currCell.second, candidateValue);
            if (result == SetValueResult::NoError) {
                currCellSolved = true;
            } else {
                // Try the next value in the cell.
                candidateValue++;
            }
        }
        if (currCellSolved) {
            currCellPos++;
        } else {
            // currCellVal >= 9 - have to rollback to the previous cell, if
            // possible.
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
        return SolverResult::HasNoSolution;
    } else {
        return SolverResult::NoError;
    }
}

// TODO: Remove this oveload of solve as soon as the Generator stops calling it!
SolverResult Solver::solve(const Board &board,
                           const vector<uint8_t> &candidates,
                           Board &solvedBoard) {
    // Checks the vector of candidate values - it must have the integers from 1
    // to 9 without repetition.
    auto minMax = minmax_element(candidates.begin(), candidates.end());
    unordered_set<uint8_t> nonRep(candidates.begin(), candidates.end());
    if (*minMax.first != 1 || *minMax.second != 9 || nonRep.size() != 9) {
        return SolverResult::InvalidatesCandidatesVector;
    }

    auto solvable = checkBoard(board);
    if (solvable != SolverResult::NoError) {
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

    solvedBoard = board;  // board is the starting point for solvedBoard.
    size_t currCellPos = 0;
    bool boardUnsolvable = false;
    while (currCellPos < emptyCells.size() && !boardUnsolvable) {
        auto currCell = emptyCells[currCellPos];
        auto currCellValue =
            solvedBoard.valueAt(currCell.first, currCell.second);
        uint8_t candidatesIdx = 0;
        if (currCellValue != 0) {
            // We're backtracking - must start with the next candidate value.
            candidatesIdx = static_cast<uint8_t>(
                distance(
                    candidates.begin(),
                    find(candidates.begin(), candidates.end(), currCellValue)) +
                1);
        }

        bool currCellSolved = false;
        while (!currCellSolved && candidatesIdx < 9) {
            auto result = solvedBoard.setValueAt(
                currCell.first, currCell.second, candidates[candidatesIdx]);
            if (result == SetValueResult::NoError) {
                currCellSolved = true;
            } else {
                // Try the next value in the cell.
                candidatesIdx++;
            }
        }
        if (currCellSolved) {
            currCellPos++;
        } else {
            // currCellVal >= 9 - have to rollback to the previous cell, if
            // possible.
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
        return SolverResult::HasNoSolution;
    } else {
        return SolverResult::NoError;
    }
}

void Solver::searchSolutions(const Board &board,
                             const SolverProgressCallback &fnProgress,
                             const SolverFinishedCallback &fnFinished,
                             int maxSolutions) {
    vector<Board> solvedBoards;

    auto solvable = checkBoard(board);
    if (solvable != SolverResult::NoError) {
        // Board is not solvable.
        _asyncSolvingActive = false;
        _asyncSolvingCancelled = false;
        if (fnFinished != nullptr) {
            fnFinished(solvable, solvedBoards);
        }
        return;
    }

    // Gather empty cells
    vector<pair<uint8_t, uint8_t>> emptyCells;

    for (uint8_t lin = 0; lin < 9; lin++) {
        for (uint8_t col = 0; col < 9; col++) {
            if (board.valueAt(lin, col) == 0) {
                emptyCells.push_back(make_pair(lin, col));
            }
        }
    }

    for (size_t i = 0; i < emptyCells.size(); i++) {
        if (_asyncSolvingCancelled) {
            _asyncSolvingActive = false;
            _asyncSolvingCancelled = false;
            if (fnFinished != nullptr) {
                fnFinished(SolverResult::AsyncSolvingCancelled, solvedBoards);
            }
            return;
        }

        auto emptyCell = emptyCells[i];

        if (fnProgress != nullptr) {
            fnProgress(((i + 1.0) / emptyCells.size()) * 100.0,
                       static_cast<unsigned int>(solvedBoards.size()));
        }

        for (uint8_t value = 1; value < 10; value++) {
            Board candidateBoard = board;
            if (candidateBoard.setValueAt(emptyCell.first, emptyCell.second,
                                          value) == SetValueResult::NoError) {
                // The current empty cell with the current value is a candidate
                // for having a solution - tries to solve it.
                Board solvedBoard;
                auto result = Solver::solve(candidateBoard, solvedBoard);
                if (result == SolverResult::NoError) {
                    // The board could be solved or the insertion of the last
                    // value solved it.
                    if (find(begin(solvedBoards), end(solvedBoards),
                             solvedBoard) == end(solvedBoards)) {
                        // Solved board is not among the current solutions; add
                        // it.
                        solvedBoards.push_back(solvedBoard);
                    }
                } else if (result == SolverResult::AlreadySolved) {
                    // The insertion of value solved candidateBoard
                    if (find(begin(solvedBoards), end(solvedBoards),
                             candidateBoard) == end(solvedBoards)) {
                        solvedBoards.push_back(candidateBoard);
                    }
                }
            }
        }
    }

    _asyncSolvingActive = false;
    _asyncSolvingCancelled = false;

    if (fnFinished != nullptr) {
        if (solvedBoards.size() < 1) {
            fnFinished(SolverResult::HasNoSolution, solvedBoards);
        } else {
            fnFinished(SolverResult::NoError, solvedBoards);
        }
    }
}

void Solver::cancelAsyncSolving() { _asyncSolvingCancelled = true; }

SolverResult Solver::checkBoard(const Board &board) {
    SolverResult result = SolverResult::NoError;

    if (board.isEmpty()) {
        result = SolverResult::EmptyBoard;
    } else if (!board.isValid()) {
        result = SolverResult::InvalidBoard;
    } else if (board.isComplete()) {
        result = SolverResult::AlreadySolved;
    }

    return result;
}
