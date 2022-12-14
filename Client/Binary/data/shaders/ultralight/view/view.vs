$input a_position
$output v_texcoord0

#include "../../common.sh"

uniform vec4 u_screen;

void main()
{
	v_texcoord0 = vec2(a_position);
	gl_Position = mul(u_modelViewProj, vec4(vec2(a_position) * u_screen.xy, 0.0, 1.0));
}