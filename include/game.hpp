#pragma once

#include <cstring>
#include <utility>
#include <random>
#include <iostream>
#include <format>

constexpr size_t MAX_ROW{ 100 };
constexpr size_t MAX_COL{ 100 };
typedef unsigned int ui;

struct RGBA8
{
    uint8_t r, g, b, a;
    RGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a):
        r(r),
        g(g),
        b(b),
        a(a){}
    RGBA8():
        r(0),
        g(0),
        b(0),
        a(0){}
    void makeAlive();
    void makeDead();
    bool isAlive();
    bool isDead();
};

inline void aliveRGBA8(RGBA8* e)
{
    e->r = 255;
    e->g = 255;
    e->b = 255;
    e->a = 255;
}

inline void deadRGBA8(RGBA8* e)
{
    e->r = 0;
    e->g = 0;
    e->b = 0;
    e->a = 255;
}


inline std::ostream& operator<<(std::ostream& os, const RGBA8& obj)
{
    if (obj.r == 255)
        os << "1";
    else os << "0";
    return os;
}

typedef RGBA8 StateType;

const int neighDir[] = {
    -1, -1,
    -1, 0,
    -1, 1,
    0, -1,
    0, 1,
    1, -1,
    1, 0,
    1, 1
};

/// @brief increment time for one step in fixed boundary condition Game Of Life. Assumes tmp already has the same boundary values as arr
/// @param arr 2-dim array that represents states
/// @param tmp an array with the exact same size as arr that stores the temporary transition
void transitionRule(ui numRow, ui numCol, StateType arr[MAX_ROW][MAX_COL], StateType tmp[MAX_ROW][MAX_COL]);
ui countLiveNeighbors(StateType arr[MAX_ROW][MAX_COL], ui rowId, ui colId);
/// @brief copy the boundary values of lh to rh
/// @param numRow number of rows (first dim)
/// @param numCol number of columns (second dim)
/// @param lh copy src
/// @param rh copy dst
void alignBV(ui numRow, ui numCol, StateType lh[MAX_ROW][MAX_COL], StateType rh[MAX_ROW][MAX_COL]);

class Game
{
    public:
    StateType state[MAX_ROW][MAX_COL];
    Game(ui numRow, ui numCol, ui seed);
    /// @brief returns the current state
    /// @return 
    StateType getState(ui rowID, ui colID)
    {
        return state[rowID][colID];
    }
    /// @brief initialize the boundary values and inner values to 0
    void init();
    /// @brief copy the current state to the given array
    /// @param dst 
    void copyState(StateType** dst);
    /// @brief move the game state one-step forward
    void step();
    /// @brief print the current state to the stdout
    void prettyPrint();
    /// @brief randomly make some cells alive
    /// @param p probability of a cell becoming alive
    void randomPlantSeed(float p);

    private:
    StateType tmp[MAX_ROW][MAX_COL];
    std::mt19937 rngGen;
    ui numRow, numCol;
};

struct CellState
{
    uint8_t state = 0;
    CellState(uint8_t _state): state{_state} {}
    CellState(): state{0}{}
    bool isAlive() const
    {
        return state != 0;
    }
    void makeAlive()
    {
        state = 1;
    }
    void makeDead()
    {
        state = 0;
    }
    int getIntVal() const
    {
        return static_cast<int>(state);
    }

};

std::ostream& operator<<(std::ostream& os, const CellState& cs);

// general concept that might be used to represent resizable grid space (not finalized)
// might as well use std::vector
template<typename T>
concept ResizableWorld =
    std::ranges::random_access_range<T> &&
    requires(T a, size_t n)
    {
        { T(n) };
        { a.resize(n) };
    };

struct GridIndex {
    int x, y;

    GridIndex(int _x, int _y) : x{_x}, y{_y} {}

    // Addition with another GridIndex
    GridIndex operator+(const GridIndex& other) const
    {
        return {x + other.x, y + other.y};
    }

    GridIndex& operator+=(const GridIndex& other)
    {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }

    // Subtraction with another GridIndex
    GridIndex operator-(const GridIndex& other) const
    {
        return {x - other.x, y - other.y};
    }

    GridIndex& operator-=(const GridIndex& other)
    {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }

    GridIndex& operator=(const GridIndex& other)
    {
        this->x = other.x;
        this->y = other.y;
        return *this;
    }

    bool operator==(const GridIndex& other)
    {
        return (this->x == other.x && this->y == other.y);
    }
};

template<ResizableWorld Container>
class CA
{
    public:
    CA(int numRow, int numCol, int seed);
    CellState& at(GridIndex idx);
    const CellState& at_c(GridIndex idx) const;
    void change(GridIndex idx, CellState newVal);
    void changeTmp(GridIndex idx, CellState newVal);
    void swap();
    virtual void step() = 0;
    virtual void plantSeed(float p) = 0;
    int sizeX() const { return m_numRow; }
    int sizeY() const { return m_numCol; }
    // If the given index is inside the world (that is, >= 0 and < m_numRow ),
    // return the same index.
    // If it is outside the world, return the coordinate as if
    // the grid were repeated outside the border (NumPy-like indexing)
    // For example, (-1, -1) should return (m_numRow-1, m_numCol-1)
    GridIndex foldIndex(GridIndex idx) const;
    std::pair<int, int> size() const { return {sizeX(), sizeY()}; }
    float drawRngFloat() { 
        auto val = m_rngGen();
        return (float) val / (float) m_rngGen.max();
    }

    private:
    Container m_world, m_tmp_world;
    std::mt19937 m_rngGen;
    int m_numRow, m_numCol;
};


template<ResizableWorld Container>
class GoL: public CA<Container>
{
    public:
    GoL(int numRow, int numCol, int seed, int bc):
        CA<Container>{numRow, numCol, seed},
        m_boundary_cnd{bc} {}
    void step() override;
    void plantSeed(float p) override;
    int countLiveNeighbors(GridIndex idx) const;
    void countLiveNeighbors(std::vector<std::vector<int>>& out) const;
    
    private:
    // 0 if fixed (to 0) b.c
    // 1 if periodical b.c
    int m_boundary_cnd = 0;
};

using StandardGoL = GoL<std::vector<std::vector<CellState>>>;

template<ResizableWorld Container>
std::ostream& operator<<(std::ostream& os, const GoL<Container>& gol);

class WorldInitializationError: public std::exception
{
    public:
    const char* what() const noexcept override
    {
        const char* what = std::format("Failed to initialize Cellular Automata: {}\n", m_msg).c_str();
        return what;
    }
    WorldInitializationError(std::string s):
    m_msg{s}
    {}
    private:
    std::string m_msg;
};