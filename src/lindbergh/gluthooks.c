#ifndef __i386__
#define __i386__
#endif
#undef __x86_64__
#include <GL/freeglut.h>
#include <GL/freeglut_std.h>
#include <GL/glx.h>
#include <time.h>

#include <SDL2/SDL.h>

#include <dlfcn.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "config.h"
#include "fps_limiter.h"
#include "sdlcalls.h"
#include "border.h"
#include "resolution.h"

extern uint32_t gId;
extern SDL_Window *SDLwindow;
extern char SDLgameTitle[];
extern fps_limit fpsLimit;
bool GLUTGame = false;

void *displayFunc = NULL;
void *reshapeFunc = NULL;
void *visibilityFunc = NULL;
void *idleFunc = NULL;

void FGAPIENTRY glutInit(int *argcp, char **argv)
{
    void FGAPIENTRY (*_glutInit)(int *argcp, char **argv) = dlsym(RTLD_NEXT, "glutInit");

    if (*argcp > 1 && gId == SEGA_RACE_TV)
    {
        if (isTestMode())
        {
            EmulatorConfig *config = getConfig();
            config->noSDL = 0;
            printf("Forced to use SDL to make Test mode render fine.\n");
        }
    }

    if (getConfig()->noSDL)
    {
        _glutInit(argcp, argv);
        return;
    }

    GLUTGame = true;
    initSDL(argcp, argv);
}

void FGAPIENTRY glutMainLoop(void)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutMainLoop)(void) = dlsym(RTLD_NEXT, "glutMainLoop");
        _glutMainLoop();
        return;
    }

    bool quit = false;
    while (!quit)
    {
        pollEvents();
        int w, h;
        SDL_GetWindowSize(SDLwindow, &w, &h);
        if (displayFunc != NULL)
            (((void *(*)(void))displayFunc)());
        if (reshapeFunc != NULL)
            (((void *(*)(int, int))reshapeFunc)(w, h));
        if (visibilityFunc != NULL)
            (((void *(*)(int))visibilityFunc)(1));
        if (idleFunc != NULL)
            (((void *(*)(void))idleFunc)());
    }
    SDL_DestroyWindow(SDLwindow);
    SDL_Quit();
}

void FGAPIENTRY glutMainLoopEvent(void)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutMainLoopEvent)(void) = dlsym(RTLD_NEXT, "glutMainLoopEvent");
        _glutMainLoopEvent();
    }
    return;
}

void FGAPIENTRY glutSwapBuffers(void)
{
    EmulatorConfig *config = getConfig();

    if (config->borderEnabled)
        drawGameBorder(config->width, config->height, config->whiteBorderPercentage, config->blackBorderPercentage);

    if (config->noSDL)
    {
        void FGAPIENTRY (*_glutSwapBuffers)(void) = dlsym(RTLD_NEXT, "glutSwapBuffers");
        _glutSwapBuffers();
        return;
    }

    switch (gId)
    {
    case OUTRUN_2_SP_SDX:
    case OUTRUN_2_SP_SDX_REVA:
    case OUTRUN_2_SP_SDX_REVA_TEST:
    case OUTRUN_2_SP_SDX_REVA_TEST2:
    case OUTRUN_2_SP_SDX_TEST:
        pollEvents();
    }

    SDL_GL_SwapWindow(SDLwindow);

    if (config->fpsLimiter)
    {
        fpsLimit.frameStart = Clock_now();
        FpsLimiter(&fpsLimit);
        fpsLimit.frameEnd = Clock_now();
    }
    char windowTitle[512];
    sprintf(windowTitle, "%s - FPS: %.2f", SDLgameTitle, calculateFps());
    SDL_SetWindowTitle(SDLwindow, windowTitle);
}

int FGAPIENTRY glutGet(GLenum type)
{
    if (getConfig()->noSDL)
    {
        int FGAPIENTRY (*_glutGet)(GLenum type) = dlsym(RTLD_NEXT, "glutGet");
        return _glutGet(type);
    }
    if (type == 0x66)
        return getConfig()->width;
    if (type == 0x67)
        return getConfig()->height;

    return 0;
}

void FGAPIENTRY glutInitWindowSize(int width, int height)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutInitWindowSize)(int width, int height) = dlsym(RTLD_NEXT, "glutInitWindowSize");
        _glutInitWindowSize(width, height);
        return;
    }
    SDL_SetWindowSize(SDLwindow, getConfig()->width, getConfig()->height);
}

void FGAPIENTRY glutInitWindowPosition(int x, int y)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutInitWindowPosition)(int width, int height) = dlsym(RTLD_NEXT, "glutInitWindowPosition");
        _glutInitWindowPosition(x, y);
        return;
    }
    SDL_SetWindowPosition(SDLwindow, x, y);
}

int FGAPIENTRY glutExtensionSupported(const char *extension)
{
    if (getConfig()->noSDL)
    {
        int FGAPIENTRY (*_glutExtensionSupported)(const char *extension) = dlsym(RTLD_NEXT, "glutExtensionSupported");
        return _glutExtensionSupported(extension);
    }
    return SDL_GL_ExtensionSupported(extension);
}

void FGAPIENTRY glutSetCursor(int glutCursor)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutSetCursor)(int glutCursor) = dlsym(RTLD_NEXT, "glutSetCursor");
        if(strcmp(getConfig()->customCursor, "") == 0)
            _glutSetCursor(glutCursor);
        return;
    }
    if (strcmp(getConfig()->customCursor, "") == 0)
    {
        SDL_Cursor *cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        SDL_SetCursor(cursor);
    }
}

void FGAPIENTRY glutDisplayFunc(void (*callback)(void))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutDisplayFunc)(void (*callback)(void)) = dlsym(RTLD_NEXT, "glutDisplayFunc");
        _glutDisplayFunc(callback);
        return;
    }
    displayFunc = callback;
}

void FGAPIENTRY glutReshapeFunc(void (*callback)(int, int))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutReshapeFunc)(void (*callback)(int, int)) = dlsym(RTLD_NEXT, "glutReshapeFunc");
        _glutReshapeFunc(callback);
        return;
    }
    reshapeFunc = callback;
}

void FGAPIENTRY glutVisibilityFunc(void (*callback)(int))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutVisibilityFunc)(void (*callback)(int)) = dlsym(RTLD_NEXT, "glutVisibilityFunc");
        _glutVisibilityFunc(callback);
        return;
    }
    visibilityFunc = callback;
}

void FGAPIENTRY glutIdleFunc(void (*callback)(void))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutIdleFunc)(void (*callback)(void)) = dlsym(RTLD_NEXT, "glutIdleFunc");
        _glutIdleFunc(callback);
        return;
    }
    idleFunc = callback;
}

void FGAPIENTRY glutInitDisplayMode(unsigned int mode)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutInitDisplayMode)(unsigned int mode) = dlsym(RTLD_NEXT, "glutInitDisplayMode");
        _glutInitDisplayMode(mode);
        return;
    }
    return;
}

void FGAPIENTRY glutGameModeString(const char *string)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutGameModeString)(const char *string) = dlsym(RTLD_NEXT, "glutGameModeString");
        _glutGameModeString(string);
        return;
    }
    return;
}

int FGAPIENTRY glutEnterGameMode(void)
{
    if (getConfig()->noSDL)
    {
        int FGAPIENTRY (*_glutEnterGameMode)(void) = dlsym(RTLD_NEXT, "glutEnterGameMode");
        char gameTitle[256] = {0};
        strcat(gameTitle, getGameName());
        glutCreateWindow(gameTitle);
        return 1;
    }
    return 0;
}

int FGAPIENTRY glutCreateWindow(const char *title)
{
    if (getConfig()->noSDL)
    {
        int FGAPIENTRY (*_glutCreateWindow)(const char *title) = dlsym(RTLD_NEXT, "glutCreateWindow");
        return _glutCreateWindow(title);
    }
    return 1;
}

void FGAPIENTRY glutJoystickFunc(void (*callback)(unsigned int, int, int, int), int pollInterval)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutJoystickFunc)(void (*callback)(unsigned int, int, int, int), int pollInterval) =
            dlsym(RTLD_NEXT, "glutJoystickFunc");
        _glutJoystickFunc(callback, pollInterval);
        return;
    }
    return;
}

void FGAPIENTRY glutPostRedisplay(void)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutPostRedisplay)(void) = dlsym(RTLD_NEXT, "glutPostRedisplay");
        _glutPostRedisplay();
        return;
    }
    return;
}

void FGAPIENTRY glutKeyboardFunc(void (*callback)(unsigned char, int, int))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutKeyboardFunc)(void (*callback)(unsigned char, int, int)) =
            dlsym(RTLD_NEXT, "glutKeyboardFunc");
        _glutKeyboardFunc(callback);
        return;
    }
    return;
}

void FGAPIENTRY glutKeyboardUpFunc(void (*callback)(unsigned char, int, int))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutKeyboardUpFunc)(void (*callback)(unsigned char, int, int)) =
            dlsym(RTLD_NEXT, "glutKeyboardUpFunc");
        _glutKeyboardUpFunc(callback);
        return;
    }
    return;
}

void FGAPIENTRY glutMouseFunc(void (*callback)(int, int, int, int))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutMouseFunc)(void (*callback)(int, int, int, int)) = dlsym(RTLD_NEXT, "glutMouseFunc");
        _glutMouseFunc(callback);
        return;
    }
    return;
}

void FGAPIENTRY glutMotionFunc(void (*callback)(int, int))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutMotionFunc)(void (*callback)(int, int)) = dlsym(RTLD_NEXT, "glutMotionFunc");
        _glutMotionFunc(callback);
        return;
    }
    return;
}

void FGAPIENTRY glutSpecialFunc(void (*callback)(int, int, int))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutSpecialFunc)(void (*callback)(int, int, int)) = dlsym(RTLD_NEXT, "glutSpecialFunc");
        _glutSpecialFunc(callback);
        return;
    }
    return;
}

void FGAPIENTRY glutSpecialUpFunc(void (*callback)(int, int, int))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutSpecialUpFunc)(void (*callback)(int, int, int)) = dlsym(RTLD_NEXT, "glutSpecialUpFunc");
        _glutSpecialUpFunc(callback);
        return;
    }
    return;
}

void FGAPIENTRY glutPassiveMotionFunc(void (*callback)(int, int))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutPassiveMotionFunc)(void (*callback)(int, int)) =
            dlsym(RTLD_NEXT, "glutPassiveMotionFunc");
        _glutPassiveMotionFunc(callback);
        return;
    }
    return;
}

void FGAPIENTRY glutEntryFunc(void (*callback)(int))
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutEntryFunc)(void (*callback)(int)) = dlsym(RTLD_NEXT, "glutEntryFunc");
        _glutEntryFunc(callback);
        return;
    }
    return;
}

void FGAPIENTRY glutLeaveGameMode()
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutLeaveGameMode)() = dlsym(RTLD_NEXT, "glutLeaveGameMode");
        _glutLeaveGameMode();
        return;
    }
    return;
}

void FGAPIENTRY glutSolidTeapot(double size)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutSolidTeapot)(double size) = dlsym(RTLD_NEXT, "glutSolidTeapot");
        _glutSolidTeapot(size);
        return;
    }
    return;
}

void FGAPIENTRY glutWireTeapot(double size)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutWireTeapot)(double size) = dlsym(RTLD_NEXT, "glutWireTeapot");
        _glutWireTeapot(size);
        return;
    }
    return;
}

void FGAPIENTRY glutSolidSphere(double radius, GLint slices, GLint stacks)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutSolidSphere)(double radius, GLint slices, GLint stacks) =
            dlsym(RTLD_NEXT, "glutSolidSphere");
        _glutSolidSphere(radius, slices, stacks);
        return;
    }
    return;
}

void FGAPIENTRY glutWireSphere(double radius, GLint slices, GLint stacks)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutWireSphere)(double radius, GLint slices, GLint stacks) =
            dlsym(RTLD_NEXT, "glutWireSphere");
        _glutWireSphere(radius, slices, stacks);
        return;
    }
    return;
}

void FGAPIENTRY glutWireCone(double base, double height, GLint slices, GLint stacks)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutWireCone)(double base, double height, GLint slices, GLint stacks) =
            dlsym(RTLD_NEXT, "glutWireCone");
        _glutWireCone(base, height, slices, stacks);
        return;
    }
    return;
}

void FGAPIENTRY glutSolidCone(double base, double height, GLint slices, GLint stacks)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutSolidCone)(double base, double height, GLint slices, GLint stacks) =
            dlsym(RTLD_NEXT, "glutSolidCone");
        _glutSolidCone(base, height, slices, stacks);
        return;
    }
    return;
}

void FGAPIENTRY glutWireCube(double dSize)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutWireCube)(double dSize) = dlsym(RTLD_NEXT, "glutWireCube");
        _glutWireCube(dSize);
        return;
    }
    return;
}

void FGAPIENTRY glutSolidCube(double dSize)
{
    if (getConfig()->noSDL)
    {
        void FGAPIENTRY (*_glutSolidCube)(double dSize) = dlsym(RTLD_NEXT, "glutSolidCube");
        _glutSolidCube(dSize);
        return;
    }
    return;
}
