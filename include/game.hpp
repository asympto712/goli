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
    CellState(bool isAlive) {
        if (isAlive) makeAlive();
        else makeDead();
    }
    bool isAlive() const
    {
        return state != 0;
    }
    void makeAlive()
    {
        state = 255;
    }
    void makeDead()
    {
        state = 0;
    }
    int getIntVal() const
    {
        // return static_cast<int>(state);
        return static_cast<int>(isAlive()? 1: 0);
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
        { a.size() };
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

    bool operator==(const GridIndex& other) {
        return (this->x == other.x && this->y == other.y);
    }
};

template<ResizableWorld Container>
class CA
{
    public:
    CA(int numRow, int numCol, int seed);
    CA(int numRow, int numCol);
    // world size defaults to 3 X 3
    CA();

    const Container& c_world() const {
        return m_world;
    }

    void seed(int _seed) {
        m_rngGen.seed(_seed);
    }
    void randomSeed(){
        std::random_device rd;
        m_rngGen.seed(rd());
    }

    CellState& at(GridIndex idx);
    const CellState& at_c(GridIndex idx) const;

    // change the state of the cell in the main buffer
    void change(GridIndex idx, CellState newVal);
    // change the state of the cell in the sub buffer
    void changeTmp(GridIndex idx, CellState newVal);

    /// @brief swap the two underlying container
    void swap();

    const int sizeX() const { return m_numRow; }
    const int sizeY() const { return m_numCol; }
    std::pair<int, int> size() const { return {sizeX(), sizeY()}; }
    
    /// @brief change the world size. If the new size would outsize the underlying container,
    /// resize the container as well. If not, it only changes the internal variables that keep track of the world size.
    /// so the underlying container might be bigger than the actual world
    void setWorldSize(int new_numRow, int new_numCol);
    /// @brief change the size of the underlying container
    void resizeContainer(int new_sizeX, int new_sizeY);
    /// @brief return the size of the underlying container
    std::pair<int, int> containerSize() const;

    // If the given index is inside the world (that is, >= 0 and < m_numRow ),
    // return the same index.
    // If it is outside the world, return the coordinate as if
    // the grid were repeated outside the border (NumPy-like indexing)
    // For example, (-1, -1) should return (m_numRow-1, m_numCol-1)
    GridIndex foldIndex(GridIndex idx) const;

    virtual void step() = 0;
    virtual void randomPopulate(float p) = 0;
    float drawRngFloat() { 
        auto val = m_rngGen();
        return (float) val / (float) m_rngGen.max();
    }

    private:
    Container m_world, m_tmp_world;
    std::mt19937 m_rngGen;
    int m_numRow = 3, m_numCol = 3;
};


template<ResizableWorld Container>
class GoL: public CA<Container>
{
    public:

    GoL(int numRow, int numCol, int seed, int bc):
        CA<Container>{numRow, numCol, seed},
        m_boundary_cnd{bc} {}
    GoL(): CA<Container>{} {}
    GoL(int numRow, int numCol):
        CA<Container>{numRow, numCol},
        m_boundary_cnd{} {}

    void step() override;

    void changeBC(int new_bc) {
        m_boundary_cnd = new_bc;
    }

    void randomPopulate(float p) override;

    int countLiveNeighbors(GridIndex idx) const;
    void countLiveNeighbors(std::vector<std::vector<int>>& out) const;
    
    private:
    // 0 if fixed (to 0) b.c
    // 1 if periodical b.c
    // default is 0
    int m_boundary_cnd = 0;
};

// The most obvious choice for the grid world container.
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


template<ResizableWorld Container>
CA<Container>::CA(int numRow, int numCol):
m_rngGen{},
m_world{{}},
m_tmp_world{}
{
    if (numRow >= 3 && numCol >= 3)
    {
        setWorldSize(numRow, numCol);
        randomSeed();
    }
    else
    {
        throw WorldInitializationError("initial dimension is too small");
    }

    // for debugging
    std::cout << "After construction\n";
    std::cout << std::format("m_world.size(): {} X {}\n", m_world.size(), m_world.begin()->size());
}

template<ResizableWorld Container>
CA<Container>::CA(int numRow, int numCol, int _seed):
CA<Container>::CA(numRow, numCol)
{
    CA<Container>::seed(_seed);
}

template<ResizableWorld Container>
CA<Container>::CA():
CA<Container>::CA(3, 3)
{}

template<ResizableWorld Container>
CellState& CA<Container>::at(GridIndex idx)
{
    return m_world[idx.x][idx.y];
}

template<ResizableWorld Container>
const CellState& CA<Container>::at_c(GridIndex idx) const
{
    return m_world[idx.x][idx.y];
}

template<ResizableWorld Container>
void CA<Container>::change(GridIndex idx, CellState newVal)
{
    CA::at(idx) = newVal;
}

template<ResizableWorld Container>
void CA<Container>::changeTmp(GridIndex idx, CellState newVal)
{
    m_tmp_world[idx.x][idx.y] = newVal;
}

template<ResizableWorld Container>
void CA<Container>::setWorldSize(int new_numRow, int new_numCol)
{
    auto [cSizeX, cSizeY] = containerSize();
    if (new_numRow > static_cast<int>(cSizeX) || new_numCol > static_cast<int>(cSizeY))
    {
        CA<Container>::resizeContainer(new_numRow, new_numCol);
        m_numRow = new_numRow;
        m_numCol = new_numCol;
    }
    else
    {
        m_numRow = new_numRow;
        m_numCol = new_numCol;
    }
}

template<ResizableWorld Container>
void CA<Container>::resizeContainer(int new_sizeX, int new_sizeY)
{
    m_world.resize(new_sizeX);
    for (auto it{m_world.begin()}; it != m_world.end(); it++)
    {
        it->resize(new_sizeY);
    }

    m_tmp_world.resize(new_sizeX);
    for (auto it{m_tmp_world.begin()}; it != m_tmp_world.end(); it++)
    {
        it->resize(new_sizeY);
    }
}

template<ResizableWorld Container>
std::pair<int, int> CA<Container>::containerSize() const
{
    int dimX{static_cast<int>(m_world.size())};
    int dimY;
    if (dimX == 0) dimY = 0;
    else dimY = static_cast<int>(m_world.begin()->size());
    return std::make_pair<int, int>(std::move(dimX), std::move(dimY));
}

template<ResizableWorld Container>
void CA<Container>::swap()
{
    std::swap(m_world, m_tmp_world); 
}

template<ResizableWorld Container>
std::ostream& operator<<(std::ostream& os, const GoL<Container>& gol)
{
    auto [numRow, numCol] = gol.size();
    for (int i{0}; i<numRow; i++)
    {
        for (int j{0}; j<numCol; j++)
        {
            os << gol.at_c({i, j}) << " ";
        }
        os << "\n";
    }
    return os;
}

template<ResizableWorld Container>
void GoL<Container>::step()
{
    auto [numRow, numCol] = CA<Container>::size();
    auto cellTransition{[](int neighbor_count, bool isAlive) -> CellState {
        if ((!isAlive && neighbor_count == 3) ||
            (isAlive && neighbor_count >= 2 && neighbor_count <= 3))
        {
            return CellState(true);
        }
        else return CellState(false);
    }};

    for (int i{0}; i<numRow; i++)
    {
        for (int j{0}; j<numCol; j++)
        {
            int count{GoL<Container>::countLiveNeighbors({i, j})};
            bool isAlive{CA<Container>::at({i,j}).isAlive()};
            auto nextState{cellTransition(count, isAlive)};
            CA<Container>::changeTmp({i,j}, nextState);
        }
    }
    CA<Container>::swap();
}

template<ResizableWorld Container>
GridIndex CA<Container>::foldIndex(GridIndex idx) const
{
    int first, second;
    if (idx.x >= 0 && idx.x < m_numRow) first = idx.x;
    else first = ((idx.x % m_numRow) + m_numRow) % m_numRow;

    if (idx.y >= 0 && idx.y < m_numCol) second = idx.y;
    else second = ((idx.y % m_numCol) + m_numCol) % m_numCol;

    return GridIndex{first, second};
}

template<ResizableWorld Container>
int GoL<Container>::countLiveNeighbors(GridIndex idx) const
{
    std::pair<int, int> size{CA<Container>::size()};
    int count{0};
    std::array<GridIndex, 8> neighbor{{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}}};
    if (m_boundary_cnd == 0)
    {
        for (auto& p: neighbor)
        {
            GridIndex folded{CA<Container>::foldIndex(idx + p)};
            if ((idx + p) == folded) count += CA<Container>::at_c(folded).getIntVal();
        }
        return count;
    }
    else if (m_boundary_cnd == 1)
    {
        for (auto& p: neighbor)
        {
            GridIndex folded{CA<Container>::foldIndex(idx + p)};
            count += CA<Container>::at_c(folded).getIntVal();
        }
        return count;
    }
    return 0;
}

template<ResizableWorld Container>
void GoL<Container>::countLiveNeighbors(std::vector<std::vector<int>>& out) const
{
    auto [numRow, numCol] = CA<Container>::size();
    for (int i{0}; i<numRow; i++)
    {
        for (int j{0}; j<numCol; j++)
        {
            out[i][j] = GoL<Container>::countLiveNeighbors({i,j});
        }
    }
}

template<ResizableWorld Container>
void GoL<Container>::randomPopulate(float p)
{
    auto [numRow, numCol] = CA<Container>::size();
    for (int i{0}; i<numRow; ++i)
    {
        for (int j{0}; j<numCol; ++j)
        {
            float res = CA<Container>::drawRngFloat();
            if (res <= p)
                CA<Container>::at({i,j}).makeAlive();
            else CA<Container>::at({i,j}).makeDead();
        }
    }
} 