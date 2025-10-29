#pragma once

#include <cstring>
#include <utility>
#include <random>
#include <iostream>

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