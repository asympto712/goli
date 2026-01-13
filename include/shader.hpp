#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <format>
#include <vector>

int readFile2String(const std::string& filePath, std::string& target);

GLuint shaderFromSource(const char* c_source_string, GLenum shaderType, std::string& shaderPath);

int linkShaderProgram(GLuint programID, std::vector<GLuint> shaderIDs);

class Shader
{
  public:
    Shader() = default;
    virtual ~Shader() = default;
    virtual void use() = 0;
};

class VFShader: public Shader
{
  public:
  VFShader(std::string& vshader_path, std::string& fshader_path);
  void use() override;

  private:
  GLuint m_programID;
};


