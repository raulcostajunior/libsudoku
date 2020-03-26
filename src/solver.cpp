#include "solver.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <set>
#include <thread>
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
                                       unsigned maxSolutions) {
    if (_asyncSolvingActive) {
        // Only one solving process can be active at once.
        return SolverResult::AsyncSolvingBusy;
    }

    auto solutions = make_shared<set<Board>>(set<Board>());

    _asyncSolvingActive = true;
    _asyncSolvingCancelled = false;

    _solveForGoodWorker =
        std::thread(&Solver::searchSolutions, this, board, fnProgress,
                    fnFinished, solutions, maxSolutions, 0);

    return SolverResult::AsyncSolvingSubmitted;
}

SolverResult Solver::solve(const Board &board, Board &solvedBoard) {
    auto solvable = checkBoard(board);
    if (solvable != SolverResult::NoError) {
        // Board is not solvable.
        return solvable;
    }

    vector<pair<uint8_t, uint8_t>> emptyCells = board.getBlankPositions();

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
                             const shared_ptr<set<Board>> solutions,
                             unsigned maxSolutions, unsigned level) {
    auto blanks = board.getBlankPositions();
    if (blanks.size() == 0) {
        // The board is a solution, no need to go on with the search.
        solutions->insert(board);
        return;
    }
    vector<set<uint8_t>> possibleValues;
    for (size_t i = 0; i < blanks.size(); i++) {
        possibleValues.emplace_back(
            board.getPossibleValues(blanks[i].first, blanks[i].second));
    }
    if (_asyncSolvingCancelled) {
        if (level == 0) {
            _asyncSolvingActive = false;
            _asyncSolvingCancelled = false;
            if (fnFinished != nullptr) {
                vector<Board> vb(solutions->cbegin(), solutions->cend());
                fnFinished(SolverResult::AsyncSolvingCancelled, vb);
            }
        }
        return;
    }
    if (solutions->size() >= maxSolutions) {
        // Found the maximum number of solutions already. No need to go
        // on.
        if (level == 0) {
            _asyncSolvingActive = false;
            _asyncSolvingCancelled = false;

            if (fnFinished != nullptr) {
                vector<Board> vb(solutions->cbegin(), solutions->cend());
                fnFinished(SolverResult::NoError, vb);
            }
        }
        return;
    }
    // Selects the position with least possible values to be the one that
    // will be filled next.
    int minSize = Board::MAX_VAL + 1;
    int possValIdx = -1;
    for (size_t j = 0; j < possibleValues.size(); j++) {
        if (possibleValues[j].size() > 0 &&
            possibleValues[j].size() < minSize) {
            possValIdx = j;
            minSize = possibleValues[j].size();
        }
    }
    if (minSize == Board::MAX_VAL + 1) {
        // There's at least one blank position for which there's no option value
        // - the board is not solvable.
        if (level == 0) {
            _asyncSolvingActive = false;
            _asyncSolvingCancelled = false;
            if (fnFinished != nullptr) {
                fnFinished(SolverResult::HasNoSolution, vector<Board>());
            }
        }
        return;
    }
    for (const auto possibleValue : possibleValues[possValIdx]) {
        Board nextBoard(board);
        nextBoard.setValueAt(blanks[possValIdx].first,
                             blanks[possValIdx].second, possibleValue);
        if (level == 0) {
            // When at first level (searching with the original board
            // puzzle), report progress
            if (fnProgress != nullptr) {
                // fnProgress(((i + 1.0) / blanks.size()) * 100.0,
                //            static_cast<unsigned>(solutions->size()));
            }
        }
        searchSolutions(nextBoard, fnProgress, fnFinished, solutions,
                        maxSolutions, level + 1);
    }
    if (level == 0) {
        // Reaching this point at level 0 means we are done.
        _asyncSolvingActive = false;
        _asyncSolvingCancelled = false;

        if (fnFinished != nullptr) {
            vector<Board> vb(solutions->cbegin(), solutions->cend());
            if (solutions->size() < 1) {
                fnFinished(SolverResult::HasNoSolution, vb);
            } else {
                fnFinished(SolverResult::NoError, vb);
            }
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
