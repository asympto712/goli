#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in int cellPos;

uniform samplerBuffer sb;
uniform mat4 view;
uniform mat4 projection;

out vec4 quadColor;

void main()
{
    gl_Position = projection * view * vec4(pos, 1.0);
    quadColor = texelFetch(sb, cellPos);
}