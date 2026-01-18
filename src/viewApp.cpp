#include "app_interface.hpp"
#include "game.hpp"
#include <ranges>
#include <cstring>
#include <cstdlib>

#ifdef VIEW_APP
#include <iostream>
#include <format>
int main(int argc, char* argv[])
{
  ViewApp app{800, 600};
  if (argc == 3)
  {
    int numRow, numCol;
    numRow = std::stoi(argv[1]);
    numCol = std::stoi(argv[2]);
    app.setWorldSize(numRow, numCol);
  }
  // app.randomPopulate(0.5);
  app.game().randomPopulate(0.1);
  // std::cout << app.c_game() << std::endl;
  std::cout << std::format("numRow: {}, numCol: {}\n", app.worldSizeX(), app.worldSizeY());

  // debugging
  // for (int i{0}; i<5; i++)
  // {
  //   app.game().step();
  //   std::cout << app.c_game() << std::endl;
  // }

  app.run();
}
#endif


ViewApp::ViewApp(int _width, int _height):
BasicApp(_width, _height),
m_game{}
{
}

ViewApp::ViewApp(int _width, int _height, const std::string& _vShaderPath, const std::string& _fShaderPath):
ViewApp(_width, _height)
{
  setShaderPath(_vShaderPath, _fShaderPath);
}

void ViewApp::setShaderPath(const std::string& _vShaderPath, const std::string& _fShaderPath)
{
  std::string& v_shader_path{vShaderPath()};
  std::string& f_shader_path{fShaderPath()};
  v_shader_path = _vShaderPath;
  f_shader_path = _fShaderPath;
}

void ViewApp::setupShader()
{
  BasicApp::setupShader(vShaderPath(), fShaderPath());
}

void ViewApp::run()
{
  ViewApp::setupShader();
  ViewApp::setupVBO();
  ViewApp::preRender();

  while (ViewApp::isRunning())
  {
    ViewApp::handleKeyEvent();

    ViewApp::render();
  }
  ViewApp::quit();
}

void ViewApp::preRender()
{
  // setup uniforms
  ViewApp::c_shader().use();
  GLint uloc{glGetUniformLocation(ViewApp::c_shader().c_ID(), "sizeX")};
  glUniform1i(uloc, ViewApp::worldSizeX());
  uloc = glGetUniformLocation(ViewApp::c_shader().c_ID(), "sizeY");
  glUniform1i(uloc, ViewApp::worldSizeY());
  
  // Set sampler to use texture unit 0
  uloc = glGetUniformLocation(ViewApp::c_shader().c_ID(), "sampler");
  glUniform1i(uloc, 0);

  ViewApp::changeLastUpdateTime(glfwGetTime());
  return;
}

void ViewApp::render()
{
  updateCellStateTexture();
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glBindVertexArray(m_VAO[0]);
  ViewApp::shader().use();
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, m_texID);
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, ViewApp::worldSizeX() * ViewApp::worldSizeY());

  glfwPollEvents();
  glfwSwapBuffers(BasicApp::c_window());

  if (!ViewApp::isStopped())
  {
    double curTime = glfwGetTime();
    if (curTime - ViewApp::c_lastUpdateTime() > ViewApp::c_updateInterval())
    {
      m_game.step();
      // std::cout << m_game << std::endl; // debugging
      ViewApp::changeLastUpdateTime(curTime);
    }
  }
}

void ViewApp::handleKeyEvent()
{
  if (checkKeyPress(GLFW_KEY_ESCAPE))
  {
    glfwSetWindowShouldClose(ViewApp::c_window(), 1);
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

void ViewApp::setupVBO()
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
  ViewApp::updateCellStateTexture();
}

void ViewApp::updateCellStateTexture()
{
  auto numRows{ViewApp::worldSizeX()};
  auto numCols{ViewApp::worldSizeY()};
  m_stateBuffer.resize(numRows * numCols);
  int idx = 0;
  for (const auto& row: ViewApp::c_game().c_world() | std::views::take(numRows))
  {
    // m_stateBuffer.insert(m_stateBuffer.end(), row.begin(), row.begin() + numCols);
    std::copy(row.begin(), row.begin() + numCols, m_stateBuffer.begin() + idx);
    idx += numCols;
  }

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
void ViewApp::CAstep()
{
  ViewApp::game().step();
}
const int ViewApp::worldSizeX() const
{
  return ViewApp::c_game().sizeX();
}
const int ViewApp::worldSizeY() const
{
  return ViewApp::c_game().sizeY();
}
const std::pair<int, int> ViewApp::worldSize() const
{
  return ViewApp::c_game().size();
}
void ViewApp::setWorldSize(int numRow, int numCol)
{
  ViewApp::game().setWorldSize(numRow, numCol);
  ViewApp::resizeStateBuffer(numRow, numCol);

  // uniform change
  ViewApp::c_shader().use();
  GLint uloc{glGetUniformLocation(ViewApp::c_shader().c_ID(), "sizeX")};
  glUniform1i(uloc, ViewApp::worldSizeX());
  uloc = glGetUniformLocation(ViewApp::c_shader().c_ID(), "sizeY");
  glUniform1i(uloc, ViewApp::worldSizeY());

}
void ViewApp::changeState(GridIndex idx, CellState newVal)
{
  ViewApp::game().change(idx, newVal);
}
void ViewApp::randomPopulate(float p)
{
  ViewApp::game().randomPopulate(p);
}

void ViewApp::resizeStateBuffer(int numRow, int numCol)
{
  m_stateBuffer.resize(numRow * numCol);
}


// internal timer related member functions

void ViewApp::changeUpdateInterval(double newVal)
{
  if (newVal > 0.01 && newVal < 10.0) ViewApp::m_updateInterval = newVal;
}
void ViewApp::mulUpdateInterval(double multiplier)
{
  double newVal{ViewApp::c_updateInterval() * multiplier};
  changeUpdateInterval(newVal);
}
void ViewApp::increUpdateInterval(double diff)
{
  double newVal{ViewApp::c_updateInterval() + diff};
  changeUpdateInterval(newVal);
}
void ViewApp::changeLastUpdateTime(double newVal)
{
  if (newVal > 0.0) ViewApp::m_lastUpdateTime = newVal;
}
