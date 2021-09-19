#version 300 es
precision highp float;

uniform vec4 u_WaterColor;
uniform vec3 u_WaterFarColor;
uniform sampler2D u_Color;
uniform sampler2D u_Depth;
uniform bool u_bInWater;
uniform bool u_bFog;
uniform vec3 u_SkyColor;
uniform int u_ScreenWidth;
uniform int u_ScreenHeight;
uniform int u_FogNear;
uniform int u_FogFar;
uniform int u_FogExponent;

in vec2 UV;

out vec4 color;

uniform float u_ViewNear;
uniform float u_ViewFar;

float linearDepth(float depthSample)
{
    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * u_ViewNear * u_ViewFar / (u_ViewFar + u_ViewNear - depthSample * (u_ViewFar - u_ViewNear));
    return zLinear;
}

float asphalt_pow(float a, int n)
{
    float result = 1.0;
    for(int i = 0; i < n; ++i)
    {
        result *= a;
    }
    return result;
}

void main()
{
    vec4 COLOR = texture(u_Color, UV);
    float DEPTH_EXP = texture(u_Depth, UV).r;
    float DEPTH = linearDepth(DEPTH_EXP);
    float DEPTH_NORMAL = DEPTH / u_ViewFar;
    vec2 PIXEL_SIZE = vec2(1.0 / float(u_ScreenWidth), 1.0 / float(u_ScreenHeight));

    color = vec4(u_SkyColor, 1.0);
    color = vec4( COLOR.rgb * vec3(COLOR.a) + color.rgb * vec3(1.0 - COLOR.a) , 1.0);

    if(u_bInWater)
    {
        color = color * u_WaterColor;
        float water_fog_near = 5.0;
        float water_fog_far = 300.0;
        float fog_amount = asphalt_pow(1.0 - (min(max((DEPTH - water_fog_near) / (water_fog_far - water_fog_near), 0.0), 1.0)), 3);
        color = vec4(color.rgb * vec3(fog_amount) + u_WaterFarColor * vec3(1.0 - fog_amount), 1.0);

    }

    if(!u_bInWater)
    {
        float fog_amount = asphalt_pow(1.0 - (min(max((DEPTH - float(u_FogNear)) / float(u_FogFar - u_FogNear), 0.0), 1.0)), u_FogExponent);
        color = vec4(color.rgb * vec3(fog_amount) + u_SkyColor * vec3(1.0 - fog_amount), 1.0);
    }

    int crosshair_width = 2;
    int crosshair_height = 20;

    if((UV.x > 0.5 - PIXEL_SIZE.x * (float(crosshair_width) / 2.0) && UV.x < 0.5 + PIXEL_SIZE.x * (float(crosshair_width) / 2.0) && UV.y > 0.5 - PIXEL_SIZE.y * (float(crosshair_height) / 2.0) && UV.y < 0.5 + PIXEL_SIZE.y * (float(crosshair_height) / 2.0))
    || (UV.y > 0.5 - PIXEL_SIZE.y * (float(crosshair_width) / 2.0) && UV.y < 0.5 + PIXEL_SIZE.y * (float(crosshair_width) / 2.0) && UV.x > 0.5 - PIXEL_SIZE.x * (float(crosshair_height) / 2.0) && UV.x < 0.5 + PIXEL_SIZE.x * (float(crosshair_height) / 2.0)))
    {
        color = vec4(vec3(1) - color.rgb, 1.0);
    }
}