#pragma once
#include <GL/gl.h>
#include <GL/glext.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct
{
    const char *search;
    const char *replacement;
} SearchReplace;

void gsEvoElfShaderPatcher();
void gl_ProgramStringARB(int vertex_prg, int program_fmt, int program_len, char *program);
void gl_ShaderSourceARB(u_int32_t shaderObj, int count, const char **const string, const int *length);
void *gl_XGetProcAddressARB(const unsigned char *procName);
void gl_MultiTexCoord2fARB(u_int32_t target, float s, float t);
void gl_Color4ub(u_char red, u_char green, u_char blue, u_char alpha);
void gl_Vertex3f(float x, float y, float z);
void gl_TexCoord2f(float s, float t);
u_char cg_GLIsProfileSupported(int profile);
int cg_GLGetLatestProfile(int profileClass);
void gl_ProgramParameters4fvNV(u_int32_t target, u_int32_t index, int32_t count, const float *v);
void cacheModedShaderFiles();
void srtvElfShaderPatcher();
void gl_BindProgramNV(u_int32_t target, u_int32_t id);
void gl_LoadProgramNV(GLenum target, GLuint program, GLsizei len, char *string);
void gl_DeleteProgramsNV(int n, const u_int32_t *programs);
void gl_EndOcclusionQueryNV(void);
void gl_BeginOcclusionQueryNV(u_int32_t id);
void glut_GameModeStringOR(const char *string);
int compileWithCGC(char *command);
void loadLibCg();
void cacheNnstdshader();
void vf5SetExposure(int var, float exposure);
void vf5SetIntensity(int var, float *intensity);
void vf5GetIdStart(void *iostream, int *idStart);
void vf5SetDiffuse(int var, float r, float g, float b, float a);
void hookVf5FSExposure(uint32_t expAddr, uint32_t intAddr, uint32_t idStartAddr, uint32_t difAddr);
char *replaceInBlock(char *source, SearchReplace *searchReplace, int searchReplaceCount, char *startSearch, char *endSearch);
char *replaceSubstring(const char *buffer, int start, int end, const char *search, const char *replace);