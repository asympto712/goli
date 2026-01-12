#include "text.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <format>


int createCharacterMap(
  std::map<GLchar, Character>& character_map
)
{
  GLuint texture;
  // init FreeType
  FT_Library ft;
  if (FT_Init_FreeType(&ft) != 0)
  {
    std::cout << "Could not initialize FreeType lib\n";
    return 1;
  }

  FT_Face face;
  if (FT_New_Face(ft, "assets/arial.ttf", 0, &face))
  {
    std::cout << "Failed to load font\n";
    return 1;
  }

  FT_Set_Pixel_Sizes(face, 0, 48);
  // disable byte-alignment restriction
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for (unsigned char c{0}; c<128; c++)
  {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0)
    {
      std::cout << "failed to load glyph\n";
      return 1;
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Character character{
      .TextureID{texture},
      .Size{glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows)},
      .Bearing{glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top)},
      .Advance{static_cast<unsigned int>(face->glyph->advance.x)}
    };

    character_map.insert({c, character});
  }
  glBindTexture(GL_TEXTURE_2D, 0); // unbind

  // no longer needed
  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  return 0;
}

int setupTextVertexObjects(GLuint& VAO, GLuint& VBO)
{
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return 0;
}

void renderText(GLuint& programID, GLuint& VAO, GLuint& VBO, std::map<GLchar, Character>& character_map, std::string text, float x, float y, float scale, glm::vec3 color)
{
  glUseProgram(programID);
  glUniform3f(glGetUniformLocation(programID, "textColor"), color.x, color.y, color.z);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(VAO);

  for (auto c{text.begin()}; c != text.end(); ++c)
  {
    Character ch{character_map.at(*c)};
    float xpos{x + ch.Bearing.x * scale};
    float ypos{y - (ch.Size.y - ch.Bearing.y) * scale};
    float w{ch.Size.x * scale};
    float h{ch.Size.y * scale};
    float vertices[6][4]{
      {xpos, ypos + h, 0.0f, 0.0f},
      {xpos, ypos,     0.0f, 1.0f},
      {xpos + w, ypos, 1.0f, 1.0f},
      {xpos, ypos + h, 0.0f, 0.0f},
      {xpos + w, ypos, 1.0f, 1.0f},
      {xpos + w, ypos + h, 1.0f, 0.0f}
    };

    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    x += (ch.Advance >> 6) * scale;
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

#ifdef TEXT_DEMO
int main()
{
  int windowHeight{600};
  int windowWidth{800};
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto window{glfwCreateWindow(windowWidth, windowHeight, "text demo", NULL, NULL)};

  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "GLAD load failed\n";
    exit(1);
  }

  glViewport(0, 0, windowWidth, windowHeight);

  GLuint VAO, VBO;
  std::map<GLchar, Character> character_map;

  if (createCharacterMap(character_map) != 0)
  {
    std::cout << "failed to create character map\n";
    exit(1);
  }

  setupTextVertexObjects(VAO, VBO);

  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLuint vshader{glCreateShader(GL_VERTEX_SHADER)};
  GLuint fshader{glCreateShader(GL_FRAGMENT_SHADER)};
  GLuint program{glCreateProgram()};

  // load shader program
  GLint success;
  char infoLog[256];
  std::ifstream file;
  std::string source;
  std::stringstream ss;
  file.open("src/shader/text.vs");
  ss << file.rdbuf();
  source = ss.str();
  const char* vsrc{source.c_str()};
  glShaderSource(vshader, 1, &vsrc, NULL);
  glCompileShader(vshader);
  if (glGetShaderiv(vshader, GL_COMPILE_STATUS, &success); !success)
  {
    glGetShaderInfoLog(vshader, 256, nullptr, infoLog);
    std::cout << std::format("vshader compilation failed. log: {}\n", infoLog);
  }
  file.close();
  source.clear();
  ss.str("");
  file.open("src/shader/text.fs");
  ss << file.rdbuf();
  source = ss.str();
  const char* fsrc{source.c_str()};
  glShaderSource(fshader, 1, &fsrc, NULL);
  glCompileShader(fshader);
  if (glGetShaderiv(fshader, GL_COMPILE_STATUS, &success); !success)
  {
    glGetShaderInfoLog(fshader, 256, nullptr, infoLog);
    std::cout << std::format("fshader compilation failed. log: {}\n", infoLog);
    exit(1);
  }

  glAttachShader(program, vshader);
  glAttachShader(program, fshader);
  glLinkProgram(program);
  if (glGetProgramiv(program, GL_LINK_STATUS, &success); !success)
  {
    glGetProgramInfoLog(program, 256, nullptr, infoLog);
    std::cout << std::format("program linking failed: {}\n", infoLog);
    exit(1);
  }

  glDeleteShader(vshader);
  glDeleteShader(fshader);

  glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight));
  glUseProgram(program);
  glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

  // glUseProgram(program);
  // GLint loc;
  // loc = glGetUniformLocation(program, "text");
  // glUniform1i(loc, 0);

  // render
  while (!glfwWindowShouldClose(window))
  {

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    glClearColor(0.2f, 0.2f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderText(
      program,
      VAO,
      VBO,
      character_map,
      "hello world!",
      25.0f,
      25.0f,
      1.0f,
      glm::vec3(0.5f, 0.8f, 0.2f)
    );

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
#endif