#version 300 es
precision highp float;

out vec4 color;

in vec2 UV;
in vec3 Normal;

uniform sampler2D u_Texture;

void main()
{
    vec4 tint;
    if(Normal.x != 0.0)
        tint = vec4(0.8, 0.8, 0.85, 1.0);
    else if (Normal.z != 0.0)
        tint = vec4(0.6, 0.6, 0.65, 1.0);
    else if(Normal.y < 0.0)
        tint = vec4(0.4, 0.4, 0.45, 1.0); 
    else
        tint = vec4(1.0, 1.0, 1.0, 1.0);

    color = texture(u_Texture, UV) * tint;
}