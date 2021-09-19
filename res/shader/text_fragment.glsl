#version 300 es
precision mediump float;
in vec2 uv;
out vec4 color;

uniform sampler2D u_CharacterImage;
uniform vec4 u_TextColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(u_CharacterImage, uv).r);
    color = u_TextColor * sampled;
}
