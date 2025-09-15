#ifndef _GNU_SOURCE
#define _GNU_SOURCE

#endif
#include <dlfcn.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_surface.h>
#include <stdlib.h>
#include <time.h>
#include <X11/Xlib.h>

#include "config.h"
#include "customCursor.h"

extern uint32_t gId;

SDL_Cursor *customCursor = NULL;
SDL_Cursor *touchCursor = NULL;

uint32_t hod4PrevValue;
bool hod4InTutorial = false;
void (*CPlayer_MainOri)(int);

int loadNewCursor(const char *cursorFileName, SDL_Cursor **cursor, int customWidth, int customHeight, int hotX, int hotY)
{
    SDL_Surface *originalSurface = IMG_Load(cursorFileName);
    if (!originalSurface)
    {
        fprintf(stderr, "Failed to load image %s: %s\n", cursorFileName, SDL_GetError());
        return 0;
    }

    SDL_Surface *scaledSurface = SDL_ScaleSurface(originalSurface, customWidth, customHeight, SDL_SCALEMODE_NEAREST);

    SDL_DestroySurface(originalSurface);

    if (!scaledSurface)
    {
        fprintf(stderr, "SDL_ScaleSurface Error: %s\n", SDL_GetError());
        return 0;
    }

    *cursor = SDL_CreateColorCursor(scaledSurface, hotX, hotY);
    if (!*cursor)
    {
        fprintf(stderr, "SDL_CreateColorCursor Error: %s\n", SDL_GetError());
    }

    SDL_DestroySurface(scaledSurface);

    return 1;
}

void loadCursors()
{
    if (getConfig()->customCursorEnabled && strcmp(getConfig()->customCursor, "") != 0)
    {
        if (!loadNewCursor(getConfig()->customCursor, (void *)&customCursor, getConfig()->customCursorWidth,
                           getConfig()->customCursorHeight, getConfig()->customCursorWidth / 2, getConfig()->customCursorHeight / 2))
        {
            fprintf(stderr, "Custom cursor could not be loaded!\n");
            exit(1);
        }
    }

    if (getConfig()->customCursorEnabled && strcmp(getConfig()->touchCursor, "") != 0)
    {
        if (!loadNewCursor(getConfig()->touchCursor, (void *)&touchCursor, getConfig()->touchCursorWidth, getConfig()->touchCursorHeight, 15,
                           15))

        {
            fprintf(stderr, "Custom Touch cursor could not be loaded!\n");
            exit(1);
        }
    }
}

void setCursor(void *cursor)
{
    if (SDL_GetCursor() != NULL)
        SDL_SetCursor(cursor);
}

void hideCursor()
{
    SDL_HideCursor();
}

void showCursor()
{
    SDL_ShowCursor();
}

void showPhCursor()
{
    SDL_ShowCursor();
}

int XDefineCursor(Display *display, Window w, Cursor cursor)
{
    int (*_xDefineCursor)(Display *, Window, Cursor) = dlsym(RTLD_NEXT, "XDefineCursor");
    if (gId == QUIZ_AXA || gId == QUIZ_AXA_LIVE)
        return 0;

    return _xDefineCursor(display, w, cursor);
}

// Functions to Show and Hide the cursor in HOD4 while not necessary
void CPlayer_Main(int param1)
{

    uint32_t value = *(unsigned int *)(uintptr_t)(param1 + 0x38);
    if (!getConfig()->hideCursor)
    {

        switch (value)
        {
            case 0:
            {
                if (hod4PrevValue != 3 || hod4InTutorial)
                {
                    hideCursor();
                }
                hod4PrevValue = 0;
            }
            break;
            case 3:
            {
                if (!hod4InTutorial)
                {
                    showCursor();
                    hod4PrevValue = 3;
                }
            }
            break;
            case 4:
            {
                hod4InTutorial = false;
            }
            case 2:
            case 5:
            case 11:
            {
                hideCursor();
            }
        }
    }
    CPlayer_MainOri(param1);
}

// Functions to Show and Hide the cursor in HOD4 while not necessary
int hod4VsPrintf(char *str, const char *format, va_list arg)
{
    int (*_vsprintf)(char *str, const char *format, va_list arg) = dlsym(RTLD_NEXT, "vsprintf");

    int res = _vsprintf(str, format, arg);

    if (strcmp(str, "free : tutorial\n") == 0)
    {
        hod4InTutorial = false;
    }

    if (strcmp(str, "entry : tutorial\n") == 0)
    {
        hod4InTutorial = true;
    }

    return res;
}