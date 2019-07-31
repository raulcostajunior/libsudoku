#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <utility>

namespace sudoku
{

enum class BoardValueError : std::uint8_t
{
    NO_ERROR,
    INVALID_VALUE,
    VALUE_ALREADY_IN_ROW,
    VALUE_ALREADY_IN_COLUMN,
    VALUE_ALREADY_IN_SECTION // In the same 3x3 board section.
};

/**
 * @brief A 9x9 Sudoku board.
*/
class Board
{

public:
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
         * @return a pair with 'true' in its first position and 'BoardValueError::NO_ERROR'
         * on its second position if the value could be set. Otherwise returns a pair
         * with 'false' in its first position and the error reason code in its second
         * position.
         */
    std::pair<bool, BoardValueError> setValueAt(std::uint8_t line, std::uint8_t column, std::uint8_t value);

    /**
         * Clears the board by assigning the value 0 to all its positions.
         */
    void clear();

    /**
     *  Returns true if none of the values in the board violates the Sudoku non-repetition 
     *  rules  across a line, a column or a 3x3 section.
     */
    bool isValid() const noexcept;


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

#endif