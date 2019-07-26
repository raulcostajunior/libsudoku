#include <cstdint>
#include <utility>

#include "board.h"

using namespace sudoku;
using namespace std;

    uint8_t Board::valueAt(uint8_t line, uint8_t column) const noexcept {
        // TODO: add real body
        return 0;
    }

    pair<bool, BoardValueError> Board::setValueAt(uint8_t line, uint8_t column, uint8_t value) {
        // TODO: add real body
        return make_pair(true, BoardValueError::NO_ERROR);
    }

    void Board::clear() {
        // TODO: add real body
    }; 

    bool Board::isValid() {
        // TODO: add real body
        return true;
    }
}