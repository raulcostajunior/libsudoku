#include <algorithm>
#include <iostream>
#include <cstdint>
#include <utility>
#include <vector>

#include "board.h"

using namespace std;

namespace sudoku { 

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

pair<bool, BoardValueError> Board::setValueAt(uint8_t line, uint8_t column, uint8_t value) noexcept
{
    // TODO: add real body
    return make_pair(true, BoardValueError::NO_ERROR);
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
    // Checks if any value other than blank repeats across
    // any line.
    for (uint8_t lin = 0; lin < 8; lin++)
    {
        for (uint8_t col = 0; col < 8; col++)
        {
            uint8_t val = _values[lin][col];
            if (val != 0)
            {
                for (uint8_t pos = col + 1; pos < 9; pos++)
                {
                    if (val == _values[lin][pos])
                    {
                        // Found a repetition in the line.
                        return false;
                    }
                }
            }
        }
    }

    // Checks if any value other than blank repeats across
    // any column.
    for (uint8_t col = 0; col < 8; col++)
    {
        for (uint8_t lin = 0; lin < 8; lin++)
        {
            uint8_t val = _values[lin][col];
            if (val != 0)
            {
                for (uint8_t pos = lin + 1; lin < 9; lin++)
                {
                    if (val == _values[pos][col])
                    {
                        // Found a repetition in the column.
                        return false;
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
        for (uint8_t lin = initialLin; lin < initialLin + 3; lin++)
        {
            for (uint8_t col = initialCol; col < initialCol + 3; col++)
            {
                uint8_t secVals[9]{0, 0, 0, 0, 0, 0, 0, 0, 0};
                uint8_t secValsIdx = 0;
                uint8_t val = _values[lin][col];
                if (val != 0)
                {
                    for (uint8_t i = 0; i < secValsIdx; i++)
                    {
                        if (secVals[i] == val)
                        {
                            // Value is repeated in the section.
                            return false;
                        }
                    }
                    // Value is new in the section; store it for
                    // comparing with other section values that will
                    // be scanned.
                    secVals[secValsIdx] = val;
                }
                secValsIdx++;
            }
        }
    }

    return true;
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


Board& Board::operator=(const Board &board) noexcept
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
    for (uint8_t lin = 0; lin < 9; lin++) {
        for (uint8_t col = 0; col < 9; col++) {
            os << board._values[lin][col] << " ";
        }
        os << endl;
    }
    return os;
}

} // namespace sudoku
