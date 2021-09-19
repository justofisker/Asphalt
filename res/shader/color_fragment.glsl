#version 300 es
precision mediump float;
uniform vec4 u_Color;

out vec4 color;

void main()
{
    color = u_Color;
}
