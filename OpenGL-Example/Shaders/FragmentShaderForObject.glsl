#version 430

uniform sampler2D BaseTexture;
uniform vec3 LightColor;
uniform int LightIsOn;

in vec3 position_in_ec;
in vec3 normal_in_ec;

in vec3 light_position_in_ec;

in vec3 color;

layout (location = 0) out vec4 final_color;

vec3 calculateLightingEquation()
{
   vec3 light_vector = light_position_in_ec - position_in_ec;

   vec3 distance_factors;
   distance_factors.x = 1.0;
   distance_factors.z = dot( light_vector, light_vector );
   distance_factors.y = sqrt( distance_factors.z );
   
   vec3 attenuation_factors = vec3(0.005, 0.005, 0.01);
   float attenuation_effect = min( 1.0 / dot( distance_factors, attenuation_factors ), 1.0 );

   light_vector = normalize( light_vector );

   vec3 computed_color = vec3(0.0);
   vec3 normal = normalize( normal_in_ec );
   if (attenuation_effect > 0.0) {
      vec3 halfway_vector = normalize( light_vector - normalize( position_in_ec ).xyz );
      float n_dot_h = max( 0.0, dot( normal, halfway_vector ) );
      computed_color += LightColor * pow( n_dot_h, 128 );
      computed_color *= attenuation_effect;
   }

   const float diffuse_reflection_coefficient = 0.7529f;
   computed_color += LightColor * diffuse_reflection_coefficient * max( dot( normal, light_vector ), 0.0 );
   return computed_color;
}

void main()
{
   final_color = vec4(color, 1.0);

   if (LightIsOn != 0) {
      final_color = vec4(calculateLightingEquation(), 1.0);
   }
}