#ifndef GRID_H
#define GRID_H

#include <cstdint>
#include <initializer_list>
#include <cassert>
#include <type_traits>
#include <iterator>
#include <array>
#include <ostream>
#include <vector>

inline
std::uint16_t getMask(const std::uint8_t digit)
{
    return 1 << (digit - 1);
}

template <typename T>
class Digit
{
public:
    typename std::enable_if<!std::is_const<T>::value, Digit&>::type
    operator=(bool value)
    {
        assert(m_digit > 0 && m_digit < 10);
        if (value)
            m_value |= getMask(m_digit);
        else {
            m_value &= ~getMask(m_digit);
        }
        return *this;
    }

    operator bool() const { assert(m_digit > 0 && m_digit < 10); return (m_value & getMask(m_digit)) != 0; }
    bool operator!() const { assert(m_digit > 0 && m_digit < 10);  return (m_value & getMask(m_digit)) == 0; }
    std::uint8_t getDigit() const { assert(m_digit > 0 && m_digit < 10); return m_digit; }

private:
    friend class Digits;
    Digit(T value, std::uint8_t digit) : m_value{value}, m_digit{digit} {}

    T               m_value;
    std::uint8_t    m_digit;
};

class Digits
{
public:
    template <typename T>
    class MyIterator
    {
    public:

        Digit<T>& operator*() { assert(m_value.m_digit > 0 && m_value.m_digit < 10); return m_value; }
        const Digit<T>& operator*() const { assert(m_value.m_digit > 0 && m_value.m_digit < 10); return m_value; }

        Digit<T>* operator->() { assert(m_value.m_digit > 0 && m_value.m_digit < 10);  return &m_value; }
        const Digit<T>* operator->() const { assert(m_value.m_digit > 0 && m_value.m_digit < 10); return &m_value; }

        MyIterator& operator++() {
            for (++m_value.m_digit; m_value.m_digit < 10; ++m_value.m_digit) {
                if (m_value)
                    break;
            }
            return *this;
        }

        MyIterator operator++(int) {
            MyIterator tmp{*this};
            ++(*this);
            return tmp;
        }

        bool operator==(const MyIterator& other) const
        {
            return (&(m_value.m_value) == &(other.m_value.m_value)) && (m_value.m_digit == other.m_value.m_digit);
        }

        bool operator!=(const MyIterator& other) const
        {
            return (&(m_value.m_value) != &(other.m_value.m_value)) || (m_value.m_digit != other.m_value.m_digit);
        }

    private:
        friend class Digits;
        MyIterator(T value) : m_value{value, 1} {
            for (; m_value.m_digit < 10; ++m_value.m_digit) {
                if (m_value)
                    break;
            }
        }

        MyIterator(T value, std::uint8_t digit) : m_value{value, digit} {}

        Digit<T>    m_value;
    };

    using iterator = MyIterator < std::uint16_t& > ;
    using const_iterator = MyIterator < const std::uint16_t& > ;

    enum Range { NONE = 0x0000, ALL = 0x01ff};

    Digits() : m_possible{0x01ff} {}
    Digits(Range range) : m_possible{range} {}
    explicit Digits(std::uint8_t n) : m_possible{getMask(n)} { assert(n > 0 && n < 10); }
    Digits(std::initializer_list<std::uint8_t> list) : m_possible{0} {
        for (const auto& el : list) {
            addPossible(el);
        }
        assert(m_possible != 0);
    }

    template <typename T>
    Digits& operator=(const Digit<T>& d) { assert(d.m_digit > 0 && d.m_digit < 10); m_possible = getMask(d.m_digit); return *this; }

    bool check(int n) const { assert(n > 0 && n < 10); return (m_possible & getMask(n)) != 0; }
    void addPossible(int n) { assert(n > 0 && n < 10); m_possible |= getMask(n); }
    void removePossible(int n) { assert(n > 0 && n < 10); m_possible &= ~getMask(n); }
    std::uint8_t size() const { return static_cast<std::uint8_t>(__builtin_popcount(m_possible)); }

    Digit<std::uint16_t&> operator[](int n) { assert(n > 0 && n < 10); return Digit<std::uint16_t&>(m_possible, n); }
    Digit<const std::uint16_t&> operator[](int n) const { assert(n > 0 && n < 10); return Digit<const std::uint16_t&>(m_possible, n); }

    iterator begin() { return iterator(m_possible); }
    iterator end() { return iterator(m_possible, 10); }

    const_iterator begin() const { return const_iterator(m_possible); }
    const_iterator end() const { return const_iterator(m_possible, 10); }

    const_iterator cbegin() const { return const_iterator(m_possible); }
    const_iterator cend() const { return const_iterator(m_possible, 10); }

    std::uint8_t getDigit() const
    {
        assert(size() == 1);
        return static_cast<std::uint8_t>(1 + __builtin_ctz(m_possible));
    }

    Digits& operator&=(const Digits other)
    {
        m_possible &= other.m_possible;
        return *this;
    }

    Digits& operator|=(const Digits other)
    {
        m_possible |= other.m_possible;
        return *this;
    }

    friend Digits operator~(Digits);
    friend bool operator==(Digits, Digits);
    friend bool operator!=(Digits, Digits);


private:

    std::uint16_t   m_possible;
};

inline
Digits operator~(Digits digits)
{
    digits.m_possible = (~digits.m_possible) & Digits::ALL;
    return digits;
}

inline
Digits operator&(Digits op0, const Digits op1)
{
    return op0 &= op1;
}

inline
Digits operator|(Digits op0, const Digits op1)
{
    return op0 |= op1;
}

inline
bool operator==(const Digits op0, const Digits op1)
{
    return op0.m_possible == op1.m_possible;
}

inline
bool operator!=(const Digits op0, const Digits op1)
{
    return op0.m_possible != op1.m_possible;
}

class Grid
{
public:
    enum State {SOLVED = 0, INCOMPLETE, UNSOLVABLE};
    Grid(const char* sudoku);
    Grid();

    std::array<Digits, 9 * 9>& getDigits() { return m_digits; }
    const std::array<Digits, 9 * 9>& getDigits() const { return m_digits; }

    Digits getSolvedInColumn(int column) const;
    Digits getSolvedInRow(int row) const;
    Digits getSolvedInBox(int x, int y) const;

    bool update();
    State getState() const;

    bool isCorrectSolution() const;

    template <typename T>
    class MyIterator
    {
    public:
        enum Type : std::uint8_t {ROW_TYPE, COL_TYPE, BOX_TYPE};
        T& operator*() { return m_ptr[m_idx]; }
        const T& operator*() const { return m_ptr[m_idx]; }

        T* operator->() { return m_ptr + m_idx; }
        const T* operator->() const { return m_ptr + m_idx; }

        MyIterator& operator++()
        {
            if (m_type == ROW_TYPE)
                ++m_idx;
            else if (m_type == COL_TYPE)
                m_idx += 9;
            else {
                ++m_idx;
                if (m_idx % 3 == 0)
                    m_idx += 6;
            }
            return *this;
        }

        MyIterator operator++(int)
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator!=(const MyIterator& other) const
        {
            return (m_ptr + m_idx) != (other.m_ptr + other.m_idx);
        }

    private:
        friend class Grid;
        MyIterator(T* ptr, Type type) : m_ptr{ptr}, m_idx{0}, m_type{type} {}

        T*              m_ptr;
        std::uint8_t    m_idx;
        Type            m_type;
    };

    using iterator = MyIterator < Digits > ;
    using const_iterator = MyIterator < const Digits > ;

    template <typename T>
    class DigitsRangeProxy
    {
    public:
        T begin() { return m_begin; }
        T end() { return m_end; }
    private:
        friend class Grid;
        DigitsRangeProxy(T b, T e) : m_begin{b}, m_end{e} {}
        T m_begin;
        T m_end;
    };

    DigitsRangeProxy<iterator> getRow(int row);
    DigitsRangeProxy<const_iterator> getRow(int row) const;

    DigitsRangeProxy<iterator> getColumn(int column);
    DigitsRangeProxy<const_iterator> getColumn(int column) const;

    DigitsRangeProxy<iterator> getBox(int x, int y);
    DigitsRangeProxy<const_iterator> getBox(int x, int y) const;

    Digits& getDigits(int x, int y)
    {
        assert(x >= 0 && x < 9 && y >= 0 && y < 9);
        return m_digits[y * 9 + x];
    }

    const Digits& getDigits(int x, int y) const
    {
        assert(x >= 0 && x < 9 && y >= 0 && y < 9);
        return m_digits[y * 9 + x];
    }

private:
    friend bool operator==(const Grid&, const Grid&);
    friend bool operator!=(const Grid&, const Grid&);

    friend std::ostream& operator<<(std::ostream&, const Grid&);
    std::array<Digits, 9 * 9>      m_digits;
};

std::ostream& operator<<(std::ostream& out, const Grid& grid);

#endif // GRID_H
