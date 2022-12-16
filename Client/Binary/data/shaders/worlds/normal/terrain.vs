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
	uvec2 rposition = a_texcoord0 + a_position;
	float visible = 1.0 - step(1.0, float(texelFetch(s_attributesTexture, ivec2(a_texcoord0), 0).x & 8u));
	
	vec3 position = vec3(rposition, 0.0);
	position.z = float(texelFetch(s_heightTexture, ivec2(rposition), 0).x) * 1.5;
	
	vec3 normal = texelFetch(s_normalTexture, ivec2(rposition), 0).xyz;
	v_color0 = vec4(texelFetch(s_lightTexture, ivec2(rposition), 0).xyz * clamp(dot(normal, vec3(0.5, -0.5, 0.5)) + 0.5, 0.0, 1.0), 1.0);
	v_color0 *= visible;
	
	v_setting = vec3(texelFetch(s_mappingTexture, ivec2(a_texcoord0), 0).xy, 0.0);
	v_setting.z = float(texelFetch(s_mappingTexture, ivec2(rposition), 0).z) / 255.0;
	
	vec4 uv1 = texelFetch(s_uvTexture, ivec2(v_setting.x, 0), 0);
	vec4 uv2 = texelFetch(s_uvTexture, ivec2(v_setting.y, 0), 0);
	
	v_texcoord0 = vec4(rposition.xyxy) * vec4(uv1.xy, uv2.xy);
	
	vec2 waterMove = vec2(u_terrainSettings.x, sin(u_terrainSettings.z + float(rposition.x) * 5.0) * u_terrainSettings.y * 0.002);
	v_texcoord0 += vec4(waterMove * step(1.0, uv1.z), waterMove * step(1.0, uv2.z));
	
	position *= visible;
	position.xy *= 100.0;
	gl_Position = mul(u_modelViewProj, vec4(position, 1.0));
}