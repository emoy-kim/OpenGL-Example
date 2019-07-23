#version 330

uniform sampler2D BaseTexture;
uniform vec3 LightColor;

in vec3 position_in_ec;
in vec3 normal_in_ec;

in vec3 color;

layout (location = 0) out vec4 final_color;

void main()
{
   final_color = vec4(color, 1.0);
}