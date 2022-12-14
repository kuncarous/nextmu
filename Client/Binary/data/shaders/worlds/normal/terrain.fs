$input v_color0, v_texcoord0, v_setting

#include "../../common.sh"

SAMPLER2DARRAY(s_textures, 4);

void main()
{
	vec4 tex1 = texture2DArray(s_textures, vec3(v_texcoord0.xy, v_setting.x)) * v_color0;
	vec4 tex2 = texture2DArray(s_textures, vec3(v_texcoord0.zw, v_setting.y)) * v_color0;
	gl_FragColor = vec4(mix(tex1.rgb, tex2.rgb, v_setting.z * step(0.25, v_setting.z)), 1.0f);
}