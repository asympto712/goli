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

#ifdef CA_DEMO
int main()
{
    const ui TimeMax = 10;
    Game game(10, 10, 12);
    game.init();
    game.randomPlantSeed(0.5);
    for (ui t=0; t<TimeMax; t++)
    {
        std::cout << "===========================" << std::endl;
        game.prettyPrint();
        game.step();
    }

    int size{10};
    StandardGoL gol{size, size, 12, 0};
    gol.plantSeed(0.5);
    std::vector<std::vector<int>> neighborCount{size};
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

    // for (int t{0}; t<5; t++)
    // {
    //     std::cout << gol << std::endl;
    //     std::cout << "=======================\n";
    //     gol.step();
    // }

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

std::ostream& operator<<(std::ostream& os, const CellState& cs)
{
    return os << (cs.isAlive()? 1: 0);
}

template<ResizableWorld Container>
CA<Container>::CA(int numRow, int numCol, int seed):
    m_rngGen{},
    m_world{},
    m_tmp_world{}
    {
        if (numRow >= 3 && numCol >= 3)
        {
            m_numRow = numRow;
            m_numCol = numCol;
            m_rngGen.seed(seed);
        }
        else
        {
            throw WorldInitializationError("initial dimension is too small");
        }

        m_world.resize(numRow);
        for (auto it{m_world.begin()}; it != m_world.end(); it++)
        {
            it->resize(numCol);
        }

        m_tmp_world.resize(numRow);
        for (auto it{m_tmp_world.begin()}; it != m_tmp_world.end(); it++)
        {
            it->resize(numCol);
        }
        
        // for debugging
        std::cout << "After construction\n";
        std::cout << std::format("m_world.size(): {} X {}\n", m_world.size(), m_world.begin()->size());
    }

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
            return CellState(1);
        }
        else return CellState(0);
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
            if ((idx + p) == folded) count += CA<Container>::at_c(idx + p).getIntVal();
        }
        return count;
    }
    else if (m_boundary_cnd == 1)
    {
        for (auto& p: neighbor)
        {
            GridIndex folded{CA<Container>::foldIndex(idx + p)};
            count += CA<Container>::at_c(idx + p).getIntVal();
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
void GoL<Container>::plantSeed(float p)
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

