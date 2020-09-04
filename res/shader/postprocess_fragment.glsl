#version 330 core

uniform vec4 u_WaterColor;
uniform sampler2D u_Color;
uniform sampler2D u_Depth;

in vec2 UV;

out vec4 color;

uniform float zNear = 0.1;
uniform float zFar = 1000.0;

float linearDepth(float depthSample)
{
    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
    return zLinear;
}

void main()
{
    //color = vec4(vec3(1 - linearDepth(texture(u_Depth, UV).r) / zFar) * texture(u_Color, UV).rgb, 1.0) * u_WaterColor;

    color = texture(u_Color, UV) * u_WaterColor;
}