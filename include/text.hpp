#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

struct Character {
  unsigned int TextureID; // ID handle of the glyph texture
  glm::ivec2 Size;
  glm::ivec2 Bearing;
  unsigned int Advance;
};