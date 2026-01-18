#pragma once

#include <glad/glad.h>
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

GLuint shaderFromSource(const char* c_source_string, GLenum shaderType, const std::string& shaderPath);

int linkShaderProgram(GLuint programID, std::vector<GLuint> shaderIDs);

class Shader
{
  public:
    Shader() = default;
    virtual ~Shader() = default;
    virtual void use() const = 0;
};

class VFShader: public Shader
{
  public:
  VFShader() = default;
  VFShader(const std::string& vshader_path, const std::string& fshader_path);
  void setup(const std::string& vshader_path, const std::string& fshader_path);
  void use() const override;
  GLuint ID() {
    return m_programID;
  }
  const GLuint c_ID() const {
    return m_programID;
  }

  private:
  GLuint m_programID;
};


