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

$input v_color0 UV0_OUTPUT UV1_OUTPUT DOWNSAMPLE_OUTPUT SDF_OUTPUT RECT_OUTPUT TILE_OUTPUT COVERAGE_OUTPUT IMAGE_OUTPUT

#include "../../common.sh"

uniform vec4 cbuffer0_ps[2];
uniform vec4 cbuffer1_ps[32];

SAMPLER2D(pattern, 0);
SAMPLER2D(ramps, 1);
SAMPLER2D(image, 2);
SAMPLER2D(glyphs, 3);
SAMPLER2D(shadow, 4);

#ifdef WAVES_BRUSH
float wave(vec2 uv, vec2 s12, vec2 t12, vec2 f12, vec2 h12)
{
	float time = cbuffer1_ps[0][0];

	vec2 x12 = sin((time * s12 + t12 + uv.x) * f12) * h12;
	float v = 0.3 - 0.2 * ((sin(time * 0.5) + 1.0f) / 2.0f);
	float d = uv.y - (v + x12.x + x12.y);

	float o = smoothstep(2.0f * dFdy(uv.y), 0.0, d);
	float i0 = 1.0f - smoothstep(0.0, 0.15, d);
	float i1 = pow((1.0 - smoothstep(0.0, 0.050, d)), 8.0);
	return o * (0.15 * i0 + 0.10 * i1);
}

vec4 GetCustomPattern()
{
	float time = cbuffer1_ps[0][0];
	vec2 uv = vec2(uv0.x, uv0.y);

	float s0 = 1.0 + 10.0 * ((sin((time + 0.3) / 2.0) + 3.0) / 6.0);
	float wave0 = wave(vec2(uv.x, uv.y - 0.25), vec2(0.735,0.255), vec2(-0.09,0.27), vec2(1.4,5.51), s0 * vec2(0.006,0.005));

	float s1 = 1.0 + 20.0 * ((sin(time / 1.5) + 3.0) / 6.0);
	float wave1 = wave(vec2(uv.x, uv.y - 0.25), vec2(0.232,0.29), vec2(0.08,-0.22), vec2(1.6,3.89), s1 * vec2(0.006,0.005));

	vec3 back = mix(vec3(0.01, 0.02, 0.3), vec3(0.03, 0.69, 0.87), (uv.x + uv.y) * 0.65);
	return vec4(back + wave0 + wave1, 1.0);
}
#endif
#ifdef PLASMA_BRUSH
vec4 GetCustomPattern()
{
	float time = cbuffer1_ps[0][0];
	vec2 size = vec2(cbuffer1_ps[0][1], cbuffer1_ps[0][2]);
	vec3 scale = vec3(cbuffer1_ps[1][0], cbuffer1_ps[1][1], cbuffer1_ps[1][2]);
	vec3 bias = vec3(cbuffer1_ps[2][0], cbuffer1_ps[2][1], cbuffer1_ps[2][2]);
	vec3 freq = vec3(cbuffer1_ps[3][0], cbuffer1_ps[3][1], cbuffer1_ps[3][2]);
	vec3 phase = vec3(cbuffer1_ps[4][0], cbuffer1_ps[4][1], cbuffer1_ps[4][2]);

	const float PI = 3.1415926;

	// https://www.bidouille.org/prog/plasma
	vec2 uv = uv0 * size;
	float v = sin(uv.x + time);
	v += sin((uv.y + time) / 2.0);
	v += sin((uv.x + uv.y + time) / 2.0);
	uv += size/2.0 * vec2(sin(time/3.0), cos(time/2.0));
	v += sin(sqrt(uv.x * uv.x + uv.y * uv.y + 1.0) + time);
	v = v / 2.0;

	vec3 col = scale + bias * cos(PI * (freq * v + phase));
	return vec4(col, 1);
}
#endif
#ifdef MONOCHROME_BRUSH
vec4 GetCustomPattern()
{
	vec4 color = vec4(cbuffer1_ps[0][0], cbuffer1_ps[0][1], cbuffer1_ps[0][2], cbuffer1_ps[0][3]);
	vec4 c = texture2D(pattern, uv0);
	float l = c.r * 0.30 + c.g * 0.59 + c.b * 0.11;
	return vec4(color.r * l, color.g * l, color.b * l, color.a);
}
#endif
#ifdef CONIC_GRADIENT_BRUSH
float T(int n)
{
	return cbuffer1_ps[1+2*n][0];
}

vec4 Color(int n)
{
	return vec4(cbuffer1_ps[2 + 2 * n][0], cbuffer1_ps[2 + 2 * n][1], cbuffer1_ps[2 + 2 * n][2], cbuffer1_ps[2 + 2 * n][3]);
}

vec4 GetCustomPattern()
{
	const float PI = 3.1415926;
	vec2 v = uv0 - vec2(0.5, 0.5);
	float x = 1.0 - (atan(v.x, v.y) + PI) / (2.0 * PI);

	int n = int(cbuffer1_ps[0][0]);

	for (int i = 0; i < n - 1; i++)
	{
		float t0 = T(i);
		float t1 = T(i + 1);

		vec4 color0 = Color(i);
		vec4 color1 = Color(i + 1);

		if (x <= t1)
		{
			return mix(color0, color1, (x - t0) / (t1 - t0));
		}
	}

	float t = T(n - 1);
	vec4 color0 = Color(n - 1);
	vec4 color1 = Color(0);

	return mix(color0, color1, (x - t) / (1.0 - t));
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
    /////////////////////////////////////////////////////
    // Fetch paint v_color0 and opacity
    /////////////////////////////////////////////////////
    #if defined(PAINT_SOLID)
        vec4 paint = v_color0;
        float opacity_ = 1.0;

    #elif defined(PAINT_LINEAR)
        vec4 paint = texture2D(ramps, v_texcoord0);
        float opacity_ = cbuffer0_ps[0][0];

    #elif defined(PAINT_RADIAL)
        float dd = cbuffer0_ps[1][0] * v_texcoord0.x - cbuffer0_ps[1][1] * v_texcoord0.y;
        float u = cbuffer0_ps[0][0] * v_texcoord0.x + cbuffer0_ps[0][1] * v_texcoord0.y + cbuffer0_ps[0][2] * 
            sqrt(v_texcoord0.x * v_texcoord0.x + v_texcoord0.y * v_texcoord0.y - dd * dd);
        vec4 paint = texture2D(ramps, vec2(u, cbuffer0_ps[1][2]));
        float opacity_ = cbuffer0_ps[0][3];

    #elif defined(PAINT_PATTERN)
        #if defined(CUSTOM_PATTERN)
            vec4 paint = GetCustomPattern();
        #elif defined(CLAMP_PATTERN)
            float inside = v_texcoord0 == clamp(v_texcoord0, v_rect.xy, v_rect.zw) ? 1.0 : 0.0;
            vec4 paint = inside * texture2D(pattern, v_texcoord0);
        #elif defined(REPEAT_PATTERN) || defined(MIRRORU_PATTERN) || defined(MIRRORV_PATTERN) || defined(MIRROR_PATTERN)
            vec2 uv = (v_texcoord0 - v_tile.xy) / v_tile.zw;
            #if defined(REPEAT_PATTERN)
                uv = fract(uv);
            #elif defined(MIRRORU_PATTERN)
                uv.x = abs(uv.x - 2.0 * floor((uv.x - 1.0) / 2.0) - 2.0);
                uv.y = fract(uv.y);
            #elif defined(MIRRORV_PATTERN)
                uv.x = fract(uv.x);
                uv.y = abs(uv.y - 2.0 * floor((uv.y - 1.0) / 2.0) - 2.0);
            #else 
                uv = abs(uv - 2.0 * floor((uv - 1.0) / 2.0) - 2.0);
            #endif
            uv = uv * v_tile.zw + v_tile.xy;
            float inside = v_texcoord0 == clamp(v_texcoord0, v_rect.xy, v_rect.zw) ? 1.0 : 0.0;
            vec4 paint = inside * texture2DGrad(pattern, uv, dFdx(v_texcoord0), dFdy(v_texcoord0));
        #else
            vec4 paint = texture2D(pattern, v_texcoord0);
        #endif
        float opacity_ = cbuffer0_ps[0][0];
    #endif

    /////////////////////////////////////////////////////
    // Apply selected effect
    /////////////////////////////////////////////////////
    #if defined(EFFECT_RGBA)
        gl_FragColor = vec4(cbuffer0_ps[0][0], cbuffer0_ps[0][1], cbuffer0_ps[0][2], cbuffer0_ps[0][3]);

    #elif defined(EFFECT_MASK)
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);

    #elif defined(EFFECT_CLEAR)
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);

    #elif defined(EFFECT_PATH)
        gl_FragColor = opacity_ * paint;

    #elif defined(EFFECT_PATH_AA)
        gl_FragColor = (opacity_ * v_coverage) * paint;

    #elif defined(EFFECT_OPACITY)
        gl_FragColor = texture2D(image, v_texcoord1) * (opacity_ * paint.a);

    #elif defined(EFFECT_SHADOW)
        vec4 shadowColor = vec4(cbuffer1_ps[0][0], cbuffer1_ps[0][1], cbuffer1_ps[0][2], cbuffer1_ps[0][3]);
        vec2 offset = vec2(cbuffer1_ps[1][0], -cbuffer1_ps[1][1]);
        vec2 uv = clamp(v_texcoord1 - offset, v_rect.xy, v_rect.zw);
        float alpha = mix(texture2D(image, uv).a, texture2D(shadow, uv).a, cbuffer1_ps[1][2]);
        vec4 img = texture2D(image, clamp(v_texcoord1, v_rect.xy, v_rect.zw));
        gl_FragColor = (img + (1.0 - img.a) * (shadowColor * alpha)) * (opacity_ * paint.a);

    #elif defined(EFFECT_BLUR)
        gl_FragColor = mix(texture2D(image, v_texcoord1), texture2D(shadow, v_texcoord1), cbuffer1_ps[0][0]) * (opacity_ * paint.a);

    #elif defined(EFFECT_SDF)
        float distance = SDF_SCALE * (texture2D(glyphs, v_texcoord1).r - SDF_BIAS);
        vec2 grad = dFdx(v_st1);

        float gradLen = length(grad);
        float scale = 1.0 / gradLen;
        float base = SDF_BASE_DEV * (1.0 - (clamp(scale, SDF_BASE_MIN, SDF_BASE_MAX) - SDF_BASE_MIN) / (SDF_BASE_MAX - SDF_BASE_MIN));
        float range = SDF_AA_FACTOR * gradLen;
        float alpha = smoothstep(base - range, base + range, distance);
        gl_FragColor = (alpha * opacity_) * paint;

    #elif defined(EFFECT_DOWNSAMPLE)
        gl_FragColor = (texture2D(pattern, v_texcoord0) + texture2D(pattern, v_texcoord1) + texture2D(pattern, v_texcoord2) + texture2D(pattern, v_texcoord3)) * 0.25;

    #elif defined(EFFECT_UPSAMPLE)
        gl_FragColor = mix(texture2D(image, v_texcoord1), texture2D(pattern, v_texcoord0), v_color0.a);

    #else
        #error EFFECT not defined
    #endif
}