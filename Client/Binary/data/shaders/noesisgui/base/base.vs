#ifdef PAINT_SOLID
    #define HAS_COLOR 1
#endif

#if defined(PAINT_LINEAR) || defined(PAINT_RADIAL) || defined(PAINT_PATTERN)
    #define HAS_UV0 1
#endif

#ifdef CLAMP_PATTERN
    #define HAS_RECT 1
#endif

#if defined(REPEAT_PATTERN) || defined(MIRRORU_PATTERN) || defined(MIRRORV_PATTERN) || defined(MIRROR_PATTERN)
    #define HAS_RECT 1
    #define HAS_TILE 1
#endif

#ifdef EFFECT_PATH_AA
    #define HAS_COVERAGE 1
#endif

#ifdef EFFECT_SDF
    #define HAS_UV1 1
    #define HAS_ST1 1
	#define SDF 1
    #define SDF_SCALE 7.96875
    #define SDF_BIAS 0.50196078431
    #define SDF_AA_FACTOR 0.65
    #define SDF_BASE_MIN 0.125
    #define SDF_BASE_MAX 0.25
    #define SDF_BASE_DEV -0.65
#endif

#ifdef EFFECT_OPACITY
    #define HAS_UV1 1
#endif

#ifdef EFFECT_SHADOW
    #define HAS_UV1 1
    #define HAS_RECT 1
#endif

#ifdef EFFECT_BLUR
    #define HAS_UV1 1
#endif

#ifdef EFFECT_DOWNSAMPLE
    #define DOWNSAMPLE 1
    #define HAS_UV0 1
    #define HAS_UV1 1
    #define HAS_UV2 1
    #define HAS_UV3 1
#endif

#ifdef EFFECT_UPSAMPLE
    #define HAS_COLOR 1
    #define HAS_UV0 1
    #define HAS_UV1 1
#endif

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
    v_rect = a_texcoord2;
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