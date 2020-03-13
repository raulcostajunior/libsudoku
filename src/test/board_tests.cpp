#define CATCH_CONFIG_MAIN

#include "../board.h"
#include "catch.hpp"

using namespace sudoku;

const Board solved_board(
    // clang-format off
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
    }  // clang-format on
);

const Board invalid_board_value_range(
    // clang-format off
    {
        2, 19, 5, 7, 4, 3, 8, 6, 1, // 19 in the second column is out of range.
        4,  3, 1, 8, 6, 5, 9, 2, 7,
        8,  7, 6, 1, 9, 2, 5, 4, 3,
        3,  8, 7, 4, 5, 9, 2, 1, 6,
        6,  1, 2, 3, 8, 7, 4, 9, 5,
        5,  4, 9, 2, 1, 6, 7, 3, 8,
        7,  6, 3, 5, 2, 4, 1, 8, 9,
        9,  2, 8, 6, 7, 1, 3, 5, 4,
        1,  5, 4, 9, 3, 8, 6, 7, 2,
    }  // clang-format on
);

const Board invalid_board_col_line(
    // clang-format off
    {
        5, 1, 6, 8, 4, 9, 7, 3, 2, // 2 is repeated in the fourth column.
        3, 2, 7, 6, 1, 5, 4, 8, 9, // 3 is repeated in the fourth column.
        8, 4, 9, 7, 2, 3, 1, 6, 5, // 9 is repeated in the fifth column.
        1, 3, 5, 2, 6, 8, 9, 4, 7, // 4 is repeated in the sixth column.
        4, 7, 2, 5, 9, 1, 3, 8, 6, // 1 is repeated in the seventh column.
        9, 6, 8, 3, 7, 4, 1, 5, 2, // 9 is repeated in the seventh column.
        2, 5, 3, 1, 8, 6, 9, 7, 4, // 8 is repeated in the eighth column.
        6, 8, 4, 2, 9, 7, 5, 1, 3, // 2 is repeated in the ninth column.
        7, 9, 1, 3, 5, 4, 6, 2, 8,
    }  // clang-format on
);

const Board invalid_board_section(
    // clang-format off
    {
        2, 9, 5, 7, 4, 3, 8, 6, 1, // 2 repeated in first section and 2nd. column.
        4, 2, 1, 8, 6, 5, 9, 3, 7, // 3 repeated in third section and 8th. column.
        8, 7, 6, 1, 9, 2, 5, 4, 3,
        3, 8, 7, 4, 5, 9, 2, 1, 6,
        6, 1, 2, 3, 8, 7, 4, 9, 5,
        5, 4, 9, 2, 1, 6, 7, 3, 8,
        7, 6, 3, 5, 2, 4, 1, 8, 9,
        9, 2, 8, 6, 7, 1, 3, 5, 4,
        1, 5, 4, 9, 3, 8, 6, 7, 2,
    }  // clang-format on
);

const Board board_with_blanks(
    // clang-format off
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
    }  // clang-format on
);

const Board clear_board(
    // clang-format off
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
    }  // clang-format on
);

TEST_CASE("Board initially empty") {
    Board b;
    REQUIRE(b.isEmpty());
}

TEST_CASE("Invalid board isn't complete") {
    REQUIRE(!invalid_board_col_line.isComplete());
    REQUIRE(!invalid_board_section.isComplete());
}

TEST_CASE("Board with blank isn't complete") {
    REQUIRE(!board_with_blanks.isComplete());
}

TEST_CASE("Clear board isn't complete") { REQUIRE(!clear_board.isComplete()); }

TEST_CASE("Clear board is empty") { REQUIRE(clear_board.isEmpty()); }

TEST_CASE("Board with value out of range is invalid") {
    REQUIRE(!invalid_board_value_range.isValid());
    const auto invalidPos = invalid_board_value_range.getInvalidPositions();
    // The invalid value, 19, is at line 0 and column 1 and is the only
    // invalid position.
    REQUIRE(invalidPos.size() == 1);
    REQUIRE(invalidPos[0].first == 0);
    REQUIRE(invalidPos[0].second == 1);
}

TEST_CASE("Board with value repeated in line / column is invalid") {
    REQUIRE(!invalid_board_col_line.isValid());
    const auto invalidPos = invalid_board_col_line.getInvalidPositions();
    // The repeated positions are commented in front of the board initialization
    // vector. There are 16 repetitions - 4 in column 3, 2 in column 4, 2 in
    // column 5, 4 in column 6, 2 in column 7 and 2 in column 8.
    REQUIRE(invalidPos.size() == 16);
    std::vector<int> invalidsPerCol(9);
    for (size_t i = 0; i < invalidPos.size(); i++) {
        invalidsPerCol[invalidPos[i].second]++;
    }
    REQUIRE(invalidsPerCol[3] == 4);
    REQUIRE(invalidsPerCol[4] == 2);
    REQUIRE(invalidsPerCol[5] == 2);
    REQUIRE(invalidsPerCol[6] == 4);
    REQUIRE(invalidsPerCol[7] == 2);
    REQUIRE(invalidsPerCol[8] == 2);
}

TEST_CASE("Board with value repeated in section is invalid") {
    REQUIRE(!invalid_board_section.isValid());
    const auto invalidPos = invalid_board_section.getInvalidPositions();
    REQUIRE(invalidPos.size() == 6);
    // All the invalid repeated values should be either '2' or '3'.
    for (size_t i = 0; i < invalidPos.size(); i++) {
        int repVal = invalid_board_section.valueAt(invalidPos[i].first,
                                                   invalidPos[i].second);
        REQUIRE((repVal == 2 || repVal == 3));
    }
}

TEST_CASE("Completed board is valid") {
    REQUIRE(solved_board.isValid());
    const auto invalidPos = solved_board.getInvalidPositions();
    REQUIRE(invalidPos.size() == 0);
}

TEST_CASE("Incomplete board can be valid") {
    REQUIRE(board_with_blanks.isValid());
    const auto invalidPos = board_with_blanks.getInvalidPositions();
    REQUIRE(invalidPos.size() == 0);
}

TEST_CASE("Board assigned from another is equal to the original") {
    Board another_solved_board;
    another_solved_board = solved_board;
    REQUIRE(another_solved_board == solved_board);
}

TEST_CASE("Board copy generates equal boards") {
    Board board_copy(solved_board);
    REQUIRE(board_copy == solved_board);
}

TEST_CASE("Set value with out-of-range value is rejected") {
    Board board(board_with_blanks);
    auto result = board.setValueAt(0, 0, 12);
    REQUIRE((result == SetValueResult::InvalidValue));
    REQUIRE(board == board_with_blanks);  // Board has not been changed.
}

TEST_CASE("Set value that makes board invalid is rejected") {
    Board board(board_with_blanks);
    auto result = board.setValueAt(0, 4, 6);  // The correct value would be 4.
    REQUIRE((result == SetValueResult::ValueInvalidatesBoard));
    REQUIRE(board == board_with_blanks);  // Board has not been changed.
}

TEST_CASE("Proper set value is accepted") {
    Board board(board_with_blanks);
    auto result = board.setValueAt(0, 4, 4);
    REQUIRE((result == SetValueResult::NoError));
    REQUIRE(board.valueAt(0, 4) == 4);
}

TEST_CASE(
    "Possible values for an empty position don't make the board invalid") {
    Board board(board_with_blanks);
    REQUIRE(board.isValid());
    const auto possibleValues = board.getPossibleValues(0, 4);
    REQUIRE(possibleValues.size() == 1);
    REQUIRE(possibleValues.count(4) == 1);
    board.setValueAt(0, 4, 4);
    REQUIRE(board.isValid());
}

TEST_CASE("No possible value is returned for a non empty position") {
    Board board(solved_board);
    const auto possibleValues = board.getPossibleValues(0, 0);
    REQUIRE(possibleValues.empty());
}

TEST_CASE(
    "Possible values doesn't include any value in same line, column or "
    "section") {
    Board board;  // Board is the clear board
    board.setValueAt(0, 0, 1);
    board.setValueAt(1, 1, 6);
    board.setValueAt(8, 1, 4);
    const auto possibleValues = board.getPossibleValues(0, 1);
    REQUIRE(possibleValues.size() == 6);
    REQUIRE(possibleValues.count(1) == 0);
    REQUIRE(possibleValues.count(6) == 0);
    REQUIRE(possibleValues.count(4) == 0);
}
