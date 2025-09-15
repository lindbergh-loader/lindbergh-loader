#include <SDL3/SDL_events.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "customCursor.h"
#include "config.h"
#include "touchScreen.h"

char phSendBuf[10];
bool phBufferFilled = false;
int phCommand = 1;
int phCommandByte = 0;
uint16_t coordX;
uint16_t coordY;
extern int phX, phY, phW, phH;
extern int phX2, phY2, phW2, phH2;
extern void *customCursor;
extern void *touchCursor;
extern bool phShowCursorInGame;

#define TOUCH_START 0x81
#define TOUCH_TOUCHING 0x82
#define TOUCH_END 0x84

void phCoordinates(int x, int y, int w, int h, int button)
{
    coordX = (float)x / ((float)w * 0.0002441406);
    coordY = (float)y / ((float)h * 0.0002441406);
    memset(phSendBuf, 0, sizeof(phSendBuf));

    if (button == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        phSendBuf[2] = TOUCH_START;
        phCommand = 1;
    }
    else if (button == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        phSendBuf[2] = TOUCH_END;
        phCommand = 3;
    }
    else if (button == SDL_EVENT_MOUSE_MOTION)
    {
        phSendBuf[2] = TOUCH_TOUCHING;
        phCommand = 2;
    }
    phSendBuf[0] = 0x55;
    phSendBuf[1] = 0x54;
    phSendBuf[3] = (char)(coordX & 0xFF);
    phSendBuf[4] = (char)((coordX >> 8) & 0xff);
    phSendBuf[5] = (char)(coordY & 0xFF);
    phSendBuf[6] = (char)((coordY >> 8) & 0xff);

    phBufferFilled = true;
}

uint8_t phCalcChecksum()
{
    int8_t checksum = -0x56;
    for (int x = 0; x < 9; x++)
    {
        checksum += phSendBuf[x];
    }
    return (uint8_t)(checksum & 0xff);
}

ssize_t phRead(int fd, void *buf, size_t count)
{
    if (!phBufferFilled)
        return 0;

    if (phCommandByte == 9)
    {
        switch (phCommand)
        {
            case 1:
                phCommand = 2;
                break;
            case 2:
                phSendBuf[2] = TOUCH_TOUCHING;
                break;
            case 3:
                phBufferFilled = false;
        }
        phSendBuf[9] = phCalcChecksum();
    }
    memcpy(buf, &phSendBuf[phCommandByte], sizeof(char));
    phCommandByte++;
    if (phCommandByte == 10)
        phCommandByte = 0;
    return 1;
}

void phTouchScreenCursor(int mX, int mY, int *motX, int *motY)
{
    if (getConfig()->emulateTouchscreen)
    {
        switch (getConfig()->phScreenMode)
        {
            case 0:
            case 2:
            case 3:
                if ((mX >= phX2 && mX <= (phX2 + phW2) && mY >= phY2 && mY <= (phY2 + phH2)) && getConfig()->hideCursor == 0)
                {
                    setCursor(touchCursor);
                    showPhCursor();
                }
                else if (getConfig()->hideCursor == 1 || phShowCursorInGame == false)
                {
                    hideCursor();
                }
                else if (phShowCursorInGame && getConfig()->hideCursor == 0)
                {
                    setCursor(customCursor);
                    showCursor();
                }
                break;
            case 4:
                if ((mX >= phX2 && mX <= (phX2 + phW2) && mY >= (phY2 + phH) && mY <= (phY2 + (phH2 + phH))) &&
                    getConfig()->hideCursor == 0)
                {
                    setCursor(touchCursor);
                    showPhCursor();
                }
                else if (getConfig()->hideCursor == 1 || phShowCursorInGame == false)
                {
                    hideCursor();
                }
                else if (phShowCursorInGame && getConfig()->hideCursor == 0)
                {
                    setCursor(customCursor);
                    showCursor();
                }
        }
    }
    if (mX < phX)
        *motX = phX;
    if (mX > (phW + phX))
        *motX = (phW + phX) - 1;
    if (mY > phH + phY)
        *motY = (phH + phY) - 1;
    if (mY < phY)
        *motY = phY;
}

bool phIsInsideTouchScreen(int mX, int mY, int *x, int *y)
{
    switch (getConfig()->phScreenMode)
    {
        case 0:
        case 2:
        case 3:
            if (mX >= phX2 && mX <= (phX2 + phW2) && mY >= phY2 && mY <= (phY2 + phH2))
            {
                *x = mX - phW;
                *y = mY - phY2;
                return true;
            }
            break;
        case 4:
            if (mX >= phX2 && mX <= (phX2 + phW2) && mY >= (phY2 + phH) && mY <= (phY2 + (phH2 + phH)))
            {
                *x = mX - phX2;
                *y = mY - phH;
                return true;
            }
    }
    return false;
}

void phTouchClick(int x, int y, int type)
{
    if (type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        phCoordinates(x, y, phW2, phH2, SDL_EVENT_MOUSE_BUTTON_DOWN);
    else if (type == SDL_EVENT_MOUSE_BUTTON_UP)
        phCoordinates(x, y, phW2, phH2, SDL_EVENT_MOUSE_BUTTON_UP);
    else if (type == SDL_EVENT_MOUSE_MOTION)
        phCoordinates(x, y, phW2, phH2, SDL_EVENT_MOUSE_MOTION);
}