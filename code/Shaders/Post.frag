#version 400

uniform vec4 DampingColor;
uniform sampler2D TexMain;
uniform sampler2D TexNormal;

in  vec2 FragTexCoords;
out vec4 OutColor;

void main()
{
    vec4 TexColor = texture2D(TexMain, FragTexCoords);
    vec3 NewColor = (TexColor.r == TexColor.g && TexColor.r == TexColor.b) 
        ? TexColor.rgb : mix(TexColor.rgb, DampingColor.rgb, DampingColor.a);
    OutColor = vec4(NewColor, TexColor.a);
}