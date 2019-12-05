#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <iostream>
#include <vector>

namespace sudoku
{

enum class SetValueResult : std::uint8_t
{
    NoError,
    InvalidValue,
    ValueInvalidatesBoard
};

template <typename CellValueType>
class BoardBase
{
    /**
         * Returns the side length of the small squares the board is composed
         * of. Each of those small squares must contain all the symbols.
         * 
         * @return the length of one of the small squares on the board.
         */
    virtual std::uint8_t smallSquareSide() const noexcept = 0;

    /**
         * Retrieves the number of distinct symbols that may appear on the
         * board.
         * 
         * @return the number of possible symbols.
         */
    std::uint8_t numberOfSymbols() const noexcept
        { return smallSquareSide()*smallSquareSide(); };

    /**
         * Retrieves the value at a given (line, column) coordinate of the
         * board. 
         * 
         * @param line the line number (starting at 0).
         * @param column the column number (starting at 0).
         * @return the value at position (line, column) of the board.
         */
    virtual CellValueType valueAt(std::uint8_t line, std::uint8_t column) const noexcept = 0;

    /**
         * Sets the value at a given (line, column) coordinate of the board. 
         * 
         * @param line the line number (starting at 0).
         * @param column the column number (starting at 0).
         * @param the value to be set at position (line, column) of the board.
         * @return a SetValueResult indicating the result of the operation. If 
         * the return is not SetValueResult::NoError, the board won't be changed.
         */
    virtual SetValueResult setValueAt(std::uint8_t line, std::uint8_t column, CellValueType value) = 0;

};

/**
 * @brief A 9x9 Sudoku board.
*/
class Board: public BoardBase<std::uint8_t>
{

public:

    Board() = default;

    explicit Board(const std::vector<std::uint8_t> &values);

    Board(const Board &board);

    /**
         * Retrieves the length of one of the nine 3x3 squares that make up
         * the full 9x9 board. For this class, obviously, this is always 3.
         * 
         * @return 3.
         */
    std::uint8_t smallSquareSide() const noexcept { return 3; };

    /**
         * Retrieves the value at a given (line, column) coordinate of the 
         * board. 
         * 
         * @param line the line number (from 0 to 8).
         * @param column the column number (from 0 to 8).
         * @return the value at position (line, column) of the board. If the
         * position is filled, the value will be a number from 1 to 9. If the
         * position is empty, the value will be 0.
         */
    std::uint8_t valueAt(std::uint8_t line, std::uint8_t column) const noexcept;

    /**
         * Sets the value at a given (line, column) coordinate of the board. 
         * 
         * @param line the line number (from 0 to 8).
         * @param column the column number (from 0 to 8).
         * @param the value to be set at position (line, column) of the board. Can
         * be a value from 0 to 9 - 0 meaning empty. 
         * @return a SetValueResult indicating the result of the operation. If 
         * the return is not SetValueResult::NO_ERROR, the board won't be changed.
         */
    SetValueResult setValueAt(std::uint8_t line, std::uint8_t column, std::uint8_t value);

    /**
         * Clears the board by assigning the value 0 to all its positions.
         */
    void clear() noexcept;

    /**
        *  Returns true if none of the values in the board violates the Sudoku non-repetition 
        *  rules  across a line, a column or a 3x3 section.
        */
    bool isValid() const noexcept;

    /**
        * Returns true if all the positions in the board are blank (equal to 0).
        */
    bool isEmpty() const noexcept;

    /*
     * Returns the number of blank positions in the board.
     */
    uint8_t blankPositionCount() const noexcept;

    /**
        * Returns true if a board has no blank position and is valid -
        * in other words, the board corresponds to a solved puzzle.
        */
    bool isComplete() const noexcept;

    bool operator==(const Board &board) const noexcept;

    Board &operator=(const Board &board) noexcept;

private:
    std::uint8_t _values[9][9]{
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0}};
};

} // namespace sudoku

std::ostream &operator<<(std::ostream &os, const sudoku::Board &board);

#endif
