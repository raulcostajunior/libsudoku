#define CATCH_CONFIG_MAIN

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include "../board.h"
#include "../solver.h"
#include "catch.hpp"

using namespace sudoku;
using namespace std;

static int _timeoutSecs = 1800;

// clang-format off

const Board clear_board (
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
    }
);

// Unsolvable board extracted from https://www.sudokudragon.com/unsolvable.htm
const Board unsolvable_board (
    {
        5, 1, 6, 8, 4, 0, 7, 3, 2,
        3, 0, 7, 6, 0, 5, 0, 0, 0,
        8, 0, 9, 7, 0, 0, 0, 6, 5,
        1, 3, 5, 0, 6, 0, 9, 0, 7,
        4, 7, 2, 5, 9, 1, 0, 0, 6,
        9, 6, 8, 3, 7, 0, 0, 5, 0,
        2, 5, 3, 1, 8, 6, 0, 7, 4,
        6, 8, 4, 2, 0, 7, 5, 0, 0,
        7, 9, 1, 0, 5, 0, 6, 0, 8
    }
);

const Board invalid_board (
    {
        2, 9, 5, 7, 4, 3, 8, 6, 2, // 2 is repeated across the first line and the last column.
        4, 3, 1, 8, 6, 5, 9, 2, 7,
        8, 7, 6, 1, 9, 2, 5, 4, 3,
        3, 8, 7, 4, 5, 9, 2, 1, 6,
        6, 1, 2, 3, 8, 7, 4, 9, 5,
        5, 4, 9, 2, 1, 6, 7, 3, 8,
        7, 6, 3, 5, 2, 4, 1, 8, 9,
        9, 2, 8, 6, 7, 1, 3, 5, 4,
        1, 5, 4, 9, 3, 8, 6, 7, 2,
    }
);

const Board solvable_board (  // Hard with just one solution
    {
        0, 0, 6, 0, 0, 8, 5, 0, 0,
        0, 0, 0, 0, 7, 0, 6, 1, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 9,
        0, 0, 0, 0, 9, 0, 0, 0, 1,
        0, 0, 1, 0, 0, 0, 8, 0, 0,
        4, 0, 0, 5, 3, 0, 0, 0, 0,
        1, 0, 7, 0, 5, 3, 0, 0, 0,
        0, 5, 0, 0, 6, 4, 0, 0, 0,
        3, 0, 0, 1, 0, 0, 0, 6, 0,
    }
);

const Board solvable_one_solution (
    {
        2, 9, 5, 7, 4, 3, 8, 6, 1,
        4, 3, 1, 8, 6, 5, 9, 2, 7,
        8, 7, 6, 1, 9, 2, 5, 4, 3,
        3, 8, 7, 4, 5, 9, 2, 1, 6,
        6, 1, 2, 3, 8, 7, 4, 9, 5,
        5, 4, 9, 2, 1, 6, 7, 3, 8,
        7, 6, 3, 5, 2, 4, 1, 8, 9,
        9, 2, 8, 6, 7, 1, 3, 5, 0,
        1, 5, 4, 9, 3, 8, 6, 7, 2,
    }
);

const Board solvable_two_solutions (
    {
        0, 9, 5, 7, 4, 3, 0, 6, 1,
        4, 3, 1, 8, 6, 5, 0, 0, 0,
        8, 0, 6, 1, 9, 2, 5, 4, 3,
        3, 8, 7, 4, 0, 0, 2, 1, 6,
        6, 1, 2, 3, 8, 7, 4, 0, 0,
        5, 4, 9, 2, 1, 6, 7, 3, 0,
        0, 6, 3, 5, 0, 4, 1, 0, 0,
        9, 0, 8, 6, 0, 1, 3, 5, 4,
        1, 5, 4, 9, 3, 8, 6, 0, 0,
    }
);

const Board solvable_three_solutions (
    {
        0, 7, 5, 8, 4, 6, 1, 0, 0,
        8, 4, 0, 0, 0, 0, 9, 7, 0,
        0, 0, 0, 0, 0, 0, 0, 4, 0,
        7, 0, 0, 5, 0, 0, 2, 9, 1,
        0, 0, 0, 0, 0, 4, 7, 0, 8,
        2, 0, 0, 0, 9, 0, 0, 5, 0,
        4, 8, 9, 0, 0, 0, 5, 0, 2,
        6, 0, 0, 0, 8, 0, 3, 0, 9,
        0, 1, 2, 6, 5, 9, 0, 0, 0,
    }
);

const Board solvable_many_solutions (
    {
        0, 9, 5, 0, 4, 0, 0, 6, 0,
        4, 0, 1, 0, 6, 5, 9, 0, 0,
        8, 0, 0, 1, 9, 0, 5, 4, 0,
        0, 0, 7, 0, 5, 0, 0, 1, 6,
        6, 1, 0, 3, 0, 7, 4, 9, 5,
        5, 4, 9, 0, 1, 6, 0, 0, 0,
        0, 6, 0, 5, 0, 0, 1, 0, 9,
        9, 0, 0, 6, 0, 1, 0, 5, 0,
        0, 5, 4, 9, 0, 0, 6, 0, 0,
    }
);

// clang-format on

SolverResult solveForGood(const Board &board, vector<Board> &solutions,
                          unsigned limit = 10000) {
    SolverResult result;
    atomic<bool> finished{false};
    atomic<unsigned> solutionsFound(0u);
    atomic<unsigned> unsolvablesFound(0u);
    atomic<double> progressPercent(0.0);
    Solver solver;

    auto currentPrecision = clog.precision();

    clog << fixed << setprecision(2);

    auto asyncSolveProgress =
        [&progressPercent, &unsolvablesFound, &solutionsFound](
            double progress, unsigned unsolvables, unsigned solutions) {
            progressPercent = progress;
            unsolvablesFound = unsolvables;
            solutionsFound = solutions;
        };

    auto asyncSolveFinished = [&solutions, &result, &finished](
                                  SolverResult solverResult,
                                  const vector<Board> &solvedBoards) {
        solutions = solvedBoards;
        result = solverResult;
        finished = true;

        if (solverResult == SolverResult::AsyncSolvingCancelled) {
            clog << "\n... AsyncSolve cancelled - timeout "
                    "after running for "
                 << _timeoutSecs << " seconds." << endl;
        } else if (solverResult == SolverResult::NoError) {
            clog << "\n.... AsyncSolve at 100.00%: " << solvedBoards.size()
                 << " solution(s) found." << endl;
        }
    };

    result = solver.asyncSolveForGood(board, asyncSolveProgress,
                                      asyncSolveFinished, limit);
    if (result != SolverResult::AsyncSolvingSubmitted) {
        return result;
    }

    // Prints out progress with "Poor's man animation".
    int numOfWaits = 0;
    string animPattern[4]{".   ", " .  ", "  . ", "   ."};
    while (!finished && numOfWaits < _timeoutSecs) {
        this_thread::sleep_for(chrono::seconds(1));
        numOfWaits++;
        if (numOfWaits > 1 && progressPercent < 100.0) {
            clog << animPattern[numOfWaits % 4] << " AsyncSolve at "
                 << progressPercent << "%: " << unsolvablesFound
                 << "non-solvables(s) and " << solutionsFound
                 << " solution(s) found so far." << endl;
        }
    }

    if (numOfWaits >= _timeoutSecs) {
        // Timed-out: cancel
        solver.cancelAsyncSolving();
        // Sleeps a while so the solver has enough time to call the
        // finished callback and update the result with
        // SolverResult::AsyncSolvingCancelled value.
        this_thread::sleep_for(chrono::seconds(1));
    }

    clog << defaultfloat << setprecision(currentPrecision);

    return result;
}

TEST_CASE("Empty board is not solvable") {
    Board solved_board;
    Solver solver;
    auto result = solver.solve(clear_board, solved_board);
    REQUIRE(result == SolverResult::EmptyBoard);
}

TEST_CASE("Invalid board is not solvable") {
    Board solved_board;
    Solver solver;
    auto result = solver.solve(invalid_board, solved_board);
    REQUIRE(result == SolverResult::InvalidBoard);
}

TEST_CASE("Cannot solve unsolvable_board") {
    Board solved_board;
    Solver solver;
    auto result = solver.solve(unsolvable_board, solved_board);
    REQUIRE(result == SolverResult::HasNoSolution);
}

TEST_CASE("Can solve solvable_board") {
    Board solved_board;
    Solver solver;
    auto result = solver.solve(solvable_board, solved_board);
    REQUIRE(result == SolverResult::NoError);
    REQUIRE(solved_board.isComplete());
}

TEST_CASE("Cannot solve solvable_board with invalid candidates vector") {
    Board solved_board;
    Solver solver;
    vector<uint8_t> candidates{1, 1, 2, 3, 4, 5, 6, 7, 9};
    auto result = solver.solve(solvable_board, candidates, solved_board);
    REQUIRE(result == SolverResult::InvalidatesCandidatesVector);
}

TEST_CASE(
    "asyncSolveForGood finds one solution for board with single "
    "solution") {
    vector<Board> solved_boards;

    auto result = solveForGood(solvable_one_solution, solved_boards);

    REQUIRE(result == SolverResult::NoError);
    REQUIRE(solved_boards.size() == 1);
    REQUIRE(solved_boards[0].isComplete());
}

TEST_CASE(
    "asyncSolveForGood finds 3 solutions for board with 3 "
    "solutions") {
    vector<Board> solved_boards;

    auto result = solveForGood(solvable_three_solutions, solved_boards);

    REQUIRE(result == SolverResult::NoError);
    REQUIRE(solved_boards.size() == 3);
    REQUIRE(solved_boards[0].isComplete());
    REQUIRE(solved_boards[1].isComplete());
}

TEST_CASE("All solutions found by asyncSolveForGood are valid") {
    vector<Board> solved_boards;

    auto result = solveForGood(solvable_many_solutions, solved_boards);

    clog << "Will verify all " << solved_boards.size() << " solutions." << endl;

    REQUIRE(result == SolverResult::NoError);
    for (size_t i = 0; i < solved_boards.size(); i++) {
        REQUIRE(solved_boards[i].isComplete());
    }
}

TEST_CASE(
    "The limit on maximum number of solutions to find is being respected") {
    vector<Board> solved_boards;

    // Finds the total number of solutions for the board with no specified
    // limits.
    auto result = solveForGood(solvable_many_solutions, solved_boards);

    size_t totalOfSolutions = solved_boards.size();
    solved_boards.clear();

    // Searches for solutions with a limit inferior to the already known total.
    result = solveForGood(solvable_many_solutions, solved_boards,
                          totalOfSolutions - 1);
    REQUIRE(solved_boards.size() == totalOfSolutions - 1);
}

TEST_CASE("Cannot spawn more than one asyncSolveForGood simultaneously") {
    // Starts a lengthy asynchronous solving ...
    Solver solver;
    auto result = solver.asyncSolveForGood(solvable_board, nullptr, nullptr);

    // Tries to start a new one - would run simultaneously with the previous.
    auto secondResult =
        solver.asyncSolveForGood(solvable_board, nullptr, nullptr);

    REQUIRE(result == SolverResult::AsyncSolvingSubmitted);
    REQUIRE(secondResult == SolverResult::AsyncSolvingBusy);

    solver.cancelAsyncSolving();  // for graceful async solving exit.
}

TEST_CASE(
    "asyncSolveForGood finds one solution for a difficult board "
    "with one solution") {
    vector<Board> solved_boards;

    // this takes long...
    auto result = solveForGood(solvable_board, solved_boards);

    REQUIRE(result == SolverResult::NoError);
    REQUIRE(solved_boards.size() == 1);
    REQUIRE(solved_boards[0].isComplete());
}