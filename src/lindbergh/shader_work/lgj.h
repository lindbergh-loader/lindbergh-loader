#include "common.h"

ShaderFilesToMod lgjShaderFilesToMod[] = {{"/shader/Cg/inc/es_shd_base.cg", 0, ""},
                                          {"/shader/Cg/inc/es_shd_vsin.cg", 0, ""},
                                          {"/extraShader/Cg/inc/es_shd_base.cg", 0, ""},
                                          {"/extraShader/Cg/inc/es_shd_vsin.cg", 0, ""},
                                          {"/extraShader/Cg/inc/lgj_shd_vsin.cg", 0, ""}};

int lgjFilesToModCount = sizeof(lgjShaderFilesToMod) / sizeof(ShaderFilesToMod);

SearchReplace lgjShaderReplaceSet1[] = {{"S_VERTEX_BLEND_BONE_MAX * 3", "210"}};

int lgjShaderReplaceCount1 = sizeof(lgjShaderReplaceSet1) / sizeof(SearchReplace);

SearchReplace lgjShaderReplaceSet2[] = {
    {"struct VS_INPUT_PT2\n{\n\tfloat4 Position   \t: ATTR0;\t// \xB0\xCC\xC3\xD6\n\tfloat2 TexCoord0 \t: ATTR8",
     "struct VS_INPUT_PT2\n{\n\tfloat4 Position   \t: ATTR0;\t// \xB0\xCC\xC3\xD6\n\tfloat2 TexCoord0 \t: TEXCOORD0"}};

int lgjShaderReplaceCount2 = sizeof(lgjShaderReplaceSet2) / sizeof(SearchReplace);

SearchReplace lgjCompiledShadersPatch[] = {
    {"MOV result.texcoord[1].xy, vertex.attrib[9];", "MOV result.texcoord[1].x, 1.0;MOV result.texcoord[1].y, 1.0;"},
    {"TEX R0.x, R0, texture[0], SHADOW2D;", "TEX R0.x, R0, texture[0], 2D;"}};

int lgjCompiledShadersPatchCount = sizeof(lgjCompiledShadersPatch) / sizeof(SearchReplace);
