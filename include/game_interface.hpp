#pragma once

#include <vector>
#include "game.hpp"

struct CellState
{
    uint8_t state = 0;
    explicit CellState(uint8_t _state): state{_state} {}
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
    bool operator==(const CellState& other) const {
        return state == other.state;
    }
};
std::ostream& operator<<(std::ostream& os, const CellState& cs);


// Interface to the "Game" class that should provide virtual functions for the responsibilities 
class GameInterface
{
    public:
    GameInterface() = default;
    ~GameInterface() = default;

    virtual void step() = 0; // move forward one step
    virtual void stepBack() = 0; // move backward one step (by storing intermediate state with caching?)
    virtual void reset() = 0; // go back to the initial state

    virtual void writeToStateBuffer(std::vector<CellState> & stateBuffer) = 0; // write the current state to the stateBuffer which is most likely managed by the application side
};

