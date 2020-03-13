#include "board.h"

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;
using namespace sudoku;

Board::Board(const vector<uint8_t> &values) noexcept {
    size_t upperBound = min(static_cast<size_t>(Board::NUM_POS), values.size());
    for (size_t i = 0; i < upperBound; i++) {
        size_t lin = i / Board::NUM_COLS;
        size_t col = i % Board::NUM_ROWS;
        _values[lin][col] = values[i];
    }
}

Board::Board(const Board &board) {
    for (uint8_t lin = 0; lin < Board::NUM_ROWS; lin++) {
        for (uint8_t col = 0; col < Board::NUM_COLS; col++) {
            _values[lin][col] = board._values[lin][col];
        }
    }
}

uint8_t Board::valueAt(uint8_t line, uint8_t column) const noexcept {
    if (line < Board::NUM_ROWS && column < Board::NUM_COLS) {
        return _values[line][column];
    }

    return 0;
}

uint8_t Board::blankPositionCount() const noexcept {
    uint8_t nBlanks = 0;

    for (uint8_t lin = 0; lin < Board::NUM_ROWS; lin++) {
        for (uint8_t col = 0; col < Board::NUM_COLS; col++) {
            if (_values[lin][col] == 0) {
                nBlanks++;
            }
        }
    }

    return nBlanks;
}

SetValueResult Board::setValueAt(uint8_t line, uint8_t column, uint8_t value) {
    if (value > 9) {
        return SetValueResult::InvalidValue;
    }

    Board valueSetBoard(*this);
    valueSetBoard._values[line][column] = value;
    if (valueSetBoard.isValid()) {
        // Value won't invalidate the board, so go ahead and set it.
        _values[line][column] = value;
        return SetValueResult::NoError;
    } else {
        return SetValueResult::ValueInvalidatesBoard;
    }
}

set<uint8_t> Board::getPossibleValues(uint8_t line, uint8_t column) const
    noexcept {
    set<uint8_t> pvs;
    if (_values[line][column] == 0) {
        // Position is empty - can go ahead and search for possible values.
        pvs.insert({1, 2, 3, 4, 5, 6, 7, 8, 9});
        for (size_t i = 0; i < Board::NUM_COLS; i++) {
            pvs.erase(_values[line][i]);
        }
        for (size_t j = 0; j < Board::NUM_ROWS; j++) {
            pvs.erase(_values[j][column]);
        }
        const size_t initLine = line / 3 * 3;
        const size_t initColumn = column / 3 * 3;
        for (size_t i = initLine; i < initLine + 3; i++) {
            for (size_t j = initColumn; j < initColumn + 3; j++) {
                pvs.erase(_values[i][j]);
            }
        }
    }
    return pvs;
}

void Board::clear() noexcept {
    for (uint8_t i = 0; i < Board::NUM_ROWS; i++) {
        for (uint8_t j = 0; j < NUM_COLS; j++) {
            _values[i][j] = 0;
        }
    }
}

bool Board::isValid() const noexcept {
    // We stop at the first invalid position because that is
    // enough to tell a board is not valid.
    const auto &invalidPositions = findInvalidPositions(true);

    return invalidPositions.empty();
}

vector<pair<uint8_t, uint8_t>> Board::getInvalidPositions() const noexcept {
    return findInvalidPositions(false);
}

vector<pair<uint8_t, uint8_t>> Board::findInvalidPositions(
    bool stopAtFirst) const noexcept {
    vector<pair<uint8_t, uint8_t>> invalidPositions;

    // Checks if any value is out of the allowed range
    for (uint8_t lin = 0; lin < Board::NUM_ROWS; lin++) {
        for (uint8_t col = 0; col < Board::NUM_COLS; col++) {
            if (_values[lin][col] > 9) {
                invalidPositions.push_back(make_pair(lin, col));
                if (stopAtFirst) return invalidPositions;
            }
        }
    }

    // Checks if any value other than blank repeats across
    // any line.
    for (uint8_t lin = 0; lin < Board::NUM_ROWS; lin++) {
        for (uint8_t col = 0; col < Board::NUM_COLS - 1; col++) {
            uint8_t val = _values[lin][col];
            if (val != 0 && val <= 9) {
                for (uint8_t pos = col + 1; pos < Board::NUM_COLS; pos++) {
                    if (val == _values[lin][pos]) {
                        // Found a repetition in the line.
                        invalidPositions.push_back(make_pair(lin, col));
                        if (stopAtFirst) return invalidPositions;
                        // Also adds the position of the repeated item to
                        // the invalid list.
                        invalidPositions.push_back(make_pair(lin, pos));
                    }
                }
            }
        }
    }

    // Checks if any value other than blank repeats across
    // any column.
    for (uint8_t col = 0; col < Board::NUM_COLS; col++) {
        for (uint8_t lin = 0; lin < Board::NUM_COLS - 1; lin++) {
            uint8_t val = _values[lin][col];
            if (val != 0 && val <= 9) {
                for (uint8_t pos = lin + 1; pos < Board::NUM_ROWS; pos++) {
                    if (val == _values[pos][col]) {
                        // Found a repetition in the column.
                        invalidPositions.push_back(make_pair(lin, col));
                        if (stopAtFirst) return invalidPositions;
                        // Also adds the position of the repeated item to
                        // the invalid list.
                        invalidPositions.push_back(make_pair(pos, col));
                    }
                }
            }
        }
    }

    // Checks if any value other than blank repeats across
    // any of the 9 3x3 sections.
    for (uint8_t sec = 0; sec < 9; sec++) {
        uint8_t initialCol = 3 * (sec % 3);
        uint8_t initialLin = 3 * (sec / 3);

        // Values already found in the section.
        // Stores the position of the value and if it has been
        // already found to be repeated.
        // The order of the data in the tuples of secVals is:
        //     0 - the value;
        //     1 - the line of the value's first occurrence;
        //     2 - the column of the value's first occurrence;
        //     3 - has the value repeated at least once?
        // The value will be store in secVals only at the time
        // it's first found on the section. More than two
        // occurrences of the value can happen; to avoid multiple
        // inclusions of the first occurrence for such cases, the
        // first occurrence of the values in the section are inserted
        // at the end of the section run for those values that had
        // at least one repetition.
        vector<tuple<uint8_t, uint8_t, uint8_t, bool>> secVals;

        for (uint8_t lin = initialLin; lin < initialLin + 3; lin++) {
            for (uint8_t col = initialCol; col < initialCol + 3; col++) {
                uint8_t val = _values[lin][col];
                if (val != 0 && val <= 9) {
                    // Value is not blank; check if it has already happened
                    // in section.
                    bool isFirstOccurrence = true;
                    for (size_t i = 0; i < secVals.size(); i++) {
                        if (get<0>(secVals[i]) == val) {
                            // Value is repeated in the section.
                            invalidPositions.push_back(make_pair(lin, col));
                            if (stopAtFirst) return invalidPositions;
                            get<3>(secVals[i]) = true;
                            isFirstOccurrence = false;
                        }
                    }
                    if (isFirstOccurrence) {
                        // Registers the first occurrence of the value in
                        // the section.
                        secVals.push_back(make_tuple(val, lin, col, false));
                    }
                }
            }
        }
        // Registers the first occurrences of repeated values in the
        // section.
        for (const auto &secVal : secVals) {
            if (get<3>(secVal)) {
                invalidPositions.push_back(
                    make_pair(get<1>(secVal), get<2>(secVal)));
                // No need to test for stopAtFirst - if it was true, this
                // point would have not been reached.
            }
        }
    }

    // Eliminates duplicates - some invalid positions might have been
    // included more than once when evaluated against different invalidation
    // conditions.
    sort(
        invalidPositions.begin(), invalidPositions.end(),
        [](const pair<uint8_t, uint8_t> &p1, const pair<uint8_t, uint8_t> &p2) {
            return (p1.first * 10 + p1.second < p2.first * 10 + p2.second);
        });

    const auto &uniquesEnd =
        unique(invalidPositions.begin(), invalidPositions.end(),
               [](const pair<uint8_t, uint8_t> &p1,
                  const pair<uint8_t, uint8_t> &p2) -> bool {
                   return (p1.first == p2.first && p1.second == p2.second);
               });

    invalidPositions.resize(
        static_cast<size_t>(distance(invalidPositions.begin(), uniquesEnd)));

    return invalidPositions;
}

bool Board::isEmpty() const noexcept {
    bool empty = true;
    for (uint8_t i = 0; i < Board::NUM_POS; i++) {
        if (_values[i / Board::NUM_ROWS][i % Board::NUM_COLS] != 0) {
            empty = false;
            break;
        }
    }
    return empty;
}

bool Board::isComplete() const noexcept {
    bool anyBlank = false;

    for (uint8_t i = 0; i < Board::NUM_POS; i++) {
        if (_values[i / Board::NUM_ROWS][i % Board::NUM_COLS] == 0) {
            anyBlank = true;
            break;
        }
    }

    if (anyBlank) {
        return false;
    } else {
        // A board with no blank position is
        // completed if and only if it is valid.
        return isValid();
    }
}

bool Board::operator==(const Board &board) const noexcept {
    for (uint8_t lin = 0; lin < Board::NUM_ROWS; lin++) {
        for (uint8_t col = 0; col < Board::NUM_COLS; col++) {
            if (_values[lin][col] != board._values[lin][col]) {
                return false;
            }
        }
    }
    return true;
}

Board &Board::operator=(const Board &board) noexcept {
    if (this == &board) {
        // There's nothing to do
        return *this;
    }
    for (uint8_t lin = 0; lin < Board::NUM_ROWS; lin++) {
        for (uint8_t col = 0; col < Board::NUM_COLS; col++) {
            _values[lin][col] = board._values[lin][col];
        }
    }
    return *this;
}

ostream &operator<<(ostream &os, const Board &board) {
    for (uint8_t lin = 0; lin < Board::NUM_ROWS; lin++) {
        for (uint8_t col = 0; col < Board::NUM_COLS; col++) {
            os << static_cast<int>(board.valueAt(lin, col)) << " ";
        }
        os << endl;
    }
    return os;
}
