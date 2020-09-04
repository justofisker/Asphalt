#version 330 core

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec2 v_UV;
layout(location = 2) in vec3 v_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 UV;
out vec3 Normal;

void main()
{
    UV = v_UV;
    Normal = v_Normal;
    gl_Position = u_Projection * u_View * u_Model * vec4(v_Position, 1.0);
}