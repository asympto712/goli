#version 330 core
in float cellValue;
// in float odd;
out vec4 FragColor;

void main() {
  FragColor = vec4(cellValue, cellValue, cellValue, 1.0);
}