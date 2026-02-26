#include "game.hpp"
#include <iostream>
#include <vector>
#include <array>
#include <tuple>


void RGBA8::makeAlive()
{
    aliveRGBA8(this);
}
void RGBA8::makeDead()
{
    deadRGBA8(this);
}
bool RGBA8::isAlive()
{
    return (r == 255);
}
bool RGBA8::isDead()
{
    return (r == 0);
}

Game::Game(ui _numRow, ui _numCol, ui seed):
    numRow(_numRow),
    numCol(_numCol),
    rngGen(),
    state{},
    tmp{}
{
    rngGen.seed(seed);
    init();
}

void Game::init()
{
    for (ui i=0; i<numRow; i++)
    {
        memset(state[i], 0, numCol * sizeof(StateType));
    }
    alignBV(numRow, numCol, state, tmp);
}

void Game::randomPlantSeed(float p)
{
    for (ui i=1; i<numRow-1; i++)
    {
        for (ui j=1; j<numCol-1; j++)
        {
            auto val = rngGen();
            float res = (float) val / (float) rngGen.max();
            if (res <= p)
                state[i][j].makeAlive();
            else state[i][j].makeDead();
        }
    }
}

void Game::step()
{
    transitionRule(numRow, numCol, state, tmp);
}

void Game::copyState(StateType** dst)
{
    memcpy(dst, state, numRow * numCol * sizeof(StateType));
}

void Game::prettyPrint()
{
    {
        using namespace std;
        for (ui i=0; i<numRow; i++)
        {
            for (ui j=0; j<numCol; j++)
            {
                cout << state[i][j] << ' ';
            }
            cout << endl;
        }
        cout << endl;
    }
}

void transitionRule(ui numRow, ui numCol, StateType arr[MAX_ROW][MAX_COL], StateType tmp[MAX_ROW][MAX_COL])
{
    for (ui i=1; i<numRow - 1; i++)
    {
        for (ui j=1; j<numCol - 1; j++)
        {
            /* Rule (from wiki article): Consider 8-neighbors.
            1. any live cell with fewer than two live neighbors dies
            2. any live cell with two or three live neighbors lives on
            3. any live cell with more than three live neighbors dies,
            4. any dead cell with exactly three live neighbors becomes alive
            */
            ui numNeighAlive = countLiveNeighbors(arr, i, j);
            StateType nextState;
            switch (numNeighAlive)
            {
                case 2:
                    if (arr[i][j].isAlive())
                        nextState.makeAlive();
                    break;
                case 3:
                    nextState.makeAlive();
                    break;
                default:
                    break;
            }
            tmp[i][j] = nextState;
        }
    }
    for (ui i=0; i<numRow; i++)
    {
        memcpy(arr[i], tmp[i], numCol * sizeof(StateType));
    }
}

void alignBV(ui numRow, ui numCol, StateType lh[MAX_ROW][MAX_COL], StateType rh[MAX_ROW][MAX_COL])
{
    memcpy(lh[0], rh[0], numCol * sizeof(StateType));
    memcpy(lh[numRow-1], rh[numRow-1], numCol * sizeof(StateType));
    for (ui i=1; i<numRow-1; i++)
    {
        rh[i][0] = lh[i][0];
        rh[i][numRow-1] = lh[i][numRow];
    }
}

ui countLiveNeighbors(StateType arr[MAX_ROW][MAX_COL], ui rowId, ui colId)
{
    ui count = 0;
    for (ui i=0; i<8; i++)
    {
        count += arr[(int)rowId + neighDir[2*i]][(int)colId + neighDir[2*i + 1]].isAlive();
    }
    return count;
}


std::ostream& operator<<(std::ostream& os, const CellState& cs)
{
    return os << (cs.isAlive()? 1: 0);
}

// ======================== Packed Cell member functions =====================================
PackedCellAlignment::PackedCellAlignment(int size): m_align(static_cast<size_t>(size)) {}
CellState PackedCellAlignment::operator[](size_t pos)
{
    QR qr{pos};
    return data()[qr.quotient].getState(qr.remainder);
}
const CellState PackedCellAlignment::operator[](size_t pos) const
{
    QR qr{pos};
    return c_data()[qr.quotient].getState(qr.remainder);
}
CellState PackedCellAlignment::makeAliveAt(int idx)
{
    QR qr{static_cast<size_t>(idx)};
    CellState old_state{data()[qr.quotient].getState(qr.remainder)};
    data()[qr.quotient].makeAliveAt(qr.remainder);
    return old_state;
}
CellState PackedCellAlignment::makeDeadAt(int idx)
{
    QR qr{static_cast<size_t>(idx)};
    CellState old_state{data()[qr.quotient].getState(qr.remainder)};
    data()[qr.quotient].makeDeadAt(qr.remainder);
    return old_state;
}
CellState PackedCellAlignment::changeAt(int idx, CellState newVal)
{
    if (newVal.isAlive()) return PackedCellAlignment::makeAliveAt(idx);
    else return PackedCellAlignment::makeDeadAt(idx);
}
bool PackedCellAlignment::isAliveAt(int idx) const
{
    QR qr{static_cast<size_t>(idx)};
    return c_data()[qr.quotient].isAliveAt(qr.remainder);
}
size_t PackedCellAlignment::size() const
{
    return 8 * c_data().size();
}
void PackedCellAlignment::resize(size_t newSize)
{
    QR qr{newSize};
    if (qr.remainder == 0) data().resize(qr.quotient);
    else data().resize(qr.quotient + 1);
}


//============================ custom definitions of some member functions to work with packed container ===================
template<>
const CellState CA<PackedCellContainer>::get_c(GridIndex idx) const 
{
    return CA<PackedCellContainer>::c_world()[idx.x][idx.y];
}
template<>
CellState CA<PackedCellContainer>::change(GridIndex idx, CellState newVal)
{
    return CA::m_world[idx.x].changeAt(idx.y, newVal);
}
template<>
CellState CA<PackedCellContainer>::changeTmp(GridIndex idx, CellState newVal)
{
    return CA::m_tmp_world[idx.x].changeAt(idx.y, newVal);
}
template<>
int GoL<PackedCellContainer>::countLiveNeighbors(GridIndex idx) const
{
    std::pair<int, int> size{CA<PackedCellContainer>::size()};
    int count{0};
    std::array<GridIndex, 8> neighbor{{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}}};
    if (m_boundary_cnd == 0)
    {
        for (auto& p: neighbor)
        {
            GridIndex folded{CA<PackedCellContainer>::foldIndex(idx + p)};
            if ((idx + p) == folded) count += CA<PackedCellContainer>::get_c(folded).getIntVal();
        }
        return count;
    }
    else if (m_boundary_cnd == 1)
    {
        for (auto& p: neighbor)
        {
            GridIndex folded{CA<PackedCellContainer>::foldIndex(idx + p)};
            count += CA<PackedCellContainer>::get_c(folded).getIntVal();
        }
        return count;
    }
    return 0;
}
template<>
void GoL<PackedCellContainer>::step()
{
    //TODO: change this, alongside all the member functions that needs changing to accommodate PackedCellPackedCellContainer
    // while refactoring, comment ok means no need to define custom behavior

    auto [numRow, numCol] = CA<PackedCellContainer>::size(); // ok
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
            int count{GoL<PackedCellContainer>::countLiveNeighbors({i, j})};
            bool isAlive{CA<PackedCellContainer>::get_c({i,j}).isAlive()};
            auto nextState{cellTransition(count, isAlive)};
            CA<PackedCellContainer>::changeTmp({i,j}, nextState);
        }
    }
    CA<PackedCellContainer>::swap();
}

template<>
std::ostream& operator<<(std::ostream& os, const GoL<PackedCellContainer>& gol)
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

// ============================custom definitions of some member functions to work with packed container======================


#ifdef CA_DEMO
int main()
{
    // const ui TimeMax = 10;
    // Game game(10, 10, 12);
    // game.init();
    // game.randomPlantSeed(0.5);
    // for (ui t=0; t<TimeMax; t++)
    // {
    //     std::cout << "===========================" << std::endl;
    //     game.prettyPrint();
    //     game.step();
    // }

    int size{10};
    StandardGoL gol{size, size, 12, 0};
    gol.randomPopulate(0.5);
    std::vector<std::vector<int>> neighborCount(size);
    for (auto it{neighborCount.begin()}; it != neighborCount.end(); ++it)
    {
        it->resize(size);
    }

    std::cout << gol << std::endl;
    gol.countLiveNeighbors(neighborCount);
    for (auto it{neighborCount.begin()}; it != neighborCount.end(); ++it)
    {
        for (auto iit{it->begin()}; iit != it->end(); ++iit)
        {
            std::cout << *iit << " ";
        }
        std::cout << std::endl;
    }

    for (int t{0}; t<5; t++)
    {
        std::cout << gol << std::endl;
        std::cout << "=======================\n";
        gol.step();
    }

    auto foldIndexCheck{[&](int x, int y){
        GridIndex folded{gol.foldIndex({x, y})};
        std::cout << std::format("({},{}) -> ({}, {})\n",x, y, folded.x, folded.y);
    }};

    foldIndexCheck(-1,-1);
    foldIndexCheck(-1,0);
    foldIndexCheck(-1,1);
    foldIndexCheck(0,-1);
    foldIndexCheck(0,1);
    foldIndexCheck(1,-1);
    foldIndexCheck(1,0);
    foldIndexCheck(1,1);
}
#endif