#ifndef __i386__
#define __i386__

#include <stdint.h>
#endif
#undef __x86_64__
#include <dlfcn.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glx.h>
#include <SDL2/SDL.h>

#include "../libxdiff/xdiff/xdiff.h"
#include "config.h"
#include "patch.h"
#include "log.h"

#include "shader_work/2spicy.h"
#include "shader_work/abc.h"
#include "shader_work/gsevo.h"
#include "shader_work/harley.h"
#include "shader_work/hod4.h"
#include "shader_work/hod4sp.h"
#include "shader_work/hodex.h"
#include "shader_work/hummer.h"
#include "shader_work/id.h"
#include "shader_work/lgj.h"
#include "shader_work/mj4.h"
#include "shader_work/or2.h"
#include "shader_work/primeval.h"
#include "shader_work/rambo.h"
#include "shader_work/rtuned.h"
#include "shader_work/srtv.h"
#include "shader_work/vf5.h"
#include "shader_work/vt3.h"

#define MAX_FILENAME_LENGTH 256
#define XDLT_STD_BLKSIZE (1024 * 8)

extern uint32_t gId;
char tmpShader[150000];
bool cachedShaderFilesLoaded = false;
int idxError = 0;
int idxVF5 = 0;
int vf5IdStart;
uint32_t vf5ExpOriAddr, vf5IntOriAddr, vf5IdStartOriAddr, vf5DifOriAddr;
char vf5StageNameAbbr[5];

typedef size_t *(*cgCreateContext_t)(void);
typedef void *(*cgCreateProgram_t)(size_t *context, int program_type, const char *program, int profile,
                                   const char *entry, const char **args);
typedef const char *(*cgGetProgramString_t)(char *program, int pname);
typedef void (*cgDestroyContext_t)(size_t *context);
typedef void (*cgDestroyProgram_t)(char *program);

bool fdHook = false;
int shaderIDX = 0;

cgCreateContext_t _cgCreateContext;
cgCreateProgram_t _cgCreateProgram;
cgGetProgramString_t _cgGetProgramString;
cgDestroyContext_t _cgDestroyContext;
cgDestroyProgram_t _cgDestroyProgram;

/**
 *   Search and Replace function.
 */
char *replaceSubstring(const char *buffer, int start, int end, const char *search, const char *replace)
{
    int searchLen = strlen(search);
    int replaceLen = strlen(replace);
    int bufferLen = strlen(buffer);

    if (start < 0 || end > bufferLen || start > end)
    {
        printf("Invalid start or end positions\n");
        return NULL;
    }

    int maxOccurrences = 0;
    const char *tmp = buffer + start;
    while ((tmp = strstr(tmp, search)) && tmp < buffer + end)
    {
        maxOccurrences++;
        tmp += searchLen;
    }
    int maxNewLen =
        (bufferLen + (maxOccurrences * (replaceLen - searchLen))) + 15; // 15 instead of 1 because it crashes with SRTV for no reason??

    char *newBuffer = malloc(maxNewLen);
    memset(newBuffer, '\0', maxNewLen);

    if (!newBuffer)
    {
        printf("Memory allocation failed\n");
        return NULL;
    }

    const char *pos = buffer + start;
    const char *endPos = buffer + end;
    char *newPos = newBuffer;

    memcpy(newPos, buffer, start);
    newPos += start;

    while (pos < endPos)
    {
        const char *foundPos = strstr(pos, search);
        if (foundPos && foundPos < endPos)
        {
            int chunkLen = foundPos - pos;
            memcpy(newPos, pos, chunkLen);
            newPos += chunkLen;

            memcpy(newPos, replace, replaceLen);
            newPos += replaceLen;
            pos = foundPos + searchLen;
        }
        else
        {
            int remainingLen = endPos - pos;
            memcpy(newPos, pos, remainingLen);
            newPos += remainingLen;
            break;
        }
    }
    strcpy(newPos, buffer + end);

    return newBuffer;
}

char *replaceInBlock(char *source, SearchReplace *searchReplace, int searchReplaceCount, char *startSearch,
                     char *endSearch)
{
    if (source == NULL || searchReplace == NULL)
        return NULL;

    int start_index = 0;
    if (startSearch != NULL && startSearch[0] != '\0')
    {
        char *start_pos = strstr(source, startSearch);
        if (start_pos == NULL)
            return source;
        start_index = start_pos - source + strlen(startSearch);
    }

    char *result = source;

    for (int i = 0; i < searchReplaceCount; i++)
    {
        const char *end_pos = result + strlen(result);

        if (endSearch[0] != '\0')
        {
            char *end_pos_candidate = strstr(result + start_index, endSearch);
            if (end_pos_candidate != NULL)
                end_pos = end_pos_candidate;
        }

        int end_index = end_pos - result;
        const char *target = searchReplace[i].search;
        const char *replacement = searchReplace[i].replacement;
        char *new_result = replaceSubstring(result, start_index, end_index, target, replacement);
        if (new_result != NULL)
        {
            result = new_result;
        }
    }
    return result;
}

/**
 *   glMultiTexCoord2fARB is replaced by glVertexAttrib2f to make it compatible
 * with Mesa.
 */
void gl_MultiTexCoord2fARB(u_int32_t target, float s, float t)
{
    glVertexAttrib2f((target - 33976), s, t);
}

/**
 *   glColor4ub is replaced by glVertexAttrib4Nub to make it compatible with
 * Mesa.
 */
void gl_Color4ub(u_char red, u_char green, u_char blue, u_char alpha)
{
    glVertexAttrib4Nub(3, red, green, blue, alpha);
}

/**
 *   glVertex3f is replaced by glVertexAttrib3f to make it compatible with Mesa.
 */
void gl_Vertex3f(float x, float y, float z)
{
    glVertexAttrib3f(0, x, y, z);
}

/**
 *   glTexCoord2f is replaced by glVertexAttrib2f to make it compatible with
 * Mesa.
 */
void gl_TexCoord2f(float s, float t)
{
    glVertexAttrib2f(8, s, t);
}

/**
 *   glProgramParameters4fvNV is replaced by glProgramEnvParameters4fvEXT as
 * glProgramParameters4fvNV does not work or does not exist in modern systems.
 */
void gl_ProgramParameters4fvNV(u_int32_t target, u_int32_t index, int count, const float *v)
{
    glProgramEnvParameters4fvEXT(target, index, count, v);
}

/**
 *   We force the cg compiler to use the profile we need to make the game work
 * on Mesa
 */
u_char cg_GLIsProfileSupported(int profile)
{
    u_char profileSupported = 0;
    switch (profile)
    {
    case 6146:
    case 6147:
    case 6148:
    case 6149:
    case 6151:
    case 7001:
    case 7010:
    case 7011:
    case 7012:
        profileSupported = 0;
        break;
    case 6150:
    case 7000:
    case 7007:
    case 7008:
    case 7009:
        profileSupported = 1;
        break;
    default:
        profileSupported = 0;
    }
    return profileSupported;
}

int cg_GLGetLatestProfile(int profileClass)
{
    int profile;

    if (profileClass == 8)
    {
        profile = 6150;
    }
    else if (profileClass == 9)
    {
        profile = 7000;
    }
    else
    {
        profile = 6145;
    }
    return profile;
}

/**
 *  Here we switch the program type from NV to ARB in Outrun
 */
void glEnable(GLenum cap)
{
    void *(*_glEnable)(GLenum cap) = dlsym(RTLD_NEXT, "glEnable");

    if ((cap == GL_FRAGMENT_PROGRAM_NV) && ((gId == OUTRUN_2_SP_SDX) || (gId == OUTRUN_2_SP_SDX_REVA)) &&
        (getConfig()->GPUVendor != NVIDIA_GPU))
    {
        cap = GL_FRAGMENT_PROGRAM_ARB;
    }
    _glEnable(cap);
}

/**
 *  Here we switch the program type from NV to ARB in Outrun
 */
void glDisable(GLenum cap)
{
    void *(*_glDisable)(GLenum cap) = dlsym(RTLD_NEXT, "glDisable");

    if ((cap == GL_FRAGMENT_PROGRAM_NV) && ((gId == OUTRUN_2_SP_SDX) || (gId == OUTRUN_2_SP_SDX_REVA)) &&
        (getConfig()->GPUVendor != NVIDIA_GPU))
    {
        cap = GL_FRAGMENT_PROGRAM_ARB;
    }
    _glDisable(cap);
}

/**
 *	The following 3 functions are replacements for SRTV and OR2
 */
void gl_BeginOcclusionQueryNV(GLuint id)
{
    glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);
}

void gl_EndOcclusionQueryNV(void)
{
    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
}

void gl_BeginConditionalRenderNVX(GLuint id)
{
    glBeginConditionalRender(id, GL_QUERY_WAIT);
}

/**
 * This function patches the shaders inside the ELF in Ghost Squad EVO.
 */
void gsEvoElfShaderPatcher()
{
    int prot = mprotect((void *)0x820B000, 0x20000, PROT_EXEC | PROT_WRITE);
    if (prot != 0)
    {
        printf("Error: Cannot unprotect memory region to change variable (%d)\n", prot);
        exit(1);
    }
    for (int i = 0; i < gsevoShaderOffsetsCount; i++)
    {
        char tmpProgram[5000];
        char *newProgram;
        memcpy(tmpProgram, (void *)(size_t)gsevoElfShaderOffsets[i].startOffset, gsevoElfShaderOffsets[i].sizeOfCode);
        memset((void *)(size_t)gsevoElfShaderOffsets[i].startOffset, '\0', gsevoElfShaderOffsets[i].totalSize);
        tmpProgram[gsevoElfShaderOffsets[i].sizeOfCode] = '\0';

        newProgram = replaceInBlock(tmpProgram, gsevoElf, gsevoElfCount, "", "");

        newProgram = replaceInBlock(newProgram, gsevoElfMesa, gsevoElfCountMesa, "", "");
        newProgram = replaceInBlock(newProgram, gsevoSet1, gsevoSetCount1, "", "");

        int newProgramLen = strlen(newProgram);

        newProgram[newProgramLen] = '\0';

        if (newProgramLen > gsevoElfShaderOffsets[i].totalSize)
        {
            printf("Error, size of modded program is bigger than the space reserved.");
            exit(1);
        }

        memcpy((void *)(size_t)gsevoElfShaderOffsets[i].startOffset, newProgram, newProgramLen);
    }
}

/**
 * This function cleans the text in the SRTV elf shaders
 * so they fit back in the elf after being patched.
 */
void cleanShaderString(char *str)
{
    char *src = str;
    char *dst = str;
    int spaceCount = 0;

    while (*src != '\0')
    {
        if (*src != '\t')
        {
            if (*src != ' ' || spaceCount == 0)
            {
                if ((*src == ' ' && *(src + 1) == ')') || (*src == ' ' && *(src + 1) == ',') ||
                    (*src == ' ' && *(src + 1) == '+') || (*src == ' ' && *(src + 1) == '='))
                    src++;
                *dst = *src;
                dst++;
                if ((*src == '(' && *(src + 1) == ' ') || (*src == ',' && *(src + 1) == ' ') ||
                    (*src == '+' && *(src + 1) == ' ') || (*src == '=' && *(src + 1) == ' '))
                    src++;
                spaceCount = (*src == ' ') ? spaceCount + 1 : 0;
            }
        }
        else
        {
            if (*(src + 1) != '\t' && (*(src + 1) > 32 || *(src + 1) < 126) && *(src + 1) != '=' && *(src + 1) != '\n')
            {
                *dst = *src;
                dst++;
            }
        }
        src++;
    }
    *dst = '\0';
}

/**
 * This function patches the shaders inside the ELF in Sega Race TV.
 */
void srtvElfShaderPatcher()
{
    int prot = mprotect((void *)0x854e000, 0x104000, PROT_EXEC | PROT_WRITE);
    if (prot != 0)
    {
        printf("Error: Cannot unprotect memory region to change variable (%d)\n", prot);
        exit(1);
    }
    for (int i = 0; i < srtvShaderOffsetsCount; i++)
    {
        char tmpProgram[10000];
        char *newProgram;
        memcpy(tmpProgram, (void *)(size_t)srtvElfShaderOffsets[i].startOffset, srtvElfShaderOffsets[i].sizeOfCode);
        memset((void *)(size_t)srtvElfShaderOffsets[i].startOffset, '\0', srtvElfShaderOffsets[i].totalSize);
        tmpProgram[srtvElfShaderOffsets[i].sizeOfCode] = '\0';

        newProgram = replaceInBlock(tmpProgram, srtvElf, srtvElfCount, "", "");

        char firstWord[8];
        memcpy(firstWord, newProgram, 7);
        firstWord[7] = '\0';

        if (((strcmp(firstWord, "uniform") != 0) && (strcmp(firstWord, "void ma") != 0)) &&
            (getConfig()->GPUVendor != NVIDIA_GPU))
        {
            memmove(newProgram + 13, newProgram, strlen(newProgram) + 1);
            memcpy(newProgram, "#version 120\n", 13);
        }

        cleanShaderString(newProgram);

        newProgram = replaceInBlock(newProgram, srtvElf2, srtvElfCount2, "", "");

        int newProgramLen = strlen(newProgram);

        newProgram[newProgramLen] = '\0';

        memcpy((void *)(size_t)srtvElfShaderOffsets[i].startOffset, newProgram, newProgramLen);
    }
}

/**
 *   We check if the shader being loaded is one of the ones in the list of files
 * we need to patch
 */
bool shaderFileInList(const char *pathname, int *idx)
{
    char cwd[256];
    int gpuVendor = getConfig()->GPUVendor;
    switch (gId)
    {
    case GHOST_SQUAD_EVOLUTION:
        if (getConfig()->GPUVendor != NVIDIA_GPU)
        {
            for (int x = 0; x < gsevoShaderPatchesCount; x++)
            {
                if ((strcmp(gsevoMesaShaderPatches[x].fileName, pathname) == 0) && (gsevoMesaShaderPatches[x].shaderBufferSize != 0))
                {
                    *idx = x;
                    return true;
                }
            }
            return false;
        }
        else
        {
            for (int x = 0; x < gsevoShaderPatchesCount; x++)
            {
                if ((strcmp(gsevoNvidiaShaderPatches[x].fileName, pathname) == 0) && (gsevoNvidiaShaderPatches[x].shaderBufferSize != 0))
                {
                    *idx = x;
                    return true;
                }
            }
            return false;
        }
        break;
    case RAMBO:
        if (getConfig()->GPUVendor != NVIDIA_GPU)
        {
            for (int x = 0; x < ramboMesaShaderPatchesCount; x++)
            {
                if ((strcmp(ramboMesaShaderPatches[x].fileName, basename((char *)pathname)) == 0) &&
                    (ramboMesaShaderPatches[x].shaderBufferSize != 0))
                {
                    *idx = x;
                    return true;
                }
            }
            return false;
        }
        else
        {
            for (int x = 0; x < ramboNvidiaShaderPatchesCount; x++)
            {
                if ((strcmp(ramboNvidiaShaderPatches[x].fileName, basename((char *)pathname)) == 0) &&
                    (ramboNvidiaShaderPatches[x].shaderBufferSize != 0))
                {
                    *idx = x;
                    return true;
                }
            }
            return false;
        }
    case THE_HOUSE_OF_THE_DEAD_4_REVA:
    case THE_HOUSE_OF_THE_DEAD_4_REVB:
    case THE_HOUSE_OF_THE_DEAD_4_REVC:
        for (int x = 0; x < hod4ShaderPatchesCount; x++)
        {
            if ((strcmp(hod4ShaderPatches[x].fileName, basename((char *)pathname)) == 0) && (hod4ShaderPatches[x].shaderBufferSize != 0))
            {
                *idx = x;
                return true;
            }
        }
        return false;
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL:
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB:
        for (int x = 0; x < hod4spShaderPatchesCount; x++)
        {
            if ((strcmp(hod4spShaderPatches[x].fileName, basename((char *)pathname)) == 0) &&
                (hod4spShaderPatches[x].shaderBufferSize != 0))
            {
                *idx = x;
                return true;
            }
        }
        return false;
    case HARLEY_DAVIDSON:
        for (int x = 0; x < harleyShaderPatchesCount; x++)
        {
            if ((strcmp(harleyShaderPatches[x].fileName, basename((char *)pathname)) == 0) &&
                (harleyShaderPatches[x].shaderBufferSize != 0))
            {
                *idx = x;
                return true;
            }
        }
        return false;
    case PRIMEVAL_HUNT:
        for (int x = 0; x < phShaderPatchesCount; x++)
        {
            if ((strcmp(basename((char *)phShaderPatches[x].fileName), basename((char *)pathname)) == 0) &&
                (phShaderPatches[x].shaderBufferSize != 0))
            {
                *idx = x;
                return true;
            }
        }
        return false;
    case HUMMER:
    case HUMMER_SDLX:
    case HUMMER_EXTREME:
    case HUMMER_EXTREME_MDX:
        for (int x = 0; x < hummerFilesToModCount; x++)
        {
            if ((strcmp(basename((char *)hummerShaderFilesToMod[x].fileName), basename((char *)pathname)) == 0) &&
                (hummerShaderFilesToMod[x].shaderBufferSize != 0))
            {
                *idx = x;
                return true;
            }
        }
        return false;
    case TOO_SPICY:
        if (gpuVendor != NVIDIA_GPU)
        {
            for (int x = 0; x < tooSpicyShaderPatchesCount; x++)
            {
                if ((strcmp(tooSpicyShaderPatches[x].fileName, basename((char *)pathname)) == 0) &&
                    (tooSpicyShaderPatches[x].shaderBufferSize != 0))
                {
                    *idx = x;
                    return true;
                }
            }
        }
        return false;
    case THE_HOUSE_OF_THE_DEAD_EX:
        if (gpuVendor != NVIDIA_GPU)
        {
            for (int x = 0; x < hodexShaderPatchesCount; x++)
            {
                if ((strcmp(hodexShaderPatches[x].fileName, basename((char *)pathname)) == 0) &&
                    (hodexShaderPatches[x].shaderBufferSize != 0))
                {
                    *idx = x;
                    return true;
                }
            }
        }
        return false;
    }
    const char *fName = strrchr(pathname, '/');
    char searchFileName[40];
    char *searchFolder1;
    char *searchFolder2;
    char *format;
    ShaderFilesToMod *filesToMod;
    int filesToModCount;

    switch (gId)
    {
    case LETS_GO_JUNGLE:
    case LETS_GO_JUNGLE_REVA:
    case LETS_GO_JUNGLE_SPECIAL:
        if (gpuVendor == NVIDIA_GPU && getConfig()->lgjRenderWithMesa != 1)
            return false;
        filesToMod = lgjShaderFilesToMod;
        filesToModCount = lgjFilesToModCount;
        searchFolder1 = "/shader/Cg";
        searchFolder2 = "/extraShader/Cg";
        format = "%s/inc%s";
        break;
    case INITIALD_4_EXP_REVB:
    case INITIALD_4_EXP_REVC:
    case INITIALD_4_EXP_REVD:
    case INITIALD_4_REVA:
    case INITIALD_4_REVB:
    case INITIALD_4_REVC:
    case INITIALD_4_REVD:
    case INITIALD_4_REVG:
    case INITIALD_5_JAP_REVA:
    case INITIALD_5_JAP_REVF:
    case INITIALD_5_EXP:
    case INITIALD_5_EXP_20:
    case INITIALD_5_EXP_20A:
        if (gpuVendor == NVIDIA_GPU)
            return false;

        // Set default values
        filesToMod = idShaderFilesToMod;
        filesToModCount = idFilesToModCount;
        searchFolder1 = "/shader/Cg/inc";
        searchFolder2 = "/data/Shader";
        format = "%s%s";

        switch (gId)
        {
        case INITIALD_4_REVA:
        case INITIALD_4_REVB:
        case INITIALD_4_REVC:
        case INITIALD_4_REVD:
        case INITIALD_4_REVG:
            filesToMod = id4jShaderFilesToMod;
            filesToModCount = id4FilesToModCount;
            break;
        }

        switch (gId)
        {
        case INITIALD_5_JAP_REVA:
        case INITIALD_5_JAP_REVF:
        case INITIALD_5_EXP:
        case INITIALD_5_EXP_20:
        case INITIALD_5_EXP_20A:
            searchFolder2 = "/data/V5SHADER";
            break;
        }
        break;
    case VIRTUA_TENNIS_3:
    case VIRTUA_TENNIS_3_TEST:
    case VIRTUA_TENNIS_3_REVA:
    case VIRTUA_TENNIS_3_REVA_TEST:
    case VIRTUA_TENNIS_3_REVB:
    case VIRTUA_TENNIS_3_REVB_TEST:
    case VIRTUA_TENNIS_3_REVC:
    case VIRTUA_TENNIS_3_REVC_TEST:
        if (gpuVendor == NVIDIA_GPU)
            return false;

        filesToMod = vt3ShaderFilesToMod;
        filesToModCount = vt3FilesToModCount;
        searchFolder1 = "/shader/Cg/inc";
        searchFolder2 = "/shader";
        format = "%s%s";
    }

    if (strstr(pathname, searchFolder1) != NULL)
    {
        sprintf((void *)searchFileName, format, searchFolder1, fName);
    }
    else if (strstr(pathname, searchFolder2) != NULL)
    {
        sprintf((void *)searchFileName, format, searchFolder2, fName);
    }
    else
    {
        return false;
    }

    for (int x = 0; x < filesToModCount; x++)
    {
        if (strcmp(filesToMod[x].fileName, searchFileName) == 0)
        {
            *idx = x;
            return true;
        }
    }
    return false;
}

/**
 *   We return the size of the shader we will use to replace the original
 * tricking ftell.
 */
long int ftellGetShaderSize(int idx)
{
    switch (gId)
    {
    case GHOST_SQUAD_EVOLUTION:
        return getConfig()->GPUVendor != NVIDIA_GPU ? gsevoMesaShaderPatches[idx].shaderBufferSize
                                                    : gsevoNvidiaShaderPatches[idx].shaderBufferSize;

    case LETS_GO_JUNGLE:
    case LETS_GO_JUNGLE_REVA:
    case LETS_GO_JUNGLE_SPECIAL:
        return lgjShaderFilesToMod[idx].shaderBufferSize;

    case INITIALD_4_EXP_REVB:
    case INITIALD_4_EXP_REVC:
    case INITIALD_4_EXP_REVD:
    case INITIALD_5_JAP_REVA:
    case INITIALD_5_JAP_REVF:
    case INITIALD_5_EXP:
    case INITIALD_5_EXP_20:
    case INITIALD_5_EXP_20A:
        return idShaderFilesToMod[idx].shaderBufferSize;

    case INITIALD_4_REVA:
    case INITIALD_4_REVB:
    case INITIALD_4_REVC:
    case INITIALD_4_REVD:
    case INITIALD_4_REVG:
        return id4jShaderFilesToMod[idx].shaderBufferSize;

    case VIRTUA_TENNIS_3:
    case VIRTUA_TENNIS_3_TEST:
    case VIRTUA_TENNIS_3_REVA:
    case VIRTUA_TENNIS_3_REVA_TEST:
    case VIRTUA_TENNIS_3_REVB:
    case VIRTUA_TENNIS_3_REVB_TEST:
    case VIRTUA_TENNIS_3_REVC:
    case VIRTUA_TENNIS_3_REVC_TEST:
        return vt3ShaderFilesToMod[idx].shaderBufferSize;

    case TOO_SPICY:
        return tooSpicyShaderPatches[idx].shaderBufferSize;

    case RAMBO:
        return getConfig()->GPUVendor != NVIDIA_GPU ? ramboMesaShaderPatches[idx].shaderBufferSize
                                                    : ramboNvidiaShaderPatches[idx].shaderBufferSize;

    case THE_HOUSE_OF_THE_DEAD_4_REVA:
    case THE_HOUSE_OF_THE_DEAD_4_REVB:
    case THE_HOUSE_OF_THE_DEAD_4_REVC:
        return hod4ShaderPatches[idx].shaderBufferSize;

    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL:
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB:
        return hod4spShaderPatches[idx].shaderBufferSize;
    case THE_HOUSE_OF_THE_DEAD_EX:
        return hodexShaderPatches[idx].shaderBufferSize;

    case HUMMER:
    case HUMMER_SDLX:
    case HUMMER_EXTREME:
    case HUMMER_EXTREME_MDX:
        return hummerShaderFilesToMod[idx].shaderBufferSize;
    case PRIMEVAL_HUNT:
        return phShaderPatches[idx].shaderBufferSize;
    case HARLEY_DAVIDSON:
        return harleyShaderPatches[idx].shaderBufferSize;
    }
    return -1;
}

/**
 *   We replace the file being loaded by fread and insted, we fill the buffer
 * with the data we want.
 */
size_t freadReplace(void *buf, size_t size, size_t count, int idx)
{
    switch (gId)
    {
    case GHOST_SQUAD_EVOLUTION:
        if (getConfig()->GPUVendor != NVIDIA_GPU)
        {
            memcpy(buf, gsevoMesaShaderPatches[idx].shaderBuffer, gsevoMesaShaderPatches[idx].shaderBufferSize);
        }
        else
        {
            memcpy(buf, gsevoNvidiaShaderPatches[idx].shaderBuffer, gsevoNvidiaShaderPatches[idx].shaderBufferSize);
        }
        return 1;

    case LETS_GO_JUNGLE:
    case LETS_GO_JUNGLE_REVA:
    case LETS_GO_JUNGLE_SPECIAL:
        memcpy(buf, lgjShaderFilesToMod[idx].shaderBuffer, size);
        return size;

    case INITIALD_4_EXP_REVB:
    case INITIALD_4_EXP_REVC:
    case INITIALD_4_EXP_REVD:
    case INITIALD_5_JAP_REVA:
    case INITIALD_5_JAP_REVF:
    case INITIALD_5_EXP:
    case INITIALD_5_EXP_20:
    case INITIALD_5_EXP_20A:
        memcpy(buf, idShaderFilesToMod[idx].shaderBuffer, size);
        return size;

    case INITIALD_4_REVA:
    case INITIALD_4_REVB:
    case INITIALD_4_REVC:
    case INITIALD_4_REVD:
    case INITIALD_4_REVG:
        memcpy(buf, id4jShaderFilesToMod[idx].shaderBuffer, size);
        return size;

    case VIRTUA_TENNIS_3:
    case VIRTUA_TENNIS_3_TEST:
    case VIRTUA_TENNIS_3_REVA:
    case VIRTUA_TENNIS_3_REVA_TEST:
    case VIRTUA_TENNIS_3_REVB:
    case VIRTUA_TENNIS_3_REVB_TEST:
    case VIRTUA_TENNIS_3_REVC:
    case VIRTUA_TENNIS_3_REVC_TEST:
        memcpy(buf, vt3ShaderFilesToMod[idx].shaderBuffer, size);
        return size;

    case TOO_SPICY:
        memcpy(buf, tooSpicyShaderPatches[idx].shaderBuffer, tooSpicyShaderPatches[idx].shaderBufferSize);
        return tooSpicyShaderPatches[idx].shaderBufferSize;

    case RAMBO:
        if (getConfig()->GPUVendor != NVIDIA_GPU)
        {
            memcpy(buf, ramboMesaShaderPatches[idx].shaderBuffer, ramboMesaShaderPatches[idx].shaderBufferSize);
            return ramboMesaShaderPatches[idx].shaderBufferSize;
        }
        else
        {
            memcpy(buf, ramboNvidiaShaderPatches[idx].shaderBuffer, ramboNvidiaShaderPatches[idx].shaderBufferSize);
            return ramboNvidiaShaderPatches[idx].shaderBufferSize;
        }

    case THE_HOUSE_OF_THE_DEAD_4_REVA:
    case THE_HOUSE_OF_THE_DEAD_4_REVB:
    case THE_HOUSE_OF_THE_DEAD_4_REVC:
        memcpy(buf, hod4ShaderPatches[idx].shaderBuffer, hod4ShaderPatches[idx].shaderBufferSize);
        return hod4ShaderPatches[idx].shaderBufferSize;

    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL:
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB:
        memcpy(buf, hod4spShaderPatches[idx].shaderBuffer, hod4spShaderPatches[idx].shaderBufferSize);
        return hod4spShaderPatches[idx].shaderBufferSize;

    case THE_HOUSE_OF_THE_DEAD_EX:
        memcpy(buf, hodexShaderPatches[idx].shaderBuffer, hodexShaderPatches[idx].shaderBufferSize);
        return hodexShaderPatches[idx].shaderBufferSize;

    case HUMMER:
    case HUMMER_SDLX:
    case HUMMER_EXTREME:
    case HUMMER_EXTREME_MDX:
        memcpy(buf, hummerShaderFilesToMod[idx].shaderBuffer, hummerShaderFilesToMod[idx].shaderBufferSize);
        return hummerShaderFilesToMod[idx].shaderBufferSize;

    case PRIMEVAL_HUNT:
        memcpy(buf, phShaderPatches[idx].shaderBuffer, phShaderPatches[idx].shaderBufferSize);
        return phShaderPatches[idx].shaderBufferSize;

    case HARLEY_DAVIDSON:
        memcpy(buf, harleyShaderPatches[idx].shaderBuffer, harleyShaderPatches[idx].shaderBufferSize);
        return harleyShaderPatches[idx].shaderBufferSize;

    default:
        return -1;
    }
}

/**
 *   Here we intercept cgGetProgramString because some games like ID4 or ID5 we
 * force the shaders to be recompiled and there is a problem that the game feeds
 * some empty shaders.So we cache a good shader and then feed that shader to the
 * game instead of an empty one.
 */
char *cgGetProgramString(char *program, int e)
{
    char *(*_cgGetProgramString)(char *program, int e) = dlsym(RTLD_NEXT, "cgGetProgramString");

    switch (gId)
    {
    case INITIALD_4_EXP_REVB:
    case INITIALD_4_EXP_REVC:
    case INITIALD_4_EXP_REVD:
    case INITIALD_4_REVA:
    case INITIALD_4_REVB:
    case INITIALD_4_REVC:
    case INITIALD_4_REVD:
    case INITIALD_4_REVG:
    case INITIALD_5_JAP_REVA:
    case INITIALD_5_JAP_REVF:
    case INITIALD_5_EXP:
    case INITIALD_5_EXP_20:
    case INITIALD_5_EXP_20A:
        break;
    default:
        return _cgGetProgramString(program, e);
    }

    char *prgstr = _cgGetProgramString(program, e);

    if ((prgstr == NULL) || (*prgstr == '\0'))
    {
        return tmpShader;
    }
    else
    {
        if (sprintf(tmpShader, "%s", prgstr) < 0)
        {
            printf("Falied to save shader for next empty one.\n");
        }
    }
    return prgstr;
}

/**
 *   We intercept glProgramStringARB to do some shader patching before the
 * shader gets loaded into the shader cache.
 */
void gl_ProgramStringARB(int target, int program_fmt, int program_len, char *program)
{
    char *newProgram;
    int newProgramLen;
    void *addr = __builtin_return_address(0);

    switch (gId)
    {
    case GHOST_SQUAD_EVOLUTION:
        if (addr == (void *)0x8179fab)
        {
            newProgram = replaceInBlock(program, gsevoSet1, gsevoSetCount1, "", "");
            int newProgramLen = strlen(newProgram);
            program = newProgram;
            program_len = newProgramLen;
        }
        break;
    case LETS_GO_JUNGLE:
    case LETS_GO_JUNGLE_REVA:
    case LETS_GO_JUNGLE_SPECIAL:
        newProgram = replaceInBlock(program, lgjCompiledShadersPatch, lgjCompiledShadersPatchCount, "", "");
        newProgramLen = strlen(newProgram);
        program = newProgram;
        program_len = newProgramLen;
        break;
    case AFTER_BURNER_CLIMAX:
    case AFTER_BURNER_CLIMAX_REVA:
    case AFTER_BURNER_CLIMAX_REVB:
    case AFTER_BURNER_CLIMAX_SDX:
    case AFTER_BURNER_CLIMAX_SDX_REVA:
    case AFTER_BURNER_CLIMAX_SE:
    case AFTER_BURNER_CLIMAX_SE_REVA:
        if (target == GL_VERTEX_PROGRAM_ARB)
        {
            newProgram = replaceInBlock(program, abcVsMesa, abcVsMesaCount, "", "");
        }
        else if (target == GL_FRAGMENT_PROGRAM_ARB)
        {
            newProgram = replaceInBlock(program, abcFsMesa, abcFsMesaCount, "", "");
        }
        newProgramLen = strlen(newProgram);
        program = newProgram;
        program_len = newProgramLen;
        break;
    case OUTRUN_2_SP_SDX:
    case OUTRUN_2_SP_SDX_REVA:
        if (target == GL_VERTEX_PROGRAM_ARB)
        {
            if (getConfig()->GPUVendor == NVIDIA_GPU)
                newProgram = program;
            else if (getConfig()->GPUVendor == ATI_GPU)
                newProgram = replaceInBlock(program, orVsAti, orVsAtiCount, "", "");
            else
                newProgram = replaceInBlock(program, orVsMesa, orVsMesaCount, "", "");
        }
        else if (target == GL_FRAGMENT_PROGRAM_ARB)
        {
            if (getConfig()->GPUVendor == NVIDIA_GPU || getConfig()->GPUVendor == ATI_GPU)
                newProgram = replaceInBlock(program, orFsNvidia, orFsNvidiaCount, "", "");
            else
                newProgram = replaceInBlock(program, orFsMesa, orFsMesaCount, "", "");
        }
        newProgramLen = strlen(newProgram);
        program = newProgram;
        program_len = newProgramLen;
        break;
    case VIRTUA_FIGHTER_5:
    case VIRTUA_FIGHTER_5_REVA:
    case VIRTUA_FIGHTER_5_REVB:
    case VIRTUA_FIGHTER_5_REVE:
    case VIRTUA_FIGHTER_5_EXPORT:
    case VIRTUA_FIGHTER_5_R:
    case VIRTUA_FIGHTER_5_R_REVD:
    case VIRTUA_FIGHTER_5_R_REVG:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000:
        if (target == GL_VERTEX_PROGRAM_ARB)
        {
            if (getConfig()->GPUVendor == ATI_GPU)
            {
                newProgram = replaceInBlock(program, vf5VsAti, vf5VsAtiCount, "", "");
                if (gId == VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA || gId == VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB ||
                    gId == VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000)
                {
                    newProgram = replaceInBlock(newProgram, vf5VsAtifs, vf5VsAtifsCount, "", "");
                }
            }
            else
            {
                newProgram = replaceInBlock(program, vf5VsMesa, vf5VsMesaCount, "", "");
            }
        }
        else if (target == GL_FRAGMENT_PROGRAM_ARB)
        {
            if (getConfig()->GPUVendor == ATI_GPU)
                newProgram = replaceInBlock(program, vf5FsAti, vf5FsAtiCount, "", "");
            else
                newProgram = replaceInBlock(program, vf5FsMesa, vf5FsMesaCount, "", "");
        }
        newProgramLen = strlen(newProgram);
        program = newProgram;
        program_len = newProgramLen;
        break;
    case R_TUNED:
        if (target == GL_VERTEX_PROGRAM_ARB)
        {
            newProgram = replaceInBlock(program, rtunedVsMesa, rtunedVsMesaCount, "", "");
        }
        else if (target == GL_FRAGMENT_PROGRAM_ARB)
        {
            newProgram = replaceInBlock(program, rtunedFsMesa, rtunedFsMesaCount, "", "");
        }
        newProgramLen = strlen(newProgram);
        program = newProgram;
        program_len = newProgramLen;
        break;
    case MJ4_REVG:
    case MJ4_EVO:
        if (target == GL_VERTEX_PROGRAM_ARB)
        {
            newProgram = replaceInBlock(program, mj4VsMesa, mj4VsMesaCount, "", "");
        }
        else if (target == GL_FRAGMENT_PROGRAM_ARB)
        {
            newProgram = replaceInBlock(program, mj4FsMesa, mj4FsMesaCount, "", "");
        }
        newProgramLen = strlen(newProgram);
        program = newProgram;
        program_len = newProgramLen;
    }
    glProgramStringARB(target, program_fmt, program_len, program);

    switch (gId)
    {
    case VIRTUA_FIGHTER_5:
    case VIRTUA_FIGHTER_5_REVA:
    case VIRTUA_FIGHTER_5_REVB:
    case VIRTUA_FIGHTER_5_REVE:
    case VIRTUA_FIGHTER_5_EXPORT:
    case VIRTUA_FIGHTER_5_R:
    case VIRTUA_FIGHTER_5_R_REVD:
    case VIRTUA_FIGHTER_5_R_REVG:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000:
        if (getConfig()->GPUVendor == ATI_GPU)
        {
            int err = glGetError();
            if (err == 1282)
            {
                char *newProgram;
                newProgram = replaceInBlock(program, vf5VsAti2, vf5VsAtiCount2, "", "");
                int newProgramLen = strlen(newProgram);
                program = newProgram;
                program_len = newProgramLen;
                glProgramStringARB(target, program_fmt, program_len, program);
            }
        }
    }

    if ((getConfig()->showDebugMessages))
    {
        GLint result;
        result = glGetError();
        if (result != GL_NO_ERROR)
        {
            fprintf(stderr, "Error: glProgramStringARB failed with error code %d.\n", result);
            //   Dump shader with error
            char filename[512] = {0};
            char path[256] = {0};
            int nchar = readlink("/proc/self/exe", path, 256 - 1);
            if (nchar != -1)
                path[nchar] = '\0';
            else
                exit(1);
            char *dir_only = dirname(path);
            FILE *f;
            sprintf(filename, "%s/Error-%d.prg", dir_only, idxError);
            // sprintf(filename, "%s/%s", dir_only, fname);
            printf("Size: %d\n", program_len);
            f = fopen(filename, "w+");
            fwrite(program, sizeof(char), program_len, f);
            fclose(f);
            idxError++;
        }
        GLint errorPos;
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

        if (errorPos != -1)
        {
            const GLubyte *errorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
            if (errorString)
            {
                fprintf(stderr, "Program error at position %d: %s\n", errorPos, errorString);
            }
            else
            {
                fprintf(stderr, "Program error at position %d: Unknown error\n", errorPos);
            }
        }
    }
    return;
}

/**
 *   We use this to show error messages in the shader compilation.
 */
void gl_GetObjectParameterivARB(GLhandleARB obj, GLenum pname, GLint *params)
{
    glGetObjectParameterivARB(obj, pname, params);

    if (getConfig()->showDebugMessages == 1)
    {
        GLchar infoLog[512];

        if ((int)*params <= 0)
        {
            glGetShaderInfoLog(obj, 512, NULL, infoLog);
            printf("%s", infoLog);
        }
    }
}

/**
 *   We intercept cgCreateProgram to force the CG shader compiler to use
 * different args depending on the shader being Vertex or Fragment.
 */
char *cgCreateProgram(uint32_t context, int program_type, const char *program, int profile, const char *entry,
                      const char **args)
{
    char *(*_cgCreateProgram)(uint32_t context, int program_type, const char *program, int profile, const char *entry,
                              const char **args) = dlsym(RTLD_NEXT, "cgCreateProgram");

    if (getConfig()->GPUVendor == NVIDIA_GPU)
    {
        if ((gId != LETS_GO_JUNGLE && gId != LETS_GO_JUNGLE_REVA && gId != LETS_GO_JUNGLE_SPECIAL) || getConfig()->lgjRenderWithMesa == 0)
            return _cgCreateProgram(context, program_type, program, profile, entry, args);
    }

    switch (gId)
    {
    case INITIALD_4_EXP_REVB:
    case INITIALD_4_EXP_REVC:
    case INITIALD_4_EXP_REVD:
    case INITIALD_4_REVA:
    case INITIALD_4_REVB:
    case INITIALD_4_REVC:
    case INITIALD_4_REVD:
    case INITIALD_4_REVG:
    case INITIALD_5_JAP_REVA:
    case INITIALD_5_JAP_REVF:
    case INITIALD_5_EXP:
    case INITIALD_5_EXP_20:
    case INITIALD_5_EXP_20A:
        if ((profile == 6148) || (profile == 6150) || profile == 7001)
            profile = 6150;
        else if ((profile == 6149) || (profile == 6151) || profile == 7000)
            profile = 7000;
        else
            printf("profile %d not supported\n", profile);
        break;
    }

    const char *op6150[] = {"NumTemps=31", "MaxInstructions=1024", "MaxAddressRegs=1", "MaxLocalParams=256", NULL};

    const char *op7000[] = {"NumTemps=256",
                            "NumInstructionSlots=1447",
                            "MaxLocalParams=256",
                            "NumTexInstructionSlots=1447",
                            "NumMathInstructionSlots=1447",
                            "MaxTexIndirections=128",
                            "MaxDrawBuffers=8",
                            NULL};

    if (profile == 6150)
    {
        args = op6150;
    }
    else if (profile == 7000)
    {
        args = op7000;
    }
    return _cgCreateProgram(context, program_type, program, profile, entry, args);
}

void gl_ShaderSourceARB(GLhandleARB shaderObj, GLsizei count, const GLcharARB **const string, const GLint *length)
{
    switch (gId)
    {
    case SEGA_RACE_TV:
    case HUMMER:
    case HUMMER_SDLX:
    case HUMMER_EXTREME:
    case HUMMER_EXTREME_MDX:
        break;
    default:
        glShaderSourceARB(shaderObj, count, string, length);
        return;
    }

    char *newProgram = malloc(250000);
    switch (gId)
    {
    case SEGA_RACE_TV:
    {
        void *addr = __builtin_return_address(0);
        if (addr <= (void *)0x82fd823)
        {
            newProgram = strdup(*string);
        }
        else if (addr == (void *)0x84bbff1)
        {
            if (getConfig()->GPUVendor != NVIDIA_GPU)
            {
                sprintf(newProgram, "#version 120\r\n%s", *string);
                newProgram = replaceInBlock(newProgram, srtvVsMesa, srtvVsMesaCount, "", "");
            }
            else
            {
                sprintf(newProgram, "%s", *string);
                newProgram = replaceInBlock(newProgram, srtvVsNvidia, srtvVsNvidiaCount, "", "");
            }
        }
        else if (addr == (void *)0x084bc06b)
        {
            if (getConfig()->GPUVendor != NVIDIA_GPU)
            {
                sprintf(newProgram, "#version 120\r\n%s", *string);
                newProgram = replaceInBlock(newProgram, srtvFsMesa, srtvFsMesaCount, "", "");
            }
            else
            {
                newProgram = strdup(*string);
            }
        }
    }
    break;
    case HUMMER:
    case HUMMER_SDLX:
    case HUMMER_EXTREME:
    case HUMMER_EXTREME_MDX:
    {
        void *addr = __builtin_return_address(0);
        if ((addr != (void *)0x830a501) && (addr != (void *)0x830a485) && (addr != (void *)0x08319841) && (addr != (void *)0x083197c5) &&
            (addr != (void *)0x83af939) && (addr != (void *)0x83af9b3) && (addr != (void *)0x83b0515) && (addr != (void *)0x83b058f))
        {
            sprintf(newProgram, "%s", hummerShaderFilesToMod[hummerExtremeShaderFileIndex].shaderBuffer);
        }
        else
        {
            glShaderSourceARB(shaderObj, count, string, length);
            return;
        }
    }
    break;
    default:
        glShaderSourceARB(shaderObj, count, string, length);
        return;
    }

    const char *newString = malloc(250000);
    memset((void *)newString, '\0', 250000);
    newString = strdup(newProgram);
    if (length != NULL)
    {
        const int newLength = hummerShaderFilesToMod[hummerExtremeShaderFileIndex].shaderBufferSize;
        glShaderSourceARB(shaderObj, count, &newString, &newLength);
    }
    else
    {
        glShaderSourceARB(shaderObj, count, &newString, length);
    }
    free((void *)newString);
    free(newProgram);
}

/**
 * This function intercepts glXGetProcAddressARB
 * to point non existing functions to existing functions
 * or replacements in modern systems.
 */
void *gl_XGetProcAddressARB(const unsigned char *procName)
{
    int dbgMsg = getConfig()->showDebugMessages;
    if (gId != HUMMER && gId != HUMMER_SDLX && gId != HUMMER_EXTREME && gId != HUMMER_EXTREME_MDX)
    {
        if (strcmp((const char *)procName, "glGenOcclusionQueriesNV") == 0)
        {
            if (dbgMsg)
            {
                printf("glGenOcclusionQueriesNV Intercepted.\n");
            }
            return (void *)glGenQueriesARB;
        }
        else if (strcmp((const char *)procName, "glDeleteOcclusionQueriesNV") == 0)
        {
            if (dbgMsg)
            {
                printf("glDeleteOcclusionQueriesNV Intercepted.\n");
            }
            return (void *)glDeleteQueriesARB;
        }
        else if (strcmp((const char *)procName, "glIsOcclusionQueryNV") == 0)
        {
            if (dbgMsg)
            {
                printf("glIsOcclusionQueryNV Intercepted.\n");
            }
            return (void *)glIsQueryARB;
        }
        else if (strcmp((const char *)procName, "glBeginOcclusionQueryNV") == 0)
        {
            if (dbgMsg)
            {
                printf("glBeginOcclusionQueryNV Intercepted.\n");
            }
            return (void *)gl_BeginOcclusionQueryNV;
        }
        else if (strcmp((const char *)procName, "glEndOcclusionQueryNV") == 0)
        {
            if (dbgMsg)
            {
                printf("glEndOcclusionQueryNV Intercepted.\n");
            }
            return (void *)gl_EndOcclusionQueryNV;
        }
        else if (strcmp((const char *)procName, "glGetOcclusionQueryivNV") == 0)
        {
            if (dbgMsg)
            {
                printf("glGetOcclusionQueryivNV Intercepted.\n");
            }
            return (void *)glGetQueryObjectuivARB;
        }
        else if (strcmp((const char *)procName, "glGetOcclusionQueryuivNV") == 0)
        {
            if (dbgMsg)
            {
                printf("glGetOcclusionQueryuivNV Intercepted.\n");
            }
            return (void *)glGenQueriesARB;
        }
        else if (strcmp((const char *)procName, "glBeginConditionalRenderNVX") == 0)
        {
            if (dbgMsg)
            {
                printf("glBeginConditionalRenderNVX Intercepted.\n");
            }
            return (void *)gl_BeginConditionalRenderNVX;
        }
        else if (strcmp((const char *)procName, "glEndConditionalRenderNVX") == 0)
        {
            if (dbgMsg)
            {
                printf("glEndConditionalRenderNVX Intercepted.\n");
            }
            return (void *)glEndConditionalRender;
        }
        if (strcmp((const char *)procName, "glProgramParameters4fvNVARB\0") == 0)
        {
            if (dbgMsg)
            {
                printf("glProgramParameters4fvNVARB Intercepted.\n");
            }
            return (void *)glProgramEnvParameters4fvEXT; // glProgramParameters4fvNV;
        }
        else if ((strcmp((const char *)procName, "glProgramEnvParameters4fvNVARB\0") == 0) ||
                 (strcmp((const char *)procName, "glProgramEnvParameters4fvARB\0") == 0))
        {
            if (dbgMsg)
            {
                printf("glProgramEnvParameters4fvNVARB Intercepted.\n");
            }
            return (void *)glProgramEnvParameters4fvEXT;
        }
        else if ((strcmp((const char *)procName, "glProgramLocalParameters4fvNVARB\0") == 0) ||
                 (strcmp((const char *)procName, "glProgramLocalParameters4fvARB\0") == 0))
        {
            if (dbgMsg)
            {
                printf("glProgramLocalParameters4fvNVARB Intercepted.\n");
            }
            return (void *)glProgramLocalParameters4fvEXT;
        }
        else if (strcmp((const char *)procName, "glGetObjectParameterivARB\0") == 0)
        {
            if (dbgMsg)
            {
                printf("glGetObjectParameterivARB Intercepted.\n");
            }
            return (void *)gl_GetObjectParameterivARB;
        }
    }
    if (strcmp((const char *)procName, "glShaderSourceARB\0") == 0)
    {
        if (dbgMsg)
        {
            printf("glShaderSourceARB Intercepted.\n");
        }
        return (void *)gl_ShaderSourceARB;
    }

    return glXGetProcAddressARB(procName);
}

/**
 *   Functions for Outrun2
 */
void gl_BindProgramNV(GLenum target, GLuint id)
{
    if (target == GL_FRAGMENT_PROGRAM_NV)
    {
        target = GL_FRAGMENT_PROGRAM_ARB;
    }
    glBindProgramARB(target, id);
}

void gl_LoadProgramNV(GLenum target, GLuint program, GLsizei len, char *string)
{
    gl_BindProgramNV(target, program);
    gl_ProgramStringARB(target, GL_PROGRAM_FORMAT_ASCII_ARB, len, string);
}

void gl_DeleteProgramsNV(GLsizei n, const GLuint *programs)
{
    glDeleteProgramsARB(n, programs);
}

/**
 *   This function hook fixed the LOD (Level of Detail) for VF5 textures with Mesa
 */
void glBindTexture(GLenum target, GLuint texture)
{
    void (*_glBindTexture)(GLenum target, GLuint texture) = dlsym(RTLD_NEXT, "glBindTexture");
    _glBindTexture(target, texture);
    switch (gId)
    {
    case VIRTUA_FIGHTER_5:
    case VIRTUA_FIGHTER_5_REVA:
    case VIRTUA_FIGHTER_5_REVB:
    case VIRTUA_FIGHTER_5_REVE:
    case VIRTUA_FIGHTER_5_EXPORT:
    case VIRTUA_FIGHTER_5_R:
    case VIRTUA_FIGHTER_5_R_REVD:
    case VIRTUA_FIGHTER_5_R_REVG:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000:
        if (getConfig()->GPUVendor != NVIDIA_GPU)
        {
            glTexParameterf(target, GL_TEXTURE_LOD_BIAS, -10.0f); // Adjust LOD bias
        }
        break;
    default:
        break;
    }
}

int parseCgcArgs(const char *input, const char ***compilerArgs, const char **outputFileName)
{
    char *token = strtok(strdup(input), " ");
    char **args = malloc(256 * sizeof(char *));
    int count = 0;
    char *profile;

    while (token != NULL)
    {
        args[count++] = token;
        token = strtok(NULL, " ");
    }

    *compilerArgs = malloc((count * sizeof(char *)) + 3);
    (*compilerArgs)[0] = "-oglsl";
    (*compilerArgs)[1] = "-nowarn=7506,7529";
    (*compilerArgs)[2] = "MaxLocalParams=256";
    int argIndex = 3;

    for (int i = 0; i < count; i++)
    {
        if (strcmp(args[i], "-o") == 0 && i + 1 < count)
        {
            *outputFileName = args[++i];
        }
        if (strcmp(args[i], "-profile") == 0 && i + 1 < count)
        {
            profile = args[++i];
        }
        else if (strncmp(args[i], "-D", 2) == 0 || strncmp(args[i], "-I", 2) == 0)
        {
            (*compilerArgs)[argIndex++] = args[i];
        }
    }
    (*compilerArgs)[argIndex] = NULL;
    free(args);
    if (strcmp(profile, "vp40") == 0)
    {
        return 6150;
    }
    else if (strcmp(profile, "fp40") == 0)
    {
        return 7000;
    }
    return 0;
}

char *findLibCg()
{
    const char *foundPath = NULL;
    FILE *libCgF = NULL;

    char appImageLib[MAX_PATH_LENGTH];
    snprintf(appImageLib, MAX_PATH_LENGTH, "%s/usr/lib32/libCg2.so", getenv("APP_IMG_ROOT"));
    char *pathsToCheck[] = {NULL, "/app/lib32/libCg2.so", appImageLib, "./libCg.so", NULL};

    if (strcmp(getConfig()->libCgPath, "") != 0)
    {
        pathsToCheck[0] = getConfig()->libCgPath;
    }

    const char *currentDir = getenv("LINDBERGH_LOADER_CURRENT_DIR");
    if (currentDir != NULL)
    {
        size_t pathLen = strlen(currentDir) + strlen("/libCg.so") + 1;
        pathsToCheck[3] = malloc(pathLen);
        if (pathsToCheck[4] != NULL)
        {
            snprintf(pathsToCheck[4], pathLen, "%s/%s", currentDir, "libCg.so");
        }
    }

    for (int i = 0; i < 5; ++i)
    {
        if (pathsToCheck[i] == NULL)
            continue;

        if (access(pathsToCheck[i], F_OK) == 0)
        {
            foundPath = pathsToCheck[i];
            break;
        }
    }

    if (foundPath != pathsToCheck[4] && pathsToCheck[4] != NULL)
    {
        free(pathsToCheck[4]);
    }

    if (foundPath == NULL)
    {
        log_error("Error: Unable to find library libCg.so\n");
        log_error("Error: libCg.so version 2.0 is needed to compile the shaders in the game's folder.\n");
        exit(EXIT_FAILURE);
    }

    char libCgVersion[9] = {0};
    libCgF = fopen(foundPath, "rb");
    if (libCgF == NULL)
    {
        log_error("Failed to open: %s\n", foundPath);
        exit(EXIT_FAILURE);
    }

    fseek(libCgF, 0x226d84L, SEEK_SET);
    if (fread(libCgVersion, 1, sizeof(libCgVersion) - 1, libCgF) != sizeof(libCgVersion) - 1)
    {
        log_error("Failed to read version from: %s\n", foundPath);
        fclose(libCgF);
        exit(EXIT_FAILURE);
    }
    fclose(libCgF);

    if (strcmp(libCgVersion, "2.0.0.12") != 0)
    {
        log_error("ERROR: Wrong libCg.so version '%s', version 2.0.0.12 is needed.\n", libCgVersion);
        exit(EXIT_FAILURE);
    }

    char *resultPath = strdup(foundPath);
    if (resultPath == NULL)
    {
        log_error("Error allocating memory for result path");
        exit(EXIT_FAILURE);
    }

    return resultPath;
}

void loadLibCg()
{
    void *handle;
    char *libCgFilePath = findLibCg();

    handle = dlopen(libCgFilePath, RTLD_NOW);
    if (!handle)
    {
        log_error("Error: Unable to find library libCg.so\n");
        log_error("Error: libCg.so version 2.0 is needed to compile the shaders in the game's folder.\n");
        exit(EXIT_FAILURE);
    }
    free(libCgFilePath);
    dlerror();

    char *error;
    _cgCreateContext = (cgCreateContext_t)dlsym(handle, "cgCreateContext");
    error = dlerror();
    if (error != NULL)
    {
        fprintf(stderr, "Error: Unable to find function cgCreateContext: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    // Get the address of the cgCreateProgram function
    _cgCreateProgram = (cgCreateProgram_t)dlsym(handle, "cgCreateProgram");
    if (error != NULL)
    {
        fprintf(stderr, "Error: Unable to find function cgCreateProgram: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    // Get the address of the cgGetProgramString function
    _cgGetProgramString = (cgGetProgramString_t)dlsym(handle, "cgGetProgramString");
    if (error != NULL)
    {
        fprintf(stderr, "Error locating cgGetProgramString: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    // Get the address of the cgDestroyContext function
    _cgDestroyContext = (cgDestroyContext_t)dlsym(handle, "cgDestroyContext");
    if (error != NULL)
    {
        fprintf(stderr, "Error locating cgDestroyContext: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    // Get the address of the cgDestroyProgram function
    _cgDestroyProgram = (cgDestroyProgram_t)dlsym(handle, "cgDestroyProgram");
    if (error != NULL)
    {
        fprintf(stderr, "Error locating cgDestroyProgram: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }
}

void cacheNnstdshader()
{
    int s;
    FILE *f;
    f = fopen("../fs/shader/nnstdshader.vert", "r");
    if (f == NULL)
    {
        printf("Error opening nnstdshader.vert.\n");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    s = ftell(f);
    nnstdshader_vert = malloc(s + 1);
    memset(nnstdshader_vert, '\0', s + 1);
    fseek(f, 0, SEEK_SET);
    fread(nnstdshader_vert, 1, s, f);
    fclose(f);

    f = fopen("../fs/shader/nnstdshader.frag", "r");
    if (f == NULL)
    {
        printf("Error opening nnstdshader.frag.\n");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    s = ftell(f);
    nnstdshader_frag = malloc(s + 1);
    memset(nnstdshader_frag, '\0', s + 1);
    fseek(f, 0, SEEK_SET);
    fread(nnstdshader_frag, 1, s, f);
    fclose(f);
}

int compileWithCGC(char *command)
{
    const char **compilerArgs = NULL;
    const char *outputFileName = NULL;

    int profile = parseCgcArgs(command, &compilerArgs, &outputFileName);

    size_t *ctx = _cgCreateContext();

    char *program = NULL;
    const char *compiledSource;

    if (profile == 6150)
    {
        program = _cgCreateProgram(ctx, 4112, nnstdshader_vert, profile, "main", compilerArgs);
    }
    else if (profile == 7000)
    {
        program = _cgCreateProgram(ctx, 4112, nnstdshader_frag, profile, "main", compilerArgs);
    }
    else
    {
        printf("Error getting the profile.\n");
        exit(1);
    }

    compiledSource = _cgGetProgramString(program, 4106);

    FILE *outputFile = fopen(outputFileName, "w+");
    if (outputFile == NULL)
    {
        perror("Error opening output file");
        exit(1);
    }

    fprintf(outputFile, "%s", compiledSource);
    fclose(outputFile);

    _cgDestroyProgram(program);
    _cgDestroyContext(ctx);
    return 0;
}

int loadShader(char *path, char *buffer)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening %s.\n", path);
        exit(1);
    }
    int s = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, buffer, s);
    close(fd);
    buffer[s] = '\0';
    return s;
}

int xdlt_load_mmfile(char const *path, mmfile_t *mf)
{
    int fd;
    int pos = 0;
    long size;
    char *blk;

    if (xdl_init_mmfile(mf, XDLT_STD_BLKSIZE, XDL_MMF_ATOMIC) < 0)
    {
        exit(1);
    }
    if ((fd = open(path, O_RDONLY)) == -1)
    {
        perror(path);
        xdl_free_mmfile(mf);
        exit(1);
    }
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, pos, SEEK_SET);
    blk = (char *)xdl_mmfile_writeallocate(mf, size);
    if (blk == NULL)
    {
        xdl_free_mmfile(mf);
        close(fd);
        exit(1);
    }
    if (read(fd, blk, (size_t)size) != (size_t)size)
    {
        perror(path);
        xdl_free_mmfile(mf);
        close(fd);
        exit(1);
    }
    close(fd);

    return 0;
}

/**
 *   Functions used by libxdiff for memory handling
 */
static void *wrap_malloc(void *priv, unsigned int size)
{
    return malloc(size);
}

static void wrap_free(void *priv, void *ptr)
{
    free(ptr);
}

static void *wrap_realloc(void *priv, void *ptr, unsigned int size)
{

    return realloc(ptr, size);
}

int handle_patch(void *priv, mmbuffer_t *mb, int nbuf)
{
    mmfile_t *patch = (mmfile_t *)priv;
    xdl_writem_mmfile(patch, mb, nbuf);

    return 0;
}

/**
 *   We load a few shader files from disk, we patch them and cache them in an
 * array to later feed them to game intercepting fopen, ftell and fread.
 */
void cacheModedShaderFiles()
{
    char cwd[256];
    char fullPath[512];
    char shaderBuffer[100000];
    char *newProgram;
    ShaderFilesToMod *shaderFilesToMod;
    int filesToModCount;

    memallocator_t malt;
    malt.priv = NULL;
    malt.malloc = wrap_malloc;
    malt.free = wrap_free;
    malt.realloc = wrap_realloc;
    xdl_set_allocator(&malt);
    mmfile_t patchedFile;
    ShaderFilesToPatch *shaderFilesToPatch;
    int shaderFilesToPatchCount;
    char folderName[50];
    sprintf(folderName, "/../fs/shader/");

    switch (gId)
    {
    case LETS_GO_JUNGLE:
    case LETS_GO_JUNGLE_REVA:
    case LETS_GO_JUNGLE_SPECIAL:
        for (int x = 0; x < lgjFilesToModCount; x++)
        {
            getcwd(cwd, sizeof(cwd));
            sprintf(fullPath, "%s%s", cwd, lgjShaderFilesToMod[x].fileName);
            loadShader(fullPath, shaderBuffer);
            newProgram = replaceInBlock(shaderBuffer, attribsSet, attribsSetCount, "", "");
            newProgram = replaceInBlock(newProgram, lgjShaderReplaceSet1, lgjShaderReplaceCount1, "", "");

            // Dirty hack to fix just 1 shader
            if (x == 3)
            {
                newProgram = replaceInBlock(newProgram, lgjShaderReplaceSet2, lgjShaderReplaceCount2, "", "");
            }

            lgjShaderFilesToMod[x].shaderBufferSize = strlen(newProgram);
            lgjShaderFilesToMod[x].shaderBuffer = strndup(newProgram, strlen(newProgram));
            free(newProgram);
        }
        break;
    case INITIALD_4_EXP_REVB:
    case INITIALD_4_EXP_REVC:
    case INITIALD_4_EXP_REVD:
    case INITIALD_4_REVA:
    case INITIALD_4_REVB:
    case INITIALD_4_REVC:
    case INITIALD_4_REVD:
    case INITIALD_4_REVG:
    case INITIALD_5_JAP_REVA:
    case INITIALD_5_JAP_REVF:
    case INITIALD_5_EXP:
    case INITIALD_5_EXP_20:
    case INITIALD_5_EXP_20A:
        if (gId == INITIALD_4_REVA || gId == INITIALD_4_REVB || gId == INITIALD_4_REVC || gId == INITIALD_4_REVD || gId == INITIALD_4_REVG)
        {
            shaderFilesToMod = id4jShaderFilesToMod;
            filesToModCount = id4FilesToModCount;
        }
        else
        {
            shaderFilesToMod = idShaderFilesToMod;
            filesToModCount = idFilesToModCount;
        }
        for (int x = 0; x < filesToModCount; x++)
        {
            if (gId == INITIALD_5_JAP_REVA || gId == INITIALD_5_JAP_REVF || gId == INITIALD_5_EXP || gId == INITIALD_5_EXP_20 ||
                gId == INITIALD_5_EXP_20A)
            {
                char *fName = shaderFilesToMod[x].fileName;
                shaderFilesToMod[x].fileName = replaceSubstring(fName, 0, strlen(fName), "Shader", "V5SHADER");
            }
            getcwd(cwd, sizeof(cwd));
            sprintf(fullPath, "%s%s", cwd, shaderFilesToMod[x].fileName);
            loadShader(fullPath, shaderBuffer);
            if (strcmp(shaderFilesToMod[x].fileName, "/data/V5SHADER/effect_p.fx") == 0)
            {
                newProgram = replaceInBlock(shaderBuffer, id5ShaderReplaceSetForSmoke, id5ShaderReplaceSetForSmokeCount, "", "");
            }
            else if (strcmp(shaderFilesToMod[x].fileName, "/data/Shader/effect_p.fx") == 0)
            {
                newProgram = strndup(shaderBuffer, strlen(shaderBuffer));
            }
            else
            {
                newProgram = replaceInBlock(shaderBuffer, attribsSet, attribsSetCount, "struct\tVS_INPUT_P", "");
                newProgram = replaceInBlock(newProgram, attribsSet, attribsSetCount, "struct VS_INPUT", "struct VS_OUTPUT");
                newProgram = replaceInBlock(newProgram, idShaderReplaceSet, idShaderReplaceCount, "", "");
            }
            shaderFilesToMod[x].shaderBufferSize = strlen(newProgram);
            shaderFilesToMod[x].shaderBuffer = strndup(newProgram, strlen(newProgram));
            free(newProgram);
        }
        break;
    case VIRTUA_TENNIS_3:
    case VIRTUA_TENNIS_3_TEST:
    case VIRTUA_TENNIS_3_REVA:
    case VIRTUA_TENNIS_3_REVA_TEST:
    case VIRTUA_TENNIS_3_REVB:
    case VIRTUA_TENNIS_3_REVB_TEST:
    case VIRTUA_TENNIS_3_REVC:
    case VIRTUA_TENNIS_3_REVC_TEST:
        for (int x = 0; x < vt3FilesToModCount; x++)
        {
            getcwd(cwd, sizeof(cwd));
            sprintf(fullPath, "%s/..%s", cwd, vt3ShaderFilesToMod[x].fileName);
            loadShader(fullPath, shaderBuffer);
            newProgram = replaceInBlock(shaderBuffer, attribsSet, attribsSetCount, "struct\tVS_INPUT_P", "");
            newProgram = replaceInBlock(newProgram, vt3ShaderReplaceSet2, vt3ShaderReplaceCount2, "Out main(", ")");
            newProgram = replaceInBlock(newProgram, vt3ShaderReplaceSet1, vt3ShaderReplaceCount1, "", "");

            vt3ShaderFilesToMod[x].shaderBufferSize = strlen(newProgram);
            vt3ShaderFilesToMod[x].shaderBuffer = strndup(newProgram, strlen(newProgram));
            free(newProgram);
        }
        break;
    case HUMMER:
    case HUMMER_SDLX:
    case HUMMER_EXTREME:
    case HUMMER_EXTREME_MDX:
        for (int x = 0; x < hummerFilesToModCount; x++)
        {
            getcwd(cwd, sizeof(cwd));
            sprintf(fullPath, "%s%s", cwd, hummerShaderFilesToMod[x].fileName);
            loadShader(fullPath, shaderBuffer);
            newProgram = replaceInBlock(shaderBuffer, hummerShaderReplaceSet1, hummerShaderReplaceSet1Count, "", "");
            if (getConfig()->GPUVendor != NVIDIA_GPU)
            {
                newProgram = replaceInBlock(newProgram, hummerShaderReplaceMesa, hummerShaderReplaceMesaCount, "", "");
            }
            if (getConfig()->hummerFlickerFix)
            {
                newProgram = replaceInBlock(newProgram, hummerShaderReplaceFlicker, hummerShaderReplaceFlickerCount, "", "");
            }
            hummerShaderFilesToMod[x].shaderBufferSize = strlen(newProgram);
            hummerShaderFilesToMod[x].shaderBuffer = strndup(newProgram, strlen(newProgram));
            free(newProgram);
        }
        break;
    default:
        switch (gId)
        {
        case TOO_SPICY:
            shaderFilesToPatch = tooSpicyShaderPatches;
            shaderFilesToPatchCount = tooSpicyShaderPatchesCount;
            break;
        case RAMBO:
            if (getConfig()->GPUVendor != NVIDIA_GPU)
            {
                shaderFilesToPatch = ramboMesaShaderPatches;
                shaderFilesToPatchCount = ramboMesaShaderPatchesCount;
            }
            else
            {
                shaderFilesToPatch = ramboNvidiaShaderPatches;
                shaderFilesToPatchCount = ramboNvidiaShaderPatchesCount;
            }
            break;
        case HARLEY_DAVIDSON:
            shaderFilesToPatch = harleyShaderPatches;
            shaderFilesToPatchCount = harleyShaderPatchesCount;
            break;
        case THE_HOUSE_OF_THE_DEAD_EX:
            shaderFilesToPatch = hodexShaderPatches;
            shaderFilesToPatchCount = hodexShaderPatchesCount;
            break;
        case THE_HOUSE_OF_THE_DEAD_4_REVA:
        case THE_HOUSE_OF_THE_DEAD_4_REVB:
        case THE_HOUSE_OF_THE_DEAD_4_REVC:
            sprintf(folderName, "/../fs/dat/");
            shaderFilesToPatchCount = hod4ShaderPatchesCount;
            shaderFilesToPatch = hod4ShaderPatches;
            break;
        case THE_HOUSE_OF_THE_DEAD_4_SPECIAL:
        case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB:
            sprintf(folderName, "/../fs/dat/");
            shaderFilesToPatchCount = hod4spShaderPatchesCount;
            shaderFilesToPatch = hod4spShaderPatches;
            break;
        case PRIMEVAL_HUNT:
            sprintf(folderName, "/../data/nnstdshader/");
            shaderFilesToPatchCount = phShaderPatchesCount;
            shaderFilesToPatch = phShaderPatches;
            break;
        case GHOST_SQUAD_EVOLUTION:
            shaderFilesToPatchCount = gsevoShaderPatchesCount;
            sprintf(folderName, "/%s", "");
            if (getConfig()->GPUVendor != NVIDIA_GPU)
            {
                shaderFilesToPatch = gsevoMesaShaderPatches;
            }
            else
            {
                shaderFilesToPatch = gsevoNvidiaShaderPatches;
            }
            break;
        }

        for (int x = 0; x < shaderFilesToPatchCount; x++)
        {
            mmfile_t oriFile, patch;
            xdemitcb_t ecb, rjecb;
            getcwd(cwd, sizeof(cwd));
            sprintf(fullPath, "%s%s%s", cwd, folderName, shaderFilesToPatch[x].fileName);
            if (xdlt_load_mmfile(fullPath, &oriFile) != 0)
            {
                printf("Error reading oriFile.\n");
                exit(1);
                return;
            }

            if (xdl_init_mmfile(&patchedFile, XDLT_STD_BLKSIZE, XDL_MMF_ATOMIC) < 0)
            {
                printf("Error Initializong patchedFile.\n");
                exit(1);
                return;
            }
            xdl_free_mmfile(&patchedFile);

            if (xdl_init_mmfile(&patch, XDLT_STD_BLKSIZE, XDL_MMF_ATOMIC) < 0)
            {
                printf("Error Initializong patch.\n");
                exit(1);
                return;
            }
            xdl_free_mmfile(&patch);

            xdl_write_mmfile(&patch, shaderFilesToPatch[x].patchBuffer, shaderFilesToPatch[x].patchBufferSize);

            ecb.priv = &patchedFile;
            ecb.outf = handle_patch;
            if (xdl_bpatch(&oriFile, &patch, &ecb))
            {
                printf("Error patching shaders, are you sure your files are original?\n");
            }
            else
            {
                char *buff = malloc(patchedFile.fsize + 1);
                xdl_seek_mmfile(&patchedFile, 0);
                xdl_read_mmfile(&patchedFile, buff, patchedFile.fsize);

                shaderFilesToPatch[x].shaderBufferSize = patchedFile.fsize;
                shaderFilesToPatch[x].shaderBuffer = malloc(patchedFile.fsize);
                memcpy((void *)shaderFilesToPatch[x].shaderBuffer, buff, patchedFile.fsize);
                free(buff);
            }
        }
    }
    if (getConfig()->showDebugMessages)
    {
        printf("Modded Shaders cached.\n");
    }
    cachedShaderFilesLoaded = true;
}

void *vt3_open(void *fb, const char *s, int mode)
{
    bool (*_vt3_open)(void *, const char *, int) = dlsym(RTLD_NEXT, "_ZNSt13basic_filebufIcSt11char_traitsIcEE4openEPKcSt13_Ios_Openmode");

    if (shaderFileInList(s, &shaderIDX))
    {
        fdHook = true;
        return (void *)_vt3_open((void *)fb, s, mode);
    }
    return (void *)_vt3_open((void *)fb, s, mode);
}

int vt3_close(void *fb)
{
    bool (*_vt3_close)(void *) = dlsym(RTLD_NEXT, "_ZNSt13basic_filebufIcSt11char_traitsIcEE5closeEv");

    if (fdHook)
    {
        fdHook = false;
        shaderIDX = 0;
    }

    return _vt3_close((void *)fb);
}

u_int32_t *vt3_tellg(u_int32_t *size, void *is)
{
    u_int32_t *(*_vt3_tellg)(void *, void *) = dlsym(RTLD_NEXT, "_ZNSi5tellgEv");

    if (fdHook)
    {
        *(int *)(size) = ftellGetShaderSize(shaderIDX);
        return size;
    }

    return _vt3_tellg(size, is);
}

void *vt3_read(void *is, char *s, long long n)
{
    void *(*_vt3_read)(void *is, char *s, long long n) = dlsym(RTLD_NEXT, "_ZNSi4readEPci");
    if (fdHook)
    {
        freadReplace(s, n, 1, shaderIDX);
        return is;
    }
    return _vt3_read(is, s, n);
}

bool vf5PatchStage()
{
    bool patch = false;
    for (int x = 0; x < vf5StageListCount; x++)
    {
        if (strcmp(vf5StageNameAbbr, vf5StageList[x].stageAbb) == 0)
        {
            patch = vf5StageList[x].patch;
        }
    }
    return patch;
}

void vf5SetExposure(int var, float exposure)
{
    if (vf5PatchStage())
    {
        exposure *= 0.70;
    }
    (((void (*)(int, float))(size_t)vf5ExpOriAddr)(var, exposure));
}

void vf5SetIntensity(int var, float *intensity)
{
    if (vf5PatchStage())
    {
        intensity[0] *= 0.80;
        intensity[1] *= 0.80;
        intensity[2] *= 0.80;
    }
    (((void (*)(int, float *))(size_t)vf5IntOriAddr)(var, intensity));
}

void vf5GetIdStart(void *iostream, int *idStart)
{
    (((void (*)(void *, int *))(size_t)vf5IdStartOriAddr)(iostream, idStart));
    vf5IdStart = *idStart;
}

void vf5SetDiffuse(int var, float r, float g, float b, float a)
{
    if (vf5IdStart < 2 && vf5PatchStage())
    {
        r = r * 0.8;
        g = g * 0.8;
        b = b * 0.8;
        a = a * 0.8;
    }
    (((void (*)(int, float, float, float, float))(size_t)vf5DifOriAddr)(var, r, g, b, a));
}

void hookVf5FSExposure(uint32_t expAddr, uint32_t intAddr, uint32_t idStartAddr, uint32_t difAddr)
{
    vf5ExpOriAddr = (*(uint32_t *)(size_t)(expAddr + 1) + expAddr + 5);
    vf5IntOriAddr = (*(uint32_t *)(size_t)(intAddr + 1) + intAddr + 5);
    vf5IdStartOriAddr = (*(uint32_t *)(size_t)(idStartAddr + 1) + idStartAddr + 5);
    vf5DifOriAddr = (*(uint32_t *)(size_t)(difAddr + 1) + difAddr + 5);
    replaceCallAtAddress(expAddr, vf5SetExposure);
    replaceCallAtAddress(intAddr, vf5SetIntensity);
    replaceCallAtAddress(idStartAddr, vf5GetIdStart);
    replaceCallAtAddress(difAddr, vf5SetDiffuse);
}
