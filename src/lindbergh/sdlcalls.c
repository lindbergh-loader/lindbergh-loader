#ifndef __i386__
#define __i386__
#include <SDL2/SDL_video.h>
#endif
#undef __x86_64__
#define GL_GLEXT_PROTOTYPES
#include <GL/freeglut.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>

#include "config.h"
#include "input.h"
#include "sdlcalls.h"
#include "fps_limiter.h"

bool SDLGame = false;
SDL_Window *SDLwindow;
SDL_GLContext SDLcontext;
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

void glutInitSDL(int *argcp, char **argv)
{
    int gId = getConfig()->crc32;

    void FGAPIENTRY (*_glutInit)(int *argcp, char **argv) = dlsym(RTLD_NEXT, "glutInit");

    if (getConfig()->noSDL)
    {
        _glutInit(argcp, argv);
        return;
    }
    // || gId == R_TUNED || gId == VIRTUA_FIGHTER_5_R || gId == VIRTUA_FIGHTER_5_R_REVD
    if (gId == OUTRUN_2_SP_SDX_TEST || gId == OUTRUN_2_SP_SDX_REVA_TEST || gId == OUTRUN_2_SP_SDX_REVA_TEST2)
    {
        _glutInit(argcp, argv);
    }

    if ((gId == AFTER_BURNER_CLIMAX) || (gId == AFTER_BURNER_CLIMAX_REVA) || (gId == AFTER_BURNER_CLIMAX_REVB) ||
        (gId == AFTER_BURNER_CLIMAX_SDX) || (gId == AFTER_BURNER_CLIMAX_SDX_REVA) || (gId == AFTER_BURNER_CLIMAX_SE) ||
        (gId == AFTER_BURNER_CLIMAX_SE_REVA))
    {
        setenv("SDL_VIDEODRIVER", "x11", 1);
    }

    SDLGame = true;

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
            handleSdlEvents(&event);
            break;
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
