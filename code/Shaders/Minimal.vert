#version 400 

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

layout(location = 0) in vec3 VertPosition; 

void main() 
{ 
    gl_Position = Projection*View*Model*vec4(VertPosition, 1.0); 
}