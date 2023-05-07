#include "./parameters.sh"

$input a_position COLOR_INPUT UV0_INPUT UV1_INPUT RECT_INPUT TILE_INPUT COVERAGE_INPUT IMAGE_INPUT
$output v_color0 UV0_OUTPUT UV1_OUTPUT DOWNSAMPLE_OUTPUT SDF_OUTPUT RECT_OUTPUT TILE_OUTPUT COVERAGE_OUTPUT IMAGE_OUTPUT

#include "../../common.sh"

uniform mat4 cbuffer0_vs;
uniform vec4 cbuffer1_vs;

float SRGBToLinear(float v)
{
    if (v <= 0.04045)
    {
      return v * (1.0 / 12.92);
    }
    else
    {
      return pow( v * (1.0 / 1.055) + 0.0521327, 2.4);
    }
}

// BGFX is missing Uint16 support...
vec4 Uint16UnormStep = vec4(
	0.0,
	0.0,
	0.0,
	0.0
);
vec4 Int16x4SNormToUnorm(vec4 value)
{
	vec4 select = vec4(lessThan(value, Uint16UnormStep));
	// Why I didn't use a single mix? because it is only supported in OpenGL 4.0+ and OpenGLES 3.0+ only
	return vec4(
		mix(value.x * 0.5, 0.5 + (1.0 + value.x) * 0.5, select.x),
		mix(value.y * 0.5, 0.5 + (1.0 + value.y) * 0.5, select.y),
		mix(value.z * 0.5, 0.5 + (1.0 + value.z) * 0.5, select.z),
		mix(value.w * 0.5, 0.5 + (1.0 + value.w) * 0.5, select.w)
	);
}

void main()
{
    gl_Position = mul(vec4(a_position, 0.0, 1.0), cbuffer0_vs);

#ifdef HAS_COLOR
  #ifdef SRGB
    v_color0.r = SRGBToLinear(a_color0.r);
    v_color0.g = SRGBToLinear(a_color0.g);
    v_color0.b = SRGBToLinear(a_color0.b);
    v_color0.a = a_color0.a;
  #else
    v_color0 = a_color0;
  #endif
#endif

#ifdef DOWNSAMPLE
    v_texcoord0 = a_texcoord0 + vec2(a_texcoord1.x, a_texcoord1.y);
    v_texcoord1 = a_texcoord0 + vec2(a_texcoord1.x, -a_texcoord1.y);
    v_texcoord2 = a_texcoord0 + vec2(-a_texcoord1.x, a_texcoord1.y);
    v_texcoord3 = a_texcoord0 + vec2(-a_texcoord1.x, -a_texcoord1.y);
#else
    #ifdef HAS_UV0
        v_texcoord0 = a_texcoord0;
    #endif
    #ifdef HAS_UV1
        v_texcoord1 = a_texcoord1;
    #endif
#endif

#ifdef SDF
    v_st1 = vec2(a_texcoord1 * vec2(cbuffer1_vs[0], cbuffer1_vs[1]));
#endif

#ifdef HAS_RECT
    v_rect = Int16x4SNormToUnorm(a_texcoord2);
#endif

#ifdef HAS_TILE
    v_tile = a_texcoord3;
#endif

#ifdef HAS_COVERAGE
    v_coverage = a_texcoord4;
#endif

#ifdef HAS_IMAGE_POSITION
    v_imagePos = a_texcoord5;
#endif
}