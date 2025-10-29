#include <game.hpp>
#include <iostream>


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

// int main()
// {
//     const ui TimeMax = 10;
//     Game game(10, 10, 12);
//     game.init();
//     game.randomPlantSeed(0.5);
//     for (ui t=0; t<TimeMax; t++)
//     {
//         std::cout << "===========================" << std::endl;
//         game.prettyPrint();
//         game.step();
//     }
// }