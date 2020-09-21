#version 400 

in vec3 PositionFrag;
in vec2 TexCoordsFrag;
in vec3 NormalFrag;
in vec3 RandColors;

uniform sampler2D Texture;

out vec4 FinalColor;

void main()
{
    vec3  N = normalize(NormalFrag);
    
    FinalColor = vec4((N+1)/2, 1.0); // Normal moved to color range 0 - 1
    //FinalColor = vec4(RandColors, 1.0);
    //FinalColor = texture(Texture, TexCoordsFrag)*FinalColor;
}