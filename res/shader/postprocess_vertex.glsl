#version 300 es
precision highp float;

layout(location = 0) in vec2 v_Position;
layout(location = 1) in vec2 v_UV;

out vec2 UV;

void main()
{
    gl_Position = vec4(v_Position, 1.0, 1.0);
    UV = v_UV;
}