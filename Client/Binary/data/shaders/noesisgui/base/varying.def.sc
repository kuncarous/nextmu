flat vec4 v_color0   : COLOR0    = vec4(1.0, 0.0, 0.0, 1.0);
vec2 v_texcoord0     : TEXCOORD0 = vec2(0.0, 0.0);
vec2 v_texcoord1     : TEXCOORD1 = vec2(0.0, 0.0);
vec2 v_texcoord2     : TEXCOORD2 = vec2(0.0, 0.0);
vec2 v_texcoord3     : TEXCOORD3 = vec2(0.0, 0.0);
vec2 v_st1           : TEXCOORD4 = vec2(0.0, 0.0);
flat vec4 v_rect     : TEXCOORD5 = vec4(0.0, 0.0, 0.0, 0.0);
flat vec4 v_tile     : TEXCOORD6 = vec4(0.0, 0.0, 0.0, 0.0);
float v_coverage     : COLOR1 = 0.0;
vec4 v_imagePos      : COLOR2 = vec4(0.0, 0.0, 0.0, 0.0);

vec2 a_position      : POSITION;
vec4 a_color0        : COLOR0;
vec2 a_texcoord0     : TEXCOORD0;
vec2 a_texcoord1     : TEXCOORD1;
vec4 a_texcoord2     : TEXCOORD2;
vec4 a_texcoord3     : TEXCOORD3;
float a_texcoord4    : TEXCOORD4;
vec4 a_texcoord5     : TEXCOORD5;