#version 400 

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 NormalMatrix;
uniform vec3 LightPosV;

layout(location = 0) in vec3 PositionVert;
layout(location = 1) in vec2 TexCoordsVert;
layout(location = 2) in vec3 NormalVert;

out vec3 LightPos;
out vec3 PositionFrag;
out vec2 TexCoordsFrag;
out vec3 NormalFrag;

void main()
{
    PositionFrag  = (M*vec4(PositionVert, 1.0f)).xyz;
    NormalFrag    = (NormalMatrix*vec4(NormalVert, 1.0f)).xyz;
    LightPos      = (V*vec4(LightPosV, 1.0)).xyz;
    TexCoordsFrag = TexCoordsVert;
    gl_Position   = P*V*M*vec4(PositionVert, 1.0f);
}