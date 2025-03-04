#ifndef __i386__
#define __i386__
#include <SDL2/SDL_keycode.h>
#include <X11/X.h>
#endif
#undef __x86_64__
#include <SDL2/SDL.h>

#include "jvs.h"
#include "config.h"

#include <GL/freeglut.h>
#include <GL/glx.h>
#include <SDL2/SDL.h>
#include <X11/Xatom.h>
#include <X11/extensions/xf86vmode.h>
#include <dlfcn.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>

GameType gameType = SHOOTING;
int jvsAnalogueInBits = 10;
extern int phX, phY, phW, phH;

int initInput()
{
    gameType = getConfig()->gameType;
    jvsAnalogueInBits = getJVSIO()->capabilities.analogueInBits;

    /* Center Steering wheel in racing games (need to add a check for when using real analog devices)*/
    if (gameType == DRIVING)
    {
        setAnalogue(ANALOGUE_1, pow(2, jvsAnalogueInBits) * 0.5);
    }

    return 0;
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
            if (mX < phX)
                mX = phX;
            if (mX > (phW + phX))
                mX = (phW + phX) - 1;
            if (mY > phH + phY)
                mY = (phH + phY) - 1;
            if (mY < phY)
                mY = phY;
            setAnalogue(ANALOGUE_1, ((double)(mX - phX) / (double)phW) * pow(2, jvsAnalogueInBits));
            setAnalogue(ANALOGUE_2, ((double)(mY - phY) / (double)phH) * pow(2, jvsAnalogueInBits));
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
            setSwitch(PLAYER_1, BUTTON_1, event_return->type == ButtonPress);
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


SDL_KeyCode convertKeycode(unsigned int xkeycode);

void sdlEventDriving(SDL_Event *event)
{
    KeyMapping keymap = getConfig()->keymap;
    switch (event->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        if (event->key.keysym.sym == convertKeycode(keymap.test))
            setSwitch(SYSTEM, BUTTON_TEST, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.service))
            setSwitch(PLAYER_1, BUTTON_SERVICE, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.coin))
            incrementCoin(PLAYER_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.start))
            setSwitch(PLAYER_1, BUTTON_START, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.up))
            setAnalogue(ANALOGUE_2, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) - 1 : 0);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.down))
            setAnalogue(ANALOGUE_3, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) - 1 : 0);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.left))
            setAnalogue(ANALOGUE_1,
                        event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.2 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.right))
            setAnalogue(ANALOGUE_1,
                        event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.8 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button1))
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button2))
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button3))
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button4))
            setSwitch(PLAYER_1, BUTTON_4, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.down))
            setSwitch(PLAYER_1, BUTTON_DOWN, event->type == SDL_KEYDOWN); // Hummer - SegaTV view change

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.up))
            setSwitch(PLAYER_1, BUTTON_UP, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.left))
            setSwitch(PLAYER_1, BUTTON_LEFT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.right))
            setSwitch(PLAYER_1, BUTTON_RIGHT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button1))
            setSwitch(PLAYER_2, BUTTON_1, event->type == SDL_KEYDOWN); // SegaTV boost

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.down))
            setSwitch(PLAYER_2, BUTTON_DOWN, event->type == SDL_KEYDOWN); // Hummer boost

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.up))
            setSwitch(PLAYER_2, BUTTON_UP, event->type == SDL_KEYDOWN);
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
        if (event->key.keysym.sym == convertKeycode(keymap.test))
            setSwitch(SYSTEM, BUTTON_TEST, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.service))
            setSwitch(PLAYER_1, BUTTON_SERVICE, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.coin))
            incrementCoin(PLAYER_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.start))
            setSwitch(PLAYER_1, BUTTON_START, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button1))
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button2))
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button3))
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button4))
        {
            setSwitch(PLAYER_1, BUTTON_4, event->type == SDL_KEYDOWN);

            // For The House of the Dead 4's Accelerometer
            setAnalogue(ANALOGUE_5, 0);
            setAnalogue(ANALOGUE_6, 0);
        }
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.up))
            setSwitch(PLAYER_1, BUTTON_UP, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.down))
            setSwitch(PLAYER_1, BUTTON_DOWN, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.left))
            setSwitch(PLAYER_1, BUTTON_LEFT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.right))
            setSwitch(PLAYER_1, BUTTON_RIGHT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.start))
            setSwitch(PLAYER_2, BUTTON_START, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button1))
            setSwitch(PLAYER_2, BUTTON_1, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button2))
            setSwitch(PLAYER_2, BUTTON_1, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button3))
            setSwitch(PLAYER_2, BUTTON_1, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.up))
            setSwitch(PLAYER_2, BUTTON_UP, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.down))
            setSwitch(PLAYER_2, BUTTON_DOWN, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.left))
            setSwitch(PLAYER_2, BUTTON_LEFT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.right))
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
            if (mX < phX)
                mX = phX;
            if (mX > (phW + phX))
                mX = (phW + phX) - 1;
            if (mY > phH + phY)
                mY = (phH + phY) - 1;
            if (mY < phY)
                mY = phY;
            setAnalogue(ANALOGUE_1, ((double)(mX - phX) / (double)phW) * pow(2, jvsAnalogueInBits));
            setAnalogue(ANALOGUE_2, ((double)(mY - phY) / (double)phH) * pow(2, jvsAnalogueInBits));
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
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_MOUSEBUTTONDOWN);
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
    default:
        break;
    }
}

// TODO: This should be adjusted
void sdlEventABC(SDL_Event *event)
{
    KeyMapping keymap = getConfig()->keymap;
    switch (event->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        if (event->key.keysym.sym == convertKeycode(keymap.test))
            setSwitch(SYSTEM, BUTTON_TEST, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.service))
            setSwitch(PLAYER_1, BUTTON_SERVICE, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.coin))
            incrementCoin(PLAYER_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.start))
            setSwitch(PLAYER_1, BUTTON_START, event->type == SDL_KEYDOWN);
        // NOTE: I'm not sure about whether it can be mapped
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
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button1))
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button2))
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button3))
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == SDLK_f)
            setAnalogue(ANALOGUE_3, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.2 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == SDLK_r)
            setAnalogue(ANALOGUE_3, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.8 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.down))
            setSwitch(PLAYER_1, BUTTON_DOWN, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.up))
            setSwitch(PLAYER_1, BUTTON_UP, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.left))
            setSwitch(PLAYER_1, BUTTON_LEFT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.right))
            setSwitch(PLAYER_1, BUTTON_RIGHT, event->type == SDL_KEYDOWN);
    }
    break;
    default:
        break;
    }
}

void sdlEventHarley(SDL_Event *event)
{
    KeyMapping keymap = getConfig()->keymap;
    switch (event->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        if (event->key.keysym.sym == convertKeycode(keymap.test))
            setSwitch(SYSTEM, BUTTON_TEST, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.service))
            setSwitch(PLAYER_1, BUTTON_SERVICE, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.coin))
            incrementCoin(PLAYER_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.start))
            setSwitch(PLAYER_1, BUTTON_START, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.up))
            setAnalogue(ANALOGUE_1, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) - 1 : 0);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.down))
            setAnalogue(ANALOGUE_4, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) - 1 : 0);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.left))
            setAnalogue(ANALOGUE_2, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.2 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.right))
            setAnalogue(ANALOGUE_2, event->type == SDL_KEYDOWN ? pow(2, jvsAnalogueInBits) * 0.8 : pow(2, jvsAnalogueInBits) * 0.5);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button1))
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button2))
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button3))
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_KEYDOWN);
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button4))
            setSwitch(PLAYER_1, BUTTON_4, event->type == SDL_KEYDOWN);
        // TODO: Is this vaild?
        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button5))
            setSwitch(PLAYER_1, BUTTON_DOWN, event->type == SDL_KEYDOWN); // Hummer - SegaTV view change

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button6))
            setSwitch(PLAYER_1, BUTTON_UP, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button7))
            setSwitch(PLAYER_1, BUTTON_LEFT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button8))
            setSwitch(PLAYER_1, BUTTON_RIGHT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button1))
            setSwitch(PLAYER_2, BUTTON_1, event->type == SDL_KEYDOWN); // SegaTV boost

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.down))
            setSwitch(PLAYER_2, BUTTON_DOWN, event->type == SDL_KEYDOWN); // Hummer boost

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.up))
            setSwitch(PLAYER_2, BUTTON_UP, event->type == SDL_KEYDOWN);
    }
    break;
    default:
        break;
    }
}

void sdlEventMahjong(SDL_Event *event)
{
    KeyMapping keymap = getConfig()->keymap;
    switch (event->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
        if (event->key.keysym.sym == convertKeycode(keymap.test))
            setSwitch(SYSTEM, BUTTON_TEST, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.service))
            setSwitch(PLAYER_1, BUTTON_SERVICE, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.coin))
            incrementCoin(PLAYER_1, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.start))
            setSwitch(PLAYER_1, BUTTON_START, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.right))
            setSwitch(PLAYER_1, BUTTON_RIGHT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.left))
            setSwitch(PLAYER_1, BUTTON_LEFT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.up))
            setSwitch(PLAYER_1, BUTTON_UP, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.down))
            setSwitch(PLAYER_1, BUTTON_DOWN, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button1))
            setSwitch(PLAYER_1, BUTTON_1, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button1))
            setSwitch(PLAYER_1, BUTTON_2, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button3))
            setSwitch(PLAYER_1, BUTTON_3, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button4))
            setSwitch(PLAYER_1, BUTTON_4, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button5))
            setSwitch(PLAYER_1, BUTTON_5, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button6))
            setSwitch(PLAYER_1, BUTTON_6, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player1.button7))
            setSwitch(PLAYER_1, BUTTON_7, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.right))
            setSwitch(PLAYER_2, BUTTON_RIGHT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.left))
            setSwitch(PLAYER_2, BUTTON_LEFT, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.up))
            setSwitch(PLAYER_2, BUTTON_UP, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.down))
            setSwitch(PLAYER_2, BUTTON_DOWN, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button1))
            setSwitch(PLAYER_2, BUTTON_1, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button2))
            setSwitch(PLAYER_2, BUTTON_2, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button3))
            setSwitch(PLAYER_2, BUTTON_3, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button4))
            setSwitch(PLAYER_2, BUTTON_4, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button5))
            setSwitch(PLAYER_2, BUTTON_5, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button6))
            setSwitch(PLAYER_2, BUTTON_6, event->type == SDL_KEYDOWN);

        else if (event->key.keysym.sym == convertKeycode(keymap.player2.button7))
            setSwitch(PLAYER_2, BUTTON_7, event->type == SDL_KEYDOWN);
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

/**
 * Convert an X11 keycode to an SDL keycode.
 * This is generated from the defualt `xmodmap -pke`
 * If any keycodes are missing, please add them here.
 */
SDL_KeyCode convertKeycode(unsigned int xkeycode) {
    switch (xkeycode) {
        case 9: return SDLK_ESCAPE;
        case 10: return SDLK_1;
        case 11: return SDLK_2;
        case 12: return SDLK_3;
        case 13: return SDLK_4;
        case 14: return SDLK_5;
        case 15: return SDLK_6;
        case 16: return SDLK_7;
        case 17: return SDLK_8;
        case 18: return SDLK_9;
        case 19: return SDLK_0;
        case 20: return SDLK_MINUS;
        case 21: return SDLK_EQUALS;
        case 22: return SDLK_BACKSPACE;
        case 23: return SDLK_TAB;
        case 24: return SDLK_q;
        case 25: return SDLK_w;
        case 26: return SDLK_e;
        case 27: return SDLK_r;
        case 28: return SDLK_t;
        case 29: return SDLK_y;
        case 30: return SDLK_u;
        case 31: return SDLK_i;
        case 32: return SDLK_o;
        case 33: return SDLK_p;
        case 34: return SDLK_LEFTBRACKET;
        case 35: return SDLK_RIGHTBRACKET;
        case 36: return SDLK_RETURN;
        case 37: return SDLK_LCTRL;
        case 38: return SDLK_a;
        case 39: return SDLK_s;
        case 40: return SDLK_d;
        case 41: return SDLK_f;
        case 42: return SDLK_g;
        case 43: return SDLK_h;
        case 44: return SDLK_j;
        case 45: return SDLK_k;
        case 46: return SDLK_l;
        case 47: return SDLK_SEMICOLON;
        case 48: return SDLK_QUOTE;
        case 49: return SDLK_BACKQUOTE;
        case 50: return SDLK_LSHIFT;
        case 51: return SDLK_BACKSLASH;
        case 52: return SDLK_z;
        case 53: return SDLK_x;
        case 54: return SDLK_c;
        case 55: return SDLK_v;
        case 56: return SDLK_b;
        case 57: return SDLK_n;
        case 58: return SDLK_m;
        case 59: return SDLK_COMMA;
        case 60: return SDLK_PERIOD;
        case 61: return SDLK_SLASH;
        case 62: return SDLK_RSHIFT;
        case 63: return SDLK_KP_MULTIPLY;
        case 64: return SDLK_LALT;
        case 65: return SDLK_SPACE;
        case 66: return SDLK_CAPSLOCK;
        case 67: return SDLK_F1;
        case 68: return SDLK_F2;
        case 69: return SDLK_F3;
        case 70: return SDLK_F4;
        case 71: return SDLK_F5;
        case 72: return SDLK_F6;
        case 73: return SDLK_F7;
        case 74: return SDLK_F8;
        case 75: return SDLK_F9;
        case 76: return SDLK_F10;
        case 77: return SDLK_NUMLOCKCLEAR;
        case 78: return SDLK_SCROLLLOCK;
        case 79: return SDLK_KP_7;
        case 80: return SDLK_KP_8;
        case 81: return SDLK_KP_9;
        case 82: return SDLK_KP_MINUS;
        case 83: return SDLK_KP_4;
        case 84: return SDLK_KP_5;
        case 85: return SDLK_KP_6;
        case 86: return SDLK_KP_PLUS;
        case 87: return SDLK_KP_1;
        case 88: return SDLK_KP_2;
        case 89: return SDLK_KP_3;
        case 90: return SDLK_KP_0;
        case 91: return SDLK_KP_DECIMAL;
        case 95: return SDLK_F11;
        case 96: return SDLK_F12;
        case 104: return SDLK_KP_ENTER;
        case 105: return SDLK_RCTRL;
        case 106: return SDLK_KP_DIVIDE;
        case 107: return SDLK_PRINTSCREEN;
        case 108: return SDLK_RALT;
        case 110: return SDLK_HOME;
        case 111: return SDLK_UP;
        case 112: return SDLK_PAGEUP;
        case 113: return SDLK_LEFT;
        case 114: return SDLK_RIGHT;
        case 115: return SDLK_END;
        case 116: return SDLK_DOWN;
        case 117: return SDLK_PAGEDOWN;
        case 118: return SDLK_INSERT;
        case 119: return SDLK_DELETE;
        case 125: return SDLK_KP_EQUALS;
        case 126: return SDLK_KP_PLUSMINUS;
        case 127: return SDLK_PAUSE;
        case 129: return SDLK_KP_DECIMAL;
        case 135: return SDLK_MENU;
        case 144: return SDLK_FIND;
        case 146: return SDLK_HELP;
        case 218: return SDLK_PRINTSCREEN;
        case 231: return SDLK_CANCEL;
        default: return SDLK_UNKNOWN;
    }
}