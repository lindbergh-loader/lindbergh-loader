#ifndef __i386__
#define __i386__
#include "customcursor.h"
#endif
#undef __x86_64__
#include <SDL2/SDL.h>

#include "jvs.h"
#include "config.h"
#include "customcursor.h"
#include "patch.h"
#include "touchscreen.h"

#include <GL/freeglut.h>
#include <GL/glx.h>
#include <SDL2/SDL.h>
#include <X11/Xatom.h>
#include <X11/extensions/xf86vmode.h>
#include <dlfcn.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include "log.h"

GameType gameType = SHOOTING;
int jvsAnalogueInBits = 10;
extern int phX, phY, phW, phH;
extern int phX2, phY2, phW2, phH2;
int phIsDragging = 0;

extern void *customCursor;
extern void *phTouchCursor;
extern bool phShowCursorInGame;

// SDL2 controllers
extern SDL_GameController **controllers;
int numControllers;
const int16_t DEADZONE = 8000; // Standard SDL2 deadzone value
const float AXIS_MAX = 32767.0f; // Maximum value for SDL2 axis

int initInput()
{
    gameType = getConfig()->gameType;
    jvsAnalogueInBits = getJVSIO()->capabilities.analogueInBits;

    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        log_warn("SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    // Get the number of available controllers
    numControllers = SDL_NumJoysticks();

    // Allocate memory for the controllers array
    controllers = (SDL_GameController **)malloc(numControllers * sizeof(SDL_GameController *));

    // Open each controller
    for (int i = 0; i < numControllers; i++) {
        controllers[i] = SDL_GameControllerOpen(i);
        if (!controllers[i]) {
            log_warn("Failed to open controller %d: %s\n", i, SDL_GetError());
        }
    }
    /* Center Steering wheel in racing games (need to add a check for when using real analog devices)*/
    if (gameType == DRIVING)
    {
        setAnalogue(ANALOGUE_1, pow(2, jvsAnalogueInBits) * 0.5);
    }

    return 0;
}

void phTouchScreenCursor(int mX, int mY, int *motX, int *motY)
{
    if (getConfig()->emulateTouchscreen)
    {
        switch (getConfig()->phMode)
        {
        case 0:
        case 2:
        case 3:
            if ((mX >= phX2 && mX <= (phX2 + phW2) && mY >= phY2 && mY <= (phY2 + phH2)) && getConfig()->hideCursor == 0)
            {
                setCursor(phTouchCursor);
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
            if ((mX >= phX2 && mX <= (phX2 + phW2) && mY >= (phY2 + phH) && mY <= (phY2 + (phH2 + phH))) && getConfig()->hideCursor == 0)
            {
                setCursor(phTouchCursor);
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

void phTouchClick(int mX, int mY, int type)
{
    int x, y;
    bool insideTouchScreen = false;
    switch (getConfig()->phMode)
    {
    case 0:
    case 2:
    case 3:
        if (mX >= phX2 && mX <= (phX2 + phW2) && mY >= phY2 && mY <= (phY2 + phH2))
        {
            x = mX - phW;
            y = mY - phY2;
            insideTouchScreen = true;
        }
        break;
    case 4:
        if (mX >= phX2 && mX <= (phX2 + phW2) && mY >= (phY2 + phH) && mY <= (phY2 + (phH2 + phH)))
        {
            x = mX - phX2;
            y = mY - phH;
            insideTouchScreen = true;
        }
    }
    if (insideTouchScreen)
    {
        if (type == ButtonPress)
            phCoordinates(x, y, phW2, phH2, ButtonPress);
        else if (type == SDL_MOUSEBUTTONDOWN)
            phCoordinates(x, y, phW2, phH2, SDL_MOUSEBUTTONDOWN);
        else if (type == ButtonRelease)
            phCoordinates(x, y, phW2, phH2, ButtonRelease);
        else if (type == SDL_MOUSEBUTTONUP)
            phCoordinates(x, y, phW2, phH2, SDL_MOUSEBUTTONUP);
        else if (type == MotionNotify)
            phCoordinates(x, y, phW2, phH2, MotionNotify);
        else if (type == SDL_MOUSEMOTION)
            phCoordinates(x, y, phW2, phH2, SDL_MOUSEMOTION);
    }
    else
    {
        if (getConfig()->noSDL)
            setSwitch(PLAYER_1, BUTTON_1, type == ButtonPress);
        else
            setSwitch(PLAYER_1, BUTTON_1, type == SDL_MOUSEBUTTONDOWN);
    }
}

/**
 * Button mapping used for driving games
 */
int XNextEventDriving(Display *display, XEvent *event_return, int returnValue)
{
    KeyMapping keymap = getConfig()->keymap;
    switch (event_return->type)
    {
    case KeyPress:
    case KeyRelease:
    {
        if (event_return->xkey.keycode == keymap.test)
            setSwitch(SYSTEM, BUTTON_TEST, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.service)
            setSwitch(PLAYER_1, BUTTON_SERVICE, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.coin)
            incrementCoin(PLAYER_1, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.up)
            setAnalogue(ANALOGUE_2, event_return->type == KeyPress ? pow(2, jvsAnalogueInBits) - 1 : 0);

        else if (event_return->xkey.keycode == keymap.player1.down)
            setAnalogue(ANALOGUE_3, event_return->type == KeyPress ? pow(2, jvsAnalogueInBits) - 1 : 0);

        else if (event_return->xkey.keycode == keymap.player1.left)
            setAnalogue(ANALOGUE_1, event_return->type == KeyPress ? pow(2, jvsAnalogueInBits) * 0.2
                                                                   : pow(2, jvsAnalogueInBits) * 0.5);

        else if (event_return->xkey.keycode == keymap.player1.right)
            setAnalogue(ANALOGUE_1, event_return->type == KeyPress ? pow(2, jvsAnalogueInBits) * 0.8
                                                                   : pow(2, jvsAnalogueInBits) * 0.5);

        else if (event_return->xkey.keycode == keymap.player1.start)
            setSwitch(PLAYER_1, BUTTON_START, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button1)
            setSwitch(PLAYER_1, BUTTON_1, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button2)
            setSwitch(PLAYER_1, BUTTON_2, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button3)
            setSwitch(PLAYER_1, BUTTON_3, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button4)
            setSwitch(PLAYER_1, BUTTON_4, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button5)
            setSwitch(PLAYER_1, BUTTON_DOWN, event_return->type == KeyPress); // Hummer - SegaTV view change

        else if (event_return->xkey.keycode == keymap.player1.button6)
            setSwitch(PLAYER_1, BUTTON_UP, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button7)
            setSwitch(PLAYER_1, BUTTON_LEFT, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button8)
            setSwitch(PLAYER_1, BUTTON_RIGHT, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.button1)
            setSwitch(PLAYER_2, BUTTON_1, event_return->type == KeyPress); // SegaTV boost

        else if (event_return->xkey.keycode == keymap.player2.down)
            setSwitch(PLAYER_2, BUTTON_DOWN, event_return->type == KeyPress); // Hummer boost

        else if (event_return->xkey.keycode == keymap.player2.up)
            setSwitch(PLAYER_2, BUTTON_UP, event_return->type == KeyPress);
    }
    break;

    default:
        break;
    }

    return returnValue;
}

/**
 * Button mapping used for Harley
 */
int XNextEventHarley(Display *display, XEvent *event_return, int returnValue)
{
    KeyMapping keymap = getConfig()->keymap;
    switch (event_return->type)
    {
    case KeyPress:
    case KeyRelease:
    {
        if (event_return->xkey.keycode == keymap.test)
            setSwitch(SYSTEM, BUTTON_TEST, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.service)
            setSwitch(PLAYER_1, BUTTON_SERVICE, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.coin)
            incrementCoin(PLAYER_1, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.up)
            setAnalogue(ANALOGUE_1, event_return->type == KeyPress ? pow(2, jvsAnalogueInBits) - 1 : 0);

        else if (event_return->xkey.keycode == keymap.player1.down)
            setAnalogue(ANALOGUE_4, event_return->type == KeyPress ? pow(2, jvsAnalogueInBits) - 1 : 0);

        else if (event_return->xkey.keycode == keymap.player1.left)
            setAnalogue(ANALOGUE_2, event_return->type == KeyPress ? pow(2, jvsAnalogueInBits) * 0.2
                                                                   : pow(2, jvsAnalogueInBits) * 0.5);

        else if (event_return->xkey.keycode == keymap.player1.right)
            setAnalogue(ANALOGUE_2, event_return->type == KeyPress ? pow(2, jvsAnalogueInBits) * 0.8
                                                                   : pow(2, jvsAnalogueInBits) * 0.5);

        else if (event_return->xkey.keycode == keymap.player1.start)
            setSwitch(PLAYER_1, BUTTON_START, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button1)
            setSwitch(PLAYER_1, BUTTON_1, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button2)
            setSwitch(PLAYER_1, BUTTON_2, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button3)
            setSwitch(PLAYER_1, BUTTON_3, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button4)
            setSwitch(PLAYER_1, BUTTON_4, event_return->type == KeyPress);
    }
    break;

    default:
        break;
    }

    return returnValue;
}

/**
 * Button mapping used for shooting games
 */
int XNextEventShooting(Display *display, XEvent *event_return, int returnValue)
{
    KeyMapping keymap = getConfig()->keymap;
    switch (event_return->type)
    {
    case KeyPress:
    case KeyRelease:
    {
        if (event_return->xkey.keycode == keymap.test)
            setSwitch(SYSTEM, BUTTON_TEST, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.service)
            setSwitch(PLAYER_1, BUTTON_SERVICE, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.coin)
            incrementCoin(PLAYER_1, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.start)
            setSwitch(PLAYER_1, BUTTON_START, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button1)
            setSwitch(PLAYER_1, BUTTON_1, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button2)
            setSwitch(PLAYER_1, BUTTON_2, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button3)
            setSwitch(PLAYER_1, BUTTON_3, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.button4)
        {
            setSwitch(PLAYER_1, BUTTON_4, event_return->type == KeyPress);

            // For The House of the Dead 4's Accelerometer
            setAnalogue(ANALOGUE_5, 0);
            setAnalogue(ANALOGUE_6, 0);
        }

        else if (event_return->xkey.keycode == keymap.player1.button5)
            setSwitch(PLAYER_1, BUTTON_5, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.up)
            setSwitch(PLAYER_1, BUTTON_UP, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.down)
            setSwitch(PLAYER_1, BUTTON_DOWN, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.left)
            setSwitch(PLAYER_1, BUTTON_LEFT, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player1.right)
            setSwitch(PLAYER_1, BUTTON_RIGHT, event_return->type == KeyPress);

        // Player 2 Controls

        else if (event_return->xkey.keycode == keymap.player2.service)
            setSwitch(PLAYER_2, BUTTON_SERVICE, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.coin)
            incrementCoin(PLAYER_2, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.start)
            setSwitch(PLAYER_2, BUTTON_START, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.button1)
            setSwitch(PLAYER_2, BUTTON_1, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.button2)
            setSwitch(PLAYER_2, BUTTON_2, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.button3)
            setSwitch(PLAYER_2, BUTTON_3, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.button4)
            setSwitch(PLAYER_2, BUTTON_4, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.button5)
            setSwitch(PLAYER_2, BUTTON_5, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.up)
            setSwitch(PLAYER_2, BUTTON_UP, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.down)
            setSwitch(PLAYER_2, BUTTON_DOWN, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.left)
            setSwitch(PLAYER_2, BUTTON_LEFT, event_return->type == KeyPress);

        else if (event_return->xkey.keycode == keymap.player2.right)
            setSwitch(PLAYER_2, BUTTON_RIGHT, event_return->type == KeyPress);
    }
    break;

    case MotionNotify:
    {
        if (getConfig()->crc32 == LETS_GO_JUNGLE || getConfig()->crc32 == LETS_GO_JUNGLE_REVA)
        {
            setAnalogue(ANALOGUE_2, (1.0 - ((double)event_return->xmotion.x / (double)getConfig()->width)) *
                                        pow(2, jvsAnalogueInBits));
            setAnalogue(ANALOGUE_1, (1.0 - ((double)event_return->xmotion.y / (double)getConfig()->height)) *
                                        pow(2, jvsAnalogueInBits));
        }
        else if (getConfig()->crc32 == PRIMEVAL_HUNT)
        {
            int mX = event_return->xmotion.x;
            int mY = event_return->xmotion.y;

            if (phIsDragging)
                phTouchClick(mX, mY, event_return->type);

            int motX = mX, motY = mY;
            phTouchScreenCursor(mX, mY, &motX, &motY);

            setAnalogue(ANALOGUE_1, ((double)(motX - phX) / (double)phW) * pow(2, jvsAnalogueInBits));
            setAnalogue(ANALOGUE_2, ((double)(motY - phY) / (double)phH) * pow(2, jvsAnalogueInBits));
        }
        else
        {
            setAnalogue(ANALOGUE_1,
                        ((double)event_return->xmotion.x / (double)getConfig()->width) * pow(2, jvsAnalogueInBits));
            setAnalogue(ANALOGUE_2,
                        ((double)event_return->xmotion.y / (double)getConfig()->height) * pow(2, jvsAnalogueInBits));
        }
        // For The House of the Dead 4's Accelerometer
        static int lastMouseX = 0;
        static int lastMouseY = 0;
        static const int shakeThresholdPixels = 100; // Adjust as needed
        int currentMouseX = event_return->xmotion.x;
        int currentMouseY = event_return->xmotion.y;

        // Calculate movement distance
        int dx = currentMouseX - lastMouseX;
        int dy = currentMouseY - lastMouseY;

        // Update last position
        lastMouseX = currentMouseX;
        lastMouseY = currentMouseY;

        // Check if mouse movement exceeds shake threshold
        if (abs(dx) > shakeThresholdPixels || abs(dy) > shakeThresholdPixels)
        {

            setAnalogue(ANALOGUE_5, 0);
            setAnalogue(ANALOGUE_6, 0);
        }
        else
        {

            setAnalogue(ANALOGUE_5, pow(2, jvsAnalogueInBits) / 2);
            setAnalogue(ANALOGUE_6, pow(2, jvsAnalogueInBits) / 2);
        }
    }
    break;

    case ButtonPress:
    case ButtonRelease:
    {
        if (event_return->xbutton.button == 1) // Trigger
        {
            if (getConfig()->crc32 == PRIMEVAL_HUNT && getConfig()->emulateTouchscreen)
            {
                int mX = event_return->xmotion.x;
                int mY = event_return->xmotion.y;
                phTouchClick(mX, mY, event_return->type);
                phIsDragging = (event_return->type == ButtonPress ? 1 : 0);
            }
            else
            {
                setSwitch(PLAYER_1, BUTTON_1, event_return->type == ButtonPress);
            }
        }
        else if (event_return->xbutton.button == 3) // Reload
        {
            if (getConfig()->crc32 == RAMBO || getConfig()->crc32 == TOO_SPICY)
            {
                setAnalogue(ANALOGUE_1, -1);
                setAnalogue(ANALOGUE_2, -1);
            }
            setSwitch(PLAYER_1, BUTTON_2, event_return->type == ButtonPress);
        }
        else if (event_return->xbutton.button == 2) // Extra Gun Button
        {
            setSwitch(PLAYER_1, BUTTON_3, event_return->type == ButtonPress);
        }
    }
    break;

    default:
        break;
    }

    return returnValue;
}

int XNextEvent(Display *display, XEvent *event_return)
{
    int (*_XNextEvent)(Display *display, XEvent *event_return) = dlsym(RTLD_NEXT, "XNextEvent");
    int returnValue = _XNextEvent(display, event_return);

    // Return now if we're not emulating JVS or if EVDEV-only mode is enabled
    if (!getConfig()->emulateJVS || getConfig()->inputMode == 2)
    {
        return returnValue;
    }

    // Select the appropriate input mapping depending on the game
    switch (gameType)
    {
    case DRIVING:
        return XNextEventDriving(display, event_return, returnValue);
        break;

    case HARLEY:
        return XNextEventHarley(display, event_return, returnValue);
        break;

    case SHOOTING:
    case FIGHTING:
    default:
        return XNextEventShooting(display, event_return, returnValue);
        break;
    }

    return returnValue;
}

void sdlEventDriving(SDL_Event *event)
{
    switch (event->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        if (event->key.keysym.sym == SDLK_t)
            setSwitch(SYSTEM, BUTTON_TEST, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_s)
            setSwitch(PLAYER_1, BUTTON_SERVICE, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_5)
            incrementCoin(PLAYER_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_1)
            setSwitch(PLAYER_1, BUTTON_START, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_UP)
            setAnalogue(ANALOGUE_2, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) - 1 : 0);
        else if (event->key.keysym.sym == SDLK_DOWN)
            setAnalogue(ANALOGUE_3, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) - 1 : 0);
        else if (event->key.keysym.sym == SDLK_LEFT)
            setAnalogue(ANALOGUE_1,
                        event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.2 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_RIGHT)
            setAnalogue(ANALOGUE_1,
                        event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.8 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_q)
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_w)
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_e)
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_r)
            setSwitch(PLAYER_1, BUTTON_4, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_z)
            setSwitch(PLAYER_1, BUTTON_DOWN, event->type == SDL_KEYDOWN); // Hummer - SegaTV view change
        else if (event->key.keysym.sym == SDLK_x)
            setSwitch(PLAYER_1, BUTTON_UP, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_c)
            setSwitch(PLAYER_1, BUTTON_LEFT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_v)
            setSwitch(PLAYER_1, BUTTON_RIGHT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_a)
            setSwitch(PLAYER_2, BUTTON_1, event->type == SDL_KEYDOWN); // SegaTV boost
        else if (event->key.keysym.sym == SDLK_LCTRL)
            setSwitch(PLAYER_2, BUTTON_DOWN, event->type == SDL_KEYDOWN); // Hummer boost
        else if (event->key.keysym.sym == SDLK_SPACE)
            setSwitch(PLAYER_2, BUTTON_UP, event->type == SDL_KEYDOWN);
    }
    break;

    // SDL2 Controllers
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERAXISMOTION:
    {
        if (getConfig()->inputMode == 0) // Only handle controller events if inputMode is 0
        {
            bool isPress = (event->type == SDL_CONTROLLERBUTTONDOWN);

            switch (event->type)
            {
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                switch (event->cbutton.button)
                {
                case SDL_CONTROLLER_BUTTON_A:
                    if (getConfig()->crc32 == INITIALD_4_REVA ||
                        getConfig()->crc32 == INITIALD_4_REVB || 
                        getConfig()->crc32 == INITIALD_4_REVC || 
                        getConfig()->crc32 == INITIALD_4_REVD || 
                        getConfig()->crc32 == INITIALD_4_REVG || 
                        getConfig()->crc32 == INITIALD_4_EXP_REVB || 
                        getConfig()->crc32 == INITIALD_4_EXP_REVC || 
                        getConfig()->crc32 == INITIALD_4_EXP_REVD || 
                        getConfig()->crc32 == INITIALD_5_JAP_REVA || 
                        getConfig()->crc32 == INITIALD_5_JAP_REVF || 
                        getConfig()->crc32 == INITIALD_5_EXP    || 
                        getConfig()->crc32 == INITIALD_5_EXP_20 || 
                        getConfig()->crc32 == INITIALD_5_EXP_20A) {
                            setSwitch(PLAYER_1, BUTTON_1, isPress); // View Change Initial D
                    }
                    else
                    {
                        setSwitch(PLAYER_1, BUTTON_DOWN, isPress); // View Change others
                    }
                    break;
                case SDL_CONTROLLER_BUTTON_B:
                    if (getConfig()->crc32 == HUMMER || getConfig()->crc32 == HUMMER_SDLX || getConfig()->crc32 == HUMMER_EXTREME || getConfig()->crc32 == HUMMER_EXTREME_MDX)
                    {
                        setSwitch(PLAYER_2, BUTTON_DOWN, isPress); // Hummer Boost
                    }
                    else if (getConfig()->crc32 == SEGA_RACE_TV)
                    {
                        setSwitch(PLAYER_2, BUTTON_1, isPress); // SegaTV Boost
                    }
                    else if (getConfig()->crc32 == R_TUNED)
                    {
                        setSwitch(PLAYER_1, BUTTON_RIGHT, isPress); // R-Tuned Boost right
                    }
                    else
                    {
                        setSwitch(PLAYER_1, BUTTON_2, isPress);
                    }
                    break;
                case SDL_CONTROLLER_BUTTON_X:
                    if (getConfig()->crc32 == R_TUNED)
                    {
                        setSwitch(PLAYER_2, BUTTON_1, isPress); // R-Tuned Boost left
                    }
                    else
                    {
                        setSwitch(PLAYER_1, BUTTON_3, isPress);
                    }                
                    break;
                case SDL_CONTROLLER_BUTTON_Y:
                    setSwitch(PLAYER_1, BUTTON_4, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    setSwitch(PLAYER_1, BUTTON_UP, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    setSwitch(PLAYER_1, BUTTON_DOWN, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                    setSwitch(PLAYER_1, BUTTON_LEFT, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    setSwitch(PLAYER_1, BUTTON_RIGHT, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_START:
                    setSwitch(PLAYER_1, BUTTON_START, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_GUIDE:
                    setSwitch(PLAYER_1, BUTTON_SERVICE, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_BACK:
                    incrementCoin(PLAYER_1, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_MISC1:
                    setSwitch(SYSTEM, BUTTON_TEST, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                    setSwitch(PLAYER_2, BUTTON_DOWN, isPress);  // Gear Down
                    break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                    setSwitch(PLAYER_2, BUTTON_UP, isPress); // Gear Down Up
                    break;
                }
                break;

            case SDL_CONTROLLERAXISMOTION:
            {
                float normalizedValue;
                // Left stick X axis
                if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_1, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_1, pow(2, jvsAnalogueInBits) / 2); // Center when in deadzone
                    }
                }
                // Left trigger (we use for Brake)
                else if (event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_3, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_3, 0); // Release when in deadzone
                    }
                }
                // Right trigger (we use for Gas)
                else if (event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_2, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_2, 0); // Release when in deadzone
                    }
                }
            }
            break;
            }
        }
    }
    break;

    default:
        break;
    }
}

void sdlEventShooting(SDL_Event *event)
{
    KeyMapping keymap = getConfig()->keymap;

    switch (event->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        if (event->key.keysym.sym == SDLK_t)
            setSwitch(SYSTEM, BUTTON_TEST, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_s)
            setSwitch(PLAYER_1, BUTTON_SERVICE, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_5)
            incrementCoin(PLAYER_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_1)
            setSwitch(PLAYER_1, BUTTON_START, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_q)
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_w)
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_e)
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_r)
        {
            setSwitch(PLAYER_1, BUTTON_4, event->type == SDL_KEYDOWN);

            // For The House of the Dead 4's Accelerometer
            setAnalogue(ANALOGUE_5, 0);
            setAnalogue(ANALOGUE_6, 0);
        }
        else if (event->key.keysym.sym == SDLK_UP)
            setSwitch(PLAYER_1, BUTTON_UP, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_DOWN)
            setSwitch(PLAYER_1, BUTTON_DOWN, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_LEFT)
            setSwitch(PLAYER_1, BUTTON_LEFT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_RIGHT)
            setSwitch(PLAYER_1, BUTTON_RIGHT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_2)
            setSwitch(PLAYER_2, BUTTON_START, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_z)
            setSwitch(PLAYER_2, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_x)
            setSwitch(PLAYER_2, BUTTON_2, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == SDLK_c)
            setSwitch(PLAYER_2, BUTTON_3, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == SDLK_v)
            setSwitch(PLAYER_2, BUTTON_4, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == SDLK_u)
            setSwitch(PLAYER_2, BUTTON_UP, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_i)
            setSwitch(PLAYER_2, BUTTON_DOWN, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_o)
            setSwitch(PLAYER_2, BUTTON_LEFT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_p)
            setSwitch(PLAYER_2, BUTTON_RIGHT, event->type == SDL_KEYDOWN);
    }
    break;

    case SDL_MOUSEMOTION:
    {
        if (getConfig()->crc32 == LETS_GO_JUNGLE || getConfig()->crc32 == LETS_GO_JUNGLE_REVA)
        {
            setAnalogue(ANALOGUE_2,
                        (1.0 - ((double)event->motion.x / (double)getConfig()->width)) * pow(2, jvsAnalogueInBits));
            setAnalogue(ANALOGUE_1,
                        (1.0 - ((double)event->motion.y / (double)getConfig()->height)) * pow(2, jvsAnalogueInBits));
        }
        else if (getConfig()->crc32 == PRIMEVAL_HUNT)
        {
            int mX = event->motion.x;
            int mY = event->motion.y;

            if (phIsDragging)
                phTouchClick(mX, mY, event->type);

            int motX = mX, motY = mY;
            phTouchScreenCursor(mX, mY, &motX, &motY);

            setAnalogue(ANALOGUE_1, ((double)(motX - phX) / (double)phW) * pow(2, jvsAnalogueInBits));
            setAnalogue(ANALOGUE_2, ((double)(motY - phY) / (double)phH) * pow(2, jvsAnalogueInBits));
        }
        else
        {
            setAnalogue(ANALOGUE_1, ((double)event->motion.x / (double)getConfig()->width) * pow(2, jvsAnalogueInBits));
            setAnalogue(ANALOGUE_2,
                        ((double)event->motion.y / (double)getConfig()->height) * pow(2, jvsAnalogueInBits));
        }
        // For The House of the Dead 4's Accelerometer
        static int lastMouseX = 0;
        static int lastMouseY = 0;
        static const int shakeThresholdPixels = 100; // Adjust as needed
        int currentMouseX = event->motion.x;
        int currentMouseY = event->motion.y;

        // Calculate movement distance
        int dx = currentMouseX - lastMouseX;
        int dy = currentMouseY - lastMouseY;

        // Update last position
        lastMouseX = currentMouseX;
        lastMouseY = currentMouseY;

        // Check if mouse movement exceeds shake threshold
        if (abs(dx) > shakeThresholdPixels || abs(dy) > shakeThresholdPixels)
        {
            setAnalogue(ANALOGUE_5, 0);
            setAnalogue(ANALOGUE_6, 0);
        }
        else
        {
            setAnalogue(ANALOGUE_5, pow(2, jvsAnalogueInBits) / 2);
            setAnalogue(ANALOGUE_6, pow(2, jvsAnalogueInBits) / 2);
        }
    }
    break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    {
        if (event->button.button == SDL_BUTTON_LEFT) // Trigger
        {
            if (getConfig()->crc32 == PRIMEVAL_HUNT && getConfig()->emulateTouchscreen)
            {
                int mX = event->motion.x;
                int mY = event->motion.y;
                phTouchClick(mX, mY, event->type);
                phIsDragging = (event->type == SDL_MOUSEBUTTONDOWN ? 1 : 0);
            }
            else
            {
                setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_MOUSEBUTTONDOWN);
            }
        }
        else if (event->button.button == SDL_BUTTON_RIGHT) // Reload
        {
            if (getConfig()->crc32 == RAMBO || getConfig()->crc32 == TOO_SPICY)
            {
                setAnalogue(ANALOGUE_1, -1);
                setAnalogue(ANALOGUE_2, -1);
            }
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_MOUSEBUTTONDOWN);
        }
        else if (event->button.button == SDL_BUTTON_MIDDLE) // Extra Gun Button
        {
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_MOUSEBUTTONDOWN);
        }
    }
    break;

    // SDL2 Controllers
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERAXISMOTION:
    {
        if (getConfig()->inputMode == 0) // Only handle controller events if inputMode is 0
        {
            bool isPress = (event->type == SDL_CONTROLLERBUTTONDOWN);

            switch (event->type)
            {
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                switch (event->cbutton.button)
                {
                case SDL_CONTROLLER_BUTTON_A:
                    setSwitch(PLAYER_1, BUTTON_1, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_B:
                    setSwitch(PLAYER_1, BUTTON_2, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_X:
                    setSwitch(PLAYER_1, BUTTON_3, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_Y:
                    setSwitch(PLAYER_1, BUTTON_4, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    setSwitch(PLAYER_1, BUTTON_UP, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    setSwitch(PLAYER_1, BUTTON_DOWN, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                    setSwitch(PLAYER_1, BUTTON_LEFT, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    setSwitch(PLAYER_1, BUTTON_RIGHT, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_START:
                    setSwitch(PLAYER_1, BUTTON_START, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_GUIDE:
                    setSwitch(PLAYER_1, BUTTON_SERVICE, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_BACK:
                    incrementCoin(PLAYER_1, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_MISC1:
                    setSwitch(SYSTEM, BUTTON_TEST, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                    setSwitch(PLAYER_1, BUTTON_5, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                    setSwitch(PLAYER_1, BUTTON_6, isPress);
                    break;
                }
                break;

            case SDL_CONTROLLERAXISMOTION:
            {
                float normalizedValue;
                // Left stick X axis
                if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_1, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_1, pow(2, jvsAnalogueInBits) / 2); // Center when in deadzone
                    }
                }
                // Left stick Y axis
                if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_2, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_2, pow(2, jvsAnalogueInBits) / 2); // Center when in deadzone
                    }
                }
            }
            break;
            }
        }
    }
    break;

    default:
        break;
    }
}

void sdlEventABC(SDL_Event *event)
{
    switch (event->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        if (event->key.keysym.sym == SDLK_t)
            setSwitch(SYSTEM, BUTTON_TEST, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_s)
            setSwitch(PLAYER_1, BUTTON_SERVICE, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_5)
            incrementCoin(PLAYER_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_1)
            setSwitch(PLAYER_1, BUTTON_START, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_UP)
            setAnalogue(ANALOGUE_2,
                        event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.2 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_DOWN)
            setAnalogue(ANALOGUE_2,
                        event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.8 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_LEFT)
            setAnalogue(ANALOGUE_1,
                        event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.2 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_RIGHT)
            setAnalogue(ANALOGUE_1,
                        event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.8 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_q)
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_w)
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_e)
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_f)
            setAnalogue(ANALOGUE_3, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.2 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_r)
            setAnalogue(ANALOGUE_3, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.8 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_z)
            setSwitch(PLAYER_1, BUTTON_DOWN, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_x)
            setSwitch(PLAYER_1, BUTTON_UP, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_c)
            setSwitch(PLAYER_1, BUTTON_LEFT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_v)
            setSwitch(PLAYER_1, BUTTON_RIGHT, event->type == SDL_KEYDOWN);
    }
    break;

    // SDL2 Controllers
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERAXISMOTION:
    {
        if (getConfig()->inputMode == 0) // Only handle controller events if inputMode is 0
        {
            bool isPress = (event->type == SDL_CONTROLLERBUTTONDOWN);

            switch (event->type)
            {
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                switch (event->cbutton.button)
                {
                case SDL_CONTROLLER_BUTTON_A:
                    setSwitch(PLAYER_1, BUTTON_1, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_B:
                    setSwitch(PLAYER_1, BUTTON_2, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_X:
                    setSwitch(PLAYER_1, BUTTON_3, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_Y:
                    setSwitch(PLAYER_1, BUTTON_4, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    setSwitch(PLAYER_1, BUTTON_UP, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    setSwitch(PLAYER_1, BUTTON_DOWN, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                    setSwitch(PLAYER_1, BUTTON_LEFT, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    setSwitch(PLAYER_1, BUTTON_RIGHT, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_START:
                    setSwitch(PLAYER_1, BUTTON_START, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_GUIDE:
                    setSwitch(PLAYER_1, BUTTON_SERVICE, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_BACK:
                    incrementCoin(PLAYER_1, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_MISC1:
                    setSwitch(SYSTEM, BUTTON_TEST, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                    setSwitch(PLAYER_1, BUTTON_5, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                    setSwitch(PLAYER_1, BUTTON_6, isPress);
                    break;
                }
                break;

            case SDL_CONTROLLERAXISMOTION:
            {
                float normalizedValue;
                // Left stick X axis
                if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_1, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_1, pow(2, jvsAnalogueInBits) / 2); // Center when in deadzone
                    }
                }
                // Left stick Y axis
                if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_2, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_2, pow(2, jvsAnalogueInBits) / 2); // Center when in deadzone
                    }
                }
                // Increase Speed
                if (event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_3, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_3, pow(2, jvsAnalogueInBits) * 0.5); // Neutral when released
                    }
                }
                // Decrease Speed
                if (event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_3, (1.0f - normalizedValue) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    }
                }
            }
            break;
            }
        }
    }
    break;

    default:
        break;
    }
}

void sdlEventHarley(SDL_Event *event)
{
    switch (event->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        if (event->key.keysym.sym == SDLK_t)
            setSwitch(SYSTEM, BUTTON_TEST, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_s)
            setSwitch(PLAYER_1, BUTTON_SERVICE, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_5)
            incrementCoin(PLAYER_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_1)
            setSwitch(PLAYER_1, BUTTON_START, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_UP)
            setAnalogue(ANALOGUE_2, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) - 1 : 0);
        else if (event->key.keysym.sym == SDLK_DOWN)
            setAnalogue(ANALOGUE_3, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) - 1 : 0);
        else if (event->key.keysym.sym == SDLK_LEFT)
            setAnalogue(ANALOGUE_1,
                        event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.2 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_RIGHT)
            setAnalogue(ANALOGUE_1,
                        event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.8 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_q)
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_w)
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_e)
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_r)
            setSwitch(PLAYER_1, BUTTON_4, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_z)
            setSwitch(PLAYER_1, BUTTON_DOWN, event->type == SDL_KEYDOWN); // Hummer - SegaTV view change
        else if (event->key.keysym.sym == SDLK_x)
            setSwitch(PLAYER_1, BUTTON_UP, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_c)
            setSwitch(PLAYER_1, BUTTON_LEFT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_v)
            setSwitch(PLAYER_1, BUTTON_RIGHT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_a)
            setSwitch(PLAYER_2, BUTTON_1, event->type == SDL_KEYDOWN); // SegaTV boost
        else if (event->key.keysym.sym == SDLK_LCTRL)
            setSwitch(PLAYER_2, BUTTON_DOWN, event->type == SDL_KEYDOWN); // Hummer boost
        else if (event->key.keysym.sym == SDLK_SPACE)
            setSwitch(PLAYER_2, BUTTON_UP, event->type == SDL_KEYDOWN);
    }
    break;

    // SDL2 Controllers
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERAXISMOTION:
    {
        if (getConfig()->inputMode == 0) // Only handle controller events if inputMode is 0
        {
            bool isPress = (event->type == SDL_CONTROLLERBUTTONDOWN);

            switch (event->type)
            {
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                switch (event->cbutton.button)
                {
                case SDL_CONTROLLER_BUTTON_A:
                    setSwitch(PLAYER_1, BUTTON_2, isPress); // View
                    break;
                case SDL_CONTROLLER_BUTTON_B:
                    setSwitch(PLAYER_1, BUTTON_1, isPress); // Change Music
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    setSwitch(PLAYER_1, BUTTON_UP, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    setSwitch(PLAYER_1, BUTTON_DOWN, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                    setSwitch(PLAYER_1, BUTTON_LEFT, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    setSwitch(PLAYER_1, BUTTON_RIGHT, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_START:
                    setSwitch(PLAYER_1, BUTTON_START, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_GUIDE:
                    setSwitch(PLAYER_1, BUTTON_SERVICE, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_BACK:
                    incrementCoin(PLAYER_1, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_MISC1:
                    setSwitch(SYSTEM, BUTTON_TEST, isPress);
                    break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                    setSwitch(PLAYER_1, BUTTON_4, isPress);  // Gear Down
                    break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                    setSwitch(PLAYER_1, BUTTON_3, isPress); // Gear Down Up
                    break;
                }
                break;

            case SDL_CONTROLLERAXISMOTION:
            {
                float normalizedValue;
                // Left stick X axis (Handle)
                if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_2, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_2, pow(2, jvsAnalogueInBits) / 2); // Center when in deadzone
                    }
                }
                // Left trigger (we use for Brake)
                else if (event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_4, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_4, 0); // Release when in deadzone
                    }
                }
                // Right trigger (we use for Gas)
                else if (event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                    if (abs(event->caxis.value) > DEADZONE) {
                        normalizedValue = event->caxis.value / AXIS_MAX;
                        setAnalogue(ANALOGUE_1, (normalizedValue + 1.0f) * (pow(2, jvsAnalogueInBits) - 1) / 2);
                    } else {
                        setAnalogue(ANALOGUE_1, 0); // Release when in deadzone
                    }
                }
            }
            break;
            }
        }
    }
    break;

    default:
        break;
    }
}

void sdlEventMahjong(SDL_Event *event)
{
    switch (event->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        if (event->key.keysym.sym == SDLK_t)
            setSwitch(SYSTEM, BUTTON_TEST, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_s)
            setSwitch(PLAYER_1, BUTTON_SERVICE, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_5)
            incrementCoin(PLAYER_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_1)
            setSwitch(PLAYER_1, BUTTON_START, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_a)
            setSwitch(PLAYER_1, BUTTON_RIGHT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_b)
            setSwitch(PLAYER_1, BUTTON_LEFT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_c)
            setSwitch(PLAYER_1, BUTTON_UP, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_d)
            setSwitch(PLAYER_1, BUTTON_DOWN, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_e)
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_f)
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_g)
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_h)
            setSwitch(PLAYER_1, BUTTON_4, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_i)
            setSwitch(PLAYER_1, BUTTON_5, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_j)
            setSwitch(PLAYER_1, BUTTON_6, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_k)
            setSwitch(PLAYER_1, BUTTON_7, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_l)
            setSwitch(PLAYER_2, BUTTON_RIGHT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_m)
            setSwitch(PLAYER_2, BUTTON_LEFT, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_n)
            setSwitch(PLAYER_2, BUTTON_UP, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_o)
            setSwitch(PLAYER_2, BUTTON_DOWN, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_p)
            setSwitch(PLAYER_2, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_q)
            setSwitch(PLAYER_2, BUTTON_2, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_r)
            setSwitch(PLAYER_2, BUTTON_3, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_u)
            setSwitch(PLAYER_2, BUTTON_4, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_v)
            setSwitch(PLAYER_2, BUTTON_5, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_w)
            setSwitch(PLAYER_2, BUTTON_6, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_x)
            setSwitch(PLAYER_2, BUTTON_7, event->type == SDL_KEYDOWN);
    }
    break;

    // SDL2 Controllers
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    {
        if (getConfig()->inputMode == 0) // Only handle controller events if inputMode is 0
        {
            bool isPress = (event->type == SDL_CONTROLLERBUTTONDOWN);

            switch (event->cbutton.button)
            {
            case SDL_CONTROLLER_BUTTON_A:
                setSwitch(PLAYER_1, BUTTON_1, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_B:
                setSwitch(PLAYER_1, BUTTON_2, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_X:
                setSwitch(PLAYER_1, BUTTON_3, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_Y:
                setSwitch(PLAYER_1, BUTTON_4, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                setSwitch(PLAYER_1, BUTTON_UP, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                setSwitch(PLAYER_1, BUTTON_DOWN, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                setSwitch(PLAYER_1, BUTTON_LEFT, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                setSwitch(PLAYER_1, BUTTON_RIGHT, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_START:
                setSwitch(PLAYER_1, BUTTON_START, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_GUIDE:
                setSwitch(PLAYER_1, BUTTON_SERVICE, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_BACK:
                incrementCoin(PLAYER_1, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_MISC1:
                setSwitch(SYSTEM, BUTTON_TEST, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                setSwitch(PLAYER_1, BUTTON_5, isPress);
                break;
            case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                setSwitch(PLAYER_1, BUTTON_6, isPress);
                break;
            }
        }
    }
    break;

    default:
        break;
    }
}

void handleSdlEvents(SDL_Event *event)
{
    // Return now if we're not emulating JVS or if EVDEV-only mode is enabled
    if (!getConfig()->emulateJVS || getConfig()->inputMode == 2)
    {
        return;
    }

    // Select the appropriate input mapping depending on the game
    switch (gameType)
    {
    case DRIVING:
        sdlEventDriving(event);
        break;
    case SHOOTING:
    case FIGHTING:
        sdlEventShooting(event);
        break;
    case ABC:
        sdlEventABC(event);
        break;
    case HARLEY:
        sdlEventHarley(event);
        break;
    case MAHJONG:
        sdlEventMahjong(event);
        break;
    default:
        // return XNextEventShooting(display, event_return, returnValue);
        break;
    }
}
