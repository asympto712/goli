#pragma once

#include <cstring>
#include <utility>
#include <random>
#include <iostream>
#include <format>
#include <ranges>
#include "game_interface.hpp"

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

struct PackedCell
{
    uint8_t data = 0;
    PackedCell() = default;
    PackedCell(uint8_t _data): data{_data} {}
    bool isAliveAt(int idx) const {
        return (data >> idx) & 1;
    }
    void makeAliveAt(int idx) {
        data |= (1 << idx);
    }
    void makeDeadAt(int idx) {
        data &= ~(1 << idx);
    }
    const CellState getState(int idx) const{
        return CellState{isAliveAt(idx)};
    }
};

struct QR
{
    size_t quotient, remainder;
    QR() = delete;
    QR(size_t dividend, size_t divisor): quotient{dividend / divisor}, remainder{dividend % divisor} {}
    QR(size_t size): QR(size, 8) {}
    QR(const QR& other) = delete;
    QR(QR&& other) = delete;
    QR& operator=(const QR& rh) = delete;
    QR& operator=(QR&& rh) = delete;
};

struct PackedCellAlignment
{
    std::vector<PackedCell> m_align;

    // constructor & assignment
    PackedCellAlignment() = default;
    PackedCellAlignment(int size);
    PackedCellAlignment(const PackedCellAlignment& _align) = default;
    PackedCellAlignment(PackedCellAlignment&& _align) = default;
    PackedCellAlignment& operator=(const PackedCellAlignment& rh) = default;
    PackedCellAlignment& operator=(PackedCellAlignment&& rh) = default;

    // access internal data
    std::vector<PackedCell>& data() {
        return m_align;
    }
    const std::vector<PackedCell>& c_data() const {
        return m_align;
    }

    // cell access & modify
    CellState operator[](size_t pos);
    const CellState operator[](size_t pos) const;
    // change the cell state to alive, returns the old state
    CellState makeAliveAt(int idx);
    // change the cell state to dead, returns the old state
    CellState makeDeadAt(int idx);
    CellState changeAt(int idx, CellState newVal);
    bool isAliveAt(int idx) const;

    // size check & resize
    size_t size() const;
    void resize(size_t newSize);
};


using PackedCellContainer = std::vector<PackedCellAlignment>;


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

// this concept requires that the direct reference to the cell state is obtainable by two fold indexing
// applies to std::vector<std::vector<CellState>>, but not to PackedCellContainer
template<typename T>
concept StateRefIndexable = 
    ResizableWorld<T> &&
    requires(T a, size_t i, size_t j) {
        { a[i][j] } -> std::convertible_to<CellState&>;
    };

template<ResizableWorld Container>
void containerAssign(Container& lh, const Container& rh);

template<ResizableWorld Container>
class CA: public GameInterface
{
    public:
    CA(int numRow, int numCol, int seed);
    CA(int numRow, int numCol);
    // world size defaults to 3 X 3
    CA();

    Container& mutable_world() {
        return m_world;
    }
    const Container& c_world() const {
        return m_world;
    }
    Container& mutable_initConfig() {
        return m_initConfig;
    }
    const Container& c_initConfig() const {
        return m_initConfig;
    }

    void seed(int _seed) {
        m_rngGen.seed(_seed);
    }
    void randomSeed(){
        std::random_device rd;
        m_rngGen.seed(rd());
    }

    // get the cell state of the position by value
    const CellState get_c(GridIndex idx) const;

    // change the state of the cell in the main buffer and return the old value
    CellState change(GridIndex idx, CellState newVal) override;
    // change the state of the cell in the sub buffer and return the old value
    CellState changeTmp(GridIndex idx, CellState newVal);

    /// @brief swap the two underlying container
    void swap();
    
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

    float drawRngFloat() { 
        auto val = m_rngGen();
        return (float) val / (float) m_rngGen.max();
    }

    // GameInterface functions
    void setWorldSize(int new_numRow, int new_numCol) override;
    const int sizeX() const override { return m_numRow; }
    const int sizeY() const override { return m_numCol; }
    std::pair<int, int> size() const override { return {sizeX(), sizeY()}; }

    private:
    Container m_world, m_tmp_world;
    Container m_initConfig;
    std::mt19937 m_rngGen;
    int m_numRow = 3, m_numCol = 3;
};

// return the mutable reference to the cell state of the position
template<StateRefIndexable SContainer>
CellState& at(CA<SContainer>& ca, GridIndex idx)
{
    return ca.mutable_world()[idx.x][idx.y];
}
// return the const reference to the cell state of the position
template<StateRefIndexable SContainer>
const CellState& at_c(const CA<SContainer>& ca, GridIndex idx)
{
    return ca.c_world()[idx.x][idx.y];
}


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

    void changeBC(int new_bc) {
        m_boundary_cnd = new_bc;
    }

    // set the initial configuration to be the current m_world
    void setInitConfig();

    int countLiveNeighbors(GridIndex idx) const;
    void countLiveNeighbors(std::vector<std::vector<int>>& out) const;

    // GameInterface functions
    void randomPopulate(float p) override;
    void step() override;
    void stepBack() override;
    void reset() override;
    void writeToStateBuffer(std::vector<CellState> & stateBuffer) const override;
    
    private:
    // 0 if fixed (to 0) b.c
    // 1 if periodical b.c
    // default is 0
    int m_boundary_cnd = 0;
};

// The most obvious choice for the grid world container.
using SimpleContainer = std::vector<std::vector<CellState>>;
using StandardGoL = GoL<SimpleContainer>;


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
m_tmp_world{{}},
m_initConfig({})
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
const CellState CA<Container>::get_c(GridIndex idx) const
{
    return m_world[idx.x][idx.y];
}


template<ResizableWorld Container>
CellState CA<Container>::change(GridIndex idx, CellState newVal)
{
    auto oldVal = std::move(m_world[idx.x][idx.y]);
    m_world[idx.x][idx.y] = newVal;
    return oldVal;
}

template<ResizableWorld Container>
CellState CA<Container>::changeTmp(GridIndex idx, CellState newVal)
{
    // CellState oldVal = std::exchange(m_tmp_world[idx.x][idx.y], newVal);
    // return oldVal;
    auto oldVal = std::move(m_tmp_world[idx.x][idx.y]);
    m_tmp_world[idx.x][idx.y] = newVal;
    return oldVal;
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
            os << gol.get_c({i, j}) << " ";
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
            bool isAlive{CA<Container>::get_c({i,j}).isAlive()};
            auto nextState{cellTransition(count, isAlive)};
            CA<Container>::changeTmp({i,j}, nextState);
        }
    }
    CA<Container>::swap();
}

template<ResizableWorld Container>
void GoL<Container>::stepBack()
{

}

template<ResizableWorld Container>
void GoL<Container>::reset()
{
    const Container& ic{GoL<Container>::c_initConfig()};
    Container& world{GoL<Container>::mutable_world()};
    containerAssign<Container>(world, ic);
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
            if ((idx + p) == folded) count += CA<Container>::get_c(folded).getIntVal();
        }
        return count;
    }
    else if (m_boundary_cnd == 1)
    {
        for (auto& p: neighbor)
        {
            GridIndex folded{CA<Container>::foldIndex(idx + p)};
            count += CA<Container>::get_c(folded).getIntVal();
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
            {
                // CA<Container>::at({i,j}).makeAlive();
                CA<Container>::change({i,j}, {true});
            }
            else 
            {
                // CA<Container>::at({i,j}).makeDead();
                CA<Container>::change({i,j}, {false});
            }
        }
    }
} 


template<ResizableWorld T>
void GoL<T>::setInitConfig()
{
    T ic = GoL<T>::mutable_initConfig();
    const T& world{GoL<T>::c_world()};
    containerAssign<T>(ic, world);
}

template<ResizableWorld Container>
void containerAssign(Container& lh, const Container& rh)
{
    lh.resize(rh.size());
    auto refIt{rh.begin()};
    for (auto it=lh.begin(); it!=lh.end(); it++)
    {
        *it = *refIt;
        refIt++;
    }
}