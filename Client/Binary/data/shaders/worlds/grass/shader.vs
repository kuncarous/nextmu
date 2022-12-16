$input a_position, a_texcoord0
$output v_color0, v_texcoord0, v_setting

#include "../../common.sh"

USAMPLER2D(s_heightTexture, 0);
SAMPLER2D(s_lightTexture, 1);
SAMPLER2D(s_normalTexture, 2);
USAMPLER2D(s_mappingTexture, 3);
SAMPLER2D(s_uvTexture, 5);
USAMPLER2D(s_attributesTexture, 6);

uniform vec4 u_terrainSettings;

void main()
{
	float visible = 1.0 - step(1.0, float(texelFetch(s_attributesTexture, ivec2(a_texcoord0), 0).x & 8u));
	vec2 settings = vec2(texelFetch(s_mappingTexture, ivec2(a_texcoord0), 0).zw) / 255.0;
	v_setting = settings.y;
	visible *= step(1.0, 1.0 - settings.x);
	visible *= step(0.001, 1.0 - settings.y);
	
	uvec2 gposition = a_texcoord0 + a_position.xx;
	uvec2 rposition = a_texcoord0 + a_position;
	
	vec3 position = vec3(gposition, 0.0);
	position.z = float(texelFetch(s_heightTexture, ivec2(gposition), 0).x) * 1.5;
	position *= visible;
	position.xy *= 100.0;
	
	float height = texelFetch(s_uvTexture, ivec2(v_setting, 0), 0).x;
	
	float apply = float(1u - a_position.y);
	position.x -= 50.0 * apply;
	position.y += sin(u_terrainSettings.z + float(rposition.x) * 5.0) * u_terrainSettings.y * apply;
	position.z += height * apply;
	
	vec3 normal = texelFetch(s_normalTexture, ivec2(rposition), 0).xyz;
	v_color0 = vec4(texelFetch(s_lightTexture, ivec2(rposition), 0).xyz * clamp(dot(normal, vec3(0.5, -0.5, 0.5)) + 0.5, 0.0, 1.0), 1.0);
	v_color0 *= visible;
	
	v_texcoord0 = vec2(rposition.x, a_position.y) * vec2(0.25, 1.0);
	
	gl_Position = mul(u_modelViewProj, vec4(position, 1.0));
}