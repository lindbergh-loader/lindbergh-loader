#include "config.h"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "patch.h"
#include "resolution.h"

extern uint32_t gId;
extern SDL_Window *SDLwindow;
int gWidth, gHeight;

int srtvX = 0;
int srtvW, srtvH;
int phX, phY, phW, phH;
int phX2, phY2, phW2, phH2;

int vf5FSwidth;

void *glVertex3fPatchRetAddrABC[4];
void *glVertex3fNoPatchRetAddrABC[4];

GLenum curTarget = 0;
GLuint curTextureID = 0;
const GLfloat *texCoordABC[4];

vertexABC vABC[4];
int texCoordIdxABC = 0;
int vertex3fIdxABC = 0;
int creditCountABC = 0;
int lockedCountABC = 0;
float prevCreditXABC = 0;
void *abcDrawSpriteCAVEAddress;
int textureIDABC;
float newWidthRange_CEI_ABC, newHeightRange_CEI_ABC;
float newWidthABC, newHeightABC;
float newReducedWidthABC, newReducedHeightABC;
float newHalfWidthABC, newHalfHeightABC;
float newWidthRangeABC, newHeightRangeABC;
double new410ABC, new330ABC, new280ABC, new200ABC;
float angleABC;

float newVT3, newVT3HW, newVT3HH;
uint32_t *intnewVT3PTr, *intnewVT3HWPTr, *intnewVT3HHPTr;

void *idDisplayTextureCAVEAddress;
void *idDrawBallonCAVEAddress;
void *id4NewCaptionYCAVEAddress;
void *id4DrawTextAddress;
void *iddrawBallonPutAddress;
void *idTestTextAddress;
int idTextShift = 0;
float id4NewCaptionY;
float idShiftX = 0.0;
float isShiftY = 0.0;

float newHeightLGJ, newWidth2LGJ, newWidthHLGJ, newWidthSLGJ;

bool myEnableScaling = false;

int hummerRespatch()
{
    void *addr = __builtin_return_address(0);
    if ((addr == (void *)0x080d8b5d) || (addr == (void *)0x080d8b93) || (addr == (void *)0x080d8bff) || (addr == (void *)0x080d8b19) ||
        (addr == (void *)0x080d8b4f) || (addr == (void *)0x080d8bbb) || (addr == (void *)0x08159eb0) || (addr == (void *)0x08159eec) ||
        (addr == (void *)0x08159f64) || (addr == (void *)0x08163490) || (addr == (void *)0x081634cc) || (addr == (void *)0x08163544))
    {
        return gWidth - 1;
    }
    else if ((addr == (void *)0x080d8b78) || (addr == (void *)0x080d8bae) || (addr == (void *)0x080d8c1a) || (addr == (void *)0x080d8b34) ||
             (addr == (void *)0x080d8b6a) || (addr == (void *)0x080d8bd6) || (addr == (void *)0x08159ece) || (addr == (void *)0x08159f0a) ||
             (addr == (void *)0x08159f82) || (addr == (void *)0x081634ae) || (addr == (void *)0x081634ea) || (addr == (void *)0x08163562))
    {
        return gHeight;
    }
    return 1280;
}

/**
 * Function which games use to scale the output, some games behave well when this is changed
 * others less so.
 */
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{

    void (*_glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) =
        dlsym(RTLD_NEXT, "glOrtho");

    void *returnAddress = __builtin_return_address(0);

    switch (gId)
    {
    case THE_HOUSE_OF_THE_DEAD_4_REVA:
    case THE_HOUSE_OF_THE_DEAD_4_REVB:
    case THE_HOUSE_OF_THE_DEAD_4_REVC:
    {
        if ((gWidth != 1280 && gHeight != 768) || (gWidth != 640 && gHeight != 480))
        {
            right = 1280.0;
            bottom = 720.0;
        }
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL:
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB:
    {
        if (gWidth != 1024 && gHeight != 768)
        {
            if (right == 1024.0 && bottom == 768.0)
            {
                right = 800.0;
                bottom = 600.0;
            }
        }
    }
    break;
    case SEGA_RACE_TV:
    {
        if (gWidth != 640 && gHeight != 480)
        {
            if ((right == 640.0 && bottom == 480.0) && (gWidth != 640 && gHeight != 480))
            {
                right = 600000.0;
                bottom = 600000.0;
            }
        }
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_EX:
    {
        if (gWidth != 1360 && gHeight != 768)
        {
            right = 600.0;
            bottom = 480.0;
        }
    }
    break;
    default:
        break;
    }

    return _glOrtho(left, right, bottom, top, zNear, zFar);
}

/**
 * Function which games use to scale the output, some games behave well when this is changed
 * others less so.
 */
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    int (*_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height) = dlsym(RTLD_NEXT, "glViewport");

    void *returnAddress = __builtin_return_address(0);

    switch (gId)
    {
    case HUMMER:
    case HUMMER_SDLX:
    {
        if (gWidth != 1280 && gHeight != 768)
        {
            if (width == 1280 && height == 768)
            {
                width = gWidth;
                height = gHeight;
            }
        }
    }
    break;
    case OUTRUN_2_SP_SDX:
    case OUTRUN_2_SP_SDX_TEST:
    case OUTRUN_2_SP_SDX_REVA:
    case OUTRUN_2_SP_SDX_REVA_TEST:
    case OUTRUN_2_SP_SDX_REVA_TEST2:
    {
        if (gWidth != 800 && gHeight != 480)
        {
            if ((width >= 800) && (width != 1024))
            {
                width = gWidth;
                height = gHeight;
            }
        }
    }
    break;

    case SEGA_RACE_TV:
    {
        if (gWidth != 640 && gHeight != 480)
        {
            if (width == 640 && height == 480 && returnAddress == (void *)0x08123932)
            {
                x = srtvX;
                width = srtvW;
                height = srtvH;
            }
            else if (width > 640)
            {
                width = 0;
                height = 0;
            }
        }
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_REVA_TEST:
    case THE_HOUSE_OF_THE_DEAD_4_REVB_TEST:
    case THE_HOUSE_OF_THE_DEAD_4_REVC_TEST:
    case THE_HOUSE_OF_THE_DEAD_EX_TEST:
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB_TEST:
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_TEST:
    {
        if (width == 640 && height == 480)
        {
            width = gWidth;
            height = gHeight;
        }
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL:
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB:
    {
        if (gWidth != 1024 && gHeight != 768)
        {
            if (width == 1024 && height == 768)
            {
                width = gWidth;
                height = gHeight;
                x = 250;
                y = 100;
            }
        }
    }
    break;
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB:
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000:
    case VIRTUA_FIGHTER_5_R:
    case VIRTUA_FIGHTER_5_R_REVD:
    case VIRTUA_FIGHTER_5_R_REVG:
    {
        if ((gWidth != 640 && gHeight != 480) || (gWidth != 1280 && gHeight != 768))
        {
            if (width == vf5FSwidth)
            {
                width = gWidth;
            }
        }
    }
    break;
    case PRIMEVAL_HUNT:
    {
        if (isTestMode() && gWidth != 1280 && gHeight != 480)
        {
            if (getConfig()->keepAspectRatio)
            {
                width = gWidth / 2;
                height = ((gWidth / 2) / 4) * 3;
                y = (gHeight - height) / 2;
            }
            else
            {
                width = gWidth / 2;
                height = gHeight;
            }
            if (x == 640)
            {
                x = width = gWidth / 2;
            }
        }
    }
    break;
    default:
        break;
    }

    _glViewport(x, y, width, height);
}

void glTexImage2D(unsigned int target, int level, int internalformat, int width, int height, int border, unsigned int format,
                  unsigned int type, const void *pixels)
{
    void (*_glTexImage2D)(unsigned int target, int level, int internalformat, int width, int height, int border, unsigned int format,
                          unsigned int type, const void *pixels) = dlsym(RTLD_NEXT, "glTexImage2D");

    if (gWidth != 800 && gHeight != 480)
    {
        switch (gId)
        {
        case OUTRUN_2_SP_SDX:
        case OUTRUN_2_SP_SDX_REVA:
        {
            void *addr = __builtin_return_address(0);
            if ((width >= 800) && (width != 1024) && (addr != (void *)0x80d78d5) && (addr != (void *)0x080d7941))
            {
                width = gWidth;
                height = gHeight;
            }
        }
        break;
        default:
            break;
        }
    }
    _glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

void glTexParameteri(unsigned int target, unsigned int pname, int param)
{

    void (*_glTexParameteri)(unsigned int, unsigned int, int) = dlsym(RTLD_NEXT, "glTexParameteri");

    switch (gId)
    {
    case OUTRUN_2_SP_SDX:
    case OUTRUN_2_SP_SDX_REVA:
    {
        if (param == GL_NEAREST)
        {
            param = GL_LINEAR;
        }
    }
    break;
    default:
        break;
    }
    _glTexParameteri(target, pname, param);
}

int isTestMode()
{
    int fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd == -1)
    {
        return false;
    }

    char buffer[4096];
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
    if (n <= 0)
    {
        close(fd);
        return false;
    }
    close(fd);

    buffer[n] = '\0';

    char *arg = buffer;
    while (arg < buffer + n)
    {
        if (strcmp(arg, "-t") == 0)
            return true;
        arg += strlen(arg) + 1;
    }
    return false;
}

void myGlBindTexture(GLenum target, GLuint texture)
{
    curTarget = target;
    curTextureID = texture;

    void (*_glBindTexture)(GLenum, GLuint) = dlsym(RTLD_NEXT, "glBindTexture");
    _glBindTexture(target, texture);
}

void scaleABCVertex()
{
    for (int idx = 0; idx < 4; idx++)
    {
        vABC[idx].x *= (gWidth / 640.0);
        vABC[idx].y *= (gHeight / 480.0);
    }
}

void scaleNumbers()
{
    lockedCountABC++;
    float newFontWidth = 8 * ((gHeight / 480.0) * 0.8);
    float newFontHeight = 16 * ((gHeight / 480.0) * 0.8);
    float newY = 8.0 * (gHeight / 480.0);
    switch (lockedCountABC)
    {
    case 1:
    {
        float shiftLeft = ((newFontWidth * 2.5)) - 20.0;
        vABC[0].x -= shiftLeft;
    }
    break;
    case 2:
    {
        float shiftLeft = ((newFontWidth * 1.5)) - 12.0;
        vABC[0].x -= shiftLeft;
    }
    break;
    case 3:
    {
        vABC[0].x -= (newFontWidth / 2.0) - 4.0;
    }
    break;
    case 4:
    {
        float shitfRight = (newFontWidth / 2.0) - 4.0;
        vABC[0].x += shitfRight;
    }
    break;
    case 5:
    {
        float shitfRight = (newFontWidth * 1.5) - 12.0;
        vABC[0].x += shitfRight;
        lockedCountABC = 0;
    }
    }
    vABC[0].y += newY;
    vABC[1].x = vABC[0].x + newFontWidth;
    vABC[1].y = vABC[0].y;
    vABC[2].x = vABC[1].x;
    vABC[2].y = vABC[0].y + newFontHeight;
    vABC[3].x = vABC[0].x;
    vABC[3].y = vABC[2].y;
}

void scaleByAngle(float percentage, float shift)
{
    percentage = (percentage == 0.0 ? 1 : percentage / 100.0);
    float sizeX, sizeY, newHalfSizeX, newHalfSizeY;
    float idx = (gHeight / 480.0) * percentage;
    switch ((int)angleABC)
    {
    case 0:
    {
        sizeX = vABC[0].x - vABC[3].x;
        sizeY = vABC[1].y - vABC[0].y;
        newHalfSizeX = ((sizeX * idx) - sizeX) / 2.0;
        newHalfSizeY = ((sizeY * idx) - sizeY) / 2.0;
        vABC[0].x += newHalfSizeX;
        vABC[0].y -= newHalfSizeY + shift;
        vABC[1].x += newHalfSizeX;
        vABC[1].y += newHalfSizeY - shift;
        vABC[2].x -= newHalfSizeX;
        vABC[2].y += newHalfSizeY - shift;
        vABC[3].x -= newHalfSizeX;
        vABC[3].y -= newHalfSizeY + shift;
    }
    break;
    case 90:
    {
        sizeX = vABC[1].x - vABC[0].x;
        sizeY = vABC[2].y - vABC[1].y;
        newHalfSizeX = ((sizeX * idx) - sizeX) / 2.0;
        newHalfSizeY = ((sizeY * idx) - sizeY) / 2.0;
        vABC[0].x -= newHalfSizeX + shift;
        vABC[0].y -= newHalfSizeY;
        vABC[1].x += newHalfSizeX - shift;
        vABC[1].y -= newHalfSizeY;
        vABC[2].x += newHalfSizeX - shift;
        vABC[2].y += newHalfSizeY;
        vABC[3].x -= newHalfSizeX + shift;
        vABC[3].y += newHalfSizeY;
    }
    break;
    case -90:
    {
        sizeX = vABC[3].x - vABC[2].x;
        sizeY = vABC[0].y - vABC[3].y;
        newHalfSizeX = ((sizeX * idx) - sizeX) / 2.0;
        newHalfSizeY = ((sizeY * idx) - sizeY) / 2.0;
        vABC[0].x += newHalfSizeX + shift;
        vABC[0].y += newHalfSizeY;
        vABC[1].x -= newHalfSizeX - shift;
        vABC[1].y += newHalfSizeY;
        vABC[2].x -= newHalfSizeX - shift;
        vABC[2].y -= newHalfSizeY;
        vABC[3].x += newHalfSizeX + shift;
        vABC[3].y -= newHalfSizeY;
    }
    break;
    case 180:
    case -180:
    {
        sizeX = vABC[2].x - vABC[1].x;
        sizeY = vABC[3].y - vABC[2].y;
        newHalfSizeX = ((sizeX * idx) - sizeX) / 2.0;
        newHalfSizeY = ((sizeY * idx) - sizeY) / 2.0;
        vABC[0].x -= newHalfSizeX;
        vABC[0].y += newHalfSizeY + shift;
        vABC[1].x -= newHalfSizeX;
        vABC[1].y -= newHalfSizeY - shift;
        vABC[2].x += newHalfSizeX;
        vABC[2].y -= newHalfSizeY - shift;
        vABC[3].x += newHalfSizeX;
        vABC[3].y += newHalfSizeY + shift;
    }
    }
}

void shiftTarget(float percentage)
{
    percentage = percentage / 100;
    float size, newHalfSize;
    float idx = (gHeight / 480.0) * percentage;
    switch ((int)angleABC)
    {
    case 0:
    {
        size = vABC[0].x - vABC[3].x;
        newHalfSize = ((size * idx) - size);
        vABC[0].x += newHalfSize;
        vABC[1].x += newHalfSize;
        vABC[2].x += newHalfSize;
        vABC[3].x += newHalfSize;
    }
    break;
    case 90:
    {
        size = vABC[2].y - vABC[1].y;
        newHalfSize = ((size * idx) - size);
        vABC[0].y -= newHalfSize;
        vABC[1].y -= newHalfSize;
        vABC[2].y -= newHalfSize;
        vABC[3].y -= newHalfSize;
    }
    break;
    case -90:
    {
        size = vABC[0].y - vABC[3].y;
        newHalfSize = ((size * idx) - size);
        vABC[0].y += newHalfSize;
        vABC[1].y += newHalfSize;
        vABC[2].y += newHalfSize;
        vABC[3].y += newHalfSize;
    }
    break;
    case 180:
    case -180:
    {
        size = vABC[2].x - vABC[1].x;
        newHalfSize = ((size * idx) - size);
        vABC[0].x -= newHalfSize;
        vABC[1].x -= newHalfSize;
        vABC[2].x -= newHalfSize;
        vABC[3].x -= newHalfSize;
    }
    break;
    }
}

void glTexCoord2fvABC(const GLfloat *v)
{
    texCoordABC[texCoordIdxABC] = v;
    texCoordIdxABC++;
    if (texCoordIdxABC == 4)
        texCoordIdxABC = 0;
}

void glVertex3fABC1(GLfloat x, GLfloat y, GLfloat z)
{
    vABC[vertex3fIdxABC].x = x;
    vABC[vertex3fIdxABC].y = y;
    vABC[vertex3fIdxABC].z = z;
    if (vertex3fIdxABC < 3)
    {
        vertex3fIdxABC++;
        return;
    }

    if (curTarget == GL_TEXTURE_2D && (gWidth != 640 || gHeight != 480))
    {
        float sizeX, sizeY;
        sizeX = vABC[1].x - vABC[0].x;
        sizeY = vABC[2].y - vABC[1].y;

        bool scale = true;

        // Cut scene video
        if (textureIDABC == 0x20350)
        {
            float idx = (gHeight / 480.0);
            if (vABC[2].y < 65.0 && vABC[3].y < 65.0)
            {
                vABC[2].y *= idx * 1.5;
                vABC[3].y *= idx * 1.5;
            }
            else if ((vABC[0].y > (gHeight - 65.0)) && (vABC[1].y > (gHeight - 65.0)))
            {
                vABC[0].y += (gHeight - vABC[0].y) - ((gHeight - vABC[0].y) * (idx * 1.5));
                vABC[1].y += (gHeight - vABC[1].y) - ((gHeight - vABC[1].y) * (idx * 1.5));
            }
            scale = false;
        }
        // 0x20001 Effect in Engage?, 0x20013 Flash effect at the beginning of the game
        else if (textureIDABC == 0x20001 || textureIDABC == 0x20013)
        {
            scale = false;
        }
        // Arrows of the red and yellow circle targets in some enemies.
        else if (textureIDABC == 0x201C0 || textureIDABC == 0x201C1 || textureIDABC == 0x201C2)
        {
            scale = false;
        }
        // 0x20250 Light blue circle in enemies when locked
        else if (textureIDABC == 0x20250)
        {
            scaleByAngle(60, 0);
            shiftTarget(60);
            scale = false;
        }
        // 0x20251 Red circle target in some enemies when locked
        else if (textureIDABC == 0x20251)
        {
            scale = false;
        }
        // 0x200fd Light Blue cross that scales when you shoot.
        else if (textureIDABC == 0x200fd)
        {
            scaleByAngle(60, 0);
            scale = false;
        }
        // 0x200fe Red cross in enemy planes when you shoot 12x12
        else if (textureIDABC == 0x200fe)
        {
            scaleByAngle(60, 0);
            scale = false;
        }
        // 0x20342, 0x20343 Star
        else if (textureIDABC == 0x20349)
        {
            scale = false;
        }
        // X between Missile Icon and Infinite Icon while in Climax mode
        else if (textureIDABC == 0x20302)
        {
            scale = true;
        }
        // Press Start Button / Insert Coins
        else if (textureIDABC == 0x20138 || textureIDABC == 0x2013a)
        {
            float newSizeX = sizeX * (gWidth / 640.0);
            float newSizeY = sizeY * (gHeight / 480.0);
            vABC[0].x = (gWidth - newSizeX) / 2;
            vABC[1].x = vABC[0].x + newSizeX;
            vABC[2].x = vABC[1].x;
            vABC[3].x = vABC[0].x;

            vABC[0].y = gHeight - ((newSizeY * 2) + 30);
            vABC[1].y = vABC[0].y;
            vABC[2].y = vABC[0].y + newSizeY;
            vABC[3].y = vABC[0].y + newSizeY;
            scale = false;
        }
        // FREE PLAY / Credits / Credit count
        else if (textureIDABC >= 0x20057 && textureIDABC <= 0x20062)
        {
            float newSizeX = sizeX * (gWidth / 640.0);
            float newSizeY = sizeY * (gHeight / 480.0);

            vABC[0].x = (gWidth - newSizeX) / 2;
            if (sizeX == 88.0)
            {
                vABC[0].x -= (16 * (gWidth / 640.0)) * 1.5;
            }
            if (sizeX == 16.0)
            {
                float creditsSize = (88 * (gWidth / 640.0)) / 2;
                if (prevCreditXABC == x)
                    creditCountABC = 1;
                if (creditCountABC == 0)
                {
                    vABC[0].x += (creditsSize - newSizeX);
                    creditCountABC += 1;
                }
                else
                {
                    vABC[0].x += (creditsSize - newSizeX) + (newSizeX * 0.86);
                    creditCountABC = 0;
                }
                prevCreditXABC = x;
            }

            vABC[1].x = vABC[0].x + newSizeX;
            vABC[2].x = vABC[1].x;
            vABC[3].x = vABC[0].x;

            vABC[0].y = gHeight - (newSizeY + 20);
            vABC[1].y = vABC[0].y;
            vABC[2].y = vABC[0].y + newSizeY;
            vABC[3].y = vABC[0].y + newSizeY;
            scale = false;
        }
        // Text LOCKED in Climax mode
        else if (textureIDABC == 0x20064)
        {

            float sizeIncX = 26.0 * (gHeight / 480.0);
            float sizeIncY = 8.0 * (gHeight / 480.0);
            vABC[0].x -= sizeIncX / 2.0;
            vABC[1].x += sizeIncX / 2.0;
            vABC[2].x += sizeIncX / 2.0;
            vABC[2].y += sizeIncY;
            vABC[3].x -= sizeIncX / 2.0;
            vABC[3].y += sizeIncY;
            scale = false;
        }
        //  0, 1, 2, 3, 4, 5, 6, 7, 8, 9 Numbers in Climax Effect and "/"
        else if (textureIDABC == 0x20065 || textureIDABC == 0x20066 || textureIDABC == 0x20067 || textureIDABC == 0x20068 ||
                 textureIDABC == 0x2009d || textureIDABC == 0x20069 || textureIDABC == 0x2006a || textureIDABC == 0x2006b ||
                 textureIDABC == 0x2006c || textureIDABC == 0x2006d || textureIDABC == 0x2006e)
        {

            scaleNumbers();
            scale = false;
        }
        // Climax effect, 0x200cf Part of the Climax effect (glow around the aim?)
        else if (textureIDABC == 0x00200 || textureIDABC == 0x200d5 || textureIDABC == 0x200d6 || textureIDABC == 0x200d7 ||
                 textureIDABC == 0x200d8 || textureIDABC == 0x200d9 || textureIDABC == 0x200DA || textureIDABC == 0x200db ||
                 textureIDABC == 0x200dc || textureIDABC == 0x200dd || textureIDABC == 0x200de || textureIDABC == 0x200df ||
                 textureIDABC == 0x200e0 || textureIDABC == 0x2013B || textureIDABC == 0x2013F || textureIDABC == 0x2013E ||
                 textureIDABC == 0x20140 || textureIDABC == 0x20146 || textureIDABC == 0x20147 ||
                 (textureIDABC >= 0x20276 && textureIDABC <= 0x20284) || textureIDABC == 0x200cf)
        {
            scale = false;
        }
        // Stage name Effect
        else if (textureIDABC == 0x200d0)
        {
            for (int x = 0; x < 4; x++)
            {
                vABC[x].x = vABC[x].x * (gWidth / 640.0);
                vABC[x].y = vABC[x].y * (gHeight / 480.0);
            }
            scale = false;
        }
        // Crosshair
        else if (textureIDABC == 0x2014d)
        {
            scaleByAngle(80, 0);
            scale = false;
        }
        // Green arrows in the crosshair
        else if (textureIDABC == 0x2014c)
        {
            float shift = (20.0 * (gHeight / 480.0) - 20.0) / 2.0;
            scaleByAngle(80, shift - 5.0);
            switch ((int)angleABC)
            {
            case 0:
            {
                // Top
                vABC[0].x -= 2;
                vABC[1].x -= 2;
                vABC[2].x -= 2;
                vABC[3].x -= 2;
            }
            break;
            case 90:
            {
                // Left
                vABC[0].x += 3;
                vABC[1].x += 3;
                vABC[2].x += 3;
                vABC[3].x += 3;
            }
            break;
            case -90:
            {
                // Right
                vABC[0].x -= 4;
                vABC[0].y -= 1;
                vABC[1].x -= 4;
                vABC[1].y -= 1;
                vABC[2].x -= 4;
                vABC[2].y -= 1;
                vABC[3].x -= 4;
                vABC[3].y -= 1;
            }
            break;
            case 180:
            case -180:
            {
                // Bottom
                vABC[0].y -= 2;
                vABC[1].y -= 2;
                vABC[2].y -= 2;
                vABC[3].y -= 2;
            }
            }
            scale = false;
        }
        // Enemy pointing Arrows in Climax Effect
        else if (textureIDABC == 0x20141 || textureIDABC == 0x20142 || textureIDABC == 0x20143 || textureIDABC == 0x20144 ||
                 textureIDABC == 0x20145)
        {
            float sizeIncX = 6.0 * (gHeight / 480.0) - 6.0;
            float sizeIncY = 10.0 * (gHeight / 480.0) - 10.0;
            vABC[0].x -= sizeIncX / 2.0;
            vABC[0].y -= sizeIncY / 2.0;
            vABC[1].x += sizeIncX / 2.0;
            vABC[1].y -= sizeIncY / 2.0;
            vABC[2].x += sizeIncX / 2.0;
            vABC[2].y += sizeIncY / 2.0;
            vABC[3].x -= sizeIncX / 2.0;
            vABC[3].y += sizeIncY / 2.0;
            scale = false;
        }
        // 0x201E2 Friend, 0x201e3 Target, 0x201C5 Enemy and 0x201C6 Missile words
        else if (textureIDABC == 0x201E2 || textureIDABC == 0x201e3 || textureIDABC == 0x201C5 || textureIDABC == 0x201C6)
        {
            float newHalfSizeX = ((sizeX * ((gHeight / 480.0) * 0.8)) - sizeX) / 2.0;
            float newSizeY = (sizeY * ((gHeight / 480.0) * 0.8)) - sizeY;
            vABC[0].x -= newHalfSizeX;
            vABC[0].y -= newSizeY * 2;
            vABC[1].x += newHalfSizeX;
            vABC[1].y -= newSizeY * 2;
            vABC[2].x += newHalfSizeX;
            vABC[2].y -= newSizeY;
            vABC[3].x -= newHalfSizeX;
            vABC[3].y -= newSizeY;
            scale = false;
        }
        // 0x201e5 Red, 0x20291 clear, 0x201e4 Blinking Arrows
        // 0x2034E Green, 0x201e6 clear, 0x20312 Blinking Arrows
        // 0x201C7 Yellow, 0x200FF clear, 0x202E7 Blinking Arrows
        else if (textureIDABC == 0x20291 || textureIDABC == 0x2034E || textureIDABC == 0x200FF || textureIDABC == 0x202E7 ||
                 textureIDABC == 0x20312 || textureIDABC == 0x201e4 || textureIDABC == 0x201e5 || textureIDABC == 0x201e6 ||
                 textureIDABC == 0x201C7)
        {
            float newHalfSizeX = ((sizeX * ((gHeight / 480.0) * 0.8)) - sizeX) / 2.0;
            float newSizeY = (sizeY * ((gHeight / 480.0) * 0.8)) - sizeY;
            vABC[0].x -= newHalfSizeX;
            vABC[0].y -= newSizeY;
            vABC[1].x += newHalfSizeX;
            vABC[1].y -= newSizeY;
            vABC[2].x += newHalfSizeX;
            vABC[3].x -= newHalfSizeX;
            scale = false;
        }
        // Fonts and Subtitles in Cut scene Y position
        else if (textureIDABC == 0x30008)
        {
            int width = gWidth;
            int height = gHeight;
            float idx = ((height / 480.0) / 10) + 1;
            float heightShiftUp = height * ((((height / 2.0) - 216) / height) - 0.05);
            if (vABC[0].y == ((height / 2.0) - 216) || vABC[0].y == (((height / 2.0) - 216) + sizeY + 2))
            {
                // Top Subtitles
                for (int x = 0; x < 4; x++)
                {
                    vABC[x].y -= heightShiftUp;
                }
            }
            else if (vABC[0].y == 446.0) // 0/8 text scaled to fit in box
            {
                float originalPosition = vABC[0].x;
                vABC[1].x -= sizeX - 8;
                vABC[2].x -= sizeX - 8;
                vABC[2].y -= sizeY - 16;
                vABC[3].y -= sizeY - 16;
                scaleABCVertex();

                float widthScaleIdx = gWidth / 640.0;
                float fontSizeScaled = 8 * (gHeight / 480.0);
                float fontWidthAfterScaled = 8 * widthScaleIdx;
                if (originalPosition == (374 - fontSizeScaled))
                {
                    // First number
                    vABC[0].x = ((374 * widthScaleIdx) - (fontWidthAfterScaled + 5));
                }
                else if (originalPosition == (374 + fontSizeScaled))
                {
                    // Second number
                    vABC[0].x = ((374 * widthScaleIdx) + (fontWidthAfterScaled + 5));
                }
                else if (originalPosition == (374 + (fontSizeScaled * 2)))
                {
                    // Third Number
                    vABC[0].x = ((374 * widthScaleIdx) + ((fontWidthAfterScaled * 2)));
                }
                vABC[1].x = vABC[0].x + fontWidthAfterScaled;
                vABC[2].x = vABC[1].x;
                vABC[3].x = vABC[0].x;
            }
            else
            {
                // Bottom Subtitles
                for (int x = 0; x < 4; x++)
                {
                    vABC[x].y *= idx;
                }
            }
            scale = false;
        }
        // MainBD text
        else if (textureIDABC == 0x3000b)
        {
            if (!isTestMode())
            {
                for (int x = 0; x < 4; x++)
                {
                    vABC[x].y *= ((gHeight / 480.0) / 10) + 1;
                }
            }
            scale = false;
        }
        if (scale)
        {
            scaleABCVertex();
        }
    }
    for (int x = 0; x < 4; x++)
    {
        glTexCoord2fv(texCoordABC[x]);
        glVertex3f(vABC[x].x, vABC[x].y, vABC[x].z);
    }
    vertex3fIdxABC = 0;
}

void glVertex3fABC2(GLfloat x, GLfloat y, GLfloat z)
{
    void (*_glVertex3f)(GLfloat x, GLfloat y, GLfloat z) = dlsym(RTLD_NEXT, "glVertex3f");

    GLfloat scaledX = x * (gWidth / 640.0);
    GLfloat scaledY = y * (gHeight / 480.0);

    _glVertex3f(scaledX, scaledY, z);
}

void glClearColorPH(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    void (*original_glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat) =
        (void (*)(GLfloat, GLfloat, GLfloat, GLfloat))dlsym(RTLD_NEXT, "glClearColor");

    if (red == 0.5f)
    {
        red = 0.00f;
        green = 0.00f;
        blue = 0.00f;
        alpha = 0.00f;
    }
    glClearColor(red, green, blue, alpha);
}

void vf5WidthFix(void *param1)
{
    if (gWidth > 1920)
        setVariable(0x86eb208, 1920);
    (((void (*)(void *))0x080d4c24)(param1));
}

__attribute__((naked)) void idDisplayTextureFixCAVE(void *p1, void *p2, int p3, int p4, int p5)
{
    __asm__ volatile("push %%esi\n\tpush %%ebx\n\tsub $0x14, %%esp\n\tjmp *%0\n\t" ::"m"(idDisplayTextureCAVEAddress));
}

__attribute__((naked)) void idDrawBallonFixCAVE(void *p1, void *p2, float p3, float p4, float p5, float p6, float p7, bool p8)
{
    __asm__ volatile("push %%edi\n\tpush %%esi\n\tpush %%ebx\n\tsub $0x50, %%esp\n\tjmp *%0\n\t" ::"m"(idDrawBallonCAVEAddress));
}

__attribute__((naked)) void yTxtPatch()
{
    asm volatile("subss %0, %%xmm0\njmp *%1\n" ::"m"(id4NewCaptionY), "m"(id4NewCaptionYCAVEAddress));
}

__attribute__((naked)) void abcDrawSpriteFixCAVE(void *p1)
{
    __asm__ volatile("push %%ebp\n\tmov $0x3, %%ecx\n\tjmp *%0\n\t" ::"m"(abcDrawSpriteCAVEAddress));
}

void abcDrawSprite(void *p1)
{
    SprArgs *x = (SprArgs *)p1;
    textureIDABC = x->id;
    Matrix4f mat = x->mat;
    angleABC = atan2(mat.m00, mat.m10) * (180.0 / M_PI);
    abcDrawSpriteFixCAVE(p1);
}

void idDisplayTexture(void *p1, void *p2, int p3, int p4, int p5)
{
    float newX = 0;
    if (p4 > 200)
        newX = ((float)gWidth / 2) - ((1360.0 / 2) - p4);
    else
        newX = p4;

    float newY = (float)gHeight * (p5 / 768.0);

    idDisplayTextureFixCAVE(p1, p2, p3, (int)newX, (int)newY);
}

void calculateShift(int W, int H, float *ShiftX, float *ShiftY, bool right)
{
    int baseWidth1 = 1360, baseHeight1 = 768;
    int baseWidth2 = 2560, baseHeight2 = 1440;

    float knownShiftX1 = 0, knownShiftY1 = 0;
    float knownShiftX2 = 250, knownShiftY2 = 100;
    if (right)
        knownShiftX2 = 900;

    *ShiftX = ((float)(W - baseWidth1) / (baseWidth2 - baseWidth1)) * (knownShiftX2 - knownShiftX1);
    *ShiftY = ((float)(H - baseHeight1) / (baseHeight2 - baseHeight1)) * (knownShiftY2 - knownShiftY1);
}

void idDrawBallon(void *p1, void *p2, float p3, float p4, float p5, float p6, float p7, bool p8)
{
    calculateShift(gWidth, gHeight, &idShiftX, &isShiftY, p8);
    idDrawBallonFixCAVE(p1, p2, p3 + idShiftX, p4 + isShiftY, p5, p6, p7, p8);
}

void idBalloonPut(void *p1, float p2, float p3, ushort p4)
{
    (((void (*)(void *p1, float p2, float p3, ushort p4))iddrawBallonPutAddress)(p1, p2 + idShiftX, p3 + isShiftY, p4));
}

void id4DrawText(void *p1, void *p2, float p3, float p4)
{
    (((void (*)(void *p1, void *p2, float p3, float p4))id4DrawTextAddress)(p1, p2, idShiftX + p3, isShiftY + p4));
}

void idTestText(long p1, long p2, unsigned int p3, char *p4)
{
    (((void (*)(long p1, long p2, unsigned int p3, char *p4))idTestTextAddress)(p1 - idTextShift, p2, p3, p4));
}

void patchABCMarkers(uint32_t addrCtrlEnemyInfo, uint32_t addrCtrlPlayerRivalFont, uint32_t addrCtrlEnemyPursuit2d)
{
    // ctrl_enemy_information
    newWidthRange_CEI_ABC = gWidth + 100.0;
    newHeightRange_CEI_ABC = gHeight + 68.0;
    uint32_t *widthRangePtr_CEI = (uint32_t *)&newWidthRange_CEI_ABC;   // 750
    uint32_t *heightRangePtr_CEI = (uint32_t *)&newHeightRange_CEI_ABC; // 548
    setVariable(addrCtrlEnemyInfo + 0x127, (size_t)widthRangePtr_CEI);  // 750
    setVariable(addrCtrlEnemyInfo + 0x13d, (size_t)heightRangePtr_CEI); // 548
    setVariable(addrCtrlEnemyInfo + 0x42e, (size_t)widthRangePtr_CEI);  // 750
    setVariable(addrCtrlEnemyInfo + 0x44c, (size_t)heightRangePtr_CEI); // 548

    // Ctrl_Enemy_Pursuit_2d and Ctrl_Player, Rival_Font
    newWidthABC = (float)gWidth;               // 640
    newHeightABC = (float)gHeight;             // 480
    newReducedWidthABC = (float)gWidth - 40;   // 600
    newReducedHeightABC = (float)gHeight - 40; // 440
    newHalfWidthABC = (float)gWidth / 2.0;     // 320
    newHalfHeightABC = (float)gHeight / 2.0;   // 240
    newWidthRangeABC = gWidth + 90.0;          // 730
    newHeightRangeABC = gHeight + 90.0;        // 570
    new410ABC = (gWidth / 2.0) + 90.0;         // 410
    new330ABC = (gHeight / 2.0) + 90.0;        // 330
    new280ABC = (gWidth / 2.0) - 40.0;         // 280
    new200ABC = (gHeight / 2.0) - 40.0;        // 200

    uint32_t *newWidthPtrABC = (uint32_t *)&newWidthABC;
    uint32_t *newHeightPtrABC = (uint32_t *)&newHeightABC;
    setVariable(addrCtrlPlayerRivalFont + 0x15b, (size_t)newWidthPtrABC);  // 640
    setVariable(addrCtrlPlayerRivalFont + 0x163, (size_t)newHeightPtrABC); // 480
    setVariable(addrCtrlEnemyPursuit2d + 0x80, (size_t)newWidthPtrABC);    // 640
    setVariable(addrCtrlEnemyPursuit2d + 0x88, (size_t)newHeightPtrABC);   // 480

    uint32_t *newReducedWidthPtrABC = (uint32_t *)&newReducedWidthABC;
    uint32_t *newReducedHeightPtrABC = (uint32_t *)&newReducedHeightABC;
    setVariable(addrCtrlPlayerRivalFont + 0x272, (size_t)newReducedWidthPtrABC);     // 600
    setVariable(addrCtrlPlayerRivalFont + 0x29f, (size_t)newReducedHeightPtrABC);    // 440
    setVariable(addrCtrlPlayerRivalFont + 0x8ff, *(uint32_t *)&newReducedHeightABC); // 440
    setVariable(addrCtrlPlayerRivalFont + 0x96b, *(uint32_t *)&newReducedHeightABC); // 440

    setVariable(addrCtrlEnemyPursuit2d + 0x138, (size_t)newReducedWidthPtrABC);     // 600
    setVariable(addrCtrlEnemyPursuit2d + 0x15f, (size_t)newReducedHeightPtrABC);    // 440
    setVariable(addrCtrlEnemyPursuit2d + 0x7b3, *(uint32_t *)&newReducedHeightABC); // 440
    setVariable(addrCtrlEnemyPursuit2d + 0x812, *(uint32_t *)&newReducedHeightABC); // 440

    uint32_t *newHalfWidthPtrABC = (uint32_t *)&newHalfWidthABC;
    uint32_t *newHalfHeightPtrABC = (uint32_t *)&newHalfHeightABC;
    setVariable(addrCtrlPlayerRivalFont + 0x346, (size_t)newHalfWidthPtrABC);  // 320
    setVariable(addrCtrlPlayerRivalFont + 0x34e, (size_t)newHalfHeightPtrABC); // 240
    setVariable(addrCtrlPlayerRivalFont + 0x4cb, (size_t)newHalfWidthPtrABC);  // 320
    setVariable(addrCtrlPlayerRivalFont + 0x4d3, (size_t)newHalfHeightPtrABC); // 240
    setVariable(addrCtrlPlayerRivalFont + 0x519, (size_t)newHalfWidthPtrABC);  // 320
    setVariable(addrCtrlPlayerRivalFont + 0x511, (size_t)newHalfHeightPtrABC); // 240
    setVariable(addrCtrlPlayerRivalFont + 0x6dc, (size_t)newHalfWidthPtrABC);  // 320
    setVariable(addrCtrlPlayerRivalFont + 0x6e4, (size_t)newHalfHeightPtrABC); // 240
    // ctrl_player_info_rival_arrow
    setVariable(addrCtrlPlayerRivalFont + 0xbc2, *(uint32_t *)&newHalfWidthABC); // 320

    setVariable(addrCtrlEnemyPursuit2d + 0x1f7, (size_t)newHalfWidthPtrABC);  // 320
    setVariable(addrCtrlEnemyPursuit2d + 0x1ff, (size_t)newHalfHeightPtrABC); // 240
    setVariable(addrCtrlEnemyPursuit2d + 0x35d, (size_t)newHalfWidthPtrABC);  // 320
    setVariable(addrCtrlEnemyPursuit2d + 0x365, (size_t)newHalfHeightPtrABC); // 240
    setVariable(addrCtrlEnemyPursuit2d + 0x391, (size_t)newHalfWidthPtrABC);  // 320
    setVariable(addrCtrlEnemyPursuit2d + 0x399, (size_t)newHalfHeightPtrABC); // 240
    setVariable(addrCtrlEnemyPursuit2d + 0x520, (size_t)newHalfWidthPtrABC);  // 320
    setVariable(addrCtrlEnemyPursuit2d + 0x528, (size_t)newHalfHeightPtrABC); // 240

    setVariable(addrCtrlPlayerRivalFont + 0x695, *(uint32_t *)&newWidthRangeABC);  // 730
    setVariable(addrCtrlPlayerRivalFont + 0x8b6, *(uint32_t *)&newHeightRangeABC); // 570
    setVariable(addrCtrlEnemyPursuit2d + 0x4ea, *(uint32_t *)&newWidthRangeABC);   // 730
    setVariable(addrCtrlEnemyPursuit2d + 0x77b, *(uint32_t *)&newHeightRangeABC);  // 570

    uint32_t *new410PtrABC = (uint32_t *)&new410ABC;
    uint32_t *new330PtrABC = (uint32_t *)&new330ABC;
    uint32_t *new280PtrABC = (uint32_t *)&new280ABC;
    uint32_t *new200PtrABC = (uint32_t *)&new200ABC;
    setVariable(addrCtrlPlayerRivalFont + 0x40e, (size_t)new410PtrABC); // 410
    setVariable(addrCtrlPlayerRivalFont + 0x44a, (size_t)new330PtrABC); // 330
    setVariable(addrCtrlPlayerRivalFont + 0x5de, (size_t)new280PtrABC); // 280
    setVariable(addrCtrlPlayerRivalFont + 0x61a, (size_t)new200PtrABC); // 200
    setVariable(addrCtrlEnemyPursuit2d + 0x2b1, (size_t)new410PtrABC);  // 410
    setVariable(addrCtrlEnemyPursuit2d + 0x2e6, (size_t)new330PtrABC);  // 330
    setVariable(addrCtrlEnemyPursuit2d + 0x442, (size_t)new280PtrABC);  // 280
    setVariable(addrCtrlEnemyPursuit2d + 0x477, (size_t)new200PtrABC);  // 200
}

void hookABCGLFunctions(uint32_t glBindTextureAddr, uint32_t glVertex3fAddr1, uint32_t glVertex3fAddr2)
{
    replaceCallAtAddress(glBindTextureAddr, myGlBindTexture);

    replaceCallAtAddress(glVertex3fAddr1, glTexCoord2fvABC);
    replaceCallAtAddress(glVertex3fAddr1 + 0x1f, glVertex3fABC1);
    replaceCallAtAddress(glVertex3fAddr1 + 0x27, glTexCoord2fvABC);
    replaceCallAtAddress(glVertex3fAddr1 + 0x46, glVertex3fABC1);
    replaceCallAtAddress(glVertex3fAddr1 + 0x4e, glTexCoord2fvABC);
    replaceCallAtAddress(glVertex3fAddr1 + 0x67, glVertex3fABC1);
    replaceCallAtAddress(glVertex3fAddr1 + 0x6f, glTexCoord2fvABC);
    replaceCallAtAddress(glVertex3fAddr1 + 0x88, glVertex3fABC1);

    replaceCallAtAddress(glVertex3fAddr2, glVertex3fABC2);
    replaceCallAtAddress(glVertex3fAddr2 + 0x29, glVertex3fABC2);
    replaceCallAtAddress(glVertex3fAddr2 + 0x52, glVertex3fABC2);
    replaceCallAtAddress(glVertex3fAddr2 + 0x7b, glVertex3fABC2);
}

GLenum curTargetGE = 0;
GLuint curTextureIDGE = 0;
bool enableScaling = false;

void glBindTextureGE(GLenum target, GLuint texture)
{
    curTargetGE = target;
    curTextureIDGE = texture;

    void *addr = __builtin_return_address(0);

    if (getConfig()->crc32 == TOO_SPICY)
    {
        if (texture == 902 || texture == 1642)
        {                         //  55 title
            enableScaling = true; // Enable scaling
        }
        else if (texture == 903 || texture == 1686)
        {
            enableScaling = false; // Disable scaling
        }
    }

    void (*_glBindTexture)(GLenum, GLuint) = dlsym(RTLD_NEXT, "glBindTexture");
    _glBindTexture(target, texture);
}

void glVertex3fHOD4(GLfloat x, GLfloat y, GLfloat z)
{
    int (*_glVertex3f)(GLfloat x, GLfloat y, GLfloat z) = dlsym(RTLD_NEXT, "glVertex3f");

    void *returnAddress = __builtin_return_address(0);

    float scaleX = gWidth / 1280.0f;
    float scaleY = gHeight / 768.0f;

    float TestX = ((((gWidth - 1280.0) / 2.0) * 1.1116317809) / 1280.0) * (1.5 / scaleX);
    float TestY = ((((gHeight - 768.0) / 2.0) * 0.666979044) / 768.0) * (1.5 / scaleX);
    float OffsetX = TestX; // test
    float OffsetY = TestY;

    float scaleZ = fabs(z) / 1.510000;

    float AdjOffsetX = OffsetX * scaleZ;
    float AdjOffsetY = OffsetY * scaleZ;

    if (z == -1.510000f)
    {                              // logo  2d elemtents target
        if (curTextureID != 260)   // target boss fight
        {
            x += OffsetX;
            y -= OffsetY;

            x *= scaleX;
            y *= scaleY;
        }
    }
    else if (z == -1.210000f)
    { // coins  start select side
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.310000f)
    { // main bd
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.509800f)
    { // frame   2d elements
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }

    else if (z == -1.4099999666f)
    { // continue
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.6099998951f)
    { // score
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.7100000381f)
    { // side select
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.5091999769f || z == -1.5094000101f || z == -1.5095999241f)
    { // bar boss fight
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.8099999428f)
    { // score BG
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }

    _glVertex3f(x, y, z);
}

void tooSpicySC(uint32_t param_1, uint32_t param_2)
{
    myEnableScaling = true;
    SDL_WarpMouseInWindow(SDLwindow, 1280 / 2, 768 / 2);
    ((void (*)(uint32_t, uint32_t))0x082fa77c)(param_1, param_2);
}

void tooSpicyPR(uint32_t param_1, uint32_t param_2)
{
    myEnableScaling = false;
    ((void (*)(uint32_t, uint32_t))0x081eb4d4)(param_1, param_2);
}

int tooSpicyVsprintf(char *str, const char *format, va_list arg)
{
    int (*_vsprintf)(char *str, const char *format, va_list arg) = dlsym(RTLD_NEXT, "vsprintf");

    int res = _vsprintf(str, format, arg);

    if (strcmp(str, "Additional 475") == 0)
    {
        myEnableScaling = false;
    }

    return res;
}

void glVertex3f2SP(GLfloat x, GLfloat y, GLfloat z)
{
    int (*_glVertex3f)(GLfloat x, GLfloat y, GLfloat z) = dlsym(RTLD_NEXT, "glVertex3f");

    void *returnAddress = __builtin_return_address(0);

    float scaleX = gWidth / 1280.0f;
    float scaleY = gHeight / 768.0f;

    float TestX = ((((gWidth - 1280.0) / 2.0) * 1.1116317809) / 1280.0) * (1.5 / scaleX);
    float TestY = ((((gHeight - 768.0) / 2.0) * 0.666979044) / 768.0) * (1.5 / scaleX);

    float OffsetX = TestX; // test
    float OffsetY = TestY;

    float scaleZ = fabs(z) / 1.510000;

    float AdjOffsetX = OffsetX * scaleZ;
    float AdjOffsetY = OffsetY * scaleZ;

    if (z == -1.000000f || z == -3.000000f)
    {
        // We skip 3d and videos
        ;
    }
    else if (z == -1.010000f)
    {
        // bd main
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.510000f)
    {
        // logo - coin/start - 2D
        if (curTextureID == 497) // right
        {
            x -= OffsetX;
            y -= OffsetY;
            x *= scaleX;
            y *= scaleY;
        }
        else if (curTextureID == 498)
        { // deadend
            ;
        }
        else
        {
            x += OffsetX;
            y -= OffsetY;

            x *= scaleX;
            y *= scaleY;
        }
    }
    else if (z == -1.460000f)
    {
        // 2D title
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.490000f)
    {
        // 2D title
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.500000f)
    {
        // 2D title
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.310000f)
    {
        // 2D
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.210000f)
    {
        // 2D
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.5109999180f)
    {
        // 2D
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z == -1.200000f)
    {
        // target -  select
        if (curTextureID == 454 || curTextureID == 461 || curTextureID == 456)
        {
            // 457  X  - 454 461 456 O
            if (myEnableScaling)
            {
                x += AdjOffsetX;
                y -= AdjOffsetY;

                x *= scaleX;
                y *= scaleY;
            }
        }
    }
    else if (z == -1.710000f)
    {
        // 2D
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }
    else if (z > -1.85f)
    {
        // 2d
        x += AdjOffsetX;
        y -= AdjOffsetY;

        x *= scaleX;
        y *= scaleY;
    }

    _glVertex3f(x, y, z);
}

void glVertex3fHarley(GLfloat x, GLfloat y, GLfloat z)
{
    int (*_glVertex3f)(GLfloat x, GLfloat y, GLfloat z) = dlsym(RTLD_NEXT, "glVertex3f");

    float scaleX = gWidth / 1360.0f;
    float scaleY = gHeight / 768.0f;

    float OffsetX = ((gWidth - 1360.0) / 2.0) * 0.85;
    float OffsetY = ((gHeight - 768.0) / 2.0) * 0.85;

    if (curTextureID == 71)
    {

        x += OffsetX;
        y -= OffsetY;
        x *= scaleX;
        y *= scaleY;
    }
    void *addr = __builtin_return_address(0);
    if (z == -5300.7529296875)
    { // 1080p
        printf("glVertex3f hooked: x=%f, y=%f, z=%f, addr: %p\n", x, y, z, addr);
        x -= 350;
        y += 150;
    }

    _glVertex3f(x, y, z);
}

void glVertex3fHodEx(GLfloat x, GLfloat y, GLfloat z)
{
    int (*_glVertex3f)(GLfloat x, GLfloat y, GLfloat z) = dlsym(RTLD_NEXT, "glVertex3f");

    float scaleX = 600.0 / gWidth;
    float scaleY = 240.0 / gHeight;

    if (z == -1.210000f && (y == 0.0035231018f || y == -0.0152667547f || y == -0.0209428426f || // coin/start
                            y == -0.0397327021f || y == -0.0454087853f || y == -0.0641986430f || y == -0.0790739134f ||
                            y == -0.0931663215f || y == -0.0692875385f || y == -0.0833799466f ||

                            y == 0.0456045270f || y == 0.0268146675f || y == 0.0211385805f || y == 0.0023487248f || y == -0.0033273622f ||
                            y == -0.0221172161f || y == -0.0787803382f || y == -0.0934598967f ||

                            y == 0.0524549857f || y == 0.0336651318f || y == 0.0279890448f || y == 0.0091991890f

                            ))
    {

        x += scaleX;
        y -= scaleY;
    }
    else if (y == 0.0042380672f || y == -0.0169522688f || y == 0.0360235684f || y == 0.0148332343f)
    { // Main bd

        x += scaleX;
        y -= scaleY;
    }

    _glVertex3f(x, y, z);
}

int initResolutionPatches()
{
    gWidth = getConfig()->width;
    gHeight = getConfig()->height;

    switch (gId)
    {
    case AFTER_BURNER_CLIMAX:
    {
        if (gWidth == 640 && gHeight == 480)
            break;
        setVariable(0x082c0308, gWidth);
        setVariable(0x082c030c, gHeight);
        float newFontScale = gHeight / 480.0;
        unsigned int *newFontScalePtr = (unsigned int *)&newFontScale;
        setVariable(0x0806cd72, *newFontScalePtr);
        abcDrawSpriteCAVEAddress = (void *)0x08076a28 + 6;
        detourFunction(0x08076a28, abcDrawSprite);

        patchABCMarkers(0x080b066a, 0x0809d3dc, 0x080b1668);
        hookABCGLFunctions(0x08076db8, 0x08076dd2, 0x08076b78);
    }
    break;
    case AFTER_BURNER_CLIMAX_REVA:
    {
        if (gWidth == 640 && gHeight == 480)
            break;
        setVariable(0x082c0a68, gWidth);
        setVariable(0x082c0a6c, gHeight);
        float newFontScale = gHeight / 480.0;
        unsigned int *newFontScalePtr = (unsigned int *)&newFontScale;
        setVariable(0x0806cd6a, *newFontScalePtr);
        abcDrawSpriteCAVEAddress = (void *)0x08076a20 + 6;
        detourFunction(0x08076a20, abcDrawSprite);

        patchABCMarkers(0x080b06be, 0x0809d430, 0x080b16bc);
        hookABCGLFunctions(0x08076db0, 0x08076dca, 0x08076b70);
    }
    break;
    case AFTER_BURNER_CLIMAX_REVB:
    {
        if (gWidth == 640 && gHeight == 480)
            break;
        setVariable(0x082C0AE8, gWidth);
        setVariable(0x082C0AEC, gHeight);
        float newFontScale = gHeight / 480.0;
        unsigned int *newFontScalePtr = (unsigned int *)&newFontScale;
        setVariable(0x0806cd72, *newFontScalePtr);
        abcDrawSpriteCAVEAddress = (void *)0x08076a28 + 6;
        detourFunction(0x08076a28, abcDrawSprite);

        patchABCMarkers(0x080b06c6, 0x0809d438, 0x080b16c4);
        hookABCGLFunctions(0x08076db8, 0x08076dd2, 0x08076b78);
    }
    break;
    case AFTER_BURNER_CLIMAX_SDX:
    {
        if (gWidth == 640 && gHeight == 480)
            break;
        setVariable(0x082c2228, gWidth);
        setVariable(0x082c222c, gHeight);
        float newFontScale = gHeight / 480.0;
        unsigned int *newFontScalePtr = (unsigned int *)&newFontScale;
        setVariable(0x0806ce2e, *newFontScalePtr);
        abcDrawSpriteCAVEAddress = (void *)0x08076ae4 + 6;
        detourFunction(0x08076ae4, abcDrawSprite);

        patchABCMarkers(0x080b0a22, 0x0809d6cc, 0x080b1a20);
        hookABCGLFunctions(0x08076e74, 0x08076e8e, 0x08076c34);
    }
    break;
    case AFTER_BURNER_CLIMAX_SDX_REVA:
    {
        if (gWidth == 640 && gHeight == 480)
            break;
        setVariable(0x082c2228, gWidth);
        setVariable(0x082c222c, gHeight);
        float newFontScale = gHeight / 480.0;
        unsigned int *newFontScalePtr = (unsigned int *)&newFontScale;
        setVariable(0x0806ce36, *newFontScalePtr);
        abcDrawSpriteCAVEAddress = (void *)0x08076aec + 6;
        detourFunction(0x08076aec, abcDrawSprite);

        patchABCMarkers(0x080b0a2a, 0x0809d6d4, 0x080b1a28);
        hookABCGLFunctions(0x08076e7c, 0x08076e96, 0x08076c3c);
    }
    break;
    case AFTER_BURNER_CLIMAX_SE:
    {
        if (gWidth == 640 && gHeight == 480)
            break;
        setVariable(0x082c0b28, gWidth);
        setVariable(0x082c0b2c, gHeight);
        float newFontScale = gHeight / 480.0;
        unsigned int *newFontScalePtr = (unsigned int *)&newFontScale;
        setVariable(0x0806cd6a, *newFontScalePtr);
        abcDrawSpriteCAVEAddress = (void *)0x08076a20 + 6;
        detourFunction(0x08076a20, abcDrawSprite);

        patchABCMarkers(0x080b06be, 0x0809d430, 0x080b16bc);
        hookABCGLFunctions(0x08076db0, 0x08076dca, 0x08076b70);
    }
    break;
    case AFTER_BURNER_CLIMAX_SE_REVA:
    {
        if (gWidth == 640 && gHeight == 480)
            break;
        setVariable(0x082c0b28, gWidth);
        setVariable(0x082c0b2c, gHeight);
        float newFontScale = gHeight / 480.0;
        unsigned int *newFontScalePtr = (unsigned int *)&newFontScale;
        setVariable(0x0806cd72, *newFontScalePtr);
        abcDrawSpriteCAVEAddress = (void *)0x08076a28 + 6;
        detourFunction(0x08076a28, abcDrawSprite);

        patchABCMarkers(0x080b06c6, 0x0809d438, 0x080b16c4);
        hookABCGLFunctions(0x08076db8, 0x08076dd2, 0x08076b78);
    }
    break;
    case GHOST_SQUAD_EVOLUTION:
    {
        // test
        // patchMemory(0x081c1d71, "38303078363030");
        setVariable(0x082c87f8, 0x00000550);
        setVariable(0x082c87fc, 0x00000300);
    }
    break;
    case HARLEY_DAVIDSON:
    {
        int w = gWidth;
        int h = gHeight;
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
            {
                setVariable(0x088a55e0, w); // render res
                patchMemory(0x08056c31, "9090909090");
            }
            break;
        }
        else if (gWidth == 640 && gHeight == 480)
        {
            break;
        }
        setVariable(0x088a57e0, w); // main res
        setVariable(0x088a57e4, h);
        setVariable(0x088a57e8, w);
        setVariable(0x088a57ec, h);
        setVariable(0x088a57f0, w);
        setVariable(0x088a57f4, h);
        setVariable(0x088a55e0, w); // render res
        patchMemory(0x08056c31, "9090909090");
        // setVariable(0x08056c2d, w); // render res
        patchMemory(0x082a8500, "9090909090"); // restype
        patchMemory(0x088a55e4, "14");
        patchMemory(0x082a2412, "00008040"); // Fixes white logo background.
        unsigned int idx = ((21 * h) / 768) + 1;
        setVariable(0x082a250e, idx);
        detourFunction(0x0804c700, glVertex3fHarley);
        detourFunction(0x0804d220, myGlBindTexture);
    }
    break;
    case HUMMER:
    {
        if (gWidth <= 1280 && gHeight <= 768)
            break;
        replaceCallAtAddress(0x080d8b58, hummerRespatch);
        replaceCallAtAddress(0x080d8b73, hummerRespatch);
        replaceCallAtAddress(0x080d8b8e, hummerRespatch);
        replaceCallAtAddress(0x080d8ba9, hummerRespatch);
        replaceCallAtAddress(0x080d8bfa, hummerRespatch);
        replaceCallAtAddress(0x080d8c15, hummerRespatch);
    }
    break;
    case HUMMER_EXTREME:
    {
        if (gWidth <= 1280 && gHeight <= 768)
            break;
        replaceCallAtAddress(0x08159eab, hummerRespatch);
        replaceCallAtAddress(0x08159ec9, hummerRespatch);
        replaceCallAtAddress(0x08159ee7, hummerRespatch);
        replaceCallAtAddress(0x08159f05, hummerRespatch);
        replaceCallAtAddress(0x08159f5f, hummerRespatch);
        replaceCallAtAddress(0x08159f7d, hummerRespatch);
    }
    break;
    case HUMMER_EXTREME_MDX:
    {
        if (gWidth <= 1280 && gHeight <= 768)
            break;
        replaceCallAtAddress(0x0816348b, hummerRespatch);
        replaceCallAtAddress(0x081634a9, hummerRespatch);
        replaceCallAtAddress(0x081634c7, hummerRespatch);
        replaceCallAtAddress(0x081634e5, hummerRespatch);
        replaceCallAtAddress(0x0816353f, hummerRespatch);
        replaceCallAtAddress(0x0816355d, hummerRespatch);
    }
    break;
    case HUMMER_SDLX:
    {
        if (gWidth <= 1280 && gHeight <= 768)
            break;
        replaceCallAtAddress(0x080d8b14, hummerRespatch);
        replaceCallAtAddress(0x080d8b2f, hummerRespatch);
        replaceCallAtAddress(0x080d8b4a, hummerRespatch);
        replaceCallAtAddress(0x080d8b65, hummerRespatch);
        replaceCallAtAddress(0x080d8bb6, hummerRespatch);
        replaceCallAtAddress(0x080d8bd1, hummerRespatch);
    }
    break;
    case INITIALD_4_EXP_REVB:
    {
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
            {
                setVariable(0x082560a2, gWidth); // Set ResX
                setVariable(0x08256162, gWidth); // Set ResX
                setVariable(0x08256222, gWidth); // Set ResX
                setVariable(0x08256281, gWidth); // Set ResX
            }
            break;
        }
        setVariable(0x0837979d, 0x0000f0e9); // Force set resolution
        setVariable(0x08379893, gWidth);     // Set ResX
        setVariable(0x08379898, gHeight);    // Set ResY
        // Renderbuffer Resolution
        setVariable(0x082560a2, gWidth);  // Set ResX
        setVariable(0x0825609a, gHeight); // Set ResY
        setVariable(0x08256162, gWidth);  // Set ResX
        setVariable(0x0825615a, gHeight); // Set ResY
        setVariable(0x08256222, gWidth);  // Set ResX
        setVariable(0x0825621a, gHeight); // Set ResY
        setVariable(0x08256281, gWidth);  // Set ResX
        setVariable(0x08256279, gHeight); // Set ResY

        setVariable(0x08256b27, gWidth);  // Set ResX
        setVariable(0x08256b1f, gHeight); // Set ResY
        setVariable(0x08256bb2, gWidth);  // Set ResX
        setVariable(0x08256baa, gHeight); // Set ResY

        idDisplayTextureCAVEAddress = (void *)0x08337360 + 5;
        detourFunction(0x08337360, idDisplayTexture);
        // FSAA
        patchMemory(0x0855e762, "9090");
        patchMemory(0x0857dfe9, "01"); // FSAA Enabled
        setVariable(0x089ea1d0, 1);    // FSAA Quality
        // Fix Subtitles Position
        id4NewCaptionY = ((gHeight - 768.0) / 2) + 64.0;
        detourFunction(0x081f0f91, yTxtPatch);
        id4NewCaptionYCAVEAddress = (void *)(0x081f0f91 + 8);
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x081ef2ee + 6;
        id4DrawTextAddress = (void *)0x081f0454;
        detourFunction(0x081ef2ee, idDrawBallon);
        replaceCallAtAddress(0x081f084a, id4DrawText);
        // Scale Testmode text
        if (isTestMode() && gWidth >= 1920)
        {
            patchMemory(0x0899bb78, "02");
            idTextShift = (gWidth == 1920 ? 27 : ((1920 / gWidth) * 27) + 2);
            idTestTextAddress = (void *)0x08579e92;
            replaceCallAtAddress(0x0835a1fe, idTestText);
        }
    }
    break;
    case INITIALD_4_EXP_REVC:
    {
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
            {
                setVariable(0x08256192, gWidth); // Set ResX
                setVariable(0x08256252, gWidth); // Set ResX
                setVariable(0x08256312, gWidth); // Set ResX
                setVariable(0x08256371, gWidth); // Set ResX
            }
            break;
        }
        setVariable(0x0837961d, 0x0000f0e9); // Force set resolution
        setVariable(0x08379713, gWidth);     // Set ResX
        setVariable(0x08379718, gHeight);    // Set ResY
        // Renderbuffer Resolution
        setVariable(0x08256192, gWidth);  // Set ResX
        setVariable(0x0825618a, gHeight); // Set ResY
        setVariable(0x08256252, gWidth);  // Set ResX
        setVariable(0x0825624a, gHeight); // Set ResY
        setVariable(0x08256312, gWidth);  // Set ResX
        setVariable(0x0825630a, gHeight); // Set ResY
        setVariable(0x08256371, gWidth);  // Set ResX
        setVariable(0x08256369, gHeight); // Set ResY
        setVariable(0x08256c17, gWidth);  // Set ResX
        setVariable(0x08256c0f, gHeight); // Set ResY
        setVariable(0x08256ca2, gWidth);  // Set ResX
        setVariable(0x08256c9a, gHeight); // Set ResY

        idDisplayTextureCAVEAddress = (void *)0x08337200 + 5;
        detourFunction(0x08337200, idDisplayTexture);
        // FSAA
        patchMemory(0x0855e4e2, "9090");
        patchMemory(0x0857dd69, "01"); // FSAA Enabled
        setVariable(0x089ea1d0, 1);    // FSAA Quality
        // Fix Subtitles
        float newCaptionY = ((gHeight - 768.0) / 2) + 64.0;
        setVariable(0x089a7d58, *(unsigned int *)&newCaptionY);
        // Fix Subtitles Position
        id4NewCaptionY = ((gHeight - 768.0) / 2) + 64.0;
        detourFunction(0x081f1051, yTxtPatch);
        id4NewCaptionYCAVEAddress = (void *)(0x081f1051 + 8);
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x081ef39e + 6;
        id4DrawTextAddress = (void *)0x081f0504;
        detourFunction(0x081ef39e, idDrawBallon);
        replaceCallAtAddress(0x081f08fc, id4DrawText);
        // Scale Testmode text
        if (isTestMode() && gWidth >= 1920)
        {
            patchMemory(0x0899bb78, "02");
            idTextShift = (gWidth == 1920 ? 27 : ((1920 / gWidth) * 27) + 2);
            idTestTextAddress = (void *)0x08579c12;
            replaceCallAtAddress(0x0835a09e, idTestText);
        }
    }
    break;
    case INITIALD_4_EXP_REVD:
    {
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
            {
                setVariable(0x0825728a, gWidth); // Set ResX
                setVariable(0x0825734a, gWidth); // Set ResX
                setVariable(0x0825740a, gWidth); // Set ResX
                setVariable(0x08257469, gWidth); // Set ResX
            }
            break;
        }
        setVariable(0x0837b12d, 0x0000f0e9); // Force set resolution
        setVariable(0x0837b223, gWidth);     // Set ResX
        setVariable(0x0837b228, gHeight);    // Set ResY
        // Renderbuffer Resolution
        setVariable(0x0825728a, gWidth);  // Set ResX
        setVariable(0x08257282, gHeight); // Set ResY
        setVariable(0x0825734a, gWidth);  // Set ResX
        setVariable(0x08257342, gHeight); // Set ResY
        setVariable(0x0825740a, gWidth);  // Set ResX
        setVariable(0x08257402, gHeight); // Set ResY
        setVariable(0x08257469, gWidth);  // Set ResX
        setVariable(0x08257461, gHeight); // Set ResY
        setVariable(0x08257d0f, gWidth);  // Set ResX
        setVariable(0x08257d07, gHeight); // Set ResY
        setVariable(0x08257d9a, gWidth);  // Set ResX
        setVariable(0x08257d92, gHeight); // Set ResY

        idDisplayTextureCAVEAddress = (void *)0x08338d50 + 5;
        detourFunction(0x08338d50, idDisplayTexture);
        // FSAA
        patchMemory(0x0855fff2, "9090");
        patchMemory(0x0857f879, "01"); // FSAA Enabled
        setVariable(0x089ed930, 1);    // FSAA Quality
        // Fix Subtitles Position
        id4NewCaptionY = ((gHeight - 768.0) / 2) + 64.0;
        detourFunction(0x081f2061, yTxtPatch);
        id4NewCaptionYCAVEAddress = (void *)(0x081f2061 + 8);
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x081f03be + 6;
        id4DrawTextAddress = (void *)0x081f1524;
        detourFunction(0x081f03be, idDrawBallon);
        replaceCallAtAddress(0x081f191a, id4DrawText);
        // Scale Testmode text
        if (isTestMode() && gWidth >= 1920)
        {
            patchMemory(0x0899f2d8, "02");
            idTextShift = (gWidth == 1920 ? 27 : ((1920 / gWidth) * 27) + 2);
            idTestTextAddress = (void *)0x0857b722;
            replaceCallAtAddress(0x0835bbee, idTestText);
        }
    }
    break;
    case INITIALD_4_REVA:
    {
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
            {
                setVariable(0x08247e77, gWidth); // Set ResX
                setVariable(0x08247f37, gWidth); // Set ResX
                setVariable(0x08247ff7, gWidth); // Set ResX
                setVariable(0x08248056, gWidth); // Set ResX
            }
            break;
        }
        setVariable(0x0835640d, 0x0000f0e9);          // Force set resolution
        setVariable(0x08356503, gWidth);              // Set ResX
        setVariable(0x08356508, gHeight);             // Set ResY
        // Renderbuffer Resolution
        setVariable(0x08247e77, gWidth);  // Set ResX
        setVariable(0x08247e6f, gHeight); // Set ResY
        setVariable(0x08247f37, gWidth);  // Set ResX
        setVariable(0x08247f2f, gHeight); // Set ResY
        setVariable(0x08247ff7, gWidth);  // Set ResX
        setVariable(0x08247fef, gHeight); // Set ResY
        setVariable(0x08248056, gWidth);  // Set ResX
        setVariable(0x0824804e, gHeight); // Set ResY
        setVariable(0x082487e7, gWidth);  // Set ResX
        setVariable(0x082487df, gHeight); // Set ResY
        setVariable(0x08248872, gWidth);  // Set ResX
        setVariable(0x0824886a, gHeight); // Set ResY

        idDisplayTextureCAVEAddress = (void *)0x083188a0 + 5;
        detourFunction(0x083188a0, idDisplayTexture);
        // FSAA
        patchMemory(0x08538c72, "9090");
        patchMemory(0x085582c9, "01"); // FSAA Enabled
        setVariable(0x089c7af0, 1);    // FSAA Quality
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x081dacde + 6;
        id4DrawTextAddress = (void *)0x081dbe42;
        detourFunction(0x081dacde, idDrawBallon);
        replaceCallAtAddress(0x081dc23a, id4DrawText);
    }
    break;
    case INITIALD_4_REVB:
    {
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
            {
                setVariable(0x08248037, gWidth); // Set ResX
                setVariable(0x082480f7, gWidth); // Set ResX
                setVariable(0x082481b7, gWidth); // Set ResX
                setVariable(0x08248216, gWidth); // Set ResX
            }
            break;
        }
        setVariable(0x0835664d, 0x0000f0e9);          // Force set resolution
        setVariable(0x08356743, gWidth);              // Set ResX
        setVariable(0x08356748, gHeight);             // Set ResY
        // Renderbuffer Resolution
        setVariable(0x08248037, gWidth);  // Set ResX
        setVariable(0x0824802f, gHeight); // Set ResY
        setVariable(0x082480f7, gWidth);  // Set ResX
        setVariable(0x082480ef, gHeight); // Set ResY
        setVariable(0x082481b7, gWidth);  // Set ResX
        setVariable(0x082481af, gHeight); // Set ResY
        setVariable(0x08248216, gWidth);  // Set ResX
        setVariable(0x0824820e, gHeight); // Set ResY
        setVariable(0x082489a7, gWidth);  // Set ResX
        setVariable(0x0824899f, gHeight); // Set ResY
        setVariable(0x08248a32, gWidth);  // Set ResX
        setVariable(0x08248a2a, gHeight); // Set ResY

        idDisplayTextureCAVEAddress = (void *)0x08318ab0 + 5;
        detourFunction(0x08318ab0, idDisplayTexture);
        // FSAA
        patchMemory(0x08538eb2, "9090");
        patchMemory(0x08558509, "01"); // FSAA Enabled
        setVariable(0x089c7dd0, 1);    // FSAA Quality
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x081dadde + 6;
        id4DrawTextAddress = (void *)0x081dbf42;
        detourFunction(0x081dadde, idDrawBallon);
        replaceCallAtAddress(0x081dc33a, id4DrawText);
    }
    break;
    case INITIALD_4_REVC:
    {
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
            {
                setVariable(0x08248ad7, gWidth); // Set ResX
                setVariable(0x08248b97, gWidth); // Set ResX
                setVariable(0x08248c57, gWidth); // Set ResX
                setVariable(0x08248cb6, gWidth); // Set ResX
            }
            break;
        }
        setVariable(0x0835eebd, 0x0000f0e9);          // Force set resolution
        setVariable(0x0835efb3, gWidth);              // Set ResX
        setVariable(0x0835efb8, gHeight);             // Set ResY
        // Renderbuffer Resolution
        setVariable(0x08248ad7, gWidth);  // Set ResX
        setVariable(0x08248acf, gHeight); // Set ResY
        setVariable(0x08248b97, gWidth);  // Set ResX
        setVariable(0x08248b8f, gHeight); // Set ResY
        setVariable(0x08248c57, gWidth);  // Set ResX
        setVariable(0x08248c4f, gHeight); // Set ResY
        setVariable(0x08248cb6, gWidth);  // Set ResX
        setVariable(0x08248cae, gHeight); // Set ResY
        setVariable(0x08249447, gWidth);  // Set ResX
        setVariable(0x0824943f, gHeight); // Set ResY
        setVariable(0x082494d2, gWidth);  // Set ResX
        setVariable(0x082494ca, gHeight); // Set ResY

        idDisplayTextureCAVEAddress = (void *)0x0831ea60 + 5;
        detourFunction(0x0831ea60, idDisplayTexture);
        // FSAA
        patchMemory(0x08541722, "9090");
        patchMemory(0x08560d79, "01"); // FSAA Enabled
        setVariable(0x089d2f70, 1);    // FSAA Quality
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x081db63e + 6;
        id4DrawTextAddress = (void *)0x081dc7a4;
        detourFunction(0x081db63e, idDrawBallon);
        replaceCallAtAddress(0x081dcb9a, id4DrawText);
    }
    break;
    case INITIALD_4_REVD:
    {
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
            {
                setVariable(0x08248f47, gWidth); // Set ResX
                setVariable(0x08249007, gWidth); // Set ResX
                setVariable(0x082490c7, gWidth); // Set ResX
                setVariable(0x08249126, gWidth); // Set ResX
            }
            break;
        }
        setVariable(0x0835c55d, 0x0000f0e9);          // Force set resolution
        setVariable(0x0835c653, gWidth);              // Set ResX
        setVariable(0x0835c658, gHeight);             // Set ResY
        // Renderbuffer Resolution
        setVariable(0x08248f47, gWidth);  // Set ResX
        setVariable(0x08248f3f, gHeight); // Set ResY
        setVariable(0x08249007, gWidth);  // Set ResX
        setVariable(0x08248fff, gHeight); // Set ResY
        setVariable(0x082490c7, gWidth);  // Set ResX
        setVariable(0x082490bf, gHeight); // Set ResY
        setVariable(0x08249126, gWidth);  // Set ResX
        setVariable(0x0824911e, gHeight); // Set ResY
        setVariable(0x082498b7, gWidth);  // Set ResX
        setVariable(0x082498af, gHeight); // Set ResY
        setVariable(0x08249942, gWidth);  // Set ResX
        setVariable(0x0824993a, gHeight); // Set ResY

        idDisplayTextureCAVEAddress = (void *)0x0831bf90 + 5;
        detourFunction(0x0831bf90, idDisplayTexture);
        // FSAA
        patchMemory(0x0853edc2, "9090");
        patchMemory(0x0855e419, "01"); // FSAA Enabled
        setVariable(0x089cff30, 1);    // FSAA Quality
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x081dbaae + 6;
        id4DrawTextAddress = (void *)0x081dcc14;
        detourFunction(0x081dbaae, idDrawBallon);
        replaceCallAtAddress(0x081dd00a, id4DrawText);
    }
    break;
    case INITIALD_4_REVG:
    {
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
            {
                setVariable(0x08262eda, gWidth); // Set ResX
                setVariable(0x08262f9a, gWidth); // Set ResX
                setVariable(0x0826305a, gWidth); // Set ResX
                setVariable(0x082630b9, gWidth); // Set ResX
            }
            break;
        }
        setVariable(0x08393fbd, 0x0000f0e9);          // Force set resolution
        setVariable(0x083940b3, gWidth);              // Set ResX
        setVariable(0x083940b8, gHeight);             // Set ResY
        // Renderbuffer Resolution
        setVariable(0x08262eda, gWidth);  // Set ResX
        setVariable(0x08262ed2, gHeight); // Set ResY
        setVariable(0x08262f9a, gWidth);  // Set ResX
        setVariable(0x08262f92, gHeight); // Set ResY
        setVariable(0x0826305a, gWidth);  // Set ResX
        setVariable(0x08263052, gHeight); // Set ResY
        setVariable(0x082630b9, gWidth);  // Set ResX
        setVariable(0x082630b1, gHeight); // Set ResY
        setVariable(0x0826395f, gWidth);  // Set ResX
        setVariable(0x08263957, gHeight); // Set ResY
        setVariable(0x082639ea, gWidth);  // Set ResX
        setVariable(0x082639e2, gHeight); // Set ResY

        idDisplayTextureCAVEAddress = (void *)0x083502b0 + 5;
        detourFunction(0x083502b0, idDisplayTexture);
        // FSAA
        patchMemory(0x08578e92, "9090");
        patchMemory(0x08598719, "01"); // FSAA Enabled
        setVariable(0x08a34670, 1);    // FSAA Quality
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x081f2b7e + 6;
        id4DrawTextAddress = (void *)0x081f3ce4;
        detourFunction(0x081f2b7e, idDrawBallon);
        replaceCallAtAddress(0x081f40da, id4DrawText);
        // Scale Testmode text
        if (isTestMode() && gWidth >= 1920)
        {
            patchMemory(0x089e15d8, "02");
            idTextShift = (gWidth == 1920 ? 27 : ((1920 / gWidth) * 27) + 2);
            idTestTextAddress = (void *)0x085945c2;
            replaceCallAtAddress(0x083748ce, idTestText);
        }
    }
    break;
    case INITIALD_5_EXP:
    {
        bool origRes = (gWidth == 1360 && gHeight == 768);
        if (!getConfig()->boostRenderRes && origRes)
        {
            break;
        }
        if (gWidth >= 1360 && gHeight >= 768)
        {
            patchMemory(0x08356054, "bb01000000eb6b"); // Prevents renrer.ini from loading
            setVariable(0x08355f30, gWidth);           // Framebuffer Main Width
            setVariable(0x08355f37, gHeight);          // Framebuffer Main Height
            setVariable(0x08355fd1, 256);              // Framebuffer Road Specular width
            setVariable(0x08355fd8, 256);              // Framebuffer Road Specular height
            setVariable(0x08355ff7, gWidth);           // Framebuffer Glare Width
            setVariable(0x08355ffe, gHeight);          // Framebuffer Glare Height
            setVariable(0x08356024, gWidth >> 2);      // Framebuffer Reduced width
            patchMemory(0x08356049, "00000001");       // Enable Cube Secular
            if (origRes)
                break;
        }
        patchMemory(0x0853d8cd, "E9f000"); // Accept different Resolutions
        setVariable(0x0853d9c3, gWidth);   // Set ResX
        setVariable(0x0853d9c8, gHeight);  // Set ResY

        // Fix Press start and Insert coins text
        idDisplayTextureCAVEAddress = (void *)0x084e2470 + 5;
        detourFunction(0x084e2470, idDisplayTexture);
        // setViewport for track selection screen
        setVariable(0x08215cae, (int)(gHeight * (112.0 / 768)));
        setVariable(0x08215cb6, (int)(gWidth * (724.0 / 1315)));
        setVariable(0x08215cbe, (int)(gHeight * (592.0 / 768)));
        setVariable(0x08215cc6, (int)(gWidth * (962.0 / 1315)));
        setVariable(0x08215cce, gHeight);
        // FSAA
        patchMemory(0x0875aaa6, "9090");
        patchMemory(0x08788949, "01"); // FSAA Enabled
        setVariable(0x08cac148, 1);    // FSAA Quality
        // Balloon fix
        idDrawBallonCAVEAddress = (void *)0x082536c8 + 6;
        iddrawBallonPutAddress = (void *)0x086d257c;
        detourFunction(0x082536c8, idDrawBallon);
        replaceCallAtAddress(0x08254b68, idBalloonPut);
        replaceCallAtAddress(0x08254c29, idBalloonPut);
        // START and VIEW CHANGE Text fix
        float explanationScaleX = gWidth - (1360.0 - 815.0);
        float explanationScaleY = (gHeight / 768.0) * 680.0;
        setVariable(0x0826ac45, *(unsigned int *)&explanationScaleY);
        setVariable(0x0826acbd, *(unsigned int *)&explanationScaleX);
        // Scale Testmode text
        if (isTestMode() && gWidth >= 1920)
        {
            patchMemory(0x08c55ca0, "02");
            idTextShift = (gWidth == 1920 ? 27 : ((1920 / gWidth) * 27) + 2);
            idTestTextAddress = (void *)0x08775c1c;
            replaceCallAtAddress(0x0850853e, idTestText);
        }
    }
    break;
    case INITIALD_5_EXP_20:
    {
        bool origRes = (gWidth == 1360 && gHeight == 768);
        if (!getConfig()->boostRenderRes && origRes)
        {
            break;
        }
        if (gWidth >= 1360 && gHeight >= 768)
        {
            patchMemory(0x08363c24, "bb01000000eb6b"); // Prevents renrer.ini from loading
            setVariable(0x08363b00, gWidth);           // Framebuffer Main Width
            setVariable(0x08363b07, gHeight);          // Framebuffer Main Height
            setVariable(0x08363ba1, 256);              // Framebuffer Road Specular width
            setVariable(0x08363ba8, 256);              // Framebuffer Road Specular height
            setVariable(0x08363bc7, gWidth);           // Framebuffer Glare Width
            setVariable(0x08363bce, gHeight);          // Framebuffer Glare Height
            setVariable(0x08363bf4, gWidth >> 2);      // Framebuffer Reduced width
            patchMemory(0x08363c19, "00000001");       // Enable Cube Secular
            if (origRes)
                break;
        }
        patchMemory(0x0855a48d, "E9f000"); // Accept different Resolutions
        setVariable(0x0855a583, gWidth);   // Set ResX
        setVariable(0x0855a588, gHeight);  // Set ResY
        // Fix Press start and Insert coins text
        idDisplayTextureCAVEAddress = (void *)0x084fdfa0 + 5;
        detourFunction(0x084fdfa0, idDisplayTexture);
        // setViewport for track selection screen
        setVariable(0x0821e75e, (int)(gHeight * (112.0 / 768)));
        setVariable(0x0821e766, (int)(gWidth * (724.0 / 1315)));
        setVariable(0x0821e76e, (int)(gHeight * (592.0 / 768)));
        setVariable(0x0821e776, (int)(gWidth * (962.0 / 1315)));
        setVariable(0x0821e77e, gHeight);
        // FSAA
        patchMemory(0x087775f6, "9090");
        patchMemory(0x087a5499, "01"); // FSAA Enabled
        setVariable(0x08cf9ce8, 1);    // FSAA Quality
        // Balloon fix
        idDrawBallonCAVEAddress = (void *)0x0825c046 + 6;
        iddrawBallonPutAddress = (void *)0x086ef13c;
        detourFunction(0x0825c046, idDrawBallon);
        replaceCallAtAddress(0x0825d4e8, idBalloonPut);
        replaceCallAtAddress(0x0825d5a9, idBalloonPut);
        // START and VIEW CHANGE Text fix
        float explanationScaleX = gWidth - (1360.0 - 815.0);
        float explanationScaleY = (gHeight / 768.0) * 680.0;
        setVariable(0x082737b5, *(unsigned int *)&explanationScaleY);
        setVariable(0x0827382d, *(unsigned int *)&explanationScaleX);
        // Scale Testmode text
        if (isTestMode() && gWidth >= 1920)
        {
            patchMemory(0x08ca3d00, "02");
            idTextShift = (gWidth == 1920 ? 27 : ((1920 / gWidth) * 27) + 2);
            idTestTextAddress = (void *)0x0879276c;
            replaceCallAtAddress(0x0852528a, idTestText);
        }
    }
    break;
    case INITIALD_5_EXP_20A:
    {
        bool origRes = (gWidth == 1360 && gHeight == 768);
        if (!getConfig()->boostRenderRes && origRes)
        {
            break;
        }
        if (gWidth >= 1360 && gHeight >= 768)
        {
            patchMemory(0x08363d94, "bb01000000eb6b"); // Prevents renrer.ini from loading
            setVariable(0x08363c70, gWidth);           // Framebuffer Main Width
            setVariable(0x08363c77, gHeight);          // Framebuffer Main Height
            setVariable(0x08363d11, 256);              // Framebuffer Road Specular width
            setVariable(0x08363d18, 256);              // Framebuffer Road Specular height
            setVariable(0x08363d37, gWidth);           // Framebuffer Glare Width
            setVariable(0x08363d3e, gHeight);          // Framebuffer Glare Height
            setVariable(0x08363d64, gWidth >> 2);      // Framebuffer Reduced width
            patchMemory(0x08363d89, "00000001");       // Enable Cube Secular
            if (origRes)
                break;
        }
        patchMemory(0x0855a6dd, "E9f000"); // Accept different Resolutions
        setVariable(0x0855a7d3, gWidth);   // Set ResX
        setVariable(0x0855a7d8, gHeight);  // Set ResY

        // Fix Press start and Insert coins text
        idDisplayTextureCAVEAddress = (void *)0x084fe1d0 + 5;
        detourFunction(0x084fe1d0, idDisplayTexture);
        // setViewport for track selection screen
        setVariable(0x0821e8ee, (int)(gHeight * (112.0 / 768)));
        setVariable(0x0821e8f6, (int)(gWidth * (724.0 / 1315)));
        setVariable(0x0821e8fe, (int)(gHeight * (592.0 / 768)));
        setVariable(0x0821e906, (int)(gWidth * (962.0 / 1315)));
        setVariable(0x0821e90e, gHeight);
        // FSAA
        patchMemory(0x08777846, "9090");
        patchMemory(0x087a56e9, "01"); // FSAA Enabled
        setVariable(0x08cf9ce8, 1);    // FSAA Quality
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x0825c1b6 + 6;
        iddrawBallonPutAddress = (void *)0x086ef38c;
        detourFunction(0x0825c1b6, idDrawBallon);
        replaceCallAtAddress(0x0825d656, idBalloonPut);
        replaceCallAtAddress(0x0825d717, idBalloonPut);
        // START and VIEW CHANGE Text fix
        float explanationScaleX = gWidth - (1360.0 - 815.0);
        float explanationScaleY = (gHeight / 768.0) * 680.0;
        setVariable(0x08273915, *(unsigned int *)&explanationScaleY);
        setVariable(0x0827398d, *(unsigned int *)&explanationScaleX);
        // Scale Testmode text
        if (isTestMode() && gWidth >= 1920)
        {
            patchMemory(0x08ca3d00, "02");
            idTextShift = (gWidth == 1920 ? 27 : ((1920 / gWidth) * 27) + 2);
            idTestTextAddress = (void *)0x087929bc;
            replaceCallAtAddress(0x085254ea, idTestText);
        }
    }
    break;
    case INITIALD_5_JAP_REVA: // ID5 - DVP-0070A
    {
        bool origRes = (gWidth == 1360 && gHeight == 768);
        if (!getConfig()->boostRenderRes && origRes)
        {
            break;
        }
        if (gWidth >= 1360 && gHeight >= 768)
        {
            patchMemory(0x083557f4, "bb01000000eb6b"); // Prevents renrer.ini from loading
            setVariable(0x083556d0, gWidth);           // Framebuffer Main Width
            setVariable(0x083556d7, gHeight);          // Framebuffer Main Height
            setVariable(0x08355771, 256);              // Framebuffer Road Specular width
            setVariable(0x08355778, 256);              // Framebuffer Road Specular height
            setVariable(0x08355797, gWidth);           // Framebuffer Glare Width
            setVariable(0x0835579e, gHeight);          // Framebuffer Glare Height
            setVariable(0x083557c4, gWidth >> 2);      // Framebuffer Reduced width
            patchMemory(0x083557e9, "00000001");       // Enable Cube Secular
            if (origRes)
                break;
        }
        patchMemory(0x0853ccdd, "E9f000"); // Accept different Resolutions
        setVariable(0x0853cdd3, gWidth);   // Set ResX
        setVariable(0x0853cdd8, gHeight);  // Set ResY

        // Fix Press start and Insert coins text
        idDisplayTextureCAVEAddress = (void *)0x084e1770 + 5;
        detourFunction(0x084e1770, idDisplayTexture);
        // setViewport for track selection screen
        setVariable(0x08214b9e, (int)(gHeight * (112.0 / 768)));
        setVariable(0x08214ba6, (int)(gWidth * (724.0 / 1315)));
        setVariable(0x08214bae, (int)(gHeight * (592.0 / 768)));
        setVariable(0x08214bb6, (int)(gWidth * (962.0 / 1315)));
        setVariable(0x08214bbe, gHeight);
        // FSAA
        patchMemory(0x08759eb6, "9090");
        patchMemory(0x08787d59, "01"); // FSAA Enabled
        setVariable(0x08cac028, 1);    // FSAA Quality
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x082526a6 + 6;
        iddrawBallonPutAddress = (void *)0x086d198c;
        detourFunction(0x082526a6, idDrawBallon);
        replaceCallAtAddress(0x08253b48, idBalloonPut);
        replaceCallAtAddress(0x08253c09, idBalloonPut);
        // START and VIEW CHANGE Text fix
        float explanationScaleX = gWidth - (1360.0 - 815.0);
        float explanationScaleY = (gHeight / 768.0) * 680.0;
        setVariable(0x08269c25, *(unsigned int *)&explanationScaleY);
        setVariable(0x08269c9d, *(unsigned int *)&explanationScaleX);
        // Scale Testmode text
        if (isTestMode() && gWidth >= 1920)
        {
            patchMemory(0x08c55b80, "02");
            idTextShift = (gWidth == 1920 ? 27 : ((1920 / gWidth) * 27) + 2);
            idTestTextAddress = (void *)0x0877502c;
            replaceCallAtAddress(0x0850783e, idTestText);
        }
    }
    break;
    case INITIALD_5_JAP_REVF: // ID5 - DVP-0070F
    {
        bool origRes = (gWidth == 1360 && gHeight == 768);
        if (!getConfig()->boostRenderRes && origRes)
        {
            break;
        }
        if (gWidth >= 1360 && gHeight >= 768)
        {
            patchMemory(0x08364484, "bb01000000eb6b"); // Prevents renrer.ini from loading
            setVariable(0x08364360, gWidth);           // Framebuffer Main Width
            setVariable(0x08364367, gHeight);          // Framebuffer Main Height
            setVariable(0x08364401, 256);              // Framebuffer Road Specular width
            setVariable(0x08364408, 256);              // Framebuffer Road Specular height
            setVariable(0x08364427, gWidth);           // Framebuffer Glare Width
            setVariable(0x0836442e, gHeight);          // Framebuffer Glare Height
            setVariable(0x08364454, gWidth >> 2);      // Framebuffer Reduced width
            patchMemory(0x08364479, "00000001");       // Enable Cube Secular
            if (origRes)
                break;
        }
        patchMemory(0x08556bbd, "E9f000"); // Accept different Resolutions
        setVariable(0x08556cb3, gWidth);   // Set ResX
        setVariable(0x08556cb8, gHeight);  // Set ResY

        // Fix Press start and Insert coins text
        idDisplayTextureCAVEAddress = (void *)0x084fa3c0 + 5;
        detourFunction(0x084fa3c0, idDisplayTexture);
        // setViewport for track selection screen
        setVariable(0x0821ec6e, (int)(gHeight * (112.0 / 768)));
        setVariable(0x0821ec76, (int)(gWidth * (724.0 / 1315)));
        setVariable(0x0821ec7e, (int)(gHeight * (592.0 / 768)));
        setVariable(0x0821ec86, (int)(gWidth * (962.0 / 1315)));
        setVariable(0x0821ec8e, gHeight);
        // FSAA
        patchMemory(0x08773d26, "9090");
        patchMemory(0x087a1bc9, "01"); // FSAA Enabled
        setVariable(0x08ce8028, 1);    // FSAA Quality
        // Ballon fix
        idDrawBallonCAVEAddress = (void *)0x0825c556 + 6;
        iddrawBallonPutAddress = (void *)0x086eb86c;
        detourFunction(0x0825c556, idDrawBallon);
        replaceCallAtAddress(0x0825d9f8, idBalloonPut);
        replaceCallAtAddress(0x0825dab9, idBalloonPut);
        // START and VIEW CHANGE Text fix
        float explanationScaleX = gWidth - (1360.0 - 815.0);
        float explanationScaleY = (gHeight / 768.0) * 680.0;
        setVariable(0x08273ce5, *(unsigned int *)&explanationScaleY);
        setVariable(0x08273d5d, *(unsigned int *)&explanationScaleX);
        // Scale Testmode text
        if (isTestMode() && gWidth >= 1920)
        {
            patchMemory(0x08c92040, "02");
            idTextShift = (gWidth == 1920 ? 27 : ((1920 / gWidth) * 27) + 2);
            idTestTextAddress = (void *)0x0878ee9c;
            replaceCallAtAddress(0x08521aaa, idTestText);
        }
    }
    break;
    case LETS_GO_JUNGLE_REVA:
    {
        setVariable(0x082e1323, gWidth);  // Set ResX
        setVariable(0x082e1330, gHeight); // Set ResY
        // FSAA
        patchMemory(0x082e12ff, "01");

        // Render Res
        setVariable(0x080ce59a, gWidth);  // 800
        setVariable(0x080ce5a4, gHeight); // 600
    }
    break;
    case LETS_GO_JUNGLE:
    {
        setVariable(0x082E006b, gWidth);  // Set ResX
        setVariable(0x082E0078, gHeight); // Set ResY
        // FSAA
        patchMemory(0x082e0047, "01");

        // Render Res
        setVariable(0x080ce2f2, gWidth);  // 800
        setVariable(0x080ce2fc, gHeight); // 600
    }
    break;
    case LETS_GO_JUNGLE_SPECIAL:
    {
        if ((gWidth == 1024 && gHeight == 768) || (gWidth == 2048 && gHeight == 768))
            break;
        setVariable(0x08303C4B, gWidth * 2); // Set ResX  Width x 2
        setVariable(0x08303C58, gHeight);    // Set ResY

        setVariable(0x080d6e8c, gWidth);  // 1024
        setVariable(0x080d6e96, gHeight); // 768

        setVariable(0x080d6ea0, gWidth);  // 800
        setVariable(0x080d6eaa, gHeight); // 600
        patchMemory(0x08303c27, "01");      // FSAA

        newHeightLGJ = (float)gHeight;
        uint32_t *newHeightPtrLGJ = (uint32_t *)&newHeightLGJ;

        newWidth2LGJ = (float)gWidth * 2.0;
        uint32_t *newWidth2PtrLGJ = (uint32_t *)&newWidth2LGJ;
        unsigned int address = 0x080cb730;

        newWidthHLGJ = (float)gWidth / 2.0;
        uint32_t *newWidthHPtrLGJ = (uint32_t *)&newWidthHLGJ;
        newWidthSLGJ = 1360.0 / (float)gWidth;
        uint32_t *newWidthSPtrLGJ = (uint32_t *)&newWidthSLGJ;
        unsigned int addressC = 0x080cb3be;

        setVariable(0x082dcd10, (size_t)newHeightPtrLGJ); // LGJSceneManagerbegin

        setVariable(address, (size_t)newHeightPtrLGJ); // StageSelect
        setVariable(address + 0xF, (size_t)newWidth2PtrLGJ);

        setVariable(addressC, (size_t)newWidthHPtrLGJ); // StageSelectonCursor
        setVariable(addressC + 0x8, (size_t)newWidthSPtrLGJ);
    }
    break;
    case MJ4_REVG:
    {
        // Not Supported yet.
        //  patchMemory(0x080524a1, "01");         // Enable Anti Alias
        //  patchMemory(0x08053668, "b80a000000"); // Skips resolution set by the Dip Switches.
        //  vf5FSwidth = (gHeight * 4) / 3;
        //  setVariable(0x08901598, vf5FSwidth);
        //  setVariable(0x0890159c, gHeight);
        //  setVariable(0x089015a4, vf5FSwidth);

        // setVariable(0x089015a8, gHeight);

        // setVariable(0x08901544, vf5FSwidth);
        // setVariable(0x08901548, gHeight);

        // setVariable(0x0890158c, vf5FSwidth);
        // setVariable(0x08901590, gHeight);
    }
    break;
    case OUTRUN_2_SP_SDX:
    {
        if (gWidth <= 800 && gHeight <= 480)
            break;
        // If resolution is not the native of the game this patch kind of fix the Sun when the LensGlare effect is
        // shown.
        if ((gWidth > 800) && (gHeight > 480))
        {
            patchMemory(0x080e8e72, "9090909090"); // removes a call to a light function
            patchMemory(0x080e8e83, "9090909090"); // removes a call to a light function
            if (!getConfig()->outrunLensGlareEnabled)
            {
                detourFunction(0x080e8b34, stubReturn); // Completely disables the lens glare effect
            }
        }
        setVariable(0x080b913a, 0x44200000);
        setVariable(0x081dae28, 0x44200000);
        setVariable(0x081dae30, 0x44200000);
    }
    break;
    case OUTRUN_2_SP_SDX_TEST:
    {
        setVariable(0x0804a490, gWidth);
        setVariable(0x0804a4ad, gHeight);
    }
    break;
    case OUTRUN_2_SP_SDX_REVA:
    {
        if (gWidth <= 800 && gHeight <= 480)
            break;
        // If resolution is not the native of the game this patch kind of fix the Sun when the LensGlare effect is
        // shown.
        if ((gWidth > 800) && (gHeight > 480))
        {
            patchMemory(0x080e8e06, "9090909090"); // removes a call to a light function
            patchMemory(0x080e8e17, "9090909090"); // removes a call to a light function
            if (!getConfig()->outrunLensGlareEnabled)
            {
                detourFunction(0x080e8ac8, stubReturn); // Completely disables the lens glare effect
            }
        }
        setVariable(0x080b913a, 0x44200000);
        setVariable(0x081dada8, 0x44200000);
        setVariable(0x081dadb0, 0x44200000);

        // Cleans patched elf floating around
        patchMemory(0x080b912e, "f043");
        patchMemory(0x080f39de, "c0ef43");
        patchMemory(0x080f3aa6, "2048");
        patchMemory(0x080f3c94, "04b91c");
        patchMemory(0x080f3cba, "20561b");
        patchMemory(0x080f40cf, "c047");
        patchMemory(0x080f40f8, "80ef43");
        patchMemory(0x080f4d80, "04b91c");
        patchMemory(0x080f4e52, "20561b");
        patchMemory(0x080fed40, "80ef43");
        patchMemory(0x080fed52, "c047");
        setVariable(0x081dadb4, 0x43f00000);
        setVariable(0x081dadbc, 0x43f00000);
        patchMemory(0x081e7b3e, "7e");
        patchMemory(0x081e7b46, "89");
    }
    break;
    case OUTRUN_2_SP_SDX_REVA_TEST:
    case OUTRUN_2_SP_SDX_REVA_TEST2:
    {
        if (gWidth <= 800 && gHeight <= 480)
            break;
        setVariable(0x0804a490, gWidth);
        setVariable(0x0804a4ad, gHeight);
    }
    break;
    case PRIMEVAL_HUNT:
    {
        if (gWidth <= 1280 && gHeight <= 480)
            break;

        if (isTestMode())
        {
            EmulatorConfig *config = getConfig();
            config->phMode = 2;
        }
        phX = 0;
        phY = 0;
        phW = gWidth;
        phH = gHeight;
        switch (getConfig()->phMode)
        {
        case 0:
        {
            return 0;
        }
        case 1: // No touch screen
        {
            if (getConfig()->keepAspectRatio)
            {
                phX = (gWidth - (gHeight / 3) * 4) / 2;
                phW = (gHeight / 3) * 4;
            }
            phW2 = 0;
            phH2 = 0;
            phX2 = phW;
            phY2 = 0;
        }
        break;
        case 2: // Side by side
        {
            if (getConfig()->keepAspectRatio)
            {
                phH = ((gWidth / 2) / 4) * 3;
                phY = (gHeight - phH) / 2;
            }
            phX = 0;
            phW = phW / 2;
            phX2 = phW;
            phY2 = phY;
            phW2 = phW;
            phH2 = phH;
        }
        break;
        case 3: // 3ds Mode 1 (Small screen to the right)
        {
            // We always force 4:3 aspect ratio for this mode.
            phX = 0;
            phW = (gHeight / 3) * 4;
            phH2 = ((gWidth - phW) / 4) * 3;

            phX2 = phW;
            phY2 = (phH - phH2) / 2;
            phW2 = gWidth - phW;
        }
        break;
        case 4: // 3ds Mode 1 (Small screen to the bottom)
        {
            // We always force 4:3 aspect ratio for this mode.
            phW = (gHeight / 3) * 4;
            phX2 = (gWidth / 2) - ((gWidth - phW) / 2);
            phW2 = gWidth - phW;
            phY2 = 0; // phH - ((phW2 / 4) * 3);
            phH2 = (phW2 / 4) * 3;

            phW = phW - phW2;
            phH = phH - phH2;
            phX = (gWidth - (phH / 3) * 4) / 2;
            phY = phH2;
        }
        break;
        }
        setVariable(0x0805b104, phX); // X 1st screen
        setVariable(0x0805b0fd, phY); // Y 1st screen
        setVariable(0x0805b0f5, phW); // Width 1st screen
        setVariable(0x0805b0ed, phH); // Height 1st screen
        setVariable(0x0805afa2, phX2); // X 2nd screen
        setVariable(0x0805af9b, phY2); // Y 2st screen
        setVariable(0x0805af93, phW2); // Width 2nd screen
        setVariable(0x0805af8b, phH2); // Height 2nd screen
        // patchMemory(0x08052cc4, "0000"); // swap screens
        detourFunction(0x0804c628, glClearColorPH);
        if (getConfig()->phMode == 4) // In mode 4, the screens are inverted.
            phY = 0;

        // Test Menu
        if (isTestMode())
        {
            setVariable(0x0807f95d, gHeight); // test   HxW
            setVariable(0x0807f965, gWidth);

            setVariable(0x08055d3e, gHeight); // text right screen   HxW
            setVariable(0x08055d46, gWidth / 2);
        }
    }
    break;
    case RAMBO:
    {
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
                setVariable(0x08416df0, gWidth); // render res
            break;
        }
        else if (gWidth == 640 && gHeight == 480)
        {
            break;
        }
        patchMemory(0x080c70d0, "9090909090"); // setresolutiontype
        setVariable(0x08416df0, gWidth);       // render res
        setVariable(0x08416d60, gWidth);       // main res
        setVariable(0x08416d64, gHeight);
        patchMemory(0x08416d68, "5005"); // Allwinres
        patchMemory(0x08416d6c, "0003");

        patchMemory(0x080ef960, "66ff"); // 2d
        patchMemory(0x0806b510, "b6");   // proper marker fix
        //  patchMemory(0x0806b4f6, "c3");    //get rid of shot marker
    }
    break;
    case R_TUNED:
    {
        patchMemory(0x08051c2b, "01");         // Enable Anti Alias
        patchMemory(0x08052d58, "b80a000000"); // Skips resolution set by the Dip Switches.
        setVariable(0x083c7db8, gWidth);
        setVariable(0x083c7dbc, gHeight);
        patchMemory(0x083c7dc0, "00");
    }
    break;
    case SEGABOOT_2_4_SYM:
    {
        patchMemory(0x08050ad3, "9090");
        patchMemory(0x08050ac4, "07");
        patchMemory(0x08050c00, "07");
        patchMemory(0x08050c12, "07");
        patchMemory(0x08050c25, "07");
        patchMemory(0x08050c37, "07");
        setVariable(0x08064611, gWidth);
        setVariable(0x080645c6, gHeight);
    }
    break;
    case SEGABOOT_2_4:
    {
        patchMemory(0x08059799, "9090");
        patchMemory(0x080597b6, "07");
        patchMemory(0x0805978e, "07");
        patchMemory(0x080597a7, "07");
        patchMemory(0x080597c7, "07");
        patchMemory(0x08059780, "07");
        patchMemory(0x08059768, "07");
        setVariable(0x0809129d, gWidth);
        setVariable(0x08091252, gHeight);
    }
    break;
    case SEGA_RACE_TV:
    {
        if (gWidth == 640 && gHeight == 480)
            break;
        if (getConfig()->keepAspectRatio)
        {
            srtvX = (gWidth - (gHeight / 3) * 4) / 2;
            srtvW = (gHeight / 3) * 4;
        }
        else
        {
            srtvW = gWidth;
        }
        srtvH = gHeight;
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_REVA:
    {
        if (gWidth == 1280 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
                setVariable(0x084c9dbc, 1280);
            return 0;
        }
        else if (gWidth == 640 && gHeight == 480)
        {
            break;
        }
        patchMemory(0x0804d142, "9090909090"); // setresolutiontype
        // patchMemory(0x084c9dbc, "0005");
        // patchMemory(0x08448458, "0005");
        // patchMemory(0x0844845c, "0003");
        setVariable(0x084c9dbc, gWidth);
        setVariable(0x08448458, gWidth);
        setVariable(0x0844845c, gHeight);
        patchMemory(0x08448460, "0005");
        patchMemory(0x08448464, "0003");
        patchMemory(0x0817ff6d, "e80ed5ecff"); // call crtgetresolution
        patchMemory(0x08448468, "0005");       // set crtgetresolution to 640x480
        patchMemory(0x0844846c, "0003");

        setVariable(0x08448338, gWidth);
        setVariable(0x0844833c, gHeight);

        detourFunction(0x0804c024, glVertex3fHOD4);
        detourFunction(0x0804ca54, myGlBindTexture);
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_REVB:
    {
        if (gWidth == 1280 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
                setVariable(0x084c3a9c, 1280);
            return 0;
        }
        else if (gWidth == 640 && gHeight == 480)
        {
            break;
        }
        patchMemory(0x0804d174, "9090909090"); // setresolutiontype
        setVariable(0x084c3a9c, gWidth);
        setVariable(0x08443118, gWidth);
        setVariable(0x0844311c, gHeight);
        patchMemory(0x08443120, "0005");
        patchMemory(0x08443124, "0003");
        patchMemory(0x0818080d, "e89eccecff"); // call crtgetresolution
        patchMemory(0x08443128, "0005");       // set crtgetresolution to 640x480
        patchMemory(0x0844312c, "0003");

        setVariable(0x08442ff8, gWidth);
        setVariable(0x08442ffc, gHeight);

        detourFunction(0x0804c014, glVertex3fHOD4);
        detourFunction(0x0804ca84, myGlBindTexture);
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_REVC:
    {
        if (gWidth == 1280 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
                setVariable(0x084c3a9c, 1280);
            return 0;
        }
        else if (gWidth == 640 && gHeight == 480)
        {
            break;
        }
        patchMemory(0x0804d174, "9090909090"); // setresolutiontype
        setVariable(0x084c3a9c, gWidth);
        setVariable(0x08443118, gWidth);
        setVariable(0x0844311c, gHeight);
        patchMemory(0x08443120, "0005"); // gallwinres (2D)
        patchMemory(0x08443124, "0003");
        patchMemory(0x0818080d, "e89eccecff"); // sideselect fix
        patchMemory(0x08443128, "0005");
        patchMemory(0x0844312c, "0003");

        setVariable(0x08442ff8, gWidth);
        setVariable(0x08442ffc, gHeight);

        detourFunction(0x0804c014, glVertex3fHOD4);
        detourFunction(0x0804ca84, myGlBindTexture);
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL:
    {
        if (gWidth <= 1024 && gHeight <= 768)
            break;
        patchMemory(0x0804d2f4, "9090909090"); // setresolutiontype
        setVariable(0x084563c4, gWidth);
        setVariable(0x08424448, gWidth);
        setVariable(0x0842444c, gHeight);
        patchMemory(0x0815bfa9, "e8de7f0100"); // sideselect fix
        patchMemory(0x08424450, "0004");
        patchMemory(0x08424454, "0003");
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB:
    {
        if (gWidth <= 1024 && gHeight <= 768)
            break;
        patchMemory(0x0804d7e2, "9090909090"); // setresolutiontype
        setVariable(0x084c35c4, gWidth);
        setVariable(0x08491648, gWidth);
        setVariable(0x0849164c, gHeight);
        patchMemory(0x081b268f, "e8264d0200"); // sideselect fix
        patchMemory(0x08491650, "0004");
        patchMemory(0x08491654, "0003");
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_EX:
    {
        if (gWidth == 1360 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
                setVariable(0x087aa080, 1280); // render res
            break;
        }
        else if (gWidth == 640 && gHeight == 480)
        {
            break;
        }
        setVariable(0x08785b80, gWidth); // main
        setVariable(0x08785b84, gHeight);

        patchMemory(0x082a3222, "02");
        setVariable(0x08785ad8, 1360); // sprite res 1280x768
        setVariable(0x08785adc, 768);
        setVariable(0x087859b8, 1360); // sprite res 640x480
        setVariable(0x087859bc, 768);

        patchMemory(0x081775d2, "b801000000"); // target
        patchMemory(0x0804e5d2, "909090909090");

        setVariable(0x087aa080, gWidth); // render res
        patchMemory(0x0804e60c, "909090909090");
        patchMemory(0x0828cc2f, "909090909090");
    }
    break;
    case TOO_SPICY:
    {
        if (gWidth == 1280 && gHeight == 768)
        {
            if (getConfig()->boostRenderRes)
                setVariable(0x08202b22, 1280); // render res
            break;
        }
        else if (gWidth == 640 && gHeight == 480)
        {
            break;
        }
        setVariable(0x08202b22, gWidth); // render res
        patchMemory(0x08202b19, "74");
        setVariable(0x08664410, gWidth); // 1280x768
        setVariable(0x08664414, gHeight);
        detourFunction(0x0804d0d0, glVertex3f2SP);
        detourFunction(0x0804dda0, myGlBindTexture);
        replaceCallAtAddress(0x080d96ff, tooSpicySC);
        replaceCallAtAddress(0x080da026, tooSpicyPR);
        detourFunction(0x0804ce10, tooSpicyVsprintf);
        patchMemory(0x081f96b9, "02"); // 5
    }
    break;
    case TOO_SPICY_TEST:
    {
        if (gWidth <= 1280 && gHeight <= 768)
            break;
        setVariable(0x080565b3, gWidth);
        setVariable(0x080565bb, gHeight);
    }
    break;
    case VIRTUA_FIGHTER_5:
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            return 0;
        patchMemory(0x08054057, "b80a000000"); // Skips resolution set by the Dip Switches.
        setVariable(0x0847cf58, gWidth);
        setVariable(0x0847cf5c, gHeight);
        replaceCallAtAddress(0x080d40c4, vf5WidthFix);
    }
    break;
    case VIRTUA_FIGHTER_5_EXPORT:
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            break;
        patchMemory(0x08052b97, "01");         // Enable Anti Alias
        patchMemory(0x08053c67, "b80a000000"); // Skips resolution set by the Dip Switches.
        setVariable(0x085259f8, gWidth);
        setVariable(0x085259fc, gHeight);
    }
    break;
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA:
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            break;
        patchMemory(0x08054b5e, "01");         // Enable Anti Alias
        patchMemory(0x08055980, "b80a000000"); // Skips resolution set by the Dip Switches.
        vf5FSwidth = (gHeight * 5) / 3;
        setVariable(0x088b2bd8, vf5FSwidth);
        setVariable(0x088b2bdc, gHeight);
        setVariable(0x088b2bec, gWidth);
        setVariable(0x088b2bf0, gHeight);
    }
    break;
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB:
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            break;
        patchMemory(0x080548c4, "01");         // Enable Anti Alias
        patchMemory(0x080556e6, "b80a000000"); // Skips resolution set by the Dip Switches.
        vf5FSwidth = (gHeight * 5) / 3;
        setVariable(0x088d2b78, vf5FSwidth);
        setVariable(0x088d2b7c, gHeight);
        setVariable(0x088d2b8c, gWidth);
        setVariable(0x088d2b90, gHeight);
    }
    break;
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000:
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            break;
        patchMemory(0x08054bfe, "01");         // Enable Anti Alias
        patchMemory(0x08055a20, "b80a000000"); // Skips resolution set by the Dip Switches.
        vf5FSwidth = (gHeight * 5) / 3;
        setVariable(0x088d53d8, vf5FSwidth);
        setVariable(0x088d53dc, gHeight);
        setVariable(0x088d53ec, gWidth);
        setVariable(0x088d53f0, gHeight);
    }
    break;
    case VIRTUA_FIGHTER_5_R:
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            break;
        patchMemory(0x0805421a, "01");         // Enable Anti Alias
        patchMemory(0x080554b0, "b80a000000"); // Skips resolution set by the Dip Switches.
        vf5FSwidth = (gHeight * 5) / 3;
        setVariable(0x08767d58, vf5FSwidth);
        setVariable(0x08767d5c, gHeight);
        setVariable(0x08767d6c, gWidth);
        setVariable(0x08767d70, gHeight);
    }
    break;
    case VIRTUA_FIGHTER_5_R_REVD:
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            break;
        patchMemory(0x080543aa, "01");         // Enable Anti Alias
        patchMemory(0x080555f6, "b80a000000"); // Skips resolution set by the Dip Switches.
        vf5FSwidth = (gHeight * 5) / 3;
        setVariable(0x08822b18, vf5FSwidth);
        setVariable(0x08822b1c, gHeight);
        setVariable(0x08822b2c, gWidth);
        setVariable(0x08822b30, gHeight);
    }
    break;
    case VIRTUA_FIGHTER_5_R_REVG:
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            break;
        patchMemory(0x0805436a, "01");         // Enable Anti Alias
        patchMemory(0x0805577c, "b80a000000"); // Skips resolution set by the Dip Switches.
        vf5FSwidth = (gHeight * 5) / 3;
        setVariable(0x0887d4d8, vf5FSwidth);
        setVariable(0x0887d4dc, gHeight);
        setVariable(0x0887d4ec, gWidth);
        setVariable(0x0887d4f0, gHeight);
    }
    break;
    case VIRTUA_FIGHTER_5_REVA:
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            break;
        patchMemory(0x8053167, "01");          // Enable Anti Alias
        patchMemory(0x080541af, "b80a000000"); // Skips resolution set by the Dip Switches.
        setVariable(0x08487df8, gWidth);
        setVariable(0x08487dfc, gHeight);
    }
    break;
    case VIRTUA_FIGHTER_5_REVB:
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            break;
        patchMemory(0x08053673, "01");         // Enable Anti Alias
        patchMemory(0x080546cf, "b80a000000"); // Skips resolution set by the Dip Switches.
        setVariable(0x08536bb8, gWidth);
        setVariable(0x08536bbc, gHeight);
    }
    break;
    case VIRTUA_FIGHTER_5_REVE: // Also the public REV C version
    {
        if ((gWidth == 640 && gHeight == 480) || (gWidth == 1280 && gHeight == 768))
            break;
        patchMemory(0x080546c7, "01");         // Enable Anti Alias
        patchMemory(0x080557a3, "b80a000000"); // Skips resolution set by the Dip Switches.
        setVariable(0x085efb18, gWidth);
        setVariable(0x085efb1c, gHeight);
    }
    break;
    case VIRTUA_TENNIS_3:
    {
        if (gWidth <= 1360 && gHeight <= 768)
            break;
        patchMemory(0x081759b3, "9090");
        setVariable(0x081759bb, gWidth);
        setVariable(0x081759c5, gHeight);

        // setPerspective
        newVT3 = gWidth / 1360.0;
        intnewVT3PTr = (uint32_t *)&newVT3;

        setVariable(0x082682b6, (size_t)intnewVT3PTr);

        // vecToScr_check
        newVT3HW = (gWidth / 1360.0) * 320.0;
        newVT3HH = (gHeight / 768.0) * 240.0;
        intnewVT3HWPTr = (uint32_t *)&newVT3HW;
        intnewVT3HHPTr = (uint32_t *)&newVT3HH;
        uint32_t addressvt31 = 0x0817a55e;
        setVariable(addressvt31, (size_t)intnewVT3HWPTr);
        setVariable(addressvt31 + 0x18, (size_t)intnewVT3HHPTr);
    }
    break;
    case VIRTUA_TENNIS_3_REVA:
    {
        if (gWidth <= 1360 && gHeight <= 768)
            break;
        patchMemory(0x081759e3, "9090");
        setVariable(0x081759eb, gWidth);
        setVariable(0x081759f5, gHeight);

        // setPerspective
        newVT3 = gWidth / 1360.0;
        intnewVT3PTr = (uint32_t *)&newVT3;

        setVariable(0x082682e6, (size_t)intnewVT3PTr);

        // vecToScr_check
        newVT3HW = (gWidth / 1360.0) * 320.0;
        newVT3HH = (gHeight / 768.0) * 240.0;
        intnewVT3HWPTr = (uint32_t *)&newVT3HW;
        intnewVT3HHPTr = (uint32_t *)&newVT3HH;
        uint32_t addressvt31 = 0x0817a58e;
        setVariable(addressvt31, (size_t)intnewVT3HWPTr);
        setVariable(addressvt31 + 0x18, (size_t)intnewVT3HHPTr);
    }
    break;
    case VIRTUA_TENNIS_3_REVB:
    {
        if (gWidth <= 1360 && gHeight <= 768)
            break;
        patchMemory(0x08175ad7, "9090");
        setVariable(0x08175adf, gWidth);
        setVariable(0x08175ae9, gHeight);

        // setPerspective
        newVT3 = gWidth / 1360.0;
        intnewVT3PTr = (uint32_t *)&newVT3;

        setVariable(0x082684d2, (size_t)intnewVT3PTr);

        // vecToScr_check
        newVT3HW = (gWidth / 1360.0) * 320.0;
        newVT3HH = (gHeight / 768.0) * 240.0;
        intnewVT3HWPTr = (uint32_t *)&newVT3HW;
        intnewVT3HHPTr = (uint32_t *)&newVT3HH;
        uint32_t addressvt31 = 0x0817a77a;
        setVariable(addressvt31, (size_t)intnewVT3HWPTr);
        setVariable(addressvt31 + 0x18, (size_t)intnewVT3HHPTr);
    }
    break;
    case VIRTUA_TENNIS_3_REVC:
    {
        if (gWidth <= 1360 && gHeight <= 768)
            break;
        patchMemory(0x08175b03, "9090");
        setVariable(0x08175b0b, gWidth);
        setVariable(0x08175b15, gHeight);

        // setPerspective
        newVT3 = gWidth / 1360.0;
        intnewVT3PTr = (uint32_t *)&newVT3;

        setVariable(0x082684fe, (size_t)intnewVT3PTr);

        // vecToScr_check
        newVT3HW = (gWidth / 1360.0) * 320.0;
        newVT3HH = (gHeight / 768.0) * 240.0;
        intnewVT3HWPTr = (uint32_t *)&newVT3HW;
        intnewVT3HHPTr = (uint32_t *)&newVT3HH;
        uint32_t addressvt31 = 0x0817a7a6;
        setVariable(addressvt31, (size_t)intnewVT3HWPTr);
        setVariable(addressvt31 + 0x18, (size_t)intnewVT3HHPTr);
    }
    break;
    case VIRTUA_TENNIS_3_REVC_TEST:
    {
        setVariable(0x0806fb3b, gWidth);
        setVariable(0x0806fb04, gHeight);
    }
    break;
    default:
        break;
    }
    return 0;
}
