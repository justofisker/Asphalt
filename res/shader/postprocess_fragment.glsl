#version 330 core

uniform vec4 u_WaterColor = vec4(0.4, 0.4, 1.0, 1.0);
uniform sampler2D u_Color;
uniform sampler2D u_Depth;

in vec2 UV;

out vec4 color;

uniform float zNear = 0.1;
uniform float zFar = 1000.0;

uniform bool u_bInWater;
uniform bool u_bFog;

float linearDepth(float depthSample)
{
    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
    return zLinear;
}

void main()
{
    vec4 COLOR = texture(u_Color, UV);
    float DEPTH_EXP = texture(u_Depth, UV).r;
    float DEPTH = linearDepth(DEPTH_EXP);
    float DEPTH_NORMAL = DEPTH / zFar;

    color = COLOR;

    if(u_bInWater)
    {
        color = color * u_WaterColor;

        color = vec4(color.rgb * vec3(pow((1.0 - DEPTH_NORMAL), 50)), 1.0);

    }

    //color = texture(u_Color, UV) * u_WaterColor;
}