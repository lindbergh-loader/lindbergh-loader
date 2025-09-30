#ifndef __i386__
#define __i386__
#endif
#undef __x86_64__
#define GL_GLEXT_PROTOTYPES

#include <GL/glx.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <SDL3/SDL.h>
#include <stdlib.h>

#include "blitStretching.h"
#include "border.h"
#include "config.h"
#include "flowControl.h"
#include "fpsLimiter.h"
#include "sdlCalls.h"
#include "touchScreen.h"

bool sdlGame = false;
extern uint32_t gId;
extern int gGrp;
extern SDL_Window *sdlWindow;
extern SDL_GLContext sdlContext;
extern Display *x11Display;
extern char sdlGameTitle[];
double localFps = 0.0;

bool createContextCalled = false;
int ctxCnt = 0;

void glXSwapBuffers(Display *dpy, GLXDrawable drawable)
{
    void (*_glXSwapBuffers)(Display *dpy, GLXDrawable drawable) = dlsym(RTLD_NEXT, "glXSwapBuffers");

    EmulatorConfig *config = getConfig();

    if (config->borderEnabled)
        drawGameBorder(config->width, config->height, config->whiteBorderPercentage, config->blackBorderPercentage);

    blitStretch();

    pollEvents();

    SDL_GL_SwapWindow(sdlWindow);

    if (config->fpsLimiter)
        frameTiming();

    char windowTitle[512];
    localFps = calculateFps();
    sprintf(windowTitle, "%s - FPS: %.2f", sdlGameTitle, localFps);
    SDL_SetWindowTitle(sdlWindow, windowTitle);
}

GLXContext glXCreateContext(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct)
{
    GLXContext (*_glXCreateContext)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct) =
        dlsym(RTLD_NEXT, "glXCreateContext");

    GLXContext ctx;
    if (ctxCnt == 0)
        ctx = (GLXContext)sdlContext;
    else
        ctx = _glXCreateContext(dpy, vis, shareList, direct);

    ctxCnt++;
    return ctx;
}

GLXFBConfig *glXChooseFBConfig(Display *dpy, int screen, const int *attrib_list, int *nelements)
{
    GLXFBConfig *(*_glXChooseFBConfig)(Display *dpy, int screen, const int *attrib_list, int *nelements) =
        dlsym(RTLD_NEXT, "glXChooseFBConfig");

    char *__GLX_VENDOR_LIBRARY_NAME = getenv("__GLX_VENDOR_LIBRARY_NAME");
    char *__NV_PRIME_RENDER_OFFLOAD = getenv("__NV_PRIME_RENDER_OFFLOAD");
    if (__GLX_VENDOR_LIBRARY_NAME == NULL)
        __GLX_VENDOR_LIBRARY_NAME = " ";
    if (__NV_PRIME_RENDER_OFFLOAD == NULL)
        __NV_PRIME_RENDER_OFFLOAD = " ";

    if (gGrp == GROUP_HOD4 || gGrp == GROUP_HOD4_SP || gId == THE_HOUSE_OF_THE_DEAD_EX || gId == TOO_SPICY || gId == QUIZ_AXA || gId == QUIZ_AXA_LIVE)
    {
        if (strcmp(__GLX_VENDOR_LIBRARY_NAME, "nvidia") == 0 && strcmp(__NV_PRIME_RENDER_OFFLOAD, "1") == 0)
        {
            for (int i = 0; attrib_list[i] != None; i += 2)
            {
                if (attrib_list[i] == GLX_DOUBLEBUFFER || attrib_list[i] == GLX_DRAWABLE_TYPE)
                {
                    int *ptr = (int *)&attrib_list[i + 1];
                    patchMemoryFromString((size_t)ptr, "01");
                }
            }
        }
    }
    return _glXChooseFBConfig(dpy, screen, attrib_list, nelements);
}

GLXPbuffer glXCreateGLXPbufferSGIX(Display *dpy, GLXFBConfigSGIX config, unsigned int width, unsigned int height, int *attrib_list)
{
    int pbufferAttribs[] = {
        GLX_PBUFFER_WIDTH, width, GLX_PBUFFER_HEIGHT, height, GLX_PRESERVED_CONTENTS, true, GLX_LARGEST_PBUFFER, true, None};
    return glXCreatePbuffer(dpy, config, pbufferAttribs);
}

void glXDestroyGLXPbufferSGIX(Display *dpy, GLXPbufferSGIX pbuf)
{
    glXDestroyPbuffer(dpy, pbuf);
}

GLXFBConfigSGIX *glXChooseFBConfigSGIX(Display *dpy, int screen, int *attrib_list, int *nelements)
{
    int gId = getConfig()->crc32;
    char *__GLX_VENDOR_LIBRARY_NAME = getenv("__GLX_VENDOR_LIBRARY_NAME");
    char *__NV_PRIME_RENDER_OFFLOAD = getenv("__NV_PRIME_RENDER_OFFLOAD");
    if (__GLX_VENDOR_LIBRARY_NAME == NULL)
        __GLX_VENDOR_LIBRARY_NAME = " ";
    if (__NV_PRIME_RENDER_OFFLOAD == NULL)
        __NV_PRIME_RENDER_OFFLOAD = " ";

    if (gGrp == GROUP_ID4_EXP || gGrp == GROUP_ID4_JAP)
    {
        if (strcmp(__GLX_VENDOR_LIBRARY_NAME, "nvidia") == 0 && strcmp(__NV_PRIME_RENDER_OFFLOAD, "1") == 0)
        {
            for (int i = 0; attrib_list[i] != None; i += 2)
            {
                if (attrib_list[i] == GLX_DOUBLEBUFFER)
                {
                    int *ptr = (int *)&attrib_list[i + 1];
                    patchMemoryFromString((size_t)ptr, "01");
                }
            }
        }
    }
    return glXChooseFBConfig(dpy, screen, attrib_list, nelements);
}

int glXGetFBConfigAttribSGIX(Display *dpy, GLXFBConfigSGIX config, int attribute, int *value)
{
    return glXGetFBConfigAttrib(dpy, config, attribute, value);
}

GLXContext glXCreateContextWithConfigSGIX(Display *dpy, GLXFBConfigSGIX config, int render_type, GLXContext share_list, Bool direct)
{
    return glXCreateNewContext(dpy, config, render_type, share_list, direct);
}

Display *glXGetCurrentDisplay(void)
{
    return x11Display;
}

int glXGetVideoSyncSGI(uint *count)
{
    static unsigned int frameCount = 0;
    *count = (frameCount++) / 2;
    return 0;
}

int glXGetRefreshRateSGI(unsigned int *rate)
{
    *rate = 60;
    return 0;
}

int glXSwapIntervalSGI(int interval)
{
    return 0;
}
