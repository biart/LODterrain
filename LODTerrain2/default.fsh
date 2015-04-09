#version 330 core

uniform vec3 wireframeColor;

in  vec3 ex_Color;
noperspective in  vec3 ex_BarycentricCoord;
out vec4 out_Color;
 
void main(void)
{
        float dist = min(min(ex_BarycentricCoord.x, ex_BarycentricCoord.y), ex_BarycentricCoord.z);
        out_Color = vec4(mix(ex_Color, wireframeColor, exp(-dist)), 1.0f);
}