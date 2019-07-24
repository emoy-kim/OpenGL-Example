#version 430

uniform mat4 ModelViewProjectionMatrix;
uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform vec3 PrimitiveColor;
uniform vec3 LightPosition;

layout (location = 0) in vec4 v_position;
layout (location = 1) in vec4 v_normal;

out vec3 position_in_ec;
out vec3 normal_in_ec;

out vec3 light_position_in_ec;

void main()
{   
   vec4 e_position = ViewMatrix * WorldMatrix * v_position;
   vec4 e_normal = transpose( inverse( ViewMatrix * WorldMatrix ) ) * v_normal;
   position_in_ec = e_position.xyz;
   normal_in_ec = e_normal.xyz;

   light_position_in_ec = vec3(ViewMatrix * vec4(LightPosition, 1.0));

   gl_Position = ModelViewProjectionMatrix * v_position;
}