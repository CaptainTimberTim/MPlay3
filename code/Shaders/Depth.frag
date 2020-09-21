#version 400 

uniform float MaxDist;
uniform vec3 CamPos;

uniform vec3 LightColor;
uniform vec3 LightPower;

uniform float Transparency;

uniform sampler2D Texture;

in vec3 LightPos;
in vec3 PositionFrag;
in vec2 TexCoordsFrag;
in vec3 NormalFrag;

out vec4 FinalColor;

// Idea:: render this into FBO then render the scene with
// normal shading, on that make edge detection and
// overlay both.

void main()
{
    vec3  N = normalize(NormalFrag);
    float D = distance(CamPos, PositionFrag);
    float ND = min(1.0 - D/MaxDist, 0.9);
    
    vec3  TC = texture2D(Texture,TexCoordsFrag).rgb;
    float TG = (TC.x + TC.y + TC.z)/3;
    //ND *= TG;
#if 0
    FinalColor = vec4((N+1)/2, 1.0); // Normal moved to color range 0 - 1
#else
    FinalColor = vec4(ND, ND+0.1, ND, 1.0-Transparency);
#endif
}