#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include "game.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// interface class for basic app
class AppInterface
{
  public:
  AppInterface() = default;
  virtual ~AppInterface() = default;

  // app related member functions
  /// @brief initialize app
  virtual void init() = 0;
  /// @brief render one frame
  virtual void render() = 0;
  /// @brief pre-render process (e.g. setting up background color, binding correct VAO, etc...)
  virtual void preRender() = 0;
  /// @brief run the app
  virtual void run() = 0;
};

class BasicApp: public AppInterface
{
  public:
  BasicApp(int _width, int _height);

  GLFWwindow* const c_window() const {
    return m_window;
  }

  VFShader& shader();
  const VFShader& c_shader() const;

  void init() override;
  void quit();

  void setWindowSize(int _width, int _height);
  void setupShader(const std::string& vShader_path, const std::string& fShader_path);

  bool isRunning() {
    return !glfwWindowShouldClose(m_window);
  }

  virtual void handleKeyEvent() = 0;
  
  private:
  VFShader m_shader;
  bool m_shaderInit = false;
  GLFWwindow* m_window = nullptr;
  int m_width = 0, m_height = 0;
};

// app that simply views the entire simulation. Demo only.
class ViewApp: public BasicApp
{
  public:

  ViewApp(int _width, int _height);
  ViewApp(int _width, int _height, const std::string& _vShaderPath, const std::string& _fShaderPath);

  void setupShader();
  void setupVBO();

  void run() override;
  void preRender() override;
  void render() override;
  void handleKeyEvent() override;
  bool checkKeyPress(int key) {
    return glfwGetKey(c_window(), key) == GLFW_PRESS;
  }
  void stop() {
    m_stopped = true;
  }
  void resume() {
    m_stopped = false;
  }
  bool isStopped() const {
    return m_stopped;
  }

  StandardGoL& game() {
    return m_game;
  }
  const StandardGoL& c_game() const {
    return m_game;
  }

  void updateCellStateTexture();

  // interface to CA
  std::vector<CellState>& stateBuffer() {
    return m_stateBuffer;
  }
  const int worldSizeX() const;
  const int worldSizeY() const;
  const std::pair<int, int> worldSize() const;
  void setWorldSize(int numRow, int numCol);
  void changeState(GridIndex idx, CellState newVal);
  void CAstep();
  void randomPopulate(float p);

  void resizeStateBuffer(int numRow, int numCol);


  const std::string& c_vShaderPath() const{
    return m_vShaderPath;
  }
  const std::string& c_fShaderPath() const{
    return m_fShaderPath;
  }
  std::string& vShaderPath() {
    return m_vShaderPath;
  }
  std::string& fShaderPath() {
    return m_fShaderPath;
  }
  void setShaderPath(const std::string& _vShaderPath, const std::string& _fShaderPath);

  const double c_updateInterval() const {
    return m_updateInterval;
  }
  void changeUpdateInterval(double newVal);
  void mulUpdateInterval(double multiplier);
  void increUpdateInterval(double diff);
  const double c_lastUpdateTime() const {
    return m_lastUpdateTime;
  }
  void changeLastUpdateTime(double newVal);
  void speedUp(double multiplier) {
    mulUpdateInterval(1.0 / multiplier);
  }

  private:
  StandardGoL m_game;
  std::string m_vShaderPath = "src/shader/view_app.vs";
  std::string m_fShaderPath = "src/shader/view_app.fs";
  GLuint m_VBO[5];
  GLuint m_VAO[5];
  GLuint m_texID;
  std::vector<CellState> m_stateBuffer;
  bool m_stopped = false;
  double m_updateInterval = 1.0; // in seconds
  double m_lastUpdateTime;
};