#include <algorithm>
#include <cstdint>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

#include "board.h"

using namespace std;
using namespace sudoku;

Board::Board(const vector<uint8_t> &values)
{
    size_t upperBound = min(static_cast<size_t>(81), values.size());
    for (size_t i = 0; i < upperBound; i++)
    {
        size_t lin = i / 9;
        size_t col = i % 9;
        _values[lin][col] = values[i];
    }
}

Board::Board(const Board &board)
{
    for (uint8_t lin = 0; lin < 9; lin++)
    {
        for (uint8_t col = 0; col < 9; col++)
        {
            _values[lin][col] = board._values[lin][col];
        }
    }
}

uint8_t Board::valueAt(uint8_t line, uint8_t column) const noexcept
{
    if (line < 9 && column < 9)
    {
        return _values[line][column];
    }

    return 0;
}

uint8_t Board::blankPositionCount() const noexcept
{
    uint8_t nBlanks = 0;

    for (uint8_t lin = 0; lin < 9; lin++)
    {
        for (uint8_t col = 0; col < 9; col++)
        {
            if (_values[lin][col] == 0)
            {
                nBlanks++;
            }
        }
    }

    return nBlanks;
}

SetValueResult Board::setValueAt(uint8_t line, uint8_t column, uint8_t value)
{
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

    return SetValueResult::NoError;
}

void Board::clear() noexcept
{
    for (uint8_t i = 0; i < 9; i++)
    {
        for (uint8_t j = 0; j < 9; j++)
        {
            _values[i][j] = 0;
        }
    }
}

bool Board::isValid() const noexcept
{
    // We stop at the first invalid position because that is
    // enough to tell a board is not valid.
    const auto &invalidPositions = findInvalidPositions(true);

    return invalidPositions.empty();
}

vector<pair<uint8_t, uint8_t>> Board::getInvalidPositions() const noexcept
{
    return findInvalidPositions(false);
}

vector<pair<uint8_t, uint8_t>> Board::findInvalidPositions(bool stopAtFirst) const noexcept
{
    vector<pair<uint8_t, uint8_t>> invalidPositions;

    // Checks if any value is out of the allowed range
    for (uint8_t lin = 0; lin < 9; lin++)
    {
        for (uint8_t col = 0; col < 9; col++)
        {
            if (_values[lin][col] > 9)
            {
                invalidPositions.push_back(make_pair(lin, col));
                if (stopAtFirst) return invalidPositions;
            }
        }
    }

    // Checks if any value other than blank repeats across
    // any line.
    for (uint8_t lin = 0; lin < 9; lin++)
    {
        for (uint8_t col = 0; col < 8; col++)
        {
            uint8_t val = _values[lin][col];
            if (val != 0 && val <= 9)
            {
                for (uint8_t pos = col + 1; pos < 9; pos++)
                {
                    if (val == _values[lin][pos])
                    {
                        // Found a repetition in the line.
                        invalidPositions.push_back(make_pair(lin, col));
                        if (stopAtFirst) return invalidPositions;
                        // Also adds the position of the repeated item to the
                        // invalid list.
                        invalidPositions.push_back(make_pair(lin, pos));
                    }
                }
            }
        }
    }

    // Checks if any value other than blank repeats across
    // any column.
    for (uint8_t col = 0; col < 9; col++)
    {
        for (uint8_t lin = 0; lin < 8; lin++)
        {
            uint8_t val = _values[lin][col];
            if (val != 0 && val <= 9)
            {
                for (uint8_t pos = lin + 1; pos < 9; pos++)
                {
                    if (val == _values[pos][col])
                    {
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
    for (uint8_t sec = 0; sec < 9; sec++)
    {
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

        for (uint8_t lin = initialLin; lin < initialLin + 3; lin++)
        {
            for (uint8_t col = initialCol; col < initialCol + 3; col++)
            {
                uint8_t val = _values[lin][col];
                if (val != 0 && val <= 9)
                {
                    // Value is not blank; check if it has already happened in section.
                    bool isFirstOccurrence = true;
                    for (size_t i = 0; i < secVals.size(); i++)
                    {
                        if (get<0>(secVals[i]) == val)
                        {
                            // Value is repeated in the section.
                            invalidPositions.push_back(make_pair(lin, col));
                            if (stopAtFirst) return invalidPositions;
                            get<3>(secVals[i]) = true;
                            isFirstOccurrence = false;
                        }
                    }
                    if (isFirstOccurrence)
                    {
                        // Registers the first occurrence of the value in the section.
                        secVals.push_back(make_tuple(val, lin, col, false));
                    }
                }
            }
        }
        // Registers the first occurrences of repeated values in the section.
        for (const auto &secVal:secVals)
        {
            if (get<3>(secVal))
            {
                invalidPositions.push_back(make_pair(get<1>(secVal), get<2>(secVal)));
                // No need to test for stopAtFirst - if it was true, this point would have
                // not been reached.
            }
        }
    }

    // Eliminates duplicates - some invalid positions might have been included more than once
    // when evaluated against different invalidation conditions.
    const auto &uniquesEnd = unique(invalidPositions.begin(), invalidPositions.end());
    invalidPositions.resize(distance(invalidPositions.begin(), uniquesEnd));

    return invalidPositions;
}

bool Board::isEmpty() const noexcept
{
    bool empty = true;
    for (uint8_t i = 0; i < 81; i++)
    {
        if (_values[i / 9][i % 9] != 0)
        {
            empty = false;
            break;
        }
    }
    return empty;
}

bool Board::isComplete() const noexcept
{
    bool anyBlank = false;

    for (uint8_t i = 0; i < 81; i++)
    {
        if (_values[i / 9][i % 9] == 0)
        {
            anyBlank = true;
            break;
        }
    }

    if (anyBlank)
    {
        return false;
    }
    else
    {
        // A board with no blank position is
        // completed if and only if it is valid.
        return isValid();
    }
}

bool Board::operator==(const Board &board) const noexcept
{
    for (uint8_t lin = 0; lin < 9; lin++)
    {
        for (uint8_t col = 0; col < 9; col++)
        {
            if (_values[lin][col] != board._values[lin][col])
            {
                return false;
            }
        }
    }
    return true;
}

Board &Board::operator=(const Board &board) noexcept
{
    for (uint8_t lin = 0; lin < 9; lin++)
    {
        for (uint8_t col = 0; col < 9; col++)
        {
            _values[lin][col] = board._values[lin][col];
        }
    }
    return *this;
}

ostream &operator<<(ostream &os, const Board &board)
{
    for (uint8_t lin = 0; lin < 9; lin++)
    {
        for (uint8_t col = 0; col < 9; col++)
        {
            os << static_cast<int>(board.valueAt(lin, col)) << " ";
        }
        os << endl;
    }
    return os;
}
