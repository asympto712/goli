#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in int cellPos;
uniform samplerBuffer sb;
out vec4 quadColor;

void main()
{
    gl_Position = vec4(pos, 1.0);
    quadColor = texelFetch(sb, cellPos);
}