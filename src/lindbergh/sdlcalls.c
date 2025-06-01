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
#include "customcursor.h"

extern uint32_t gId;
bool SDLWindowCreated = false;
extern bool GLUTGame;
SDL_Window *SDLwindow;
SDL_GLContext SDLcontext;
Display *x11Display;
Window x11Window;
char SDLgameTitle[256] = {0};
extern fps_limit fpsLimit;

extern void *customCursor;
extern void *phTouchCursor;

int glutInitialized = 0;

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

void initSDL(int *argcp, char **argv)
{
    void FGAPIENTRY (*_glutInit)(int *argcp, char **argv) = dlsym(RTLD_NEXT, "glutInit");

    switch (gId)
    {
    case OUTRUN_2_SP_SDX_TEST:
    case OUTRUN_2_SP_SDX_REVA_TEST:
    case OUTRUN_2_SP_SDX_REVA_TEST2:
        if (!glutInitialized)
        {
            glutInitialized = 1;
            _glutInit(argcp, argv);
            return;
        }
    }

    switch (gId)
    {
    case AFTER_BURNER_CLIMAX:
    case AFTER_BURNER_CLIMAX_REVA:
    case AFTER_BURNER_CLIMAX_REVB:
    case AFTER_BURNER_CLIMAX_SDX:
    case AFTER_BURNER_CLIMAX_SDX_REVA:
    case AFTER_BURNER_CLIMAX_SE:
    case AFTER_BURNER_CLIMAX_SE_REVA:
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
    switch (gId)
    {
    case AFTER_BURNER_CLIMAX:
    case AFTER_BURNER_CLIMAX_REVA:
    case AFTER_BURNER_CLIMAX_REVB:
    case AFTER_BURNER_CLIMAX_SDX:
    case AFTER_BURNER_CLIMAX_SDX_REVA:
    case AFTER_BURNER_CLIMAX_SE:
    case AFTER_BURNER_CLIMAX_SE_REVA:
    case R_TUNED:
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
    case GHOST_SQUAD_EVOLUTION:
    case SEGA_RACE_TV:
    case MJ4_REVG:
    case MJ4_EVO:
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    }

    uint32_t windowFlags = SDL_WINDOW_OPENGL;

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

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Your bg color

    // Force initial clear
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(SDLwindow);

    // Now show the window
    SDL_ShowWindow(SDLwindow);

    printf("  SDL RESOLUTION: %dx%d\n\n", getConfig()->width, getConfig()->height);

    if (getConfig()->fullscreen)
    {
        SDL_SetWindowFullscreen(SDLwindow, SDL_WINDOW_FULLSCREEN);
    }

    loadCursors();
    setCursor(customCursor);

    if (getConfig()->hideCursor || gId == PRIMEVAL_HUNT)
        SDL_ShowCursor(SDL_DISABLE);
}

void sdlQuit()
{
    SDL_DestroyWindow(SDLwindow);
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
