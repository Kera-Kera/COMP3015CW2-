#version 460

uniform float LineWidth;
uniform vec4 LineColor;
uniform vec4 LightPosition;
uniform vec3 LightIntensity;
uniform vec3 Kd;
uniform vec3 Ka;

in vec3 GNormal;
in vec3 GPosition;

flat in int GIsEdge;

layout ( location = 0) out vec4 FragColor;

const int levels = 3;
const float scaleFactor = 1.0 / levels;


vec3 toonShade()
{
    vec3 s = normalize(LightPosition.xyz - GPosition.xyz);
    vec3 ambient = Ka;
    float cosine = dot(s,GNormal);
    vec3 diffuse = Kd * ceil(cosine * levels) * scaleFactor;

    return LightIntensity * (ambient + diffuse);
}


void main() {
    if(GIsEdge == 1)
    {
        FragColor = LineColor;
    }
    else
    {
        FragColor = vec4(toonShade(), 1.0);
    }
}
