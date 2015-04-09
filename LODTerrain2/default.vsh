#version 330 core

uniform mat4 MVPmatrix;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Color;

out vec3 vert_Color;

void main(void)
{
        gl_Position = MVPmatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
        vert_Color = in_Color;
}