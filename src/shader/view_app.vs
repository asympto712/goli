#version 330 core
layout (location = 0) in vec2 aPos;
uniform int sizeX;
uniform int sizeY;
uniform sampler2D sampler;
out float cellValue;

// for debugging
// out float odd;

ivec2 getCellCoord(int);
vec2 computeOffset(int);

void main() {
  vec2 offset = computeOffset(gl_InstanceID);
  vec2 localPos = 2 * aPos * vec2(1.0 / float(sizeX), 1.0 / float(sizeY));
  gl_Position = vec4(localPos + offset, 0.0, 1.0);
  ivec2 cellCoord = getCellCoord(gl_InstanceID);
  cellValue = texelFetch(sampler, cellCoord, 0).r;

  // for debugging
  // odd = float(gl_InstanceID % 2);
}

// get the offset from the instance ID
vec2 computeOffset(int idx) {
  ivec2 cellCoord = getCellCoord(idx);
  
  // range [0,1] X [0,1]
  vec2 fCoord = vec2(float(cellCoord.x) / float(sizeX), float(cellCoord.y) / float(sizeY));

  return fCoord * 2.0 - 1.0;
}

// get the cell coordinate in the texel system from the instance index
ivec2 getCellCoord(int idx) {
  int divisor = 0;
  while (idx - sizeX * divisor >= 0)
  {
    divisor++;
  }
  divisor--;

  return ivec2(idx - sizeX * divisor, divisor);
}