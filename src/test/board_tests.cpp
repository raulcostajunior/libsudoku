#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "../board.h"

using namespace sudoku;


    Board solved_board(
        {
            2, 9, 5, 7, 4, 3, 8, 6, 1,
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

    Board invalid_board_col_line(
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


    Board invalid_board_section(
        {
            2, 9, 5, 7, 4, 3, 8, 6, 1, // 2 is in first 3x3 section and 3 in third (top-bottom; left-right)
            4, 2, 1, 8, 6, 5, 9, 3, 7,
            8, 7, 6, 1, 9, 2, 5, 4, 3,
            3, 8, 7, 4, 5, 9, 2, 1, 6,
            6, 1, 2, 3, 8, 7, 4, 9, 5,
            5, 4, 9, 2, 1, 6, 7, 3, 8,
            7, 6, 3, 5, 2, 4, 1, 8, 9,
            9, 2, 8, 6, 7, 1, 3, 5, 4,
            1, 5, 4, 9, 3, 8, 6, 7, 2,
        }
    );

    Board board_with_blanks(
        // Despite being incomplete, this board has no repetition violation - so it should be valid.
        {
            2, 9, 5, 7, 0, 3, 8, 6, 1, // a blank in the fifth column of first row 
            4, 3, 1, 8, 6, 5, 9, 2, 7,
            8, 7, 6, 1, 9, 2, 5, 4, 3,
            3, 8, 7, 4, 5, 9, 2, 1, 6,
            6, 1, 2, 3, 8, 7, 4, 9, 5,
            5, 4, 9, 2, 1, 6, 7, 3, 8,
            7, 6, 3, 5, 2, 4, 1, 8, 9,
            9, 2, 8, 6, 7, 1, 3, 5, 4,
            1, 0, 4, 9, 3, 8, 6, 7, 2, // a blank in the second column of last row
        }
    );

    Board clear_board(
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

TEST_CASE("Board initially empty")
{
    Board b;
    REQUIRE(b.isEmpty());
}

TEST_CASE("Invalid board isn't complete")
{
    REQUIRE(!invalid_board_col_line.isComplete());
    REQUIRE(!invalid_board_section.isComplete());
}

TEST_CASE("Board with blank isn't complete")
{
    REQUIRE(!board_with_blanks.isComplete());
}

TEST_CASE("Clear board isn't complete")
{
    REQUIRE(!clear_board.isComplete());
}

TEST_CASE("Clear board is empty")
{
    REQUIRE(clear_board.isEmpty());
}

TEST_CASE("Board with value repeated in line / column is invalid")
{
    REQUIRE(!invalid_board_col_line.isValid());
}

TEST_CASE("Board with value repeated in section is invalid")
{
    REQUIRE(!invalid_board_section.isValid());
}

TEST_CASE("Completed board is valid")
{ 
    REQUIRE(solved_board.isValid());
}

TEST_CASE("Incomplete board can be valid")
{ 
    REQUIRE(board_with_blanks.isValid());
}

TEST_CASE("Board assigned from another is equal to the original") 
{
    Board another_solved_board;
    another_solved_board = solved_board;
    REQUIRE(another_solved_board == solved_board);
}

TEST_CASE("Board copy generates equal boards") 
{
    Board board_copy(solved_board);
    REQUIRE(board_copy == solved_board);
}
