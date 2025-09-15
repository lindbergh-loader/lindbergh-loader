#ifndef __i386__
#define __i386__
#endif
#undef __x86_64__
#include <GL/glx.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <X11/Xatom.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/Xlib.h>

#include "config.h"
#include "securityBoard.h"
#include "sdlCalls.h"

extern bool gettingGPUVendor;

extern bool creatingWindow;

Window window;

extern Display *x11Display;
extern Window x11Window;

extern void *customCursor;
extern void *touchCursor;

/**
 * Stop the house of the dead games turning keyboard repeating off.
 */
int XAutoRepeatOff(Display *display)
{
    return 0;
}

Display *XOpenDisplay(const char *display_name)
{
    Display *(*_XOpenDisplay)(const char *display_name) = dlsym(RTLD_NEXT, "XOpenDisplay");

    if (gettingGPUVendor || creatingWindow)
    {
        return _XOpenDisplay(display_name);
    }
    else
    {
        startSDL(0, 0);
        return x11Display;
    }
}

Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width,
                     int depth, unsigned int class, Visual *visual, unsigned long valueMask, XSetWindowAttributes *attributes)
{
    Window (*_XCreateWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height,
                             unsigned int border_width, int depth, unsigned int class, Visual *visual, unsigned long valueMask,
                             XSetWindowAttributes *attributes) = dlsym(RTLD_NEXT, "XCreateWindow");

    if ((gettingGPUVendor || creatingWindow))
        window = _XCreateWindow(display, parent, x, y, width, height, border_width, depth, class, visual, valueMask, attributes);
    else
        window = x11Window;

    return window;
}

// Here we prevent the games to change the window properties
void XSetWMProperties(Display *display, Window w, XTextProperty *window_name, XTextProperty *icon_name, char **argv, int argc,
                      XSizeHints *normal_hints, XWMHints *wm_hints, XClassHint *class_hints)
{
    return;
}

int XMapWindow(Display *display, Window window)
{
    return 0;
}

int XPending(Display *display)
{
    return 0;
}

int XGrabPointer(Display *display, Window grab_window, Bool owner_events, unsigned int event_mask, int pointer_mode, int keyboard_mode,
                 Window confine_to, Cursor cursor, Time time)
{
    return 0;
}

int XSetInputFocus(Display *display, Window window, int revert_to_window, Time time)
{
    return 0;
}

Bool XTranslateCoordinates(Display *display, Window src_w, Window dest_w, int src_x, int src_y, int *dest_x_return, int *dest_y_return,
                           Window *child_return)
{
    return true;
}

int XMoveWindow(Display * display, Window w, int x, int y)
{
    return 0;
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
    int (*_XF86VidModeGetAllModeLines)(Display *display, int screen, int *modecount_return, XF86VidModeModeInfo ***modesinfo) =
        dlsym(RTLD_NEXT, "XF86VidModeGetAllModeLines");

    if (_XF86VidModeGetAllModeLines(display, screen, modecount_return, modesinfo) != 1)
    {
        printf("Error: Could not get list of screen modes.\n");
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

void glGenFencesNV(int n, uint32_t *fences)
{
    static unsigned int curf = 1;
    while (n--)
    {
        *fences++ = curf++;
    }
    return;
}

void glDeleteFencesNV(int a, const uint32_t *b)
{
    return;
}
