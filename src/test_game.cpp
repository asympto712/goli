#include "game.hpp"
#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <cassert>
#include <format>

template<typename T>
void resize(std::vector<std::vector<T>>& arr, int nrow, int ncol)
{
  arr.resize(nrow);
  for (auto it{arr.begin()}; it != arr.end(); it++)
  {
    it->resize(ncol);
  }
}

// baseline implementation of Game Of Life transition rule for reference
CellState referenceRuleForGoL(CellState cur_state, int neighbor_count)
{
  CellState next_state;
  if (cur_state.isAlive() && neighbor_count >= 2 && neighbor_count <= 3) next_state.makeAlive();
  else if (!cur_state.isAlive() && neighbor_count == 3) next_state.makeAlive();
  else next_state.makeDead();

  return next_state;
}

template<ResizableWorld Container>
void foldIndexCheck(int x, int y, const GoL<Container>& gol)
{
  GridIndex folded{gol.foldIndex({x,y})};
  std::cout << std::format("({},{}) -> ({}, {})\n",x, y, folded.x, folded.y);
}

template<ResizableWorld Container>
void checkGameRule(int sizeX, int sizeY)
{
  GoL<Container> gol{sizeX, sizeY, 11, 0}; // seed random
  gol.randomPopulate(0.4);
  std::vector<std::vector<int>> neighbor_count(static_cast<size_t>(gol.sizeX()));
  resize(neighbor_count, gol.sizeX(), gol.sizeY());
  std::vector<std::vector<CellState>> cur_state(static_cast<size_t>(gol.sizeX()));
  resize(cur_state, gol.sizeX(), gol.sizeY());


  auto test_cycle{[&](int n){ 
    for (int ii{0}; ii<n; ii++)
    {
      // store the neighbor count
      gol.countLiveNeighbors(neighbor_count);
      // copy the current cell states to cur_state
      for (int i{0}; i<gol.sizeX(); i++)
      {
        for (int j{0}; j<gol.sizeY(); j++)
        {
          cur_state[i][j] = gol.get_c({i,j});
        }
      }

      // for debug
      // std::cout << gol << std::endl;

      // move one step
      gol.step();

      // check against the reference
      for (int i{0}; i<gol.sizeX(); i++)
      {
        for (int j{0}; j<gol.sizeY(); j++)
        {
          CellState new_state{referenceRuleForGoL(cur_state[i][j], neighbor_count[i][j])};
          if (new_state != gol.get_c({i,j}))
          {
            std::cout << std::format("Expected {} at {},{}. Instead got {}\n", new_state.getIntVal(), i, j, gol.get_c({i,j}).getIntVal());
            std::cout << gol << "\n";
            gol.swap();
            std::cout << std::format("neighborcount at {},{}: {} before step\n", i, j, gol.countLiveNeighbors({i,j})); 
            std::cout << std::format("cell state at {},{}: {} before step\n", i, j, gol.get_c({i,j}).isAlive());
            gol.swap();
            std::cout << "all neighbor counts: \n";
            for (auto& row: neighbor_count)
            {
              for (auto& e: row)
              {
                std::cout << e << " ";
              }
              std::cout << "\n";
            }
            exit(1);
          }
        }
      }
    }
  }
  };

  test_cycle(10);
}

template<ResizableWorld Container>
void checkNeighborCountRule()
{
  // initialize
  constexpr int size = 5;
  constexpr uint8_t init[size][size]{
    {255,  0,  0,  0,  0},
    {255, 255,  0, 255,  0},
    { 0, 255, 255, 255, 255},
    { 0,  0, 255,  0, 255},
    { 0, 255,  0, 255, 255}
  };
  // -1 = 255 in uint8_t
  constexpr int ncount_ref[size][size]{
    {2, 3, 2, 1, 1},
    {3, 4, 5, 3, 3},
    {3, 4, 5, 5, 3},
    {2, 4, 5, 7, 4},
    {1, 1, 3, 3, 2}
  };
  GoL<Container> gol{size, size};
  for (int i{0}; i<size; i++)
  {
    for (int j{0}; j<size; j++)
    {
      gol.change({i,j}, CellState{init[i][j]});
    }
  }
  std::cout << "Initialization complete\n";
  std::cout << gol;

  // store neighbor count
  std::vector<std::vector<int>> neighbor_count{{}};
  resize(neighbor_count, size, size);
  gol.countLiveNeighbors(neighbor_count);

  // compare
  for (int i{0}; i<size; i++)
  {
    for (int j{0}; j<size; j++)
    {
      assert(neighbor_count[i][j] == ncount_ref[i][j]);
    }
  }
  
}

void checkPackedCell()
{
  constexpr uint8_t _data = 0b11101100;
  PackedCell cell1{_data};
  PackedCell cell2{};
  for (int i{0}; i<8; i++)
  {
    std::cout << std::format("(cell1.data >> {}) & 1 = {}\n", i, cell1.isAliveAt(i));
    if (cell1.isAliveAt(i)) cell2.makeAliveAt(i);
    else cell2.makeDeadAt(i);
  }
  std::cout << std::format("cell1 = {:#b}\n", cell1.data) << std::endl;
  std::cout << std::format("cell2 = {:#b}\n", cell2.data) << std::endl;
}

int main()
{
  checkPackedCell();
  checkNeighborCountRule<PackedCellContainer>();
  checkGameRule<PackedCellContainer>(50, 50);
  checkNeighborCountRule<SimpleContainer>();
  checkGameRule<SimpleContainer>(50, 50);
}
