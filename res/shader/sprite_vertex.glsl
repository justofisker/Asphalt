#version 300 es
precision mediump float;

layout(location = 0) in vec4 v_Position;

out vec2 TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * v_Position;
    TexCoord = vec2(v_Position.x, v_Position.y);
}
