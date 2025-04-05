#include "common.h"

// Fragment
SearchReplace rtunedFsMesa[] = {
    {"OPTION NV_fragment_program2;", "#OPTION NV_fragment_program2;"},
    {"SHORT TEMP", "TEMP"},
    {"SHORT OUTPUT", "OUTPUT"},
    {"LONG OUTPUT", "OUTPUT"},
    {"ATTRIB a_facing = fragment.facing;", "#ATTRIB a_facing = fragment.facing;"},
    {"TXL ", "TEX "},
    //{"MUL o_color, col, state.material.emission;", "MUL o_color, col, 0;"}, // boost effect disabled
    {"NRMH normal_w, a_normal_t;",
     "DP3 _tmp0.x, a_normal_t, a_normal_t;RSQ _tmp1.x, _tmp0.x;MUL normal_w, a_normal_t, _tmp1.x;MOV normal_w.w, 1.0;"},
    {"NRMH camera_w, a_camera_w;",
     "DP3 _tmp0.x, a_camera_w, a_camera_w;RSQ _tmp1.x, _tmp0.x;MUL camera_w, a_camera_w, _tmp1.x;MOV camera_w.w, 1.0;"},
    {"NRMH normal_w, normal_w;",
     "DP3 _tmp0.x, normal_w, normal_w;RSQ _tmp1.x, _tmp0.x;MUL normal_w, normal_w, _tmp1.x;MOV normal_w.w, 1.0;"},
    {"NRMH normal, a_normal;",
     "DP3 _tmp0.x, a_normal, a_normal;RSQ _tmp1.x, _tmp0.x;MUL normal, a_normal, _tmp1.x;MOV normal.w, 1.0;"},
    {"NRMH normal, normal;",
     "DP3 _tmp0.x, normal, normal;RSQ _tmp1.x, _tmp0.x;MUL normal, normal, _tmp1.x;MOV normal.w, 1.0;"},
    {"NRMH eye, a_eye;", "DP3 _tmp0.x, a_eye, a_eye;RSQ _tmp1.x, _tmp0.x;MUL eye, a_eye, _tmp1.x;MOV eye.w, 1.0;"},
    {"POW tmp.w, tmp.w, 5.0;",
     "MUL _tmp0.x, tmp.w, tmp.w;MUL _tmp1.x, _tmp0.x, tmp.w;MUL tmp.w, _tmp1.x, tmp.w;MUL tmp.w, tmp.w, tmp.w;"},
    {"POW tmp.x, tmp.x, 8.0;", "MUL _tmp0.x, tmp.x, tmp.x;MUL _tmp1.x, _tmp0.x, _tmp0.x;MUL tmp.x, _tmp1.x, _tmp1.x;"},
    {"SUBC ", "SUB "},
    {"MOVC ", "MOV "},
    {"(LT.w)", " "},
    {"(GT.w)", " "},
    {"RET;", " "},
    {"DIV col.x, sum.x, col.x;", "RCP _tmp0.x, col.x;MUL col.x, sum.x, _tmp0.x;"},
    {"DIV col.y, sum.y, col.y;", "RCP _tmp0.y, col.y;MUL col.y, sum.y, _tmp0.y;"},
    {"DIV col.z, sum.z, col.z;", "RCP _tmp0.z, col.z;MUL col.z, sum.z, _tmp0.z;"},
    {"DP2 tmp.w, tmp, tmp;", "MUL _tmp0.x, tmp.x, tmp.x;MUL _tmp0.y, tmp.y, tmp.y;ADD tmp.w, _tmp0.x, _tmp0.y;"},
    {"DDX ds.xy, a_tex_color0;", "MOV ds.xy, a_tex_color0;"},
    {"DDY du.xy, a_tex_color0;", "MOV du.xy, a_tex_color0;"},
    {"DP2 ds.w, ds, ds;", "MUL _tmp0.x, ds.x, ds.x;MUL _tmp0.y, ds.y, ds.y;ADD ds.w, _tmp0.x, _tmp0.y;"},
    {"DP2 du.w, du, du;", "MUL _tmp1.x, du.x, du.x;MUL _tmp1.y, du.y, du.y;ADD du.w, _tmp1.x, _tmp1.y;"}};

int rtunedFsMesaCount = sizeof(rtunedFsMesa) / sizeof(SearchReplace);

// Vertex
SearchReplace rtunedVsMesa[] = {
    {"OPTION NV_vertex_program3;", "#OPTION NV_vertex_program3;"},
    {"ATTRIB ", "#ATTRIB "},
    {"OUTPUT ", "#OUTPUT "},
    {"a_position", "vertex.position"},
    {"o_position", "result.position"},
    {"o_tex0", "result.texcoord[0]"},
    {"o_tex1", "result.texcoord[1]"},
    {"o_tex2", "result.texcoord[2]"},
    {"o_tex3", "result.texcoord[3]"},
    {"o_tex4", "result.texcoord[4]"},
    {"o_tex5", "result.texcoord[5]"},
    {"o_tex6", "result.texcoord[6]"},
    {"o_tex7", "result.texcoord[7]"},
    {"a_tex0", "vertex.texcoord[0]"},
    {"a_tex1", "vertex.texcoord[1]"},
    {"a_tex2", "vertex.texcoord[2]"},
    {"a_tex3", "vertex.texcoord[3]"},
    {"_a_mtxidx", "vertex.attrib[15]"},
    {"_a_weight", "vertex.attrib[1]"},
    {"o_color_f0", "result.color"},
    {"o_color_f1", "result.color.secondary"},
    {"o_fog", "result.fogcoord"},
    {"vertex_position_m", "vertex.position"},
    {"vertex_normal_m", "vertex.normal"},
    {"vertex_tangent_m", "vertex.attrib[6]"},
    {"vertex_texcoord0", "vertex.texcoord[0]"},
    {"vertex_texcoord1", "vertex.texcoord[1]"},
    {"out_position_s", "result.position"},
    {"out_fresnel", "result.color.secondary"},
    {"out_texcoord0", "result.texcoord[0]"},
    {"out_texcoord1", "result.texcoord[1]"},
    {"out_normal_w", "result.texcoord[5]"},
    {"out_tangent_w", "result.texcoord[3]"},
    {"out_binormal_w", "result.texcoord[4]"},
    {"out_camera_w", "result.texcoord[6]"},
    {"a_color", "vertex.color"},
    {"a_normal", "vertex.normal"},
    {"a_tangent", "vertex.attrib[6]"},
    {"o_color_b0", "result.color.back"},
    {"o_color_b1", "result.color.back.secondary"},
    {"out_normal", "result.texcoord[5]"},
    {"out_tangent", "result.texcoord[3]"},
    {"out_binormal", "result.texcoord[4]"},
    {"o_eye", "result.texcoord[6]"},
    {"o_lit_hit", "result.texcoord[2]"},
    {"o_tex_shadow0", "result.texcoord[7]"},
    {"a_morph_position", "vertex.attrib[10]"},
    {"a_morph_normal", "vertex.attrib[11]"},
    {"a_morph_tangent", "vertex.attrib[12]"},
    {"a_morph_texcoord1", "vertex.attrib[14]"},
    {"a_morph_texcoord", "vertex.attrib[13]"},
    {"a_morph_color", "vertex.attrib[5]"},
    {"a_instance_matrix0", "vertex.attrib[12]"},
    {"a_instance_matrix1", "vertex.attrib[13]"},
    {"a_instance_matrix2", "vertex.attrib[14]"},
    {"a_instance_param", "vertex.attrib[15]"},
    {"o_reflect", "result.texcoord[5]"},
    {"o_normal", " result.texcoord[5]"},
    {"o_tangent", " result.texcoord[3]"},
    {"o_binormal", " result.texcoord[4]"},
    {"o_color", "result.color"},
    {"o_exposure", "result.texcoord[1]"},
    {"POW luce.w, luce.x, 8.0;",
     "MUL _tmp0.x, luce.x, luce.x;MUL _tmp1.x, _tmp0.x, _tmp0.x;MUL luce.w, _tmp1.x, _tmp1.x;"},
    {"TEX tmp, { 0.0, 0.0 }, texture[3], 2D;", "MOV tmp, { 0.0, 0.0, 0.0, 1.0};"},
    {"SUBC ", "SUB "},
    {"MOVC ", "MOV "},
    {"ARL _adr, vertex.attrib[15];", "ARL _adr.x, vertex.attrib[15].x;"},
    {"DP4 _tmp0.x, _mtxpal[_adr.y ]", "ARL _adr.x, vertex.attrib[15].y;DP4 _tmp0.x, _mtxpal[_adr.x ]"},
    {"DP4 _tmp0.y, _mtxpal[_adr.y+1]", "DP4 _tmp0.y, _mtxpal[_adr.x+1]"},
    {"DP4 _tmp0.z, _mtxpal[_adr.y+2]", "DP4 _tmp0.z, _mtxpal[_adr.x+2]"},
    {"DP3 _tmp0.x, _mtxpal[_adr.y ]", "DP3 _tmp0.x, _mtxpal[_adr.x ]"},
    {"DP3 _tmp0.y, _mtxpal[_adr.y+1]", "DP3 _tmp0.y, _mtxpal[_adr.x+1]"},
    {"DP3 _tmp0.z, _mtxpal[_adr.y+2]", "DP3 _tmp0.z, _mtxpal[_adr.x+2]"},
    {"DP4 _tmp0.x, _mtxpal[_adr.z ]", "ARL _adr.x, vertex.attrib[15].z;DP4 _tmp0.x, _mtxpal[_adr.x ]"},
    {"DP4 _tmp0.y, _mtxpal[_adr.z+1]", "DP4 _tmp0.y, _mtxpal[_adr.x+1]"},
    {"DP4 _tmp0.z, _mtxpal[_adr.z+2]", "DP4 _tmp0.z, _mtxpal[_adr.x+2]"},
    {"DP3 _tmp0.x, _mtxpal[_adr.z ]", "DP3 _tmp0.x, _mtxpal[_adr.x ]"},
    {"DP3 _tmp0.y, _mtxpal[_adr.z+1]", "DP3 _tmp0.y, _mtxpal[_adr.x+1]"},
    {"DP3 _tmp0.z, _mtxpal[_adr.z+2]", "DP3 _tmp0.z, _mtxpal[_adr.x+2]"},
    {"DP4 _tmp0.x, _mtxpal[_adr.w ]", "ARL _adr.x, vertex.attrib[15].w;DP4 _tmp0.x, _mtxpal[_adr.x ]"},
    {"DP4 _tmp0.y, _mtxpal[_adr.w+1]", "DP4 _tmp0.y, _mtxpal[_adr.x+1]"},
    {"DP4 _tmp0.z, _mtxpal[_adr.w+2]", "DP4 _tmp0.z, _mtxpal[_adr.x+2]"},
    {"DP3 _tmp0.x, _mtxpal[_adr.w ]", "DP3 _tmp0.x, _mtxpal[_adr.x ]"},
    {"DP3 _tmp0.y, _mtxpal[_adr.w+1]", "DP3 _tmp0.y, _mtxpal[_adr.x+1]"},
    {"DP3 _tmp0.z, _mtxpal[_adr.w+2]", "DP3 _tmp0.z, _mtxpal[_adr.x+2]"},
    {"BRA skinning_end (EQ.y);", ""},
    {"BRA skinning_end (EQ.z);", ""},
    {"BRA skinning_end (EQ.w);", ""},
    {"BRA skinning_pn_end (EQ.y);", ""},
    {"BRA skinning_pn_end (EQ.z);", " "},
    {"BRA skinning_pn_end (EQ.w);", " "},
    {"skinning_end:", " "},
    {"skinning_pn_end:", " "},
    {"(GT.w)", " "},
    {"(GT.x)", " "},
    {"(EQ.y)", " "},
    {"POW tmp.w, tmp.w, 5.0;", "MUL tmp.w, tmp.w, tmp.w;MUL tmp.w, tmp.w, tmp.w;MUL tmp.w, tmp.w, tmp.w;"},
    {"POW luce.w, luce.x, 8.0;",
     "MUL _tmp0.x, luce.x, luce.x;MUL _tmp0.y, _tmp0.x, _tmp0.x;MUL luce.w, _tmp0.y, _tmp0.y;"},
    {"POW tmp.x, tmp.w, 5.0;", "MUL _tmp0.x, tmp.w, tmp.w;MUL _tmp0.y, _tmp0.x, _tmp0.x;MUL tmp.x, _tmp0.y, tmp.w;"},
    {"DP3_SAT tmp.w, camera_direction_w, vertex_normal_w;",
     "DP3 tmp.w, camera_direction_w, vertex_normal_w;MAX tmp.w, tmp.w, 0.0;MIN tmp.w, tmp.w, 1.0;"},
    {"MUL_SAT _tmp0.w, _tmp0.w, state.fog.params.w;",
     "MUL _tmp0.w, _tmp0.w, state.fog.params.w;MAX _tmp0.w, _tmp0.w, 0.0;MIN _tmp0.w, _tmp0.w, 1.0;"},
    {"DP3_SAT luce.x, -eyevec, lit_dir;",
     "DP3 luce.x, -eyevec, lit_dir;MAX luce.x, luce.x, 0.0;MIN luce.x, luce.x, 1.0;"},
    {"MUL_SAT luce.y, luce.y, luce.y;", "MUL luce.y, luce.y, luce.y;MAX luce.y, luce.y, 0.0;MIN luce.y, luce.y, 1.0;"},
    {"SUB_SAT tmp.w, 1.0, tmp.w;", "SUB tmp.w, 1.0, tmp.w;MAX tmp.w, tmp.w, 0.0;MIN tmp.w, tmp.w, 1.0;"},
    {"DP3_SAT diff.w, normal_w, tmp;", "DP3 diff.w, normal_w, tmp;MAX diff.w, diff.w, 0.0;MIN diff.w, diff.w, 1.0;"}};

int rtunedVsMesaCount = sizeof(rtunedVsMesa) / sizeof(SearchReplace);
