#include <SDL2/SDL_events.h>
#include <X11/X.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

char phSendBuf[10];
bool phBufferFilled = false;
int phCommand = 1;
int phCommandByte = 0;
uint16_t coordX;
uint16_t coordY;
#define TOUCH_START 0x81
#define TOUCH_TOUCHING 0x82
#define TOUCH_END 0x84

void phCoordinates(int x, int y, int w, int h, int button)
{
    coordX = (float)x / ((float)w * 0.0002441406);
    coordY = (float)y / ((float)h * 0.0002441406);
    memset(phSendBuf, 0, sizeof(phSendBuf));

    if (button == SDL_MOUSEBUTTONDOWN || button == ButtonPress)
    {
        phSendBuf[2] = TOUCH_START;
        phCommand = 1;
    }
    else if (button == SDL_MOUSEBUTTONUP || button == ButtonRelease)
    {
        phSendBuf[2] = TOUCH_END;
        phCommand = 3;
    }
    else if (button == SDL_MOUSEMOTION || button == MotionNotify) 
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
    for (int x = 0; x < 9;x++)
    {
        checksum += phSendBuf[x];
    }
    return (uint8_t)(checksum & 0xff);
}

ssize_t phRead(int fd, void *buf, size_t count)
{
    if(!phBufferFilled)
        return 0;


    if(phCommandByte == 9)
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