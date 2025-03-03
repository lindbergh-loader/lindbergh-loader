#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

void gsEvoElfShaderPatcher();
void gl_ProgramStringARB(int vertex_prg, int program_fmt, int program_len, char *program);
void gl_ShaderSourceARB(u_int32_t shaderObj, int count, const char **const string, const int *length);
#ifdef __cplusplus
extern "C"
{
#endif
    bool shaderFileInList(const char *pathname, int *idx);
    size_t freadReplace(void *buf, size_t size, size_t count, int idx);
    long int ftellGetShaderSize(int idx);
#ifdef __cplusplus
}
#endif
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
void gl_LoadProgramNV(u_int32_t target, u_int32_t program, int len, const u_int8_t *string);
void gl_DeleteProgramsNV(int n, const u_int32_t *programs);
void gl_EndOcclusionQueryNV(void);
void gl_BeginOcclusionQueryNV(u_int32_t id);
u_int32_t Gl_VertexShader_createFromAsmFile(char *filename);
u_int32_t Gl_FragmentShader_createShaderFromFile(char *filename);
void gl_TexImage2DOR(u_int32_t target, int level, int internalformat, int width, int height, int border,
                     u_int32_t format, u_int32_t type, const void *pixels);
void gl_InitWindowSizeOR(int width, int height);
void glut_GameModeStringOR(const char *string);
char *replaceSubstring(const char *buffer, int start, int end, const char *search, const char *replace);
int compileWithCGC(char *command);
void loadLibCg();
void cacheNnstdshader();
void *vt3_open(void *fb, const char *s, int mode);
int vt3_close(void *fb);
u_int32_t *vt3_tellg(u_int32_t *size, void *is);
void *vt3_read(void *is, char *s, long long n);
void vf5SetExposure(int var, float exposure);
void vf5SetIntensity(int var, float *intensity);
void vf5GetIdStart(void *iostream, int *idStart);
void vf5SetDiffuse(int var, float r, float g, float b, float a);
void hookVf5FSExposure(uint32_t expAddr, uint32_t intAddr, uint32_t idStartAddr, uint32_t difAddr);