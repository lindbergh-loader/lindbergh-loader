#define GL_GLEXT_PROTOTYPES
#ifndef __i386__
#define __i386__
#endif
#undef __x86_64__
#include <GL/freeglut.h>
#include <GL/glx.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdbool.h>

#include "blitStretching.h"
#include "border.h"
#include "config.h"
#include "fpsLimiter.h"
#include "resolution.h"
#include "sdlCalls.h"
#include "touchScreen.h"

extern uint32_t gId;
extern int gGrp;
extern int gWidth;
extern int gHeight;
extern SDL_Window *sdlWindow;
extern Display *x11Display;
extern char sdlGameTitle[];
extern char current_text[256];
extern double localFps;
bool winHidden = true;

void *displayFunc = NULL;
void *reshapeFunc = NULL;
void *visibilityFunc = NULL;
void *idleFunc = NULL;

extern int drawableW;
extern int drawableH;
extern SDL_Renderer *fontRenderer;
extern TTF_Font *font;
int penX = 0;
int penY = 0;

void FGAPIENTRY glutInit(int *argcp, char **argv)
{
    void FGAPIENTRY (*_glutInit)(int *argcp, char **argv) = dlsym(RTLD_NEXT, "glutInit");
    startSDL(argcp, argv);
}

void FGAPIENTRY glutMainLoop(void)
{

    bool quit = false;
    while (!quit)
    {
        pollEvents();
        int w, h;
        SDL_GetWindowSize(sdlWindow, &w, &h);
        if (displayFunc != NULL)
            (((void *(*)(void))displayFunc)());
        if (reshapeFunc != NULL)
            (((void *(*)(int, int))reshapeFunc)(w, h));
        if (visibilityFunc != NULL)
            (((void *(*)(int))visibilityFunc)(1));
        if (idleFunc != NULL)
            (((void *(*)(void))idleFunc)());
    }
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
}

void FGAPIENTRY glutMainLoopEvent(void)
{
    return;
}

void FGAPIENTRY glutSwapBuffers(void)
{
    EmulatorConfig *config = getConfig();

    switch (gGrp)
    {
        case GROUP_OUTRUN:
        case GROUP_OUTRUN_TEST:
        {
            pollEvents();
        }
    }

    blitStretch();

    if (config->borderEnabled)
        drawGameBorder(drawableW, drawableH, config->whiteBorderPercentage, config->blackBorderPercentage);

    SDL_GL_SwapWindow(sdlWindow);

    if (config->fpsLimiter)
        frameTiming();

    char windowTitle[512];
    sprintf(windowTitle, "%s - FPS: %.2f", sdlGameTitle, calculateFps());
    SDL_SetWindowTitle(sdlWindow, windowTitle);
}

int FGAPIENTRY glutGet(GLenum type)
{
    if (type == 0x66)
        return gWidth;
    if (type == 0x67)
        return gHeight;

    return 0;
}

void FGAPIENTRY glutInitWindowSize(int width, int height)
{
    return;
}

void FGAPIENTRY glutInitWindowPosition(int x, int y)
{
    return;
}

int FGAPIENTRY glutExtensionSupported(const char *extension)
{
    return SDL_GL_ExtensionSupported(extension);
}

void FGAPIENTRY glutSetCursor(int glutCursor)
{
    if (strcmp(getConfig()->customCursor, "") == 0 && strcmp(getConfig()->touchCursor, "") == 0)
    {
        SDL_Cursor *cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
        SDL_SetCursor(cursor);
    }
}

void FGAPIENTRY glutDisplayFunc(void (*callback)(void))
{
    displayFunc = callback;
}

void FGAPIENTRY glutReshapeFunc(void (*callback)(int, int))
{
    return;
    // reshapeFunc = callback;
}

void FGAPIENTRY glutVisibilityFunc(void (*callback)(int))
{
    visibilityFunc = callback;
}

void FGAPIENTRY glutIdleFunc(void (*callback)(void))
{
    idleFunc = callback;
}

void FGAPIENTRY glutInitDisplayMode(unsigned int mode)
{
    return;
}

void FGAPIENTRY glutGameModeString(const char *string)
{
    return;
}

int FGAPIENTRY glutEnterGameMode(void)
{
    return 0;
}

int FGAPIENTRY glutCreateWindow(const char *title)
{
    return 1;
}

void FGAPIENTRY glutJoystickFunc(void (*callback)(unsigned int, int, int, int), int pollInterval)
{
    return;
}

void FGAPIENTRY glutPostRedisplay(void)
{
    return;
}

void FGAPIENTRY glutKeyboardFunc(void (*callback)(unsigned char, int, int))
{
    return;
}

void FGAPIENTRY glutKeyboardUpFunc(void (*callback)(unsigned char, int, int))
{
    return;
}

void FGAPIENTRY glutMouseFunc(void (*callback)(int, int, int, int))
{
    return;
}

void FGAPIENTRY glutMotionFunc(void (*callback)(int, int))
{
    return;
}

void FGAPIENTRY glutSpecialFunc(void (*callback)(int, int, int))
{
    return;
}

void FGAPIENTRY glutSpecialUpFunc(void (*callback)(int, int, int))
{
    return;
}

void FGAPIENTRY glutPassiveMotionFunc(void (*callback)(int, int))
{
    return;
}

void FGAPIENTRY glutEntryFunc(void (*callback)(int))
{
    return;
}

void FGAPIENTRY glutLeaveGameMode()
{
    return;
}

void FGAPIENTRY glutSolidTeapot(double size)
{
    return;
}

void FGAPIENTRY glutWireTeapot(double size)
{
    return;
}

void FGAPIENTRY glutSolidSphere(double radius, GLint slices, GLint stacks)
{
    return;
}

void FGAPIENTRY glutWireSphere(double radius, GLint slices, GLint stacks)
{
    return;
}

void FGAPIENTRY glutWireCone(double base, double height, GLint slices, GLint stacks)
{
    return;
}

void FGAPIENTRY glutSolidCone(double base, double height, GLint slices, GLint stacks)
{
    return;
}

void FGAPIENTRY glutWireCube(double dSize)
{
    return;
}

void FGAPIENTRY glutSolidCube(double dSize)
{
    return;
}

int FGAPIENTRY glutBitmapWidth(void *fontID, int character)
{
    int width = 0;
    if (fontID == GLUT_BITMAP_9_BY_15)
        width = 9;
    return width;
}

void convertSurfaceTo1Bit(SDL_Surface *surface, uint8_t *outBitmap, int pitch)
{
    SDL_LockSurface(surface);
    for (int y = 0; y < surface->h; ++y)
    {
        uint8_t byte = 0;
        int bit = 0;
        int sdlY = surface->h - 1 - y;

        Uint8 *row = (Uint8 *)surface->pixels + sdlY * surface->pitch;

        for (int x = 0; x < surface->w; ++x)
        {
            if (row[x])
                byte |= (1 << (7 - bit));

            bit++;
            if (bit == 8 || x == surface->w - 1)
            {
                outBitmap[y * pitch + x / 8] = byte;
                byte = 0;
                bit = 0;
            }
        }
    }
    SDL_UnlockSurface(surface);
}

void glutBitmapCharacter(void *fontID, int character)
{
    GLint glColor[4];
    glGetIntegerv(GL_CURRENT_COLOR, glColor);

    SDL_Color sdlColor;
    sdlColor.r = (Uint8)(glColor[0] * 255);
    sdlColor.g = (Uint8)(glColor[1] * 255);
    sdlColor.b = (Uint8)(glColor[2] * 255);
    sdlColor.a = (Uint8)(glColor[3] * 255);

    GLint glPos[4];
    glGetIntegerv(GL_CURRENT_RASTER_POSITION, glPos);

    if (penX != glPos[0])
        penX = glPos[0];

    penY = glPos[1];

    SDL_Surface *glyph = TTF_RenderGlyph_Solid(font, character, sdlColor);
    if (!glyph)
    {
        printf("Failed to render glyph: %s\n", SDL_GetError());
        return;
    }

    int pitch = (glyph->w + 7) / 8;
    uint8_t *bitmap = malloc(pitch * glyph->h);
    convertSurfaceTo1Bit(glyph, bitmap, pitch);

    GLint swbytes, lsbfirst, rowlen, skiprows, skippix, align;

    glGetIntegerv(GL_UNPACK_SWAP_BYTES, &swbytes);
    glGetIntegerv(GL_UNPACK_LSB_FIRST, &lsbfirst);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowlen);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skiprows);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skippix);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);

    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBitmap(glyph->w, glyph->h, 0, 0, (float)(glyph->w), 0.0, bitmap);
    glGetIntegerv(GL_CURRENT_RASTER_POSITION, glPos);

    glPixelStorei(GL_UNPACK_SWAP_BYTES, swbytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, lsbfirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, rowlen);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, skiprows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, skippix);
    glPixelStorei(GL_UNPACK_ALIGNMENT, align);

    free(bitmap);
    SDL_DestroySurface(glyph);
}