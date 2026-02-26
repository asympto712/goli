#include "shader.hpp"

int readFile2String(const std::string& filePath, std::string& target)
{
  if ( !target.empty() ) 
  {
    std::cout << "Target string is not empty. Aborting...\n";
    return 1;
  }
  std::ifstream file{filePath};
  std::stringstream ss;
  ss << file.rdbuf();
  target = ss.str();
  file.close();
  return 0;
}

GLuint shaderFromSource(const char* c_source_string, GLenum shaderType, const std::string& shaderPath)
{
  GLuint shader{glCreateShader(shaderType)};
  GLint success;
  char infoLog[256];

  glShaderSource(shader, 1, &c_source_string, nullptr);
  glCompileShader(shader);
  if (glGetShaderiv(shader, GL_COMPILE_STATUS, &success); !success)
  {
      glGetShaderInfoLog(shader, 256, nullptr, infoLog);
      std::cout <<
      std::format("Shader compilation failed for {}. Log: \n{}", shaderPath, infoLog)
      <<
      std::endl;
  }

  return shader;
}

int linkShaderProgram(GLuint programID, std::vector<GLuint> shaderIDs)
{
  for (auto&& shader: shaderIDs)
  {
    glAttachShader(programID, shader);
  }
  glLinkProgram(programID);

  GLint success;
  char infoLog[256];
  if (glGetProgramiv(programID, GL_LINK_STATUS, &success); !success)
  {
    glGetProgramInfoLog(programID, 256, nullptr, infoLog);
    std::cout << std::format("program linking failed. Log:\n{}\n", infoLog);
    return 1;
  }
  
  for (auto&& shader: shaderIDs)
  {
    glDeleteShader(shader);
  }
  return 0;
}

VFShader::VFShader(const std::string& vshader_path, const std::string& fshader_path)
{
  VFShader::setup(vshader_path, fshader_path);
}

void VFShader::setup(const std::string& vshader_path, const std::string& fshader_path)
{
  m_programID = glCreateProgram();
  std::string source_code;
  const char* c_source_code;

  readFile2String(vshader_path, source_code);
  c_source_code = source_code.c_str();
  auto vShaderId{shaderFromSource(c_source_code, GL_VERTEX_SHADER, vshader_path)};
  source_code.clear();

  readFile2String(fshader_path, source_code);
  c_source_code = source_code.c_str();
  auto fShaderId{shaderFromSource(c_source_code, GL_FRAGMENT_SHADER, fshader_path)};

  linkShaderProgram(m_programID, std::vector<GLuint>{vShaderId, fShaderId});
}

void VFShader::use() const
{
  glUseProgram(m_programID);
}
