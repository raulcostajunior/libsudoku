#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../board.h"
#include "../solver.h"

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

TEST_CASE("Empty board is not solvable")
{
    Board solved_board;
    auto result = Solver::solve(clear_board, solved_board);
    REQUIRE(result.first == false);
    REQUIRE(result.second == SolverError::EMPTY_BOARD);
}

TEST_CASE("Invalid board is not solvable")
{
    Board solved_board;
    auto result = Solver::solve(invalid_board, solved_board);
    REQUIRE(result.first == false);
    REQUIRE(result.second == SolverError::INVALID_BOARD);
}

TEST_CASE("Cannot solve unsolvable_board")
{
    Board solved_board;
    auto result = Solver::solve(unsolvable_board, solved_board);
    REQUIRE(result.first == false);
    REQUIRE(result.second == SolverError::HAS_NO_SOLUTION);
}

TEST_CASE("Can solve solvable_board")
{
    Board solved_board;
    auto result = Solver::solve(solvable_board, solved_board);
    REQUIRE(result.first == true);
    REQUIRE(solved_board.isComplete());
}