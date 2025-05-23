#include "common.h"

// Fragment
SearchReplace vf5FsMesa[] = {
    {"OPTION NV_fragment_program2;", ""},
    {"SHORT TEMP", "TEMP"},
    {"SHORT OUTPUT", "OUTPUT"},
    {"LONG OUTPUT", "OUTPUT"},
    {"ATTRIB a_facing = fragment.facing;", "#ATTRIB a_facing = fragment.facing;"},
    //{"MAD diff.xyz, diff, state.light[0].diffuse, luce;", "MAD diff.xyz, diff, state.light[0].diffuse, luce;MAX diff,
    // diff, 0.8;"},  //diffuse
    {"TXL ", "TEX "},

    {"NRMH normal, a_normal;",
     "DP3 _tmp0.x, a_normal, a_normal;RSQ _tmp0.y, _tmp0.x;MUL normal, a_normal, _tmp0.y;MOV normal.w, 1.0;"},
    {"NRMH normal, normal;",
     "DP3 _tmp0.x, normal, normal;RSQ _tmp0.y, _tmp0.x;MUL normal, normal, _tmp0.y;MOV normal.w, 1.0;"},
    {"NRMH eye, a_eye;", "DP3 _tmp0.x, a_eye, a_eye;RSQ _tmp0.y, _tmp0.x;MUL eye, a_eye, _tmp0.y;MOV eye.w, 1.0;"},
    {"NRMH aniso_tangent.xyz, a_aniso_tangent;",
     "DP3 _tmp0.x, a_aniso_tangent, a_aniso_tangent;RSQ _tmp0.y, _tmp0.x;MUL aniso_tangent.xyz, a_aniso_tangent, "
     "_tmp0.y;MOV aniso_tangent.w, 1.0;"},
    {"NRMH aniso_tangent.xyz, aniso_tangent;", "DP3 _tmp0.x, aniso_tangent, aniso_tangent;RSQ _tmp0.y, _tmp0.x;MUL "
                                               "aniso_tangent.xyz, aniso_tangent, _tmp0.y;MOV aniso_tangent.w, 1.0;"},
    {"NRMH normal_t, normal_t;",
     "DP3 _tmp0.x, normal_t, normal_t;RSQ _tmp0.y, _tmp0.x;MUL normal_t, normal_t, _tmp0.y;MOV normal_t.w, 1.0;"},
    {"NRMH normal_w, normal_w;",
     "DP3 _tmp0.x, normal_w, normal_w;RSQ _tmp0.y, _tmp0.x;MUL normal_w, normal_w, _tmp0.y;MOV normal_w.w, 1.0;"},
    {"NRM normal_w.rgb, normal_w;",
     "DP3 _tmp0.x, normal_w, normal_w;RSQ _tmp0.y, _tmp0.x;MUL _tmp1.x, normal_w.x, _tmp0.y;MUL _tmp1.y, normal_w.y, "
     "_tmp0.y;MUL _tmp1.z, normal_w.z, _tmp0.y;MOV normal_w.x, _tmp1.x;MOV normal_w.y, _tmp1.y;MOV normal_w.z, "
     "_tmp1.z;MOV normal_w.w, 1.0;"},
    {"NRM eye_w, eye_w;", "DP3 _tmp0.x, eye_w, eye_w;RSQ _tmp0.y, _tmp0.x;MUL eye_w, eye_w, _tmp0.y;MOV eye_w.w, 1.0;"},
    {"NRM normal, normal;",
     "DP3 _tmp0.x, normal, normal;RSQ _tmp0.y, _tmp0.x;MUL normal, normal, _tmp0.y;MOV normal.w, 1.0;"},
    {"NRM normal_t, normal_t;",
     "DP3 _tmp0.x, normal_t, normal_t;RSQ _tmp0.y, _tmp0.x;MUL normal_t, normal_t, _tmp0.y;MOV normal_t.w, 1.0;"},
    {"NRM half, half;", "DP3 _tmp0.x, half, half;RSQ _tmp0.y, _tmp0.x;MUL half, half, _tmp0.y;MOV half.w, 1.0;"}, // FS
    /*{"NRMH ", "#NRMH "}, {"NRM ", "#NRM "},*/
    {"POW tmp.x, tmp.x, 8.0;", "MOV _tmp0.x, 8.0;POW tmp.x, tmp.x, _tmp0.x;"},
    {"POW phase.z, phase.w, 32.0;", "MOV _tmp0.x, 32.0;POW phase.z, phase.w, _tmp0.x;"},
    // {"#POW tmp.y, tmp.y, p_shininess.x;",
    //  "MUL _tmp0.x, tmp.y, tmp.y;MUL _tmp0.y, _tmp0.x, _tmp0.x;MUL tmp.y, _tmp0.y, _tmp0.y; "},
    // {"#POW tmp.z, tmp.z, p_shininess.x;",
    //  "MUL _tmp1.x, tmp.z, tmp.z;MUL _tmp1.y, _tmp1.x, _tmp1.x;MUL tmp.z, _tmp1.y, _tmp1.y; "},
    {"POW tmp.y, tmp.y, p_shininess.x;",
     "MUL _tmp0.x, tmp.y, tmp.y;MUL _tmp0.y, _tmp0.x, _tmp0.x;MUL tmp.y, _tmp0.y, _tmp0.y; "},
    {"POW tmp.z, tmp.z, p_shininess.x;",
     "MUL _tmp1.x, tmp.z, tmp.z;MUL _tmp1.y, _tmp1.x, _tmp1.x;MUL tmp.z, _tmp1.y, _tmp1.y; "},
    {"DP2 tmp.x, ref, ref;", "MUL _tmp0.x, ref.x, ref.x;MUL _tmp0.y, ref.y, ref.y;ADD tmp.x, _tmp0.x, _tmp0.y;"},
    {"DP2 tmp.w, tmp, tmp;", "MUL _tmp0.x, tmp.x, tmp.x;MUL _tmp0.y, tmp.y, tmp.y;ADD tmp.w, _tmp0.x, _tmp0.y;"},
    {"DP2 tmp.w, tmp, tmp;", "MUL _tmp0.x, tmp.x, tmp.x;MUL _tmp0.y, tmp.y, tmp.y;ADD tmp.w, _tmp0.x, _tmp0.y;"},
    {"DDX ds.xy, a_tex_color0;", "MOV ds.xy, a_tex_color0;"},
    {"DDY du.xy, a_tex_color0;", "MOV du.xy, a_tex_color0;"},
    {"DP2 ds.w, ds, ds;", "MUL _tmp0.x, ds.x, ds.x;MUL _tmp0.y, ds.y, ds.y;ADD ds.w, _tmp0.x, _tmp0.y;"},
    {"DP2 du.w, du, du;", "MUL _tmp1.x, du.x, du.x;MUL _tmp1.y, du.y, du.y;ADD du.w, _tmp1.x, _tmp1.y;"},
    {"SUBC ", "SUB "},
    {"MOVC ", "MOV "},
    {"(LT.w)", " "},
    {"(GT.w)", " "},
    {"(GT.x)", " "},
    {"(EQ.x)", " "},
    {"(EQ.y)", " "},
    {"(EQ.z)", " "},
    {"RET;", " "},
    {"DIV col.x, sum.x, col.x;", "RCP _tmp0.x, col.x;MUL col.x, sum.x, _tmp0.x;"},
    {"DIV col.y, sum.y, col.y;", "RCP _tmp0.y, col.y;MUL col.y, sum.y, _tmp0.y;"},
    {"DIV col.z, sum.z, col.z;", "RCP _tmp0.z, col.z;MUL col.z, sum.z, _tmp0.z;"}};

int vf5FsMesaCount = sizeof(vf5FsMesa) / sizeof(SearchReplace);

// Vertex
SearchReplace vf5VsMesa[] = {
    {"OPTION NV_vertex_program3;", "#OPTION NV_vertex_program3;"},
    /*{"_SAT", " "},*/
    {"ARL _adr, _a_mtxidx", "ARL _adr.x, _a_mtxidx.x"},
    {"ADD_SAT o_color_f1.x, tmp.x, tmp.y;",
     "ADD _tmp0, tmp.x, tmp.y;MAX _tmp0, _tmp0, 0.0;MIN _tmp0, _tmp0, 1.0;MOV o_color_f1.x, _tmp0;"},
    {"DP3_SAT o_tex0, -eyevec, normal_v;",
     "DP3 _tmp0, -eyevec, normal_v;MAX _tmp0, _tmp0, 0.0;MIN _tmp0, _tmp0, 1.0;MOV o_tex0, _tmp0;"},
    {"ATTRIB a_color = vertex.texcoord[7];", " "},
    {"MUL result.texcoord[7], psize.x, a_color;",
     "MUL result.texcoord[7], psize.x, { 0.0, 0.0, 0.0, 0.0};"},                                       // water particle
    {"MOV result.texcoord[7], vertex.texcoord[7];", "MOV result.texcoord[7], { 0.0, 0.0, 0.0, 0.0};"}, // ripple
    {"POW tmp.x, tmp.x, 60;", "MOV _tmp0.x, 60.0;POW tmp.x, tmp.x, _tmp0.x;"},
    {", a_tex1", ", vertex.texcoord[1]"},
    {", a_tex2", ", vertex.texcoord[2]"},
    {"COS tmp.x, a_normal.x;", "MUL _tmp0.x, vertex.normal.x, {0.63662};MUL _tmp1.x, _tmp0.x, _tmp0.x;RCP _tmp2.x, "
                               "_tmp1.x;MUL _tmp2.x, _tmp2.x, 0.5;SUB tmp.x, 1.0, _tmp1.x;"},
    {"BRA skinning_end (EQ.y);", ""},
    {"BRA skinning_end (EQ.z);", ""},
    {"BRA skinning_end (EQ.w);", ""},
    {"BRA skinning_pn_end (EQ.y);", ""},
    {"BRA skinning_pn_end (EQ.z);", " "},
    {"BRA skinning_pn_end (EQ.w);", " "},
    {"BRA skinning_p_end (EQ.y);", ""},
    {"BRA skinning_p_end (EQ.z);", " "},
    {"BRA skinning_p_end (EQ.w);", " "},

    {"BRA skinning_pn_end0 (EQ.y);", "#"},
    {"BRA skinning_pn_end1 (EQ.z);", " "},
    {"BRA skinning_pn_end2 (EQ.w);", " "},
    {"skinning_pn_end0: skinning_pn_end1: skinning_pn_end2:", ""},
    {"BRA skinning_end0 (EQ.y);", ""},
    {"BRA skinning_end1 (EQ.z);", ""},
    {"BRA skinning_end2 (EQ.w);", ""},
    {"skinning_end0: skinning_end1: skinning_end2:", "#"},
    {"BRA skinning_p_end0 (EQ.y);", "#"},
    {"BRA skinning_p_end1 (EQ.z);", " "},
    {"BRA skinning_p_end2 (EQ.w);", " "},
    {"skinning_p_end0: skinning_p_end1: skinning_p_end2:", ""},
    {"ATTRIB a_tex2 = vertex.texcoord[2];", "#ATTRIB a_tex2 = vertex.texcoord[2];"},
    {"ATTRIB a_tex3 = vertex.texcoord[3];", "#ATTRIB a_tex3 = vertex.texcoord[3];"},
    {"skinning_end:", " "},
    {"skinning_pn_end:", " "},
    {"skinning_p_end:", " "},

    {"BRA face0_light_end (EQ.w);", ""}, // vf5r
    {"BRA face1_light_end (EQ.z);", ""},

    {"SGT _tmp2.y, pos_w.yyyy, program.env[0].w;", "SLT _tmp2.y, program.env[0].w, pos_w.yyyy;"},
    {"SGT _tmp2.y, pos_w.yyyy, program.env[1].w;", "SLT _tmp2.y, program.env[1].w, pos_w.yyyy;"},

    {"TEX _tmp1, _tmp0, texture[14], 2D;", "MOV _tmp1, { 0.0, 0.0, 0.0, 0.0};"}, // fog
    //{"TEX displace, tmp, texture[14], 2D;", "MOV displace, { 0.0, 0.0, 0.0, 0.0};"}, // snow
    {"TEX displace, tmp, texture[14], 2D;", "MOV tmp, a_position;FRC displace.x, tmp.x;FRC displace.y, tmp.z;MUL "
                                            "displace, displace, {0.15, 0.15, 0.15, 0};"}, // snow
    {"TEX displace, a_tex0, texture[14], 2D;", "MOV displace, { 0.0, 0.0, 0.0, 0.0};"},    // water
    {"TEX tmp, { 0.0, 0.0 }, texture[3], 2D;", "MOV tmp, { 0.4, 0.4, 0.4, 0.4};"},         // tone X
    {"SUBC ", "SUB "},
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
    {"ADD_SAT result.color.secondary.x, tmp.x, tmp.y;",
     "ADD _tmp0, tmp.x, tmp.y;MAX _tmp1, _tmp0, 0.0;MIN _tmp1, _tmp1, 1.0;MOV result.color.secondary.x, _tmp1;"}, // NA
    {"DP3_SAT result.texcoord[0], -eyevec, normal_v;",
     "DP3 _tmp0, -eyevec, normal_v;MAX _tmp1, _tmp0, 0.0;MIN _tmp1, _tmp1, 1.0;MOV result.texcoord[0], _tmp1;"}, // NA
    {"MOVC ", "MOV "},
    {"face0_light_end:", " "},
    {"face1_light_end:", " "},
    {"(GT.w)", " "},
    {"(GT.x)", " "},
    {"RET;", " "},
    {"(EQ.y)", " "},
    {"MULC ", "MUL "},

    {"POW tmp.w, tmp.w, 5.0;", "MOV _tmp0.x, 5.0;POW tmp.w, tmp.w, _tmp0.x;"},
    {"POW luce.w, luce.x, 8.0;", "MOV _tmp0.x, 8.0;POW luce.w, luce.x, _tmp0.x;"},
    {"POW tmp.x, tmp.x, 5.0;", "MOV _tmp0.x, 5.0;POW tmp.x, tmp.x, _tmp0.x;"},
    {"POW tmp.x, tmp.w, 5.0;", "MOV _tmp0.x, 5.0;POW tmp.x, tmp.w, _tmp0.x;"},
    {"POW fresnel.w, fresnel.w, 5;", "MOV _tmp0.x, 5.0;POW fresnel.w, fresnel.w, _tmp0.x;"},
    // {"#POW diff.y, diff.y, face_lit_attenuation.w;",
    //  "MUL _tmp0.x, diff.y, diff.y;MUL _tmp0.y, _tmp0.x, _tmp0.x;MUL tmp.x, _tmp0.y, _tmp0.y;"},
    {"POW diff.y, diff.y, face_lit_attenuation.w;",
     "MUL _tmp0.x, diff.y, diff.y;MUL _tmp0.y, _tmp0.x, _tmp0.x;MUL tmp.x, _tmp0.y, _tmp0.y;"},
    {"MUL_SAT _tmp0.w, _tmp0.w, state.fog.params.w;",
     "MUL _tmp0.w, _tmp0.w, state.fog.params.w;MAX _tmp0.w, _tmp0.w, 0.0;MIN _tmp0.w, _tmp0.w, 1.0;"},
    {"MUL_SAT _tmp0.w, _tmp0.w, program.env[14].w;",
     "MUL _tmp0.w, _tmp0.w, program.env[14].w;MAX _tmp0.w, _tmp0.w, 0.0;MIN _tmp0.w, _tmp0.w, 1.0;"},
    {"MUL_SAT luce.y, luce.y, luce.y;", "MUL luce.y, luce.y, luce.y;MAX luce.y, luce.y, 0.0;MIN luce.y, luce.y, 1.0;"},
    {"MUL_SAT tmp.w, tmp.w, program.env[19].w;",
     "MUL tmp.w, tmp.w, program.env[19].w;MAX tmp.w, tmp.w, 0.0;MIN tmp.w, tmp.w, 1.0;"},
    {"SUB_SAT tmp.w, 1.0, tmp.w;", "SUB tmp.w, 1.0, tmp.w;MAX tmp.w, tmp.w, 0.0;MIN tmp.w, tmp.w, 1.0;"},
    {"SUB_SAT tmp.xy, 1.0, tmp;", "SUB tmp.x, 1.0, tmp.x;SUB tmp.y, 1.0, tmp.y;MAX tmp.x, tmp.x, 0.0;MAX tmp.y, tmp.y, "
                                  "0.0;MIN tmp.x, tmp.x, 1.0;MIN tmp.y, tmp.y, 1.0;"},
    {"DP3_SAT diff.w, normal_w, tmp;", "DP3 diff.w, normal_w, tmp;MAX diff.w, diff.w, 0.0;MIN diff.w, diff.w, 1.0;"},
    {"DP3_SAT luce.x, -eyevec, lit_dir;",
     "DP3 luce.x, -eyevec, lit_dir;MAX luce.x, luce.x, 0.0;MIN luce.x, luce.x, 1.0;"},
    {"DP3_SAT tmp.x, normal_w, lit_dir_w;",
     "DP3 tmp.x, normal_w, lit_dir_w;MAX tmp.x, tmp.x, 0.0;MIN tmp.x, tmp.x, 1.0;"},
    {"DP3_SAT tmp.w, lit_dir_w, normal_w;",
     "DP3 tmp.w, lit_dir_w, normal_w;MAX tmp.w, tmp.w, 0.0;MIN tmp.w, tmp.w, 1.0;"},
    {"DP3_SAT diff, normal_w, program.env[15];",
     "DP3 diff, normal_w, program.env[15];MAX diff, diff, 0.0;MIN diff, diff, 1.0;"},
    {"DP3_SAT amb, normal_w, program.env[16];",
     "DP3 amb, normal_w, program.env[16];MAX amb, amb, 0.0;MIN amb, amb, 1.0;"},
    {"MUL_SAT tmp.w, tmp.w, program.env[14].w;",
     "MUL tmp.w, tmp.w, program.env[14].w;MAX tmp.w, tmp.w, 0.0;MIN tmp.w, tmp.w, 1.0;"},
    {"MUL_SAT _tmp0.w, _tmp0.w, 2.0;",
     "MUL _tmp0.w, _tmp0.w, 2.0;MAX _tmp0.w, _tmp0.w, 0.0;MIN _tmp0.w, _tmp0.w, 1.0;"},
    {"SUB_SAT fresnel.w, 1, fresnel.w;",
     "SUB fresnel.w, 1, fresnel.w;MAX fresnel.w, fresnel.w, 0.0;MIN fresnel.w, fresnel.w, 1.0; "},
    {"MAD_SAT psize.y, psize.x, -0.5, 1.1;",
     "MAD psize.y, psize.x, -0.5, 1.1;MAX psize.y, psize.y, 0.0;MIN psize.y, psize.y, 1.0;"},           // vf5r
    {"MAD_SAT result.texcoord[1], psize.x, -0.5, 1.1;", "MAD result.texcoord[1], psize.x, -0.5, 1.1;"}, // vf5r
    {"DP3_SAT diff.x, normal_v, { 0, 0, 1 };",
     "DP3 diff.x, normal_v, { 0, 0, 1 };MAX diff.x, diff.x, 0.0;MIN diff.x, diff.x, 1.0;"},
    {"MAD_SAT psize.y, psize.x, -0.5, 1.5;",
     "MAD psize.y, psize.x, -0.5, 1.5;MAX psize.y, psize.y, 0.0;MIN psize.y, psize.y, 1.0;"},
    {"MAD_SAT result.texcoord[1], psize.x, -0.5, 1.5;",
     "MAD _tmp0, psize.x, -0.5, 1.5;MAX _tmp0, _tmp0, 0.0;MIN _tmp0, _tmp0, 1.0;MOV result.texcoord[1], _tmp0;"},
    {"POW diff.x, diff.x, 0.4;", "MOV _tmp0.x, 0.4;POW diff.x, diff.x, _tmp0.x;"},
    {"POW tmp.w, tmp.w, {5.0}.x;", "MOV _tmp0.x, 5.0;POW tmp.x, tmp.w, _tmp0.x;"},
    {"COS tmp.x, vertex.normal.x;", "MUL _tmp0.x, vertex.normal.x, {0.63662};MUL _tmp1.x, _tmp0.x, _tmp0.x;RCP "
                                    "_tmp2.x, _tmp1.x;MUL _tmp2.x, _tmp2.x, 0.5;SUB tmp.x, 1.0, _tmp1.x;"}};

int vf5VsMesaCount = sizeof(vf5VsMesa) / sizeof(SearchReplace);

// Fragment
SearchReplace vf5FsAti[] = {{"POW tmp.x, tmp.x, 8.0;", "MOV _tmp0.x, 8.0;POW tmp.x, tmp.x, _tmp0.x;"},
                            {"POW phase.z, phase.w, 32.0;", "MOV _tmp0.x, 32.0;POW phase.z, phase.w, _tmp0.x;"},
                            {"POW tmp.y, tmp.y, p_shininess.x;",
                             "MUL _tmp0.x, tmp.y, tmp.y;MUL _tmp0.y, _tmp0.x, _tmp0.x;MUL tmp.y, _tmp0.y, _tmp0.y; "},
                            {"POW tmp.z, tmp.z, p_shininess.x;",
                             "MUL _tmp1.x, tmp.z, tmp.z;MUL _tmp1.y, _tmp1.x, _tmp1.x;MUL tmp.z, _tmp1.y, _tmp1.y; "}};

int vf5FsAtiCount = sizeof(vf5FsAti) / sizeof(SearchReplace);

// Vertex
SearchReplace vf5VsAti[] = {
    {"ATTRIB ", "#ATTRIB "},
    {"OUTPUT ", "#OUTPUT "},
    {"_a_mtxidx", "vertex.attrib[15]"},
    {"_a_weight", "vertex.attrib[1]"},
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
    {"a_position", "vertex.position"},
    {"a_color", "vertex.color"},
    {"a_normal", "vertex.normal"},
    {"a_tangent", "vertex.attrib[6]"},
    {"a_tex0", "vertex.texcoord[0]"},
    {"a_tex1", "vertex.texcoord[1]"},
    {"a_tex2", "vertex.texcoord[2]"},
    {"a_tex3", "vertex.texcoord[3]"},
    {"a_size_alpha", "vertex.texcoord[6]"},
    {"a_size", "vertex.texcoord[6]"},
    {"o_position", "result.position"},
    {"o_color_f0", "result.color"},
    {"o_color_f1", "result.color.secondary"},
    {"o_color_b0", "result.color.back"},
    {"o_color_b1", "result.color.back.secondary"},
    {"o_fog", "result.fogcoord"},
    {"o_tex0", "result.texcoord[0]"},
    {"o_tex1", "result.texcoord[1]"},
    {"o_tex2", "result.texcoord[2]"},
    {"o_tex_shadow0", "result.texcoord[7]"},
    {"o_tex_shadow1", "result.texcoord[2]"},
    {"o_normal", "result.texcoord[5]"},
    {"o_aniso_tangent", " result.texcoord[7]"},
    {"TEMP aniso_tangent;", "TEMP anisoLLOADER_tangent;"},
    {" o_tangent", " result.texcoord[3]"},
    {"o_binormal", "result.texcoord[4]"},
    {"o_eye", "result.texcoord[6]"},
    {"o_reflect_v", "result.texcoord[7]"},
    {"o_reflect", "result.texcoord[5]"},
    {"o_color", "result.color"},
    {"o_psize", "result.pointsize"},
    {"o_lit_hit", "result.texcoord[2]"},
    {"o_tex3", "result.texcoord[3]"},
    {"o_tex4", "result.texcoord[4]"},
    {"o_tex5", "result.texcoord[5]"},
    {"o_tex6", "result.texcoord[6]"},
    {"o_tex7", "result.texcoord[7]"},
    {"o_exposure", "result.texcoord[1]"},

    {"LLOADER", ""},

    {"POW tmp.x, tmp.x, 60;", "MOV _tmp0.x, 60.0;POW tmp.x, tmp.x, _tmp0.x;"},
    {"POW tmp.w, tmp.w, 5.0;", "MOV _tmp0.x, 5.0;POW tmp.w, tmp.w, _tmp0.x;"},
    {"POW luce.w, luce.x, 8.0;", "MOV _tmp0.x, 8.0;POW luce.w, luce.x, _tmp0.x;"},
    {"POW tmp.x, tmp.x, 5.0;", "MOV _tmp0.x, 5.0;POW tmp.x, tmp.x, _tmp0.x;"},
    {"POW tmp.x, tmp.w, 5.0;", "MOV _tmp0.x, 5.0;POW tmp.x, tmp.w, _tmp0.x;"},
    {"POW fresnel.w, fresnel.w, 5;", "MOV _tmp0.x, 5.0;POW fresnel.w, fresnel.w, _tmp0.x;"},
    {"POW diff.y, diff.y, face_lit_attenuation.w;",
     "MUL _tmp0.x, diff.y, diff.y;MUL _tmp0.y, _tmp0.x, _tmp0.x;MUL tmp.x, _tmp0.y, _tmp0.y;"},
    {"POW diff.x, diff.x, 0.4;", "MOV _tmp0.x, 0.4;POW diff.x, diff.x, _tmp0.x;"},
    {"POW tmp.w, tmp.w, {5.0}.x;", "MOV _tmp0.x, 5.0;POW tmp.x, tmp.w, _tmp0.x;"},

    {"BRA skinning_end (EQ.y);", ""},
    {"BRA skinning_end (EQ.z);", ""},
    {"BRA skinning_end (EQ.w);", ""},
    {"BRA skinning_pn_end (EQ.y);", ""},
    {"BRA skinning_pn_end (EQ.z);", " "},
    {"BRA skinning_pn_end (EQ.w);", " "},
    {"BRA skinning_p_end (EQ.y);", ""},
    {"BRA skinning_p_end (EQ.z);", " "},
    {"BRA skinning_p_end (EQ.w);", " "},
    {"BRA skinning_pn_end0 (EQ.y);", "#"},
    {"BRA skinning_pn_end1 (EQ.z);", " "},
    {"BRA skinning_pn_end2 (EQ.w);", " "},
    {"skinning_pn_end0: skinning_pn_end1: skinning_pn_end2:", ""},
    {"BRA skinning_end0 (EQ.y);", ""},
    {"BRA skinning_end1 (EQ.z);", ""},
    {"BRA skinning_end2 (EQ.w);", ""},
    {"skinning_end0: skinning_end1: skinning_end2:", "#"},
    {"BRA skinning_p_end0 (EQ.y);", "#"},
    {"BRA skinning_p_end1 (EQ.z);", " "},
    {"BRA skinning_p_end2 (EQ.w);", " "},
    {"skinning_p_end0: skinning_p_end1: skinning_p_end2:", ""},
    {"skinning_end:", " "},
    {"skinning_pn_end:", " "},
    {"skinning_p_end:", " "},
    {"BRA face0_light_end (EQ.w);", ""}, // vf5r
    {"BRA face1_light_end (EQ.z);", ""},

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
};

int vf5VsAtiCount = sizeof(vf5VsAti) / sizeof(SearchReplace);

SearchReplace vf5VsAti2[] = {
    {"PARAM _mtxpal[] = {program.env[32 .. 224]};", "#PARAM _mtxpal[] = {program.env[32 .. 224]};"}};

int vf5VsAtiCount2 = sizeof(vf5VsAti2) / sizeof(SearchReplace);

SearchReplace vf5VsAtifs[] = {
    {"TEX displace, vertex.texcoord[0], texture[14], 2D;", "MOV displace, { 0.0, 0.0, 0.0, 0.0};"}}; // Sand

int vf5VsAtifsCount = sizeof(vf5VsAtifs) / sizeof(SearchReplace);

typedef struct
{
    const char *stageAbb;
    const char *StageName;
    bool patch;
} Vf5StagesList;

Vf5StagesList vf5StageList[] = {{"are", "Arena", true},
                                {"aur", "Aurora", false},
                                {"ban", "Great Wall", false},
                                {"bar", "Terrace", false},
                                {"cas", "Palace", false},
                                {"djo", "Statues", false},
                                {"du1", "Sanctuary Snow", false},
                                {"du2", "Sanctuary Fire", false},
                                {"du3", "Sanctuary Sunny", true},
                                {"du4", "Sactuary Thunder", false},
                                {"du5", "Sanctuary du5", false},
                                {"evo00", "evo00", false},
                                {"evo01", "evo01", false},
                                {"evo02", "evo02", false},
                                {"evo03", "evo03", false},
                                {"evo04", "evo04", false},
                                {"evo05", "evo05", false},
                                {"evo06", "evo06", false},
                                {"evo07", "evo07", false},
                                {"evo08", "evo08", false},
                                {"evo09", "evo09", false},
                                {"gym", "Training Room", true},
                                {"hai", "Broken House", false},
                                {"jin", "Shrine", true},
                                {"nyc", "City", false},
                                {"riv", "River", true},
                                {"sin", "Deep Mountain", false},
                                {"slk", "Ruins", true},
                                {"smo", "Sumo Ring", true},
                                {"tak", "Water Falls", true},
                                {"tan", "Grassland", true},
                                {"ter", "Temple", true},
                                {"trm", "trm", false},
                                {"trs", "trs", false},
                                {"ts2", "ts2", false},
                                {"ts3", "ts3", false},
                                {"tst", "Test", false},
                                {"umi", "Island", false},
                                {"yuk", "Snow Mountain", false}};

int vf5StageListCount = sizeof(vf5StageList) / sizeof(Vf5StagesList);
