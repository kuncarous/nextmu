#ifdef HAS_COLOR
#define COLOR_INPUT , a_color0
#define COLOR_OUTPUT , v_color0
#else
#define COLOR_INPUT
#define COLOR_OUTPUT
#endif

#ifdef HAS_UV0
#define UV0_INPUT , a_texcoord0
#define UV0_OUTPUT , v_texcoord0
#else
#define UV0_INPUT
#define UV0_OUTPUT
#endif

#ifdef HAS_UV1
#define UV1_INPUT , a_texcoord1
#define UV1_OUTPUT , v_texcoord1
#else
#define UV1_INPUT
#define UV1_OUTPUT
#endif

#ifdef DOWNSAMPLE
#define DOWNSAMPLE_OUTPUT , v_texcoord2, v_texcoord3
#else
#define DOWNSAMPLE_OUTPUT
#endif

#ifdef SDF
#define SDF_OUTPUT , v_st1
#else
#define SDF_OUTPUT
#endif

#ifdef HAS_RECT
#define RECT_INPUT , a_texcoord2
#define RECT_OUTPUT , v_rect
#else
#define RECT_INPUT
#define RECT_OUTPUT
#endif

#ifdef HAS_TILE
#define TILE_INPUT , a_texcoord3
#define TILE_OUTPUT , v_tile
#else
#define TILE_INPUT
#define TILE_OUTPUT
#endif

#ifdef HAS_COVERAGE
#define COVERAGE_INPUT , a_texcoord4
#define COVERAGE_OUTPUT , v_coverage
#else
#define COVERAGE_INPUT
#define COVERAGE_OUTPUT
#endif

#ifdef HAS_IMAGE_POSITION
#define IMAGE_INPUT , a_texcoord5
#define IMAGE_OUTPUT , v_imagePos
#else
#define IMAGE_INPUT
#define IMAGE_OUTPUT
#endif