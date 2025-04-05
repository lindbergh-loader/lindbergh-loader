#include "common.h"

ShaderFilesToMod idShaderFilesToMod[] = {{"/shader/Cg/inc/es_shd_base.cg", 0, ""},
                                         {"/shader/Cg/inc/es_shd_vsin.cg", 0, ""},
                                         {"/data/Shader/blur_v.fx", 0, ""},
                                         {"/data/Shader/course_v.fx", 0, ""},
                                         {"/data/Shader/edge_v.fx", 0, ""},
                                         {"/data/Shader/glareGenerator_v.fx", 0, ""},
                                         {"/data/Shader/occlusionCuller_v.fx", 0, ""},
                                         {"/data/Shader/stretchFrameBuffer_v.fx", 0, ""},
                                         {"/data/Shader/car_v.fx", 0, ""},
                                         {"/data/Shader/depthConverage_v.fx", 0, ""},
                                         {"/data/Shader/effect_v.fx", 0, ""},
                                         {"/data/Shader/glareSampler_v.fx", 0, ""},
                                         {"/data/Shader/shadowMap_v.fx", 0, ""},
                                         {"/data/Shader/sun_v.fx", 0, ""},
                                         {"/data/Shader/combiner_v.fx", 0, ""},
                                         {"/data/Shader/dof_v.fx", 0, ""},
                                         {"/data/Shader/glareFinalizer_v.fx", 0, ""},
                                         {"/data/Shader/morphImage_v.fx", 0, ""},
                                         {"/data/Shader/shigeno_v.fx", 0, ""},
                                         {"/data/Shader/effect_p.fx", 0, ""}};

int idFilesToModCount = sizeof(idShaderFilesToMod) / sizeof(ShaderFilesToMod);

ShaderFilesToMod id4jShaderFilesToMod[] = {{"/shader/Cg/inc/es_shd_base.cg", 0, ""},
                                           {"/shader/Cg/inc/es_shd_vsin.cg", 0, ""},
                                           {"/data/Shader/blur_v.fx", 0, ""},
                                           {"/data/Shader/course_v.fx", 0, ""},
                                           {"/data/Shader/edge_v.fx", 0, ""},
                                           {"/data/Shader/glareGenerator_v.fx", 0, ""},
                                           {"/data/Shader/occlusionCuller_v.fx", 0, ""},
                                           {"/data/Shader/car_v.fx", 0, ""},
                                           {"/data/Shader/effect_v.fx", 0, ""},
                                           {"/data/Shader/glareSampler_v.fx", 0, ""},
                                           {"/data/Shader/shadowMap_v.fx", 0, ""},
                                           {"/data/Shader/sun_v.fx", 0, ""},
                                           {"/data/Shader/combiner_v.fx", 0, ""},
                                           {"/data/Shader/dof_v.fx", 0, ""},
                                           {"/data/Shader/glareFinalizer_v.fx", 0, ""},
                                           {"/data/Shader/morphImage_v.fx", 0, ""},
                                           {"/data/Shader/shigeno_v.fx", 0, ""}};

int id4FilesToModCount = sizeof(id4jShaderFilesToMod) / sizeof(ShaderFilesToMod);

SearchReplace idShaderReplaceSet[] = {
    {"const float4x4  mtxWorldViewProj;", "const float4x4  mtxWorldViewProj : C0;"},
    {"const uniform float4x4  mtxWorldViewProj;", "const uniform float4x4  mtxWorldViewProj : C0;"},
    {"float4x4    mtxWorldViewProj;", "float4x4    mtxWorldViewProj : C0;"},
    {"const uniform float4x4  g_mtxWorldViewProj;", "const uniform float4x4  g_mtxWorldViewProj : C0;"},
    {"uniform\tfloat4x4\tmtxWorldViewProj;", "uniform\tfloat4x4\tmtxWorldViewProj : C0;"},
    {"const uniform float2 f2UV_Offset", "const uniform float2 f2UV_Offset : C0"},
    {"const float4x4 g_m4WorldViewProj;", "const float4x4 g_m4WorldViewProj : C0;"}};

int idShaderReplaceCount = sizeof(idShaderReplaceSet) / sizeof(SearchReplace);

SearchReplace id5ShaderReplaceSetForSmoke[] = {
    {"h4OutputColor.a = h4Color_ColorMap.a;",
     "#if CV_USE_NORMAL_XY_AS_UV && !CV_ENABLE_MATERIAL_COLOR\nh4OutputColor.rgb = half3(0.5h, 0.5h, "
     "0.5h);\nh4OutputColor.a = 0.2h;\n#else\nh4OutputColor.a = h4Color_ColorMap.a;\n#endif"}};

int id5ShaderReplaceSetForSmokeCount = sizeof(id5ShaderReplaceSetForSmoke) / sizeof(SearchReplace);
