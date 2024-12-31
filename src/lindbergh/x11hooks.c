#ifndef __i386__
#define __i386__

#endif
#undef __x86_64__
#include <GL/glx.h>
#include <GL/freeglut.h>
#include <X11/Xatom.h>
#include <X11/extensions/xf86vmode.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "config.h"
#include "securityboard.h"
#include "fps_limiter.h"
#include "../libulog/ulog.h"

extern bool SDLGame;
extern bool gettingGPUVendor;

Window win;

/**
 * Stop the house of the dead games turning keyboard repeating off.
 */
int XAutoRepeatOff(Display *display)
{
    return 0;
}

Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height,
                     unsigned int border_width, int depth, unsigned int class, Visual *visual, unsigned long valueMask,
                     XSetWindowAttributes *attributes)
{
    Window (*_XCreateWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height,
                             unsigned int border_width, int depth, unsigned int class, Visual *visual,
                             unsigned long valueMask, XSetWindowAttributes *attributes) =
        dlsym(RTLD_NEXT, "XCreateWindow");

    if (SDLGame || gettingGPUVendor)
    {
        Window window = _XCreateWindow(display, parent, x, y, width, height, border_width, depth, class, visual,
                                       valueMask, attributes);
        return window;
    }

    width = getConfig()->width;
    height = getConfig()->height;

    // Ensure that the windows will respond with keyboard and mouse events
    attributes->event_mask = attributes->event_mask | KeyPressMask | KeyReleaseMask | PointerMotionMask;
    // attributes->override_redirect = False;

    Window window =
        _XCreateWindow(display, parent, x, y, width, height, border_width, depth, class, visual, valueMask, attributes);
    printf("  X11 RESOLUTION: %dx%d\n\n", width, height);

    if (getConfig()->fullscreen)
    {
        Atom wm_state = XInternAtom(display, "_NET_WM_STATE", true);
        Atom wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", true);
        XChangeProperty(display, window, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *)&wm_fullscreen, 1);
    }

    win = window;

    return window;
}

int XGrabPointer(Display *display, Window grab_window, Bool owner_events, unsigned int event_mask, int pointer_mode,
                 int keyboard_mode, Window confine_to, Cursor cursor, Time time)
{
    int (*_XGrabPointer)(Display *display, Window grab_window, Bool owner_events, unsigned int event_mask,
                         int pointer_mode, int keyboard_mode, Window confine_to, Cursor cursor, Time time) =
        dlsym(RTLD_NEXT, "XGrabPointer");
    int returnValue = _XGrabPointer(display, grab_window, owner_events, event_mask, pointer_mode, keyboard_mode,
                                    confine_to, cursor, time);
    XUngrabPointer(display, time);
    return returnValue;
}

int XGrabKeyboard(Display *display, Window grab_window, Bool owner_events, int pointer_mode, int keyboard_mode,
                  Time time)
{
    int (*_XGrabKeyboard)(Display *display, Window grab_window, Bool owner_events, int pointer_mode, int keyboard_mode,
                          Time time) = dlsym(RTLD_NEXT, "XGrabKeyboard");
    int returnValue = _XGrabKeyboard(display, grab_window, owner_events, pointer_mode, keyboard_mode, time);
    XUngrabKeyboard(display, time);
    return returnValue;
}

int XDefineCursor(Display *display, Window w, Cursor cursor)
{
    return 0;
}

int XStoreName(Display *display, Window w, const char *window_name)
{
    int (*_XStoreName)(Display *display, Window w, const char *window_name) = dlsym(RTLD_NEXT, "XStoreName");
    char windowTitle[512];
    sprintf(windowTitle, "%s - FPS: %.2f", getGameName(), calculateFps());
    return _XStoreName(display, w, windowTitle);
}

int XSetStandardProperties(Display *display, Window window, const char *window_name, const char *icon_name,
                           Pixmap icon_pixmap, char **argv, int argc, XSizeHints *hints)
{
    int (*_XSetStandardProperties)(Display *display, Window window, const char *window_name, const char *icon_name,
                                   Pixmap icon_pixmap, char **argv, int argc, XSizeHints *hints) =
        dlsym(RTLD_NEXT, "XSetStandardProperties");
    char gameTitle[256] = {0};
    strcat(gameTitle, getGameName());
    return _XSetStandardProperties(display, window, gameTitle, icon_name, icon_pixmap, argv, argc, hints);
}

Bool XF86VidModeSwitchToMode(Display *display, int screen, XF86VidModeModeInfo *modesinfo)
{
    return 0;
}

Bool XF86VidModeGetViewPort(Display *display, int screen, int *x_return, int *y_return)
{
    return 0;
}

Bool XF86VidModeGetModeLine(Display *display, int screen, int *dotclock_return, XF86VidModeModeLine *modeline)
{
    return 0;
}

int XF86VidModeGetAllModeLines(Display *display, int screen, int *modecount_return, XF86VidModeModeInfo ***modesinfo)
{
    int (*_XF86VidModeGetAllModeLines)(Display *display, int screen, int *modecount_return,
                                       XF86VidModeModeInfo ***modesinfo) =
        dlsym(RTLD_NEXT, "XF86VidModeGetAllModeLines");

    if (_XF86VidModeGetAllModeLines(display, screen, modecount_return, modesinfo) != 1)
    {
        log_error("Could not get list of screen modes.\n");
        exit(1);
    }
    else
    {
        XF86VidModeModeInfo **modes = *modesinfo;
        modes[0]->hdisplay = getConfig()->width;
        modes[0]->vdisplay = getConfig()->height;
    }
    return true;
}

int XPending(Display *display)
{
    int (*_XPending)(Display *display) = dlsym(RTLD_NEXT, "XPending");
    if (SDLGame)
        return 0;
    return _XPending(display);
}

Cursor XCreatePixmapCursor(Display *display, Pixmap source, Pixmap mask, XColor *fg, XColor *bg, unsigned int x,
                           unsigned int y)
{
    int (*_XCreatePixmapCursor)(Display *display, Pixmap source, Pixmap mask, XColor *fg, XColor *bg, unsigned int x,
                                unsigned int y) = dlsym(RTLD_NEXT, "XCreatePixmapCursor");
    if (SDLGame)
        return 0;
    return _XCreatePixmapCursor(display, source, mask, fg, bg, x, y);
}

int XFreePixmap(Display *display, Pixmap pixmap)
{
    int (*_XFreePixmap)(Display *display, Pixmap pixmap) = dlsym(RTLD_NEXT, "XFreePixmap");
    if (SDLGame)
        return 0;
    return _XFreePixmap(display, pixmap);
}

Pixmap XCreateBitmapFromData(Display *display, Drawable drawable, const char *data, unsigned int x, unsigned int y)
{
    Pixmap (*_XCreateBitmapFromData)(Display *display, Drawable drawable, const char *data, unsigned int x,
                                     unsigned int y) = dlsym(RTLD_NEXT, "XCreateBitmapFromData");
    if (SDLGame)
        return 0;
    return _XCreateBitmapFromData(display, drawable, data, x, y);
}

int XFlush(Display *display)
{
    int (*_XFlush)(Display *display) = dlsym(RTLD_NEXT, "XFlush");
    if (SDLGame)
        return 0;
    return _XFlush(display);
}

typedef unsigned int uint;

void glGenFencesNV(int n, uint *fences)
{
    static unsigned int curf = 1;
    while (n--)
    {
        *fences++ = curf++;
    }
    return;
}

void glDeleteFencesNV(int a, const uint *b)
{
    return;
}
