#ifndef __i386__
#define __i386__
#endif
#undef __x86_64__
#define GL_GLEXT_PROTOTYPES
#include <GL/freeglut.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <X11/Xlib.h>

#include "config.h"
#include "input.h"
#include "sdlcalls.h"
#include "fps_limiter.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_ONLY_PNG
#define STBI_SUPPORT_ZLIB
#define STBI_NO_SIMD

bool SDLWindowCreated = false;
extern bool GLUTGame;
SDL_Window *SDLwindow;
SDL_GLContext SDLcontext;
Display *x11Display;
Window x11Window;
char SDLgameTitle[256] = {0};
extern fps_limit fpsLimit;

extern SDL_GameController **controllers;
extern int numControllers;

void GLAPIENTRY openglDebugCallback2(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                     const GLchar *message, const void *userParam)
{
    if (id == 1099)
        return;
    // Print the debug message details
    printf("OpenGL Debug Message:\n");
    printf("Source: 0x%x\n", source);
    printf("Type: 0x%x\n", type);
    printf("ID: %u\n", id);
    printf("Severity: 0x%x\n", severity);
    printf("Message: %s\n", message);

    // You can filter based on severity or other attributes if needed
    if (severity == GL_DEBUG_SEVERITY_HIGH)
    {
        printf("This is a high severity error!\n");
    }
}

SDL_Surface *createSurfaceFromStb(unsigned char *data, int width, int height, int channels)
{
    Uint32 rmask, gmask, bmask, amask;

    if (channels == 4)
    { // RGBA
        rmask = 0x000000FF;
        gmask = 0x0000FF00;
        bmask = 0x00FF0000;
        amask = 0xFF000000;
    }
    else if (channels == 3)
    { // RGB
        rmask = 0x000000FF;
        gmask = 0x0000FF00;
        bmask = 0x00FF0000;
        amask = 0;
    }
    else
    {
        fprintf(stderr, "Unsupported image format with %d channels.\n", channels);
        return NULL;
    }

    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(data, width, height, channels * 8, width * channels, rmask, gmask, bmask, amask);

    if (!surface)
    {
        fprintf(stderr, "SDL_CreateRGBSurfaceFrom Error: %s\n", SDL_GetError());
    }
    return surface;
}

int loadNewCursor(char *cursorFileName)
{
    int width, height, channels;
    unsigned char *imageData = stbi_load(cursorFileName, &width, &height, &channels, 0);
    if (!imageData)
    {
        fprintf(stderr, "Failed to load image: %s\n", cursorFileName);
        SDL_Quit();
        return 0;
    }

    SDL_Surface *originalSurface = createSurfaceFromStb(imageData, width, height, channels);
    if (!originalSurface)
    {
        stbi_image_free(imageData);
        SDL_Quit();
        return 0;
    }

    int newWidth = getConfig()->customCursorWidth;
    int newHeight = getConfig()->customCursorHeight;
    SDL_Surface *resizedSurface =
        SDL_CreateRGBSurface(0, newWidth, newHeight, originalSurface->format->BitsPerPixel, originalSurface->format->Rmask,
                             originalSurface->format->Gmask, originalSurface->format->Bmask, originalSurface->format->Amask);
    if (!resizedSurface)
    {
        fprintf(stderr, "SDL_CreateRGBSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(originalSurface);
        stbi_image_free(imageData);
        SDL_Quit();
        return 0;
    }

    SDL_Rect srcRect = {0, 0, originalSurface->w, originalSurface->h};
    SDL_Rect destRect = {0, 0, newWidth, newHeight};
    if (SDL_BlitScaled(originalSurface, &srcRect, resizedSurface, &destRect) != 0)
    {
        fprintf(stderr, "SDL_BlitScaled Error: %s\n", SDL_GetError());
        SDL_FreeSurface(resizedSurface);
        SDL_FreeSurface(originalSurface);
        stbi_image_free(imageData);
        SDL_Quit();
        return 0;
    }

    SDL_Cursor *customCursor = SDL_CreateColorCursor(resizedSurface, newWidth / 2, newHeight / 2);
    if (!customCursor)
    {
        fprintf(stderr, "SDL_CreateColorCursor Error: %s\n", SDL_GetError());
        SDL_FreeSurface(resizedSurface);
        SDL_FreeSurface(originalSurface);
        stbi_image_free(imageData);
        SDL_Quit();
        return 0;
    }

    SDL_SetCursor(customCursor);

    SDL_FreeSurface(resizedSurface);
    SDL_FreeSurface(originalSurface);
    stbi_image_free(imageData);

    return 1;
}

void initSDL(int *argcp, char **argv)
{
    int gId = getConfig()->crc32;

    void FGAPIENTRY (*_glutInit)(int *argcp, char **argv) = dlsym(RTLD_NEXT, "glutInit");

    if (getConfig()->noSDL && GLUTGame)
    {
        _glutInit(argcp, argv);
        return;
    }

    if ((gId == OUTRUN_2_SP_SDX_TEST || gId == OUTRUN_2_SP_SDX_REVA_TEST || gId == OUTRUN_2_SP_SDX_REVA_TEST2) && GLUTGame)
    {
        _glutInit(argcp, argv);
    }

    if ((gId == AFTER_BURNER_CLIMAX) || (gId == AFTER_BURNER_CLIMAX_REVA) || (gId == AFTER_BURNER_CLIMAX_REVB) ||
        (gId == AFTER_BURNER_CLIMAX_SDX) || (gId == AFTER_BURNER_CLIMAX_SDX_REVA) || (gId == AFTER_BURNER_CLIMAX_SE) ||
        (gId == AFTER_BURNER_CLIMAX_SE_REVA))
    {
        setenv("SDL_VIDEODRIVER", "x11", 1);
    }

    SDLWindowCreated = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // Double buffering
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 1);   // 24-bit depth buffer
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);   // Set the alpha size to 8 bits
    if (gId == AFTER_BURNER_CLIMAX || gId == AFTER_BURNER_CLIMAX_REVA || gId == AFTER_BURNER_CLIMAX_REVB ||
        gId == AFTER_BURNER_CLIMAX_SDX || gId == AFTER_BURNER_CLIMAX_SDX_REVA || gId == AFTER_BURNER_CLIMAX_SE ||
        gId == AFTER_BURNER_CLIMAX_SE_REVA || gId == R_TUNED || gId == VIRTUA_FIGHTER_5 || gId == VIRTUA_FIGHTER_5_REVA ||
        gId == VIRTUA_FIGHTER_5_REVB || gId == VIRTUA_FIGHTER_5_REVE || gId == VIRTUA_FIGHTER_5_EXPORT || gId == VIRTUA_FIGHTER_5_R ||
        gId == VIRTUA_FIGHTER_5_R_REVD || gId == VIRTUA_FIGHTER_5_R_REVG || gId == VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA ||
        gId == VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB || gId == VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000 || gId == GHOST_SQUAD_EVOLUTION ||
        gId == SEGA_RACE_TV || gId == MJ4_REVG || gId == MJ4_EVO)
    {
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    }

    uint32_t windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

    strcat(SDLgameTitle, getGameName());

    SDLwindow = SDL_CreateWindow(SDLgameTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, getConfig()->width,
                                 getConfig()->height, windowFlags);
    if (!SDLwindow)
    {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(SDLwindow, &info))
    {
        if (info.subsystem == SDL_SYSWM_X11)
        {
            x11Display = info.info.x11.display;
            x11Window = info.info.x11.window;
        }
        else
        {
            fprintf(stderr, "This program is not running on X11.\n");
        }
    }
    else
    {
        fprintf(stderr, "SDL_GetWindowWMInfo Error: %s\n", SDL_GetError());
    }

    SDLcontext = SDL_GL_CreateContext(SDLwindow);
    if (!SDLcontext)
    {
        fprintf(stderr, "OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    printf("  SDL RESOLUTION: %dx%d\n\n", getConfig()->width, getConfig()->height);

    if (getConfig()->fullscreen)
    {
        SDL_SetWindowFullscreen(SDLwindow, SDL_WINDOW_FULLSCREEN);
        }

    if (strcmp(getConfig()->customCursor, "") != 0)
    {
        if (!loadNewCursor(getConfig()->customCursor))
        {
            fprintf(stderr, "Custom cursor could not be loaded!\n");
            exit(1);
        }
    }

    if (getConfig()->hideCursor)
        SDL_ShowCursor(SDL_DISABLE);
}

void sdlQuit()
{
    SDL_DestroyWindow(SDLwindow);
    
    // Close each controller
    for (int i = 0; i < numControllers; i++) {
        if (controllers[i]) {
            SDL_GameControllerClose(controllers[i]);
        }
    }

    // Free the allocated memory
    free(controllers);
    
    SDL_Quit();
    exit(0);
}

void pollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            sdlQuit();
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERAXISMOTION:
            handleSdlEvents(&event);
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                SDL_Quit();
            }
            break;

        default:
            break;
        }
    }
}
