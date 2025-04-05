#include "common.h"

int hummerExtremeShaderFileIndex;

ShaderFilesToMod hummerShaderFilesToMod[] = {{"/data-global/shader/gauss2.frag", 0, ""},
                                             {"/data-global/shader/gauss2.vert", 0, ""},
                                             {"/data-global/shader/gauss2_a.frag", 0, ""},
                                             {"/data-global/shader/gauss2_a.vert", 0, ""},
                                             {"/data-global/shader/gauss2_a_to_c.frag", 0, ""},
                                             {"/data-global/shader/gauss2_a_to_c.vert", 0, ""},
                                             {"/data-global/shader/gauss_x.frag", 0, ""},
                                             {"/data-global/shader/gauss_x.vert", 0, ""},
                                             {"/data-global/shader/gauss_y.frag", 0, ""},
                                             {"/data-global/shader/gauss_y.vert", 0, ""},
                                             {"/data-global/shader/hdr.frag", 0, ""},
                                             {"/data-global/shader/hdr.vert", 0, ""},
                                             {"/data-global/nnstdshader/nnstdshader.frag", 0, ""},
                                             {"/data-global/nnstdshader/nnstdshader.vert", 0, ""}};

int hummerFilesToModCount = sizeof(hummerShaderFilesToMod) / sizeof(ShaderFilesToMod);

SearchReplace hummerShaderReplaceSet1[] = {
    {"#version 120", "//#version 120"},
    {"for ( int litno = 0; litno < min( 8, NNGLD_PROF_MAX_PARALLEL_LIGHT + NNGLD_PROF_MAX_POINT_LIGHT + "
     "NNGLD_PROF_MAX_SPOT_LIGHT ); litno++ ){",
     "for ( int litno = 0; float(litno) < min( 8.0, float(NNGLD_PROF_MAX_PARALLEL_LIGHT + NNGLD_PROF_MAX_POINT_LIGHT + "
     "NNGLD_PROF_MAX_SPOT_LIGHT) ); litno++ ){"}};

int hummerShaderReplaceSet1Count = sizeof(hummerShaderReplaceSet1) / sizeof(SearchReplace);

SearchReplace hummerShaderReplaceMesa[] = {
    {"vec3 glare = texture2DLod(glare_texture, uv2, 1.5).rgb;", "vec3 glare = texture2D(glare_texture, uv2, 1.5).rgb;"},
    {"glare += texture2DLod(glare_texture, uv2, 3.5).rgb;", "glare += texture2D(glare_texture, uv2, 3.5).rgb;"},
    {"glare *= 0.5f;", "glare *= 0.5;"},
    {"gl_FragColor.rgb = (color - 1.0) * 5.0f;", "gl_FragColor.rgb = (color - 1.0) * 5.0;"}};

int hummerShaderReplaceMesaCount = sizeof(hummerShaderReplaceMesa) / sizeof(SearchReplace);

SearchReplace hummerShaderReplaceFlicker[] = {
    {"#pragma optionNV(fastmath on)", "#pragma optionNV(fastmath off)"},
    {"#pragma optionNV(fastprecision on)", "#pragma optionNV(fastprecision off)"}};

int hummerShaderReplaceFlickerCount = sizeof(hummerShaderReplaceFlicker) / sizeof(SearchReplace);