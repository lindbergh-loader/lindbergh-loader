#include <dlfcn.h>
#define GL_GLEXT_PROTOTYPES
#ifndef __i386__
#define __i386__
#endif
#undef __x86_64__
#include <GL/glx.h>
#include <SDL3/SDL.h>
#include <stdbool.h>

#include "blitStretching.h"
#include "config.h"
#include "log.h"
#include "resolution.h"

extern uint32_t gId;
extern int gGrp;
extern int gWidth;
extern int gHeight;
extern SDL_Window *sdlWindow;

int blitWidth = 0;
int blitHeight = 0;

int fboInitialized = false;
GLuint fboId = 0;
GLuint fboTextureId = 0;

int drawableW = 1;
int drawableH = 1;

Dest dest;

void initBlitting()
{
    blitSetWidthandHeightSize();
    blitInitializeFbo();
    dest.gameScale = 1.0f;
}

void blitSetWidthandHeightSize()
{
    if (gGrp == GROUP_ID4_EXP || gGrp == GROUP_ID4_JAP || gGrp == GROUP_ID5)
    {
        if (isTestMode())
        {
            blitWidth = 1360;
            blitHeight = 768;
        }
        else
        {
            blitWidth = gWidth;
            blitHeight = gHeight;
        }
    }
    else if (gId == GHOST_SQUAD_EVOLUTION || gGrp == GROUP_VT3_TEST)
    {
        blitWidth = 640;
        blitHeight = 480;
    }
    else if (gId == QUIZ_AXA || gId == QUIZ_AXA_LIVE || gId == MJ4_REVG || gId == MJ4_EVO)
    {
        blitWidth = 1024;
        blitHeight = 768;
    }
    else
    {
        blitWidth = gWidth;
        blitHeight = gHeight;
    }
}

int blitInitializeFbo()
{
    glGenFramebuffers(1, &fboId);
    if (fboId == 0)
    {
        log_error("Failed to generate FBO.\n");
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    glGenTextures(1, &fboTextureId);
    if (fboTextureId == 0)
    {
        log_error("Failed to generate FBO texture.\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fboId);
        fboId = 0;
        return 0;
    }
    glBindTexture(GL_TEXTURE_2D, fboTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, blitWidth, blitHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextureId, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        log_error("FBO is not complete! Status: 0x%x\n", status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteTextures(1, &fboTextureId);
        glDeleteFramebuffers(1, &fboId);
        fboTextureId = 0;
        fboId = 0;
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fboInitialized = true;
    return 1;
}

void blitStretch()
{
    if (!fboInitialized)
        return;

    if (fboInitialized && fboId > 0 && fboTextureId > 0 && sdlWindow)
    {
        SDL_GetWindowSizeInPixels(sdlWindow, &drawableW, &drawableH);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);
        glBlitFramebuffer(0, 0, blitWidth, blitHeight, 0, 0, blitWidth, blitHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        float gameAspect = (float)blitWidth / (float)blitHeight;
        float windowAspect = (float)drawableW / (float)drawableH;

        dest.W = drawableW;
        dest.H = drawableH;

        if (windowAspect > gameAspect)
        {
            dest.W = (GLsizei)(drawableH * gameAspect);
            dest.X = (drawableW - dest.W) / 2;
        }
        else if (windowAspect < gameAspect)
        {
            dest.H = (GLsizei)(drawableW / gameAspect);
            dest.Y = (drawableH - dest.H) / 2;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        glBlitFramebuffer(0, 0, blitWidth, blitHeight, dest.X, dest.Y, dest.X + dest.W, dest.Y + dest.H, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}