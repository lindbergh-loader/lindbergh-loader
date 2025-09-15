#ifndef __i386__
#define __i386__
#endif
#undef __x86_64__
#include <GL/gl.h>
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

bool gettingGPUVendor;

int getGPUVendorID()
{
    int vendorId;
    EmulatorConfig *config = getConfig();
    gettingGPUVendor = true;
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    uint32_t windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;

    SDL_Window *window = SDL_CreateWindow("OpenGL", 640, 480, windowFlags);
    if (!window)
    {
        fprintf(stderr, "Here Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context)
    {
        fprintf(stderr, "OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    config->GPUVendorString = strdup((char *)glGetString(GL_VENDOR));
    
    if (!config->GPUVendorString)
    {
        fprintf(stderr, "Error: Could not get renderer string\n");
        SDL_DestroyWindow(window);
        return ERROR_GPU;
    }
    gettingGPUVendor = false;
    SDL_DestroyWindow(window);

    if (strstr(config->GPUVendorString, "NVIDIA") != NULL)
        vendorId = NVIDIA_GPU;
    else if (strstr(config->GPUVendorString, "AMD") != NULL)
        vendorId = AMD_GPU;
    else if (strstr(config->GPUVendorString, "Intel") != NULL)
        vendorId = INTEL_GPU;
    else if (strstr(config->GPUVendorString, "ATI") != NULL)
        vendorId = ATI_GPU;
    else if (strstr(config->GPUVendorString, "Error") != NULL)
        vendorId = ERROR_GPU;
    else
        vendorId = UNKNOWN_GPU;
    return vendorId;
}

void getGPUVendor()
{
    EmulatorConfig *config = getConfig();
    if (config->GPUVendor == AUTO_DETECT_GPU)
        config->GPUVendor = getGPUVendorID();
    else if (config->GPUVendor == NVIDIA_GPU)
        config->GPUVendorString = "NVIDIA";
    else if (config->GPUVendor == AMD_GPU)
        config->GPUVendorString = "AMD";
    else if (config->GPUVendor == INTEL_GPU)
        config->GPUVendorString = "Intel";
    else if (config->GPUVendor == ATI_GPU)
        config->GPUVendorString = "ATI";
    else
        config->GPUVendorString = "Unknown";
}