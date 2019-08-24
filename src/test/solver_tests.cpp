#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../board.h"
#include "../solver.h"

#include <atomic>
#include <iostream>

using namespace sudoku;
using namespace std;

const Board clear_board(
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
const Board unsolvable_board(
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

const Board invalid_board(
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

const Board solvable_board(
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

const Board solvable_one_solution(
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

const Board solvable_two_solutions(
    {
        0, 9, 5, 7, 4, 3, 8, 6, 1,
        4, 3, 1, 8, 6, 5, 9, 0, 0,
        8, 0, 6, 1, 9, 2, 5, 4, 3,
        3, 8, 7, 4, 5, 9, 2, 1, 6,
        6, 1, 2, 3, 8, 7, 4, 9, 5,
        5, 4, 9, 2, 1, 6, 7, 3, 8,
        0, 6, 3, 5, 0, 4, 1, 8, 9,
        9, 0, 8, 6, 0, 1, 3, 5, 4,
        1, 5, 4, 9, 3, 8, 6, 0, 0,
    }
);

void asyncSolveProgress(double progressPercent, unsigned solutionsFound) {
    clog << "Async solve at " << progressPercent << "%: "
      << solutionsFound << " solution(s) found so far." << endl;
}

TEST_CASE("Empty board is not solvable")
{
    Board solved_board;
    Solver solver;
    auto result = solver.solve(clear_board, solved_board);
    REQUIRE(result == SolverResult::EMPTY_BOARD);
}

TEST_CASE("Invalid board is not solvable")
{
    Board solved_board;
    Solver solver;
    auto result = solver.solve(invalid_board, solved_board);
    REQUIRE(result == SolverResult::INVALID_BOARD);
}

TEST_CASE("Cannot solve unsolvable_board")
{
    Board solved_board;
    Solver solver;
    auto result = solver.solve(unsolvable_board, solved_board);
    REQUIRE(result == SolverResult::HAS_NO_SOLUTION);
}

TEST_CASE("Can solve solvable_board")
{
    Board solved_board;
    Solver solver;
    auto result = solver.solve(solvable_board, solved_board);
    REQUIRE(result == SolverResult::NO_ERROR);
    REQUIRE(solved_board.isComplete());
}

TEST_CASE("solveForGood finds one solution for board with single solution")
{
    vector<Board> solved_boards;
    SolverResult result;
    atomic<bool> finished{false};

    Solver solver;
    auto asyncSolveFinished = 
         [&solved_boards, &result, &finished] (SolverResult solverResult, vector<Board> solutions) 
         {
             solved_boards = solutions;
             result = solverResult;
             finished = true;
         };

    solver.asyncSolveForGood(solvable_one_solution, asyncSolveProgress, asyncSolveFinished);

    int numOfWaits = 0;
    while (!finished && numOfWaits < 60) 
    {
        this_thread::sleep_for(chrono::seconds(1));
        numOfWaits++;
    }

    REQUIRE(finished);
    REQUIRE(result == SolverResult::NO_ERROR);
    REQUIRE(solved_boards.size() == 1);
    REQUIRE(solved_boards[0].isComplete());
}

TEST_CASE("solveForGood finds two solutions for board with two solutions")
{
    vector<Board> solved_boards;
    SolverResult result;

    // Solver solver;
    // auto result = solver.solveForGood(solvable_two_solutions, solved_boards);
    // REQUIRE(result == SolverResult::NO_ERROR);
    // REQUIRE(solved_boards.size() == 2);
    // REQUIRE(solved_boards[0].isComplete());
    // REQUIRE(solved_boards[1].isComplete());
}

TEST_CASE("All solutions found by solveForGood are valid")
{
    // vector<Board> solved_boards;
    // Solver solver;
    // bool started = solver.asyncSolveForGood(solvable_board, solved_boards);
    // REQUIRE(result == SolverResult::NO_ERROR);
    // for (size_t i = 0; i < solved_boards.size(); i++) {
    //    REQUIRE(solved_boards[i].isComplete()); 
    // }
}

TEST_CASE("Cannot spawn more than one asyncSolveForGood simultaneously")
{
    // TODO: write test case.
}
