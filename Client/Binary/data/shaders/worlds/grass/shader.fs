$input v_color0, v_texcoord0, v_setting

#include "../../common.sh"

SAMPLER2DARRAY(s_textures, 4);

void main()
{
	vec4 color = texture2DArray(s_textures, vec3(v_texcoord0, v_setting)) * v_color0;
	color *= step(0.25, color.a);
	gl_FragColor = color;
}