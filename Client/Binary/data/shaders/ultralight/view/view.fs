$input v_texcoord0

#include "../../common.sh"

SAMPLER2D(s_view, 0);

void main()
{
	gl_FragColor = texture2D(s_view, v_texcoord0);
}