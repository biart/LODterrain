#version 330 core

uniform float wireframeThickness;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vert_Color[];
out vec3 ex_Color;
noperspective out vec3 ex_BarycentricCoord;

void main ()
{
    for ( int i = 0; i < gl_in.length (); i++ )
    {
        vec2 p0 = gl_in[i + 0].gl_Position.xy / gl_in[i + 0].gl_Position.w;
        vec2 p1 = gl_in[i + 1].gl_Position.xy / gl_in[i + 1].gl_Position.w;
        vec2 p2 = gl_in[i + 2].gl_Position.xy / gl_in[i + 2].gl_Position.w;

        vec2 v0 = p2 - p1;
        vec2 v1 = p2 - p0;
        vec2 v2 = p1 - p0;

        float area = abs(v1.x*v2.y - v1.y*v2.x) / wireframeThickness;

        ex_BarycentricCoord = vec3(area / length(v0), 0.0, 0.0);
        ex_Color = vert_Color[i];
        gl_Position = gl_in [i].gl_Position;
        EmitVertex ();
        i++;

        ex_BarycentricCoord = vec3(0.0, area / length(v1), 0.0);
        ex_Color = vert_Color[i];
        gl_Position = gl_in [i].gl_Position;
        EmitVertex ();
        i++;

        ex_BarycentricCoord = vec3(0.0, 0.0, area / length(v2));
        ex_Color = vert_Color[i];
        gl_Position = gl_in [i].gl_Position;
        EmitVertex ();
    }
	
    EndPrimitive ();
}