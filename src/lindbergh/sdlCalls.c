#ifndef __i386__
#define __i386__
#endif
#undef __x86_64__
#define GL_GLEXT_PROTOTYPES
#include <dlfcn.h>
// #include <fontconfig/fontconfig.h>
#include <GL/glx.h>
#include <GL/freeglut.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdbool.h>
#include <unistd.h>

#include "blitStretching.h"
#include "config.h"
#include "customCursor.h"
#include "sdlInput.h"
#include "jvs.h"
#include "resources/LiberationMono-Regular.h"
#include "log.h"
#include "sdlCalls.h"

extern uint32_t gId;
extern int gGrp;
extern int gWidth;
extern int gHeight;
extern int drawableW;
extern int drawableH;

bool creatingWindow = false;
SDL_Window *sdlWindow = NULL;
SDL_GLContext sdlContext = NULL;
Display *x11Display = NULL;
Window x11Window;
char sdlGameTitle[256] = {0};

extern SDL_Cursor *customCursor;
extern SDL_Cursor *touchCursor;

int glutInitialized = 0;

bool isFullScreen = false;
bool sdlFontInit = false;
SDL_Renderer *fontRenderer;
TTF_Font *font;

void GLAPIENTRY openglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                                    const void *userParam)
{
    if (id == 1099)
        return;
    printf("OpenGL Debug Message:\n");
    printf("Source: 0x%x\n", source);
    printf("Type: 0x%x\n", type);
    printf("ID: %u\n", id);
    printf("Severity: 0x%x\n", severity);
    printf("Message: %s\n", message);

    if (severity == GL_DEBUG_SEVERITY_HIGH)
    {
        printf("This is a high severity error!\n");
    }
}

int initSDL()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD))
    {
        log_error("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (gGrp == GROUP_OUTRUN_TEST)
    {
        if (!TTF_Init())
        {
            log_error("SDL_ttf could not initialize! SDL_ttf Error: %s\n", SDL_GetError());
            return 1;
        }
        SDL_IOStream *rw = SDL_IOFromConstMem(LiberationMonoRegular_ttf, LiberationMonoRegular_ttf_length);
        float fontSize = 16.0;
        if (gWidth > 640)
            fontSize = fontSize * (((float)gWidth / 640.0f) + ((float)gHeight / 480.0f)) / 2.0f;

        font = TTF_OpenFontIO(rw, 1, fontSize);
        fontRenderer = SDL_CreateRenderer(sdlWindow, "");
    }
    return 0;
}

void startSDL(int *argcp, char **argv)
{
    creatingWindow = true;
    int numDisplays;
    SDL_DisplayID *sdlDisplayId = SDL_GetDisplays(&numDisplays);
    if (numDisplays > 1)
        log_warn("More than 1 display detected, will use the first one.\n");

    const SDL_DisplayMode *displayMode = SDL_GetCurrentDisplayMode(sdlDisplayId[0]);
    if (displayMode == NULL)
    {
        log_error("SDL_GetCurrentDisplayMode Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // Double buffering
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);  // 24-bit depth buffer
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);   // Set the alpha size to 8 bits
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    if (gGrp == GROUP_ABC || gGrp == GROUP_VF5 || gId == R_TUNED || gId == GHOST_SQUAD_EVOLUTION || gId == SEGA_RACE_TV ||
        gId == MJ4_REVG || gId == MJ4_EVO)
    {
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    }

    uint32_t windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;

    strcat(sdlGameTitle, getGameName());

    // Hacky way to make AxA games render the characters properly
    if (gId == QUIZ_AXA || gId == QUIZ_AXA_LIVE)
        sdlWindow = SDL_CreateWindow(sdlGameTitle, 1024, 768, windowFlags);
    else
        sdlWindow = SDL_CreateWindow(sdlGameTitle, gWidth, gHeight, windowFlags);

    if (!sdlWindow)
    {
        SDL_Quit();
        log_error("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if (gId == GHOST_SQUAD_EVOLUTION || gId == MJ4_EVO || gId == MJ4_REVG)
        SDL_SetWindowSize(sdlWindow, gWidth, gHeight + 1);

    x11Display = (Display *)SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
    x11Window = (Window)SDL_GetNumberProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
    if (!x11Display || !x11Window)
    {
        log_error("This program is not running on X11 or failed to get window/display.\n");
    }

    sdlContext = SDL_GL_CreateContext(sdlWindow);
    if (!sdlContext)
    {
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
        log_error("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if (getConfig()->showDebugMessages)
    {
        // Enable OpenGL debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglDebugCallback, NULL);
    }

    // If games are any of the LGJ or Primeval Hunt, we prevent the window to resize.
    if ((gGrp == GROUP_LGJ || gId == PRIMEVAL_HUNT) && !getConfig()->fullscreen)
        SDL_SetWindowMaximumSize(sdlWindow, gWidth, gHeight);
    else if (gGrp != GROUP_LGJ)
        SDL_SetWindowMaximumSize(sdlWindow, displayMode->w, displayMode->h);

    SDL_SetWindowMinimumSize(sdlWindow, gWidth, gHeight);

    // Hacky way to make AxA games render the characters properly
    if (gId == QUIZ_AXA || gId == QUIZ_AXA_LIVE)
        SDL_SetWindowPosition(sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    if (getConfig()->fullscreen)
    {
        SDL_SetWindowFullscreenMode(sdlWindow, NULL);
        SDL_SetWindowFullscreen(sdlWindow, true);
        isFullScreen = true;
    }

    initBlitting();

    SDL_ShowWindow(sdlWindow);

    Uint64 startTime = SDL_GetTicks();
    int running = 1;

    // We clear the window background
    while (running)
    {
        if (SDL_GetTicks() - startTime >= 1000)
            running = 0;
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(sdlWindow);
    }

    if (gId != PRIMEVAL_HUNT)
        SDL_SetWindowResizable(sdlWindow, true);

    creatingWindow = false;

    printf("  RESOLUTION: %dx%d\n", gWidth, gHeight);

    loadCursors();
    if (customCursor)
        setCursor(customCursor);

    if ((gId == MJ4_REVG || gId == MJ4_EVO || gId == QUIZ_AXA || gId == QUIZ_AXA_LIVE) && strcmp(getConfig()->touchCursor, "") != 0 &&
        getConfig()->emulateTouchscreen)
        setCursor(touchCursor);
    else if (getConfig()->hideCursor)
        SDL_HideCursor();
}

void sdlQuit()
{
    setSwitch(SYSTEM, BUTTON_TEST, 1);
    usleep(50000);
    setSwitch(SYSTEM, BUTTON_TEST, 0);
    usleep(50000);
    SDL_DestroyWindow(sdlWindow);
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
            case SDL_EVENT_KEY_DOWN:
            {
                if (gGrp != GROUP_LGJ && gId != PRIMEVAL_HUNT)
                {
                    SDL_Keymod mod = SDL_GetModState();
                    if ((event.key.key == SDLK_RETURN && (mod & SDL_KMOD_ALT)) || event.key.key == SDLK_F11)
                    {
                        if (isFullScreen)
                        {
                            if (!SDL_SetWindowFullscreen(sdlWindow, 0))
                            {
                                log_error("Error setting windowed mode: %s\n", SDL_GetError());
                            }
                            else
                            {
                                if ((long long)gWidth * 3 == (long long)gHeight * 4)
                                    SDL_SetWindowSize(sdlWindow, gWidth + 1, gHeight);
                                else
                                    SDL_SetWindowSize(sdlWindow, gWidth, gHeight + 1);
                                isFullScreen = false;
                                if (getConfig()->fullscreen)
                                    SDL_SetWindowBordered(sdlWindow, true);
                            }
                        }
                        else
                        {
                            if (!SDL_SetWindowFullscreen(sdlWindow, true))
                            {
                                log_error("Error setting fullscreen mode: %s\n", SDL_GetError());
                            }
                            else
                            {
                                isFullScreen = true;
                            }
                        }
                        break;
                    }
                }
            }
            case SDL_EVENT_KEY_UP:
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            case SDL_EVENT_MOUSE_MOTION:
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
            case SDL_EVENT_JOYSTICK_BUTTON_UP:
            case SDL_EVENT_JOYSTICK_AXIS_MOTION:
            case SDL_EVENT_JOYSTICK_HAT_MOTION:
                processSdlEvent(&event);
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                sdlQuit();
                break;
            case SDL_EVENT_WINDOW_RESTORED:
            {
                if (((long long)gWidth * 3 == (long long)gHeight * 4) || gGrp == GROUP_HUMMER)
                    SDL_SetWindowSize(sdlWindow, gWidth + 1, gHeight);
                else
                    SDL_SetWindowSize(sdlWindow, gWidth, gHeight + 1);
            }
            break;
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
                processSdlEvent(&event);
            default:
                break;
        }
    }
    if (gGrp == GROUP_HOD4 || gGrp == GROUP_HOD4_TEST)
        updateGunShake();
    updateCombinedAxes();
    processChangedActions();
}
