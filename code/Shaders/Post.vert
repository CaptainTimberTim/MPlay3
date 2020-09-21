#version 400

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoords;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec2 FragTexCoords;

void main()
{
    FragTexCoords = TexCoords;
    gl_Position = Projection*View*Model*vec4(Position, 1.0);
}