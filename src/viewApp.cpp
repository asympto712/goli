#include "render_context_interface.hpp"
#include "game.hpp"
#include <ranges>
#include <cstring>
#include <cstdlib>

#ifdef VIEW_renderContext
#include <iostream>
#include <format>
constexpr int DefaultNumRow = 3;
constexpr int DefaultNumCol = 3;
constexpr float DefaultPopRate = 0.1;
int main(int argc, char* argv[])
{
  StandardGoL game{};
  int numRow, numCol;
  float popRate;
  // if at least 2 arguments are supplied try interpreting the 1st and 2nd as the world dimension
  if (argc >= 3)
  {
    try {
      numRow = std::stoi(argv[1]);
      numCol = std::stoi(argv[2]);
    } catch (std::invalid_argument& e) {
      std::cout << std::format("Could not parse 1st & 2nd arguments as world size: {}\n", e.what());
    } 
  } else {
    // the number of arguments insufficient, so use default values
    numRow = DefaultNumRow;
    numCol = DefaultNumCol;
    popRate = DefaultPopRate;
  }
  // if the 3rd argument is supplied try interpreting it as popRate
  if (argc == 4)
  {
    try {
      popRate = std::stof(argv[3]);
    }
    catch (std::invalid_argument& e) {
      std::cout << std::format("Could not parse 3rd command line argument as population rate: {}\n", e.what());
    } 
    if (popRate < 0.0 || popRate > 1.0)
    {
      std::cout << std::format("Cannot interpret the third argument as the propability of live cell. Set the value to be between 0.0 and 1.0. Input value: {}\n", popRate);
    }
  }
  game.setWorldSize(numRow, numCol);
  game.randomPopulate(popRate);
  ViewRenderContext renderContext{800, 600, game};
  // std::cout << renderContext.c_game() << std::endl;
  std::cout << std::format("numRow: {}, numCol: {}, popRate:{}\n", renderContext.worldSizeX(), renderContext.worldSizeY(), popRate);

  // debugging
  // for (int i{0}; i<5; i++)
  // {
  //   renderContext.game().step();
  //   std::cout << renderContext.c_game() << std::endl;
  // }

  renderContext.run();
}
#endif


ViewRenderContext::ViewRenderContext(int _width, int _height, GameInterface& game):
BasicRenderContext(_width, _height),
m_game{game}
{
}

ViewRenderContext::ViewRenderContext(int _width, int _height, GameInterface& game, const std::string& _vShaderPath, const std::string& _fShaderPath):
ViewRenderContext(_width, _height, game)
{
  setShaderPath(_vShaderPath, _fShaderPath);
}

void ViewRenderContext::setShaderPath(const std::string& _vShaderPath, const std::string& _fShaderPath)
{
  std::string& v_shader_path{vShaderPath()};
  std::string& f_shader_path{fShaderPath()};
  v_shader_path = _vShaderPath;
  f_shader_path = _fShaderPath;
}

void ViewRenderContext::setupShader()
{
  BasicRenderContext::setupShader(vShaderPath(), fShaderPath());
}

void ViewRenderContext::run()
{
  ViewRenderContext::setupShader();
  ViewRenderContext::setupVBO();
  ViewRenderContext::preRender();

  while (ViewRenderContext::isRunning())
  {
    ViewRenderContext::handleKeyEvent();

    ViewRenderContext::render();
  }
  ViewRenderContext::quit();
}

void ViewRenderContext::preRender()
{
  // setup uniforms
  ViewRenderContext::c_shader().use();
  GLint uloc{glGetUniformLocation(ViewRenderContext::c_shader().c_ID(), "sizeX")};
  glUniform1i(uloc, ViewRenderContext::worldSizeX());
  uloc = glGetUniformLocation(ViewRenderContext::c_shader().c_ID(), "sizeY");
  glUniform1i(uloc, ViewRenderContext::worldSizeY());
  
  // Set sampler to use texture unit 0
  uloc = glGetUniformLocation(ViewRenderContext::c_shader().c_ID(), "sampler");
  glUniform1i(uloc, 0);

  ViewRenderContext::changeLastUpdateTime(glfwGetTime());
  return;
}

void ViewRenderContext::render()
{
  updateCellStateTexture();
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glBindVertexArray(m_VAO[0]);
  ViewRenderContext::shader().use();
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, m_texID);
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, ViewRenderContext::worldSizeX() * ViewRenderContext::worldSizeY());

  glfwPollEvents();
  glfwSwapBuffers(BasicRenderContext::c_window());

  if (!ViewRenderContext::isStopped())
  {
    double curTime = glfwGetTime();
    if (curTime - ViewRenderContext::c_lastUpdateTime() > ViewRenderContext::c_updateInterval())
    {
      m_game.step();
      // std::cout << m_game << std::endl; // debugging
      ViewRenderContext::changeLastUpdateTime(curTime);
    }
  }
}

void ViewRenderContext::handleKeyEvent()
{
  if (checkKeyPress(GLFW_KEY_ESCAPE))
  {
    glfwSetWindowShouldClose(ViewRenderContext::c_window(), 1);
  }
  else if (checkKeyPress(GLFW_KEY_S))
  {
    if (isStopped()) resume();
    else stop();
  }
  else if (checkKeyPress(GLFW_KEY_UP))
  {
    speedUp(1.25);
  }
  else if (checkKeyPress(GLFW_KEY_DOWN))
  {
    speedUp(0.75);
  }

}

void ViewRenderContext::setupVBO()
{
  // use instanced drawing
  float rect[]{
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
  };
  glGenBuffers(1, &m_VBO[0]);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(float), rect, GL_STATIC_DRAW);

  glGenVertexArrays(1, &m_VAO[0]);
  glBindVertexArray(m_VAO[0]);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // cell state as texture
  glGenTextures(1, &m_texID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_texID);
  
  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  // prepare flattened vector for data transfer from game world to texture
  ViewRenderContext::updateCellStateTexture();
}

void ViewRenderContext::updateCellStateTexture()
{
  auto numRows{c_game().sizeX()};
  auto numCols{c_game().sizeY()};
  ViewRenderContext::c_game().writeToStateBuffer(m_stateBuffer);

  glBindTexture(GL_TEXTURE_2D, m_texID);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RED,
    numRows,
    numCols,
    0,
    GL_RED,
    GL_UNSIGNED_BYTE,
    m_stateBuffer.data()
  );
}

// internal CA (game) interface
void ViewRenderContext::CAstep()
{
  ViewRenderContext::game().step();
}
const int ViewRenderContext::worldSizeX() const
{
  return ViewRenderContext::c_game().sizeX();
}
const int ViewRenderContext::worldSizeY() const
{
  return ViewRenderContext::c_game().sizeY();
}
const std::pair<int, int> ViewRenderContext::worldSize() const
{
  return ViewRenderContext::c_game().size();
}
void ViewRenderContext::setWorldSize(int numRow, int numCol)
{
  ViewRenderContext::game().setWorldSize(numRow, numCol);
  ViewRenderContext::resizeStateBuffer(numRow, numCol);

  // uniform change
  ViewRenderContext::c_shader().use();
  GLint uloc{glGetUniformLocation(ViewRenderContext::c_shader().c_ID(), "sizeX")};
  glUniform1i(uloc, ViewRenderContext::worldSizeX());
  uloc = glGetUniformLocation(ViewRenderContext::c_shader().c_ID(), "sizeY");
  glUniform1i(uloc, ViewRenderContext::worldSizeY());

}
void ViewRenderContext::changeState(GridIndex idx, CellState newVal)
{
  ViewRenderContext::game().change(idx, newVal);
}
void ViewRenderContext::randomPopulate(float p)
{
  ViewRenderContext::game().randomPopulate(p);
}

void ViewRenderContext::resizeStateBuffer(int numRow, int numCol)
{
  m_stateBuffer.resize(numRow * numCol);
}


// internal timer related member functions

void ViewRenderContext::changeUpdateInterval(double newVal)
{
  if (newVal > 0.01 && newVal < 10.0) ViewRenderContext::m_updateInterval = newVal;
}
void ViewRenderContext::mulUpdateInterval(double multiplier)
{
  double newVal{ViewRenderContext::c_updateInterval() * multiplier};
  changeUpdateInterval(newVal);
}
void ViewRenderContext::increUpdateInterval(double diff)
{
  double newVal{ViewRenderContext::c_updateInterval() + diff};
  changeUpdateInterval(newVal);
}
void ViewRenderContext::changeLastUpdateTime(double newVal)
{
  if (newVal > 0.0) ViewRenderContext::m_lastUpdateTime = newVal;
}
