#version 300 es
precision highp float;

layout(location = 0) in vec3 v_Position;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(v_Position, 1.0);
    gl_Position.z = gl_Position.z - 0.0003;
}
