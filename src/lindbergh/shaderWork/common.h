#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../shaderPatches.h"

typedef struct
{
    uint32_t startOffset; // Offset inside the ELF
    uint32_t totalSize;   // Available size for the shader program
    uint32_t sizeOfCode;  // Size used by the original shader program
} ElfShaderOffsets;

typedef struct
{
    char *fileName;
    size_t shaderBufferSize;
    char *shaderBuffer;
} ShaderFilesToMod;

typedef struct
{
    const char *fileName;
    size_t patchBufferSize;
    const unsigned char *patchBuffer;
    size_t shaderBufferSize;
    const char *shaderBuffer;
} ShaderFilesToPatch;

typedef struct
{
    const char *stageAbb;
    const char *StageName;
    bool patch;
} Vf5StagesList;

extern SearchReplace attribsSet[];
extern int attribsSetCount;
extern char *nnstdshader_vert;
extern char *nnstdshader_frag;
