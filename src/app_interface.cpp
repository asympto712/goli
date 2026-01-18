#include "app_interface.hpp"

void appWindowSizeCallBack(GLFWwindow* window, int width, int height)
{
    glfwSetWindowSize(window, width, height);
}

BasicApp::BasicApp(int _width, int _height)
{
  BasicApp::setWindowSize(_width, _height);
  BasicApp::init();
}

void BasicApp::init()
{
  if (m_width < 10 || m_height < 10)
  {
    std::cout << "Window size is too small\n";
    return;
  }
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  m_window = glfwCreateWindow(m_width, m_height, "Game of Life Interactive(GoLI)", NULL, NULL);

  if (m_window == nullptr)
  {
    std::cout << "window creation failed\n";
    return;
  }

  glfwMakeContextCurrent(m_window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
      std::cout << "GLAD load failed" << std::endl;
  }

  glViewport(0, 0, m_width, m_height);

  glfwSetWindowSizeCallback(m_window, appWindowSizeCallBack);

}

void BasicApp::setWindowSize(int _width, int _height)
{
  if (_width < 10 || _height < 10)
  {
    std::cout << "Window size is too small\n";
    return;
  }
  else
  {
    m_width = _width;
    m_height = _height;
  }
}

void BasicApp::setupShader(const std::string& vShader_path, const std::string& fShader_path)
{
  m_shader.setup(vShader_path, fShader_path);
  m_shaderInit = true;
}

VFShader& BasicApp::shader()
{
  return m_shader;
}

const VFShader& BasicApp::c_shader() const
{
  return m_shader;
}

void BasicApp::quit()
{
  glfwTerminate();
}