$input a_position
$output v_color0

#include "../common.sh"

uniform vec4 u_bboxMin;
uniform vec4 u_bboxMax;

void main()
{
	v_color0 = vec4(0.5, 0.5, 0.5, 1.0);
	gl_Position = mul(
		u_modelViewProj,
		vec4(
			mix(u_bboxMin.xyz, u_bboxMax.xyz, a_position),
			1.0
		)
	);
}