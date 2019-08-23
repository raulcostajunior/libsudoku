#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../board.h"

using namespace sudoku;

const Board solved_board(
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

const Board invalid_board_col_line(
    {
        5, 1, 6, 8, 4, 9, 7, 3, 2, // 2 is repeated in the third and last columns.
        3, 2, 7, 6, 1, 5, 4, 8, 9,
        8, 4, 9, 7, 2, 3, 1, 6, 5,
        1, 3, 5, 2, 6, 8, 9, 4, 7,
        4, 7, 2, 5, 9, 1, 3, 8, 6,
        9, 6, 8, 3, 7, 4, 1, 5, 2,
        2, 5, 3, 1, 8, 6, 9, 7, 4,
        6, 8, 4, 2, 9, 7, 5, 1, 3,
        7, 9, 1, 3, 5, 4, 6, 2, 8,
    }
);


const Board invalid_board_section(
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

const Board board_with_blanks(
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

TEST_CASE("Set value with out-of-range value is rejected")
{
    Board board(board_with_blanks);
    auto result = board.setValueAt(0, 0, 12);
    REQUIRE((result == SetValueResult::INVALID_VALUE));
    REQUIRE(board == board_with_blanks); // Board has not been changed.
}

TEST_CASE("Set value that makes board invalid is rejected")
{
    Board board(board_with_blanks);
    auto result = board.setValueAt(0, 4, 6); // The correct value would be 4.
    REQUIRE((result == SetValueResult::VALUE_INVALIDATES_BOARD));
    REQUIRE(board == board_with_blanks); // Board has not been changed.
}

TEST_CASE("Proper set value is accepted")
{
    Board board(board_with_blanks);
    auto result = board.setValueAt(0, 4, 4);
    REQUIRE((result == SetValueResult::NO_ERROR));
    REQUIRE(board.valueAt(0, 4) == 4);
}
