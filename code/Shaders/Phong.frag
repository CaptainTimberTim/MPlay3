#version 400 

uniform vec3 LightColor;
uniform vec3 LightPower;

uniform vec3 DiffuseColor;
uniform vec3 SpecularColor;
uniform vec3 AmbientColor;
uniform float SpecExp;
uniform float Transparency;
uniform vec3 Tf;

uniform sampler2D Texture;
uniform sampler2D BumpTex;

in vec3 LightPos;
in vec3 PositionFrag;
in vec2 TexCoordsFrag;
in vec3 NormalFrag;

out vec4 FinalColor;

void main()
{
    vec3 N = normalize(NormalFrag);
    vec3 L = normalize(LightPos - PositionFrag);
    vec3 E = normalize(-PositionFrag);
    vec3 R = normalize(reflect(-L, N));
    
    vec3 H = normalize(L+E);
    float Distance = distance(LightPos, PositionFrag);
    
    vec3 AmbColor = (AmbientColor.x < 0) ? vec3(1.0, 1.0, 1.0) : AmbientColor;
    vec3 AmbientComponent = AmbColor*LightColor*4*(LightPower/Distance);
    
    vec3 DiffColor = (DiffuseColor.x < 0) ? vec3(1.0, 1.0, 1.0) : DiffuseColor;
    float Diff = max(dot(N, L), 0.0);
    vec3 DiffuseComponent = Diff*DiffColor*LightColor*LightPower;
    
    vec3 SpecColor = (SpecularColor.x < 0) ? vec3(1.0, 1.0, 1.0) : SpecularColor;
    float Spec = pow(max(dot(E, R), 0.0), SpecExp);
    vec3 SpecularComponent = Spec*SpecColor*LightColor;
    
    // NOTE:: Doing this to ignore lighting when the normals are wrong (specific for sibenik)
    DiffuseComponent = (Tf.x < 1||Tf.y < 1||Tf.z < 1) ? DiffColor : DiffuseComponent;
    
    vec3 DiffuseTexColor = texture2D(Texture,TexCoordsFrag).rgb;
    // Multiplying DiffuseTexColor into both components, may lead to 
    // weird optics
    DiffuseComponent *= DiffuseTexColor;
    AmbientComponent *= DiffuseTexColor;
    //SpecularComponent*= DiffuseTexColor;
    
#if 0
    FinalColor = vec4((N+1)/2, 1.0); // Normal moved to color range 0 - 1
#else
    FinalColor = vec4(AmbientComponent+DiffuseComponent+SpecularComponent, 1-Transparency);
    //FinalColor = vec4(AmbientComponent, 1-Transparency);
#endif
}