#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_ONLY_PNG
#define STBI_SUPPORT_ZLIB
#define STBI_NO_SIMD


#include <SDL2/SDL.h>
#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

#include "config.h"

extern Display *x11Display;
extern Window window;

void *customCursor;
void *phTouchCursor;
void *blankCursor;

SDL_Surface *createSurfaceFromStb(unsigned char *data, int width, int height, int channels)
{
    Uint32 rmask, gmask, bmask, amask;
    if (channels == 4)
    {
        rmask = 0x000000FF;
        gmask = 0x0000FF00;
        bmask = 0x00FF0000;
        amask = 0xFF000000;
    }
    else if (channels == 3)
    {
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

    return SDL_CreateRGBSurfaceFrom(data, width, height, channels * 8, width * channels, rmask, gmask, bmask, amask);
}

int loadNewCursor(const char *cursorFileName, void **cursor, int customWidth, int customHeight)
{
    int width, height, channels;
    unsigned char *imageData = stbi_load(cursorFileName, &width, &height, &channels, 0);
    if (!imageData)
    {
        fprintf(stderr, "Failed to load image: %s\n", cursorFileName);
        return 0;
    }

    SDL_Surface *originalSurface = createSurfaceFromStb(imageData, width, height, channels);
    if (!originalSurface)
    {
        stbi_image_free(imageData);
        return 0;
    }

    SDL_Surface *resizedSurface =
        SDL_CreateRGBSurface(0, customWidth, customHeight, originalSurface->format->BitsPerPixel, originalSurface->format->Rmask,
                             originalSurface->format->Gmask, originalSurface->format->Bmask, originalSurface->format->Amask);
    if (!resizedSurface)
    {
        fprintf(stderr, "SDL_CreateRGBSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(originalSurface);
        stbi_image_free(imageData);
        return 0;
    }

    SDL_Rect srcRect = {0, 0, originalSurface->w, originalSurface->h};
    SDL_Rect destRect = {0, 0, customWidth, customHeight};
    if (SDL_BlitScaled(originalSurface, &srcRect, resizedSurface, &destRect) != 0)
    {
        fprintf(stderr, "SDL_BlitScaled Error: %s\n", SDL_GetError());
        SDL_FreeSurface(resizedSurface);
        SDL_FreeSurface(originalSurface);
        stbi_image_free(imageData);
        return 0;
    }

    if (getConfig()->noSDL == 0)
    {
        *cursor = SDL_CreateColorCursor(resizedSurface, customWidth / 2, customHeight / 2);
        if (!*cursor)
        {
            fprintf(stderr, "SDL_CreateColorCursor Error: %s\n", SDL_GetError());
            SDL_FreeSurface(resizedSurface);
            SDL_FreeSurface(originalSurface);
            stbi_image_free(imageData);
            return 0;
        }

        SDL_FreeSurface(resizedSurface);
        SDL_FreeSurface(originalSurface);
    }
    else 
    {
        XcursorImage *xcursor = XcursorImageCreate(resizedSurface->w, resizedSurface->h);
        if (!xcursor)
            return None;

        xcursor->xhot = customWidth / 2;
        xcursor->yhot = customHeight / 2;
        xcursor->width = resizedSurface->w;
        xcursor->height = resizedSurface->h;

        // Convert SDL surface to Xcursor format (32-bit ARGB)
        for (int y = 0; y < resizedSurface->h; ++y)
        {
            for (int x = 0; x < resizedSurface->w; ++x)
            {
                Uint32 pixel = ((Uint32 *)resizedSurface->pixels)[y * resizedSurface->w + x];
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixel, resizedSurface->format, &r, &g, &b, &a);
                xcursor->pixels[y * resizedSurface->w + x] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }

        *cursor = (void*)XcursorImageLoadCursor(x11Display, xcursor);
        XcursorImageDestroy(xcursor);

    }
    return 1;
}

void loadCursors()
{
    if (strcmp(getConfig()->customCursor, "") != 0)
    {
        if (!loadNewCursor(getConfig()->customCursor, (void *)&customCursor, getConfig()->customCursorWidth,
                           getConfig()->customCursorHeight))
        {
            fprintf(stderr, "Custom cursor could not be loaded!\n");
            exit(1);
        }
    }

    if (strcmp(getConfig()->phTouchCursor, "") != 0)
    {
        if (!loadNewCursor(getConfig()->phTouchCursor, (void *)&phTouchCursor, getConfig()->phTouchCursorWidth,
                            getConfig()->phTouchCursorHeight))

        {
            fprintf(stderr, "Custom cursor could not be loaded!\n");
            exit(1);
        }
    }

    if(getConfig()->noSDL)
    {
        XColor dummyColor;
        Pixmap blankPixmap;
        blankPixmap = XCreatePixmap(x11Display, window, 1, 1, 1);
        if (!blankPixmap)
        {
            fprintf(stderr, "Failed to create blank pixmap\n");
        }

        blankCursor = (void*)XCreatePixmapCursor(x11Display, blankPixmap, blankPixmap, &dummyColor, &dummyColor, 0, 0);
        if (!blankCursor)
        {
            fprintf(stderr, "Failed to create blank cursor\n");
            XFreePixmap(x11Display, blankPixmap);
        }
    }
}

void setCursor(void *cursor)
{
    if (getConfig()->noSDL == 0)
    {
        if (SDL_GetCursor() != NULL)
            SDL_SetCursor(cursor);
    }
    else
    {
        XDefineCursor(x11Display, window, (Cursor)cursor);
        XFlush(x11Display);
    }
}

void hideCursor()
{
    if (getConfig()->noSDL == 0)
    {
        SDL_ShowCursor(SDL_DISABLE);
    }
    else 
    {
        XDefineCursor(x11Display, window, (Cursor)blankCursor);
        XFlush(x11Display);
    }
}

void showCursor()
{
    if (getConfig()->noSDL == 0)
    {
        SDL_ShowCursor(SDL_ENABLE);
    }
    else
    {
        XDefineCursor(x11Display, window, (Cursor)customCursor);
        XFlush(x11Display);
    }
}

void showPhCursor()
{
    if (getConfig()->noSDL == 0)
    {
        SDL_ShowCursor(SDL_ENABLE);
    }
    else
    {
        XDefineCursor(x11Display, window, (Cursor)phTouchCursor);
        XFlush(x11Display);
    }
}