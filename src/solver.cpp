#include "solver.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "board.h"

using sudoku::Board;
using sudoku::Solver;
using sudoku::SolverResult;
using std::make_pair;
using std::pair;
using std::make_shared;
using std::set;
using std::shared_ptr;
using std::vector;
using std::unordered_set;

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

    auto solutions = make_shared<vector<Board>>(vector<Board>());

    _asyncSolvingActive = true;
    _asyncSolvingCancelled = false;

    if (_solveForGoodWorker.joinable()) {
        _solveForGoodWorker.join();
    }

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
    SolverResult result;
    auto solutions = make_shared<vector<Board>>(vector<Board>());
    searchSolutions(
        board, nullptr,
        [&result](SolverResult solverResult, const vector<Board>&) {
            result = solverResult;
            // Ignore the vector of solutions passed as an argument to the
            // callback. The vector of solutions will be pointed by the
            // shared_ptr passed as the next argument to searchSolutions. The
            // callback argument is used when searchSolutions is called
            // asynchronously in a worker thread and the callback is the only
            // way to communicate the results before finishing the worker
            // thread.
        },
        solutions, 1, 0);
    if (!solutions->empty()) {
        solvedBoard = (*solutions)[0];
    }
    return result;
}

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
                emptyCells.emplace_back(lin, col);
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
            // currCellVal >= 9 - have to roll back to the previous cell, if
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
    }
    return SolverResult::NoError;
}

void Solver::searchSolutions(const Board &board,
                             const SolverProgressCallback &fnProgress,
                             const SolverFinishedCallback &fnFinished,
                             const shared_ptr<vector<Board>>& solutions,
                             unsigned maxSolutions, unsigned level) {
    static unsigned unsolvablesFound = 0;
    static double progressPercent = 0.0;
    auto blanks = board.getBlankPositions();
    if (blanks.empty()) {
        // The board is a solution, no need to go on with the search.
        solutions->push_back(board);
        return;
    }
    vector<set<uint8_t>> possibleValues;
    possibleValues.reserve(blanks.size());
    for (auto & blank : blanks) {
        possibleValues.emplace_back(
            board.getPossibleValues(blank.first, blank.second));
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
    // Selects the position with the least possible values to be the one that
    // will be filled next.
    size_t minSize = Board::MAX_VAL + 1;
    int possValIdx = -1;
    for (size_t j = 0; j < possibleValues.size(); j++) {
        if (!possibleValues[j].empty() &&
            possibleValues[j].size() < minSize) {
            possValIdx = static_cast<int>(j);
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
    vector<uint8_t> possVals(possibleValues[possValIdx].cbegin(),
                             possibleValues[possValIdx].cend());
    for (size_t i = 0; i < possVals.size(); i++) {
        Board nextBoard(board);
        nextBoard.setValueAt(blanks[possValIdx].first,
                             blanks[possValIdx].second, possVals[i]);
        if (level == 0) {
            // When at first level (searching with the original board
            // puzzle), update progress (a rough approximation based on the
            // progress of depth first searches for each possible value at the
            // initial search node).
            progressPercent =
                ((static_cast<double>(i) + 1.0) /
                  static_cast<double>(possVals.size())) * 100.0;
        }
        if (fnProgress != nullptr) {
            fnProgress(progressPercent, unsolvablesFound,
                       static_cast<unsigned>(solutions->size()));
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
            if (solutions->empty()) {
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
