#version 400 

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 NormalMatrix;

layout(location = 0) in vec3 PositionVert;
layout(location = 1) in vec2 TexCoordsVert;
layout(location = 2) in vec3 NormalVert;
layout(location = 3) in vec3 RandomColors;

out vec3 PositionFrag;
out vec2 TexCoordsFrag;
out vec3 NormalFrag;
out vec3 RandColors;

void main()
{
    PositionFrag  = (M*vec4(PositionVert, 1.0f)).xyz;
    NormalFrag    = (NormalMatrix*vec4(NormalVert, 1.0f)).xyz;
    TexCoordsFrag = TexCoordsVert;
    RandColors    = RandomColors;
    gl_Position   = P*V*M*vec4(PositionVert, 1.0f);
}