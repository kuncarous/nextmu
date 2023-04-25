@echo off

echo RGBA
call "../../compiler" input:base.vs type:vertex outdir:..\rgba output:shader.vs defines:"DUMMY_DEFINE"
call "../../compiler" input:base.fs type:fragment outdir:..\rgba output:shader.fs defines:"EFFECT_RGBA"

call "../../compiler" input:base.vs type:vertex outdir:..\mask output:shader.vs defines:"DUMMY_DEFINE"
call "../../compiler" input:base.fs type:fragment outdir:..\mask output:shader.fs defines:"EFFECT_MASK"

call "../../compiler" input:base.vs type:vertex outdir:..\clear output:shader.vs defines:"DUMMY_DEFINE"
call "../../compiler" input:base.fs type:fragment outdir:..\clear output:shader.fs defines:"EFFECT_CLEAR"

call "../../compiler" input:base.vs type:vertex outdir:..\path_solid output:shader.vs defines:"HAS_COLOR"
call "../../compiler" input:base.fs type:fragment outdir:..\path_solid output:shader.fs defines:"EFFECT_PATH;PAINT_SOLID"

call "../../compiler" input:base.vs type:vertex outdir:..\path_linear output:shader.vs defines:"HAS_UV0"
call "../../compiler" input:base.fs type:fragment outdir:..\path_linear output:shader.fs defines:"EFFECT_PATH;PAINT_LINEAR"

call "../../compiler" input:base.vs type:vertex outdir:..\path_radial output:shader.vs defines:"HAS_UV0"
call "../../compiler" input:base.fs type:fragment outdir:..\path_radial output:shader.fs defines:"EFFECT_PATH;PAINT_RADIAL"

call "../../compiler" input:base.vs type:vertex outdir:..\path_pattern output:shader.vs defines:"HAS_UV0"
call "../../compiler" input:base.fs type:fragment outdir:..\path_pattern output:shader.fs defines:"EFFECT_PATH;PAINT_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_pattern_clamp output:shader.vs defines:"HAS_UV0;HAS_RECT"
call "../../compiler" input:base.fs type:fragment outdir:..\path_pattern_clamp output:shader.fs defines:"EFFECT_PATH;PAINT_PATTERN;CLAMP_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_pattern_repeat output:shader.vs defines:"HAS_UV0;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_pattern_repeat output:shader.fs defines:"EFFECT_PATH;PAINT_PATTERN;REPEAT_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_pattern_mirroru output:shader.vs defines:"HAS_UV0;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_pattern_mirroru output:shader.fs defines:"EFFECT_PATH;PAINT_PATTERN;MIRRORU_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_pattern_mirrorv output:shader.vs defines:"HAS_UV0;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_pattern_mirrorv output:shader.fs defines:"EFFECT_PATH;PAINT_PATTERN;MIRRORV_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_pattern_mirror output:shader.vs defines:"HAS_UV0;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_pattern_mirror output:shader.fs defines:"EFFECT_PATH;PAINT_PATTERN;MIRROR_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_aa_solid output:shader.vs defines:"HAS_COLOR;HAS_COVERAGE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_aa_solid output:shader.fs defines:"EFFECT_PATH_AA;PAINT_SOLID"

call "../../compiler" input:base.vs type:vertex outdir:..\path_aa_linear output:shader.vs defines:"HAS_UV0;HAS_COVERAGE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_aa_linear output:shader.fs defines:"EFFECT_PATH_AA;PAINT_LINEAR"

call "../../compiler" input:base.vs type:vertex outdir:..\path_aa_radial output:shader.vs defines:"HAS_UV0;HAS_COVERAGE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_aa_radial output:shader.fs defines:"EFFECT_PATH_AA;PAINT_RADIAL"

call "../../compiler" input:base.vs type:vertex outdir:..\path_aa_pattern output:shader.vs defines:"HAS_UV0;HAS_COVERAGE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_aa_pattern output:shader.fs defines:"EFFECT_PATH_AA;PAINT_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_aa_pattern_clamp output:shader.vs defines:"HAS_UV0;HAS_COVERAGE;HAS_RECT"
call "../../compiler" input:base.fs type:fragment outdir:..\path_aa_pattern_clamp output:shader.fs defines:"EFFECT_PATH_AA;PAINT_PATTERN;CLAMP_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_aa_pattern_repeat output:shader.vs defines:"HAS_UV0;HAS_COVERAGE;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_aa_pattern_repeat output:shader.fs defines:"EFFECT_PATH_AA;PAINT_PATTERN;REPEAT_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_aa_pattern_mirroru output:shader.vs defines:"HAS_UV0;HAS_COVERAGE;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_aa_pattern_mirroru output:shader.fs defines:"EFFECT_PATH_AA;PAINT_PATTERN;MIRRORU_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_aa_pattern_mirrorv output:shader.vs defines:"HAS_UV0;HAS_COVERAGE;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_aa_pattern_mirrorv output:shader.fs defines:"EFFECT_PATH_AA;PAINT_PATTERN;MIRRORV_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\path_aa_pattern_mirror output:shader.vs defines:"HAS_UV0;HAS_COVERAGE;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\path_aa_pattern_mirror output:shader.fs defines:"EFFECT_PATH_AA;PAINT_PATTERN;MIRROR_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_solid output:shader.vs defines:"HAS_COLOR;HAS_UV1;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_solid output:shader.fs defines:"EFFECT_SDF;PAINT_SOLID"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_linear output:shader.vs defines:"HAS_UV0;HAS_UV1;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_linear output:shader.fs defines:"EFFECT_SDF;PAINT_LINEAR"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_radial output:shader.vs defines:"HAS_UV0;HAS_UV1;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_radial output:shader.fs defines:"EFFECT_SDF;PAINT_RADIAL"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_pattern output:shader.vs defines:"HAS_UV0;HAS_UV1;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_pattern output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_pattern_clamp output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_pattern_clamp output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN;CLAMP_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_pattern_repeat output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_pattern_repeat output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN;REPEAT_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_pattern_mirroru output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_pattern_mirroru output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN;MIRRORU_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_pattern_mirrorv output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_pattern_mirrorv output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN;MIRRORV_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_pattern_mirror output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_pattern_mirror output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN;MIRROR_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_lcd_solid output:shader.vs defines:"HAS_COLOR;HAS_UV1;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_lcd_solid output:shader.fs defines:"EFFECT_SDF;PAINT_SOLID"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_lcd_linear output:shader.vs defines:"HAS_UV0;HAS_UV1;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_lcd_linear output:shader.fs defines:"EFFECT_SDF;PAINT_LINEAR"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_lcd_radial output:shader.vs defines:"HAS_UV0;HAS_UV1;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_lcd_radial output:shader.fs defines:"EFFECT_SDF;PAINT_RADIAL"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_lcd_pattern output:shader.vs defines:"HAS_UV0;HAS_UV1;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_lcd_pattern output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_lcd_pattern_clamp output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_lcd_pattern_clamp output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN;CLAMP_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_lcd_pattern_repeat output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_lcd_pattern_repeat output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN;REPEAT_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_lcd_pattern_mirroru output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_lcd_pattern_mirroru output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN;MIRRORU_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_lcd_pattern_mirrorv output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_lcd_pattern_mirrorv output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN;MIRRORV_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\sdf_lcd_pattern_mirror output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE;SDF"
call "../../compiler" input:base.fs type:fragment outdir:..\sdf_lcd_pattern_mirror output:shader.fs defines:"EFFECT_SDF;PAINT_PATTERN;MIRROR_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\opacity_solid output:shader.vs defines:"HAS_COLOR;HAS_UV1"
call "../../compiler" input:base.fs type:fragment outdir:..\opacity_solid output:shader.fs defines:"EFFECT_OPACITY;PAINT_SOLID"

call "../../compiler" input:base.vs type:vertex outdir:..\opacity_linear output:shader.vs defines:"HAS_UV0;HAS_UV1"
call "../../compiler" input:base.fs type:fragment outdir:..\opacity_linear output:shader.fs defines:"EFFECT_OPACITY;PAINT_LINEAR"

call "../../compiler" input:base.vs type:vertex outdir:..\opacity_radial output:shader.vs defines:"HAS_UV0;HAS_UV1"
call "../../compiler" input:base.fs type:fragment outdir:..\opacity_radial output:shader.fs defines:"EFFECT_OPACITY;PAINT_RADIAL"

call "../../compiler" input:base.vs type:vertex outdir:..\opacity_pattern output:shader.vs defines:"HAS_UV0;HAS_UV1"
call "../../compiler" input:base.fs type:fragment outdir:..\opacity_pattern output:shader.fs defines:"EFFECT_OPACITY;PAINT_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\opacity_pattern_clamp output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT"
call "../../compiler" input:base.fs type:fragment outdir:..\opacity_pattern_clamp output:shader.fs defines:"EFFECT_OPACITY;PAINT_PATTERN;CLAMP_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\opacity_pattern_repeat output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\opacity_pattern_repeat output:shader.fs defines:"EFFECT_OPACITY;PAINT_PATTERN;REPEAT_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\opacity_pattern_mirroru output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\opacity_pattern_mirroru output:shader.fs defines:"EFFECT_OPACITY;PAINT_PATTERN;MIRRORU_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\opacity_pattern_mirrorv output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\opacity_pattern_mirrorv output:shader.fs defines:"EFFECT_OPACITY;PAINT_PATTERN;MIRRORV_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\opacity_pattern_mirror output:shader.vs defines:"HAS_UV0;HAS_UV1;HAS_RECT;HAS_TILE"
call "../../compiler" input:base.fs type:fragment outdir:..\opacity_pattern_mirror output:shader.fs defines:"EFFECT_OPACITY;PAINT_PATTERN;MIRROR_PATTERN"

call "../../compiler" input:base.vs type:vertex outdir:..\upsample output:shader.vs defines:"HAS_COLOR;HAS_UV0;HAS_UV1"
call "../../compiler" input:base.fs type:fragment outdir:..\upsample output:shader.fs defines:"EFFECT_UPSAMPLE"

call "../../compiler" input:base.vs type:vertex outdir:..\downsample output:shader.vs defines:"HAS_UV0;HAS_UV1;DOWNSAMPLE"
call "../../compiler" input:base.fs type:fragment outdir:..\downsample output:shader.fs defines:"EFFECT_DOWNSAMPLE"

call "../../compiler" input:base.vs type:vertex outdir:..\shadow output:shader.vs defines:"HAS_COLOR;HAS_UV1;HAS_RECT"
call "../../compiler" input:base.fs type:fragment outdir:..\shadow output:shader.fs defines:"EFFECT_SHADOW;PAINT_SOLID"

call "../../compiler" input:base.vs type:vertex outdir:..\blur output:shader.vs defines:"HAS_COLOR;HAS_UV1"
call "../../compiler" input:base.fs type:fragment outdir:..\blur output:shader.fs defines:"EFFECT_BLUR;PAINT_SOLID"