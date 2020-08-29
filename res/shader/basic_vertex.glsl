#version 460 core

layout(location = 0) in vec2 v_Position;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    gl_Position = vec4(v_Position, 1.0 , 1.0);
}