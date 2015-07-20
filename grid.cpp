#include <iostream>
#include "grid.h"

Grid::Grid()
{
}

Grid::Grid(const char* sudoku)
{
    assert(sudoku != nullptr);
    for (int y = 0; y < 9; ++y) {
        for (int x = 0; x < 9; ++x) {
            std::uint8_t digit = static_cast<std::uint8_t>(*sudoku - '0');
            if (digit == 0)
                m_digits[y * 9 + x] = Digits(Digits::ALL);
            else
                m_digits[y * 9 + x] = Digits(digit);
            ++sudoku;
        }
        if (y < 8) {
            assert(*sudoku == '\n');
            ++sudoku;
        }
    }
}

Grid::DigitsRangeProxy<Grid::iterator> Grid::getRow(const int row)
{
    assert(row >= 0 && row < 9);
    return DigitsRangeProxy<iterator>(iterator(&m_digits[0] + row * 9, iterator::ROW_TYPE),
                                      iterator(&m_digits[0] + row * 9 + 9, iterator::ROW_TYPE));
}

Grid::DigitsRangeProxy<Grid::const_iterator> Grid::getRow(const int row) const
{
    assert(row >= 0 && row < 9);
    return DigitsRangeProxy<const_iterator>(const_iterator(&m_digits[0] + row * 9, const_iterator::ROW_TYPE),
                                            const_iterator(&m_digits[0] + row * 9 + 9, const_iterator::ROW_TYPE));
}

Grid::DigitsRangeProxy<Grid::iterator> Grid::getColumn(const int column)
{
    assert(column >= 0 && column < 9);
    return DigitsRangeProxy<iterator>(iterator(&m_digits[0] + column, iterator::COL_TYPE),
                                      iterator(&m_digits[0] + column + 9 * 9, iterator::COL_TYPE));
}

Grid::DigitsRangeProxy<Grid::const_iterator> Grid::getColumn(const int column) const
{
    assert(column >= 0 && column < 9);
    return DigitsRangeProxy<const_iterator>(const_iterator(&m_digits[0] + column, const_iterator::COL_TYPE),
                                            const_iterator(&m_digits[0] + column + 9 * 9, const_iterator::COL_TYPE));
}


Grid::DigitsRangeProxy<Grid::iterator> Grid::getBox(const int x, const int y)
{
    assert(x >= 0 && x < 3 && y >= 0 && y < 3);
    const int offset = y * 27 + x * 3;
    return DigitsRangeProxy<iterator>(iterator(&m_digits[0] + offset, iterator::BOX_TYPE),
                                      iterator(&m_digits[0] + offset + 27, iterator::BOX_TYPE));
}

Grid::DigitsRangeProxy<Grid::const_iterator> Grid::getBox(const int x, const int y) const
{
    assert(x >= 0 && x < 3 && y >= 0 && y < 3);
    const int offset = y * 27 + x * 3;
    return DigitsRangeProxy<const_iterator>(const_iterator(&m_digits[0] + offset, const_iterator::BOX_TYPE),
                                            const_iterator(&m_digits[0] + offset + 27, const_iterator::BOX_TYPE));
}


Digits Grid::getSolvedInColumn(const int column) const
{
    Digits result(Digits::NONE);
    for (const auto& el : getColumn(column)) {
        if (el.size() == 1)
            result |= el;
    }
    return result;
}

Digits Grid::getSolvedInRow(const int row) const
{
    Digits result(Digits::NONE);
    for (const auto& el : getRow(row)) {
        if (el.size() == 1)
            result |= el;
    }
    return result;
}

Digits Grid::getSolvedInBox(const int x, const int y) const
{
    Digits result(Digits::NONE);
    for (const auto& el : getBox(x, y)) {
        if (el.size() == 1)
            result |= el;
    }
    return result;
}

bool Grid::update()
{
    bool result = false;

    // columns
    for (int column = 0; column < 9; ++column) {
        const auto todo = ~getSolvedInColumn(column);
        for (auto& d : getColumn(column)) {
            if (d.size() > 1) {
                const auto tmp = d;
                d &= todo;
                result |= (tmp != d);
            }
        }
    }

    // rows
    for (int row = 0; row < 9; ++row) {
        const auto todo = ~getSolvedInRow(row);
        for (auto& d : getRow(row)) {
            if (d.size() > 1) {
                const auto tmp = d;
                d &= todo;
                result |= (d != tmp);
            }
        }
    }

    // boxes
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            const auto todo = ~getSolvedInBox(x, y);
            for (auto& d : getBox(x, y)) {
                if (d.size() > 1) {
                    const auto tmp = d;
                    d &= todo;
                    result |= (tmp != d);
                }
            }
        }
    }

    return result;
}

Grid::State Grid::getState() const
{
    for (const auto& el : m_digits) {
        const auto s = el.size();
        if (s == 0)
            return UNSOLVABLE;
        else if (s > 1)
            return INCOMPLETE;
    }
    return SOLVED;
}

bool Grid::isCorrectSolution() const
{
    for (int i = 0; i < 9; ++i) {
        int sum_col = 0;
        for (const auto& el : getColumn(i))
            if (el.size() == 1)
                sum_col += el.getDigit();
        if (sum_col != 45)
            return false;
        int sum_row = 0;
        for (const auto& el : getRow(i))
            if (el.size() == 1)
                sum_row += el.getDigit();
        if (sum_row != 45)
            return false;
    }
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            int sum = 0;
            for (const auto& el : getBox(x, y))
                if (el.size() == 1)
                    sum += el.getDigit();
            if (sum != 45)
                return false;
        }
    }
    return true;
}

bool operator==(const Grid& g0, const Grid& g1)
{
    for (int i = 0; i < 9 * 9; ++i) {
        if (g0.m_digits[i] != g1.m_digits[i])
            return false;
    }
    return true;
}

bool operator!=(const Grid& g0, const Grid& g1)
{
    for (int i = 0; i < 9 * 9; ++i) {
        if (g0.m_digits[i] != g1.m_digits[i])
            return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& out, const Grid& grid)
{
    int cnt = 0;
    for (int y = 0; y < 9; ++y) {
        if (y % 3 == 0)
            out << " +-------+-------+-------+\n";
        for (int x = 0; x < 9; ++x) {
            if (x % 3 == 0)
                out << " |";
            const Digits& d = grid.m_digits[cnt++];
            out << ' ';
            if (d.size() == 1)
                out << static_cast<int>(d.getDigit());
            else
                out << '.';
        }
        out << " |\n";
    }
    out << " +-------+-------+-------+\n";
    return out;
}
