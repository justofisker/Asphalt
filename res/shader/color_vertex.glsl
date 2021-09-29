#version 300 es
precision highp float;

layout(location = 0) in vec3 v_Position;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform vec3 u_Size;

void main()
{
    vec3 scaled_position = vec3(v_Position.x * u_Size.x, v_Position.y * u_Size.y, v_Position.z * u_Size.z);
    gl_Position = u_Projection * u_View * u_Model * vec4(scaled_position, 1.0);
    gl_Position.z = gl_Position.z - 0.0003;
}
