#pragma once

#include <vector>
#include "game.hpp"

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

    bool operator==(const GridIndex& other) const {
        return (this->x == other.x && this->y == other.y);
    }

    bool operator==(GridIndex& other) {
        return (this->x == other.x && this->y == other.y);
    }
};

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

    virtual void writeToStateBuffer(std::vector<CellState> & stateBuffer) const = 0; // write the current state to the stateBuffer which is most likely managed by the application side

    // additional interface required for working with ViewApp
    virtual const int sizeX() const = 0;
    virtual const int sizeY() const = 0;
    virtual std::pair<int, int> size() const = 0;
    /// @brief change the world size. If the new size would outsize the underlying container,
    /// resize the container as well. If not, it only changes the internal variables that keep track of the world size.
    /// so the underlying container might be bigger than the actual world
    virtual void setWorldSize(int numRow, int numCol) = 0;
    virtual CellState change(GridIndex idx, CellState newVal) = 0;
    virtual void randomPopulate(float p) = 0;

};

