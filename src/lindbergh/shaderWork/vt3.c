#include "common.h"

ShaderFilesToMod vt3ShaderFilesToMod[] = {{"/shader/baka.fx", 0, ""},        {"/shader/main_p.fx", 0, ""},
                                          {"/shader/main_v.fx", 0, ""},      {"/shader/no_transform_v.fx", 0, ""},
                                          {"/shader/synthesis_v.fx", 0, ""}, {"/shader/Cg/inc/es_shd_vsin.cg", 0, ""}};

int vt3FilesToModCount = sizeof(vt3ShaderFilesToMod) / sizeof(ShaderFilesToMod);

SearchReplace vt3ShaderReplaceSet1[] = {
    {"half face : FACE;", "//half face : FACE;"},
    {"n = (i.face > 0) ? -n : n;", "//n = (i.face > 0) ? -n : n;"},
    {"#if PS_3_0_READY && NOISE", "#define PS_3_0_READY (0)\n#if PS_3_0_READY && NOISE"}};

int vt3ShaderReplaceCount1 = sizeof(vt3ShaderReplaceSet1) / sizeof(SearchReplace);

SearchReplace vt3ShaderReplaceSet2[] = {{"POSITION0", "ATTR0"},   {"COLOR", "ATTR3"},       {"NORMAL0", "ATTR2"},
                                        {"TEXCOORD0", "ATTR8"},   {"TEXCOORD1", "ATTR9"},   {"TANGENT", "ATTR14"},
                                        {"BLENDWEIGHT", "ATTR1"}, {"BLENDINDICES", "ATTR7"}};

int vt3ShaderReplaceCount2 = sizeof(vt3ShaderReplaceSet2) / sizeof(SearchReplace);
