#define GL_GLEXT_PROTOTYPES
#ifndef __i386__
#define __i386__
#include <GL/gl.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_video.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include <time.h>
#endif
#undef __x86_64__
#include <GL/glx.h>
#include <GL/glxext.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>
#include <sys/time.h>

#include "fps_limiter.h"
#include "sdlcalls.h"
#include "patch.h"
#include "config.h"
#include "border.h"

bool sdlGame = false;
extern SDL_Window *SDLwindow;
extern SDL_GLContext SDLcontext;
extern char SDLgameTitle[];
extern fps_limit fpsLimit;
extern Window win;

void glXToSDLSwapBuffers(Display *dpy, GLXDrawable drawable)
{
    pollEvents();

    EmulatorConfig *config = getConfig();
    if (config->borderEnabled)
        drawGameBorder(config->width, config->height, config->whiteBorderPercentage, config->blackBorderPercentage);

    SDL_GL_SwapWindow(SDLwindow);
    if (getConfig()->fpsLimiter)
    {
        fpsLimit.frameStart = Clock_now();
        FpsLimiter(&fpsLimit);
        fpsLimit.frameEnd = Clock_now();
    }
    char windowTitle[512];
    sprintf(windowTitle, "%s - FPS: %.2f", SDLgameTitle, calculateFps());
    SDL_SetWindowTitle(SDLwindow, windowTitle);
}

int glxSDLmyCreateWindow(int *param1)
{
    glutInitSDL(0, 0);
    char *buf = malloc(512);
    memset(buf, '\0', 512);
    // param1[0x17] = (intptr_t)(void *)buf; // window;
    sdlGame = true;
    return 1;
}

XVisualInfo *glXChooseVisual(Display *dpy, int screen, int *attribList)
{
    XVisualInfo *(*_glXChooseVisual)(Display *dpy, int screen, int *attribList) = dlsym(RTLD_NEXT, "glXChooseVisual");
    if (sdlGame)
        return 0;
    return _glXChooseVisual(dpy, screen, attribList);
}

void GLAPIENTRY openglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                                    const void *userParam)
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

void glXSwapBuffers(Display *dpy, GLXDrawable drawable)
{
    void (*_glXSwapBuffers)(Display *dpy, GLXDrawable drawable) = dlsym(RTLD_NEXT, "glXSwapBuffers");

    EmulatorConfig *config = getConfig();

    // SDL2 controller polling for X11 rendered games
    pollEvents();

    if (config->borderEnabled)
        drawGameBorder(config->width, config->height, config->whiteBorderPercentage, config->blackBorderPercentage);

    int gId = config->crc32;
    if (getConfig()->noSDL && (gId == OUTRUN_2_SP_SDX || gId == OUTRUN_2_SP_SDX_REVA || gId == OUTRUN_2_SP_SDX_REVA_TEST ||
                               gId == OUTRUN_2_SP_SDX_REVA_TEST2 || gId == OUTRUN_2_SP_SDX_TEST))
    {
        XEvent event;
        while (XPending(dpy))
        {
            XNextEvent(dpy, &event);
        }
    }
    _glXSwapBuffers(dpy, drawable);
    if (config->fpsLimiter)
    {
        fpsLimit.frameStart = Clock_now();
        FpsLimiter(&fpsLimit);
        fpsLimit.frameEnd = Clock_now();
    }
    XStoreName(dpy, win, "");
}

GLXContext glXCreateContext(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct)
{
    GLXContext (*_glXCreateContext)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct) =
        dlsym(RTLD_NEXT, "glXCreateContext");
    if (sdlGame)
    {
        return SDLcontext;
    }
    GLXContext ctx = _glXCreateContext(dpy, vis, shareList, direct);
    if (getConfig()->showDebugMessages)
    {
        // Enable OpenGL debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Make the callback synchronous
        glDebugMessageCallback(openglDebugCallback, NULL);
    }

    return ctx;
}

GLXContext glXGetCurrentContext()
{
    GLXContext (*_glXGetCurrentContext)() = dlsym(RTLD_NEXT, "glXGetCurrentContext");
    if (sdlGame)
        return SDLcontext;
    return _glXGetCurrentContext();
}

GLXPbuffer glXCreatePbuffer(Display *dpy, GLXFBConfig config, const int *attrib_list)
{
    GLXPbuffer (*_glXCreatePbuffer)(Display *dpy, GLXFBConfig config, const int *attrib_list) = dlsym(RTLD_NEXT, "glXCreatePbuffer");
    if (sdlGame)
        return 0;
    return _glXCreatePbuffer(dpy, config, attrib_list);
}

void glXDestroyContext(Display *dpy, GLXContext ctx)
{
    void (*_glXDestroyContext)(Display *dpy, GLXContext ctx) = dlsym(RTLD_NEXT, "glXDestroyContext");
    if (sdlGame)
        return;
    _glXDestroyContext(dpy, ctx);
}

void glXDestroyPbuffer(Display *dpy, GLXPbuffer pbuf)
{
    void (*_glXDestroyPbuffer)(Display *dpy, GLXPbuffer pbuf) = dlsym(RTLD_NEXT, "glXDestroyPbuffer");
    if (sdlGame)
        return;
    _glXDestroyPbuffer(dpy, pbuf);
}

XVisualInfo *glXGetVisualFromFBConfig(Display *dpy, GLXFBConfig config)
{
    XVisualInfo *(*_glXGetVisualFromFBConfig)(Display *dpy, GLXFBConfig config) = dlsym(RTLD_NEXT, "glXGetVisualFromFBConfig");
    if (sdlGame)
        return 0;
    return _glXGetVisualFromFBConfig(dpy, config);
}

Bool glXMakeCurrent(Display *dpy, GLXDrawable drawable, GLXContext ctx)
{
    Bool (*_glXMakeCurrent)(Display *dpy, GLXDrawable drawable, GLXContext ctx) = dlsym(RTLD_NEXT, "glXMakeCurrent");
    if (sdlGame)
    {
        SDL_GL_MakeCurrent(SDLwindow, SDLcontext);
        return 1;
    }
    return _glXMakeCurrent(dpy, drawable, ctx);
}

Bool glXQueryExtension(Display *dpy, int *errorBase, int *eventBase)
{
    Bool (*_glXQueryExtension)(Display *dpy, int *errorBase, int *eventBase) = dlsym(RTLD_NEXT, "glXQueryExtension");
    if (sdlGame)
        return 0;
    return _glXQueryExtension(dpy, errorBase, eventBase);
}

GLXFBConfig *glXChooseFBConfig(Display *dpy, int screen, const int *attrib_list, int *nelements)
{
    GLXFBConfig *(*_glXChooseFBConfig)(Display *dpy, int screen, const int *attrib_list, int *nelements) =
        dlsym(RTLD_NEXT, "glXChooseFBConfig");

    int gId = getConfig()->crc32;
    char *__GLX_VENDOR_LIBRARY_NAME = getenv("__GLX_VENDOR_LIBRARY_NAME");
    char *__NV_PRIME_RENDER_OFFLOAD = getenv("__NV_PRIME_RENDER_OFFLOAD");
    if (__GLX_VENDOR_LIBRARY_NAME == NULL)
        __GLX_VENDOR_LIBRARY_NAME = " ";
    if (__NV_PRIME_RENDER_OFFLOAD == NULL)
        __NV_PRIME_RENDER_OFFLOAD = " ";
    if ((strcmp(__GLX_VENDOR_LIBRARY_NAME, "nvidia") == 0) && (strcmp(__NV_PRIME_RENDER_OFFLOAD, "1") == 0) &&
        ((gId == THE_HOUSE_OF_THE_DEAD_4_REVA) || (gId == THE_HOUSE_OF_THE_DEAD_4_REVB) || (gId == THE_HOUSE_OF_THE_DEAD_4_REVC) ||
         (gId == THE_HOUSE_OF_THE_DEAD_4_SPECIAL) || (gId == THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB) || (gId == THE_HOUSE_OF_THE_DEAD_EX) ||
         (gId == TOO_SPICY)))
    {
        for (int i = 0; attrib_list[i] != None; i += 2)
        {
            if (attrib_list[i] == GLX_DOUBLEBUFFER)
            {
                int *ptr = (int *)&attrib_list[i + 1];
                patchMemory((intptr_t)ptr, "01");
                setVariable((intptr_t)ptr, GLX_DONT_CARE);
            }
        }
    }
    return _glXChooseFBConfig(dpy, screen, attrib_list, nelements);
}

GLXContext glXCreateNewContext(Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct)
{
    GLXContext (*_glXCreateNewContext)(Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct) =
        dlsym(RTLD_NEXT, "glXCreateNewContext");

    GLXContext ctx = _glXCreateNewContext(dpy, config, render_type, share_list, direct);

    if (getConfig()->showDebugMessages)
    {
        // Enable OpenGL debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglDebugCallback, NULL);
    }
    return ctx;
}

GLXPbuffer glXCreateGLXPbufferSGIX(Display *dpy, GLXFBConfigSGIX config, unsigned int width, unsigned int height, int *attrib_list)
{
    if (sdlGame)
        return 0;
    int pbufferAttribs[] = {
        GLX_PBUFFER_WIDTH, width, GLX_PBUFFER_HEIGHT, height, GLX_PRESERVED_CONTENTS, true, GLX_LARGEST_PBUFFER, true, None};
    return glXCreatePbuffer(dpy, config, pbufferAttribs);
}

void glXDestroyGLXPbufferSGIX(Display *dpy, GLXPbufferSGIX pbuf)
{
    if (sdlGame)
        return;
    glXDestroyPbuffer(dpy, pbuf);
}

GLXFBConfigSGIX *glXChooseFBConfigSGIX(Display *dpy, int screen, int *attrib_list, int *nelements)
{
    if (sdlGame)
        return 0;
    int gId = getConfig()->crc32;
    char *__GLX_VENDOR_LIBRARY_NAME = getenv("__GLX_VENDOR_LIBRARY_NAME");
    char *__NV_PRIME_RENDER_OFFLOAD = getenv("__NV_PRIME_RENDER_OFFLOAD");
    if (__GLX_VENDOR_LIBRARY_NAME == NULL)
        __GLX_VENDOR_LIBRARY_NAME = " ";
    if (__NV_PRIME_RENDER_OFFLOAD == NULL)
        __NV_PRIME_RENDER_OFFLOAD = " ";
    if ((strcmp(__GLX_VENDOR_LIBRARY_NAME, "nvidia") == 0) && (strcmp(__NV_PRIME_RENDER_OFFLOAD, "1") == 0) &&
        ((gId == INITIALD_4_REVA) || (gId == INITIALD_4_REVB) || (gId == INITIALD_4_REVC) || (gId == INITIALD_4_REVD) ||
         (gId == INITIALD_4_REVG) || (gId == INITIALD_4_EXP_REVB) || (gId == INITIALD_4_EXP_REVC) || gId == INITIALD_4_EXP_REVD))
    {
        for (int i = 0; attrib_list[i] != None; i += 2)
        {
            if (attrib_list[i] == GLX_DOUBLEBUFFER)
            {
                int *ptr = (int *)&attrib_list[i + 1];
                patchMemory((intptr_t)ptr, "01");
            }
        }
    }
    return glXChooseFBConfig(dpy, screen, attrib_list, nelements);
}

int glXGetFBConfigAttribSGIX(Display *dpy, GLXFBConfigSGIX config, int attribute, int *value)
{
    if (sdlGame)
        return 0;
    return glXGetFBConfigAttrib(dpy, config, attribute, value);
}

GLXContext glXCreateContextWithConfigSGIX(Display *dpy, GLXFBConfigSGIX config, int render_type, GLXContext share_list, Bool direct)
{
    if (sdlGame)
        return 0;
    return glXCreateNewContext(dpy, config, render_type, share_list, direct);
}

const char *glXGetClientString(Display *dpy, int name)
{
    const char *(*_glXGetClientString)(Display *dpy, int name) = dlsym(RTLD_NEXT, "glXGetClientString");
    if (sdlGame)
        return "SDLGame";
    return _glXGetClientString(dpy, name);
}

Display *glXGetCurrentDisplay(void)
{
    Display *(*_glXGetCurrentDisplay)(void) = dlsym(RTLD_NEXT, "glXGetCurrentDisplay");
    if (sdlGame)
        return (Display *)SDLwindow;
    return _glXGetCurrentDisplay();
}

const char *glXQueryExtensionsString(Display *dpy, int screen)
{
    const char *(*_glXQueryExtensionsString)(Display *dpy, int screen) = dlsym(RTLD_NEXT, "glXQueryExtensionsString");
    if (sdlGame)
        return "";
    return _glXQueryExtensionsString(dpy, screen);
}

const char *glXQueryServerString(Display *dpy, int screen, int name)
{
    const char *(*_glXQueryServerString)(Display *dpy, int screen, int name) = dlsym(RTLD_NEXT, "glXQueryServerString");
    if (sdlGame)
        return "";
    return _glXQueryServerString(dpy, screen, name);
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
