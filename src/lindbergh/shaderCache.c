#include <dlfcn.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../libxdiff/xdiff/xdiff.h"

#include "config.h"
#include "shaderPatches.h"
#include "shaderWork/common.h"

#include "shaderWork/2spicy.h"
#include "shaderWork/axa.h"
#include "shaderWork/gsevo.h"
#include "shaderWork/harley.h"
#include "shaderWork/hod4.h"
#include "shaderWork/hodex.h"
#include "shaderWork/hummer.h"
#include "shaderWork/id.h"
#include "shaderWork/lgj.h"
#include "shaderWork/primeval.h"
#include "shaderWork/rambo.h"
#include "shaderWork/vt3.h"

extern uint32_t gId;
extern int gGrp;

bool cachedShaderFilesLoaded = false;
bool fdHook = false;
int shaderIDX = 0;

SearchReplace attribsSet[] = {{"POSITION;", "ATTR0;"},   {"COLOR0;", "ATTR3;"},    {"COLOR;", "ATTR3;"},       {"BINORMAL;", "ATTR15;"},
                              {"NORMAL;", "ATTR2;"},     {"TEXCOORD0;", "ATTR8;"}, {"TEXCOORD1;", "ATTR9;"},   {"TEXCOORD2;", "ATTR10;"},
                              {"TEXCOORD3;", "ATTR11;"}, {"TANGENT;", "ATTR14;"},  {"BLENDWEIGHT;", "ATTR1;"}, {"BLENDINDICES;", "ATTR7;"}};

int attribsSetCount = sizeof(attribsSet) / sizeof(SearchReplace);

#define XDLT_STD_BLKSIZE (1024 * 8)

/**
 *   We check if the shader being loaded is one of the ones in the list of files
 * we need to patch
 */
bool shaderFileInList(const char *pathname, int *idx)
{
    char cwd[256];
    int gpuVendor = getConfig()->GPUVendor;
    if (gId == GHOST_SQUAD_EVOLUTION)
    {
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
    }
    else if (gId == RAMBO || gId == RAMBO_CHINA)
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
    else if (gGrp == GROUP_HOD4)
    {
        for (int x = 0; x < hod4ShaderPatchesCount; x++)
        {
            if ((strcmp(hod4ShaderPatches[x].fileName, basename((char *)pathname)) == 0) && (hod4ShaderPatches[x].shaderBufferSize != 0))
            {
                *idx = x;
                return true;
            }
        }
        return false;
    }
    else if (gGrp == GROUP_HOD4_SP)
    {
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
    }
    else if (gId == HARLEY_DAVIDSON)
    {
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
    }
    else if (gId == PRIMEVAL_HUNT)
    {
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
    }
    else if (gGrp == GROUP_HUMMER)
    {
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
    }
    else if (gId == TOO_SPICY)
    {
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
    }
    else if (gId == THE_HOUSE_OF_THE_DEAD_EX)
    {
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
    else if(gId == QUIZ_AXA)
    {
        for (int x = 0; x < axaShaderPatchesCount; x++)
        {
            if ((strcmp(basename((char *)axaShaderPatches[x].fileName), basename((char *)pathname)) == 0) &&
                (axaShaderPatches[x].shaderBufferSize != 0))
            {
                *idx = x;
                return true;
            }
        }
        return false;
    }
    else if (gId == QUIZ_AXA_LIVE)
    {
        for (int x = 0; x < axalShaderPatchesCount; x++)
        {
            if ((strcmp(basename((char *)axalShaderPatches[x].fileName), basename((char *)pathname)) == 0) &&
                (axalShaderPatches[x].shaderBufferSize != 0))
            {
                *idx = x;
                return true;
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

    if (gGrp == GROUP_LGJ)
    {
        if (gpuVendor == NVIDIA_GPU && getConfig()->lgjRenderWithMesa != 1)
            return false;
        filesToMod = lgjShaderFilesToMod;
        filesToModCount = lgjFilesToModCount;
        searchFolder1 = "/shader/Cg";
        searchFolder2 = "/extraShader/Cg";
        format = "%s/inc%s";
    }
    else if (gGrp == GROUP_ID4_EXP || gGrp == GROUP_ID4_JAP || gGrp == GROUP_ID5)
    {
        if (gpuVendor == NVIDIA_GPU)
            return false;

        // Set default values
        filesToMod = idShaderFilesToMod;
        filesToModCount = idFilesToModCount;
        searchFolder1 = "/shader/Cg/inc";
        searchFolder2 = "/data/Shader";
        format = "%s%s";

        if (gGrp == GROUP_ID4_JAP)
        {
            filesToMod = id4jShaderFilesToMod;
            filesToModCount = id4FilesToModCount;
        }
        else if (gGrp == GROUP_ID5)
        {
            searchFolder2 = "/data/V5SHADER";
        }
    }
    else if (gGrp == GROUP_VT3 || gGrp == GROUP_VT3_TEST)
    {
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
    if (gId == GHOST_SQUAD_EVOLUTION)
    {
        return getConfig()->GPUVendor != NVIDIA_GPU ? gsevoMesaShaderPatches[idx].shaderBufferSize
                                                    : gsevoNvidiaShaderPatches[idx].shaderBufferSize;
    }
    else if (gGrp == GROUP_LGJ)
    {
        return lgjShaderFilesToMod[idx].shaderBufferSize;
    }
    else if (gGrp == GROUP_ID4_EXP || gGrp == GROUP_ID5)
    {
        return idShaderFilesToMod[idx].shaderBufferSize;
    }
    else if (gGrp == GROUP_ID4_JAP)
    {
        return id4jShaderFilesToMod[idx].shaderBufferSize;
    }
    else if (gGrp == GROUP_VT3 || gGrp == GROUP_VT3_TEST)
    {
        return vt3ShaderFilesToMod[idx].shaderBufferSize;
    }
    else if (gId == TOO_SPICY)
    {
        return tooSpicyShaderPatches[idx].shaderBufferSize;
    }
    else if (gId == RAMBO || gId == RAMBO_CHINA)
    {
        return getConfig()->GPUVendor != NVIDIA_GPU ? ramboMesaShaderPatches[idx].shaderBufferSize
                                                    : ramboNvidiaShaderPatches[idx].shaderBufferSize;
    }
    else if (gGrp == GROUP_HOD4)
    {
        return hod4ShaderPatches[idx].shaderBufferSize;
    }
    else if (gGrp == GROUP_HOD4_SP)
    {
        return hod4spShaderPatches[idx].shaderBufferSize;
    }
    else if (gId == THE_HOUSE_OF_THE_DEAD_EX)
    {
        return hodexShaderPatches[idx].shaderBufferSize;
    }
    else if (gGrp == GROUP_HUMMER)
    {
        return hummerShaderFilesToMod[idx].shaderBufferSize;
    }
    else if (gId == PRIMEVAL_HUNT)
    {
        return phShaderPatches[idx].shaderBufferSize;
    }
    else if (gId == HARLEY_DAVIDSON)
    {
        return harleyShaderPatches[idx].shaderBufferSize;
    }
    else if (gId == QUIZ_AXA)
    {
        return axaShaderPatches[idx].shaderBufferSize;
    }
    else if (gId == QUIZ_AXA_LIVE)
    {
        return axalShaderPatches[idx].shaderBufferSize;
    }
    return -1;
}

/**
 *   We replace the file being loaded by fread and insted, we fill the buffer
 * with the data we want.
 */
size_t freadReplace(void *buf, size_t size, size_t count, int idx)
{
    if (gId == GHOST_SQUAD_EVOLUTION)
    {
        if (getConfig()->GPUVendor != NVIDIA_GPU)
        {
            memcpy(buf, gsevoMesaShaderPatches[idx].shaderBuffer, gsevoMesaShaderPatches[idx].shaderBufferSize);
        }
        else
        {
            memcpy(buf, gsevoNvidiaShaderPatches[idx].shaderBuffer, gsevoNvidiaShaderPatches[idx].shaderBufferSize);
        }
        return 1;
    }
    else if (gGrp == GROUP_LGJ)
    {
        memcpy(buf, lgjShaderFilesToMod[idx].shaderBuffer, size);
        return size;
    }
    else if (gGrp == GROUP_ID4_EXP || gGrp == GROUP_ID5)
    {
        memcpy(buf, idShaderFilesToMod[idx].shaderBuffer, size);
        return size;
    }
    else if (gGrp == GROUP_ID4_JAP)
    {
        memcpy(buf, id4jShaderFilesToMod[idx].shaderBuffer, size);
        return size;
    }
    else if (gGrp == GROUP_VT3 || gGrp == GROUP_VT3_TEST)
    {
        memcpy(buf, vt3ShaderFilesToMod[idx].shaderBuffer, size);
        return size;
    }
    else if (gId == TOO_SPICY)
    {
        memcpy(buf, tooSpicyShaderPatches[idx].shaderBuffer, tooSpicyShaderPatches[idx].shaderBufferSize);
        return tooSpicyShaderPatches[idx].shaderBufferSize;
    }
    else if (gId == RAMBO || gId == RAMBO_CHINA)
    {
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
    }
    else if (gGrp == GROUP_HOD4)
    {
        memcpy(buf, hod4ShaderPatches[idx].shaderBuffer, hod4ShaderPatches[idx].shaderBufferSize);
        return hod4ShaderPatches[idx].shaderBufferSize;
    }
    else if (gGrp == GROUP_HOD4_SP)
    {
        memcpy(buf, hod4spShaderPatches[idx].shaderBuffer, hod4spShaderPatches[idx].shaderBufferSize);
        return hod4spShaderPatches[idx].shaderBufferSize;
    }
    else if (gId == THE_HOUSE_OF_THE_DEAD_EX)
    {
        memcpy(buf, hodexShaderPatches[idx].shaderBuffer, hodexShaderPatches[idx].shaderBufferSize);
        return hodexShaderPatches[idx].shaderBufferSize;
    }
    else if (gGrp == GROUP_HUMMER)
    {
        memcpy(buf, hummerShaderFilesToMod[idx].shaderBuffer, hummerShaderFilesToMod[idx].shaderBufferSize);
        return hummerShaderFilesToMod[idx].shaderBufferSize;
    }
    else if (gId == PRIMEVAL_HUNT)
    {
        memcpy(buf, phShaderPatches[idx].shaderBuffer, phShaderPatches[idx].shaderBufferSize);
        return phShaderPatches[idx].shaderBufferSize;
    }
    else if (gId == HARLEY_DAVIDSON)
    {
        memcpy(buf, harleyShaderPatches[idx].shaderBuffer, harleyShaderPatches[idx].shaderBufferSize);
        return harleyShaderPatches[idx].shaderBufferSize;
    }
    else if (gId == QUIZ_AXA)
    {
        memcpy(buf, axaShaderPatches[idx].shaderBuffer, axaShaderPatches[idx].shaderBufferSize);
        return axaShaderPatches[idx].shaderBufferSize;
    }
    else if (gId == QUIZ_AXA_LIVE)
    {
        memcpy(buf, axalShaderPatches[idx].shaderBuffer, axalShaderPatches[idx].shaderBufferSize);
        return axalShaderPatches[idx].shaderBufferSize;
    }
    return -1;
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

    if (gGrp == GROUP_LGJ)
    {
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
    }
    else if (gGrp == GROUP_ID4_EXP || gGrp == GROUP_ID4_JAP || gGrp == GROUP_ID5)
    {
        if (gGrp == GROUP_ID4_JAP)
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
            if (gGrp == GROUP_ID5)
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
    }
    else if (gGrp == GROUP_VT3 || gGrp == GROUP_VT3_TEST)
    {
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
    }
    else if (gGrp == GROUP_HUMMER)
    {
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
    }
    else
    {
        if (gId == TOO_SPICY)
        {
            shaderFilesToPatch = tooSpicyShaderPatches;
            shaderFilesToPatchCount = tooSpicyShaderPatchesCount;
        }
        else if (gId == RAMBO || gId == RAMBO_CHINA)
        {
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
        }
        else if (gId == HARLEY_DAVIDSON)
        {
            shaderFilesToPatch = harleyShaderPatches;
            shaderFilesToPatchCount = harleyShaderPatchesCount;
        }
        else if (gId == THE_HOUSE_OF_THE_DEAD_EX)
        {
            shaderFilesToPatch = hodexShaderPatches;
            shaderFilesToPatchCount = hodexShaderPatchesCount;
        }
        else if (gGrp == GROUP_HOD4)
        {
            sprintf(folderName, "/../fs/dat/");
            shaderFilesToPatchCount = hod4ShaderPatchesCount;
            shaderFilesToPatch = hod4ShaderPatches;
        }
        else if (gGrp == GROUP_HOD4_SP)
        {
            sprintf(folderName, "/../fs/dat/");
            shaderFilesToPatchCount = hod4spShaderPatchesCount;
            shaderFilesToPatch = hod4spShaderPatches;
        }
        else if (gId == PRIMEVAL_HUNT)
        {
            sprintf(folderName, "/../data/nnstdshader/");
            shaderFilesToPatchCount = phShaderPatchesCount;
            shaderFilesToPatch = phShaderPatches;
        }
        else if (gId == GHOST_SQUAD_EVOLUTION)
        {
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
        }
        else if (gId == QUIZ_AXA)
        {
            sprintf(folderName, "/../../data/shader/");
            shaderFilesToPatchCount = axaShaderPatchesCount;
            shaderFilesToPatch = axaShaderPatches;
        }
        else if (gId == QUIZ_AXA_LIVE)
        {
            sprintf(folderName, "/../../data/shader/");
            shaderFilesToPatchCount = axalShaderPatchesCount;
            shaderFilesToPatch = axalShaderPatches;
        }
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

    if (getConfig()->showDebugMessages)
    {
        printf("Modded Shaders cached.\n");
    }
    cachedShaderFilesLoaded = true;
}

void *vt3Open(void *fb, const char *s, int mode)
{
    bool (*_vt3_open)(void *, const char *, int) = dlsym(RTLD_NEXT, "_ZNSt13basic_filebufIcSt11char_traitsIcEE4openEPKcSt13_Ios_Openmode");

    if (shaderFileInList(s, &shaderIDX))
    {
        fdHook = true;
        return (void *)_vt3_open((void *)fb, s, mode);
    }
    return (void *)_vt3_open((void *)fb, s, mode);
}

int vt3Close(void *fb)
{
    bool (*_vt3_close)(void *) = dlsym(RTLD_NEXT, "_ZNSt13basic_filebufIcSt11char_traitsIcEE5closeEv");

    if (fdHook)
    {
        fdHook = false;
        shaderIDX = 0;
    }

    return _vt3_close((void *)fb);
}

u_int32_t *vt3Tellg(u_int32_t *size, void *is)
{
    u_int32_t *(*_vt3_tellg)(void *, void *) = dlsym(RTLD_NEXT, "_ZNSi5tellgEv");

    if (fdHook)
    {
        *(int *)(size) = ftellGetShaderSize(shaderIDX);
        return size;
    }

    return _vt3_tellg(size, is);
}

void *vt3Read(void *is, char *s, long long n)
{
    void *(*_vt3_read)(void *is, char *s, long long n) = dlsym(RTLD_NEXT, "_ZNSi4readEPci");
    if (fdHook)
    {
        freadReplace(s, n, 1, shaderIDX);
        return is;
    }
    return _vt3_read(is, s, n);
}
