#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "config.h"

typedef struct GameData {
    uint32_t crc32;
    const char *gameTitle;
    const char *gameShortTitle;
    const char *gameDVP;
    const char *gameID;
    const char *gameReleaseYear;
    const char *gameNativeResolutions;
    GameStatus gameStatus;
    JVSIOType jvsIOType;
    GameType gameType;
    int width;
    int height;
    GameGroup gameGroup;
    int emulateRideboard;
    int emulateDriveboard;
    int emulateMotionboard;
    int emulateHW210CardReader;
    int emulateIDCardReader;
    int emulateTouchscreen;
    Colour gameLindberghColour;
} GameData;

const GameData *getGameData(uint32_t elf_crc);

#endif // GAME_DATA_H
