#include <cstdint>
#include <utility>

#include "board.h"

using namespace sudoku;
using namespace std;

uint8_t Board::valueAt(uint8_t line, uint8_t column) const noexcept
{
    if (line < 9 && column < 9)
    {
        return _values[line][column];
    }

    return 0;
}

pair<bool, BoardValueError> Board::setValueAt(uint8_t line, uint8_t column, uint8_t value)
{
    // TODO: add real body
    return make_pair(true, BoardValueError::NO_ERROR);
}

void Board::clear()
{
    for (uint8_t i = 0; i < 9; i++) {
        for (uint8_t j = 0; j < 9; j++) {
            _values[i][j] = 0;
        }
    }
}

bool Board::isValid() const noexcept
{
    // TODO: add real body
    return true;
}
