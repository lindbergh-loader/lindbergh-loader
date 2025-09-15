#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "gameData.h"
#include "gpuVendor.h"
#include "log.h"
#include "iniParser.h"

EmulatorConfig config = {0};

extern uint32_t elf_crc;

FILE *configFile = NULL;

#define CONFIG_PATH "lindbergh.ini"
#define MAX_LINE_LENGTH 1024

const char *LindbergColourStrings[] = {"Lindbergh Yellow", "Lindbergh Red", "Lindbergh Blue", "Lindbergh Silver", "Lindbergh RedEX"};

const char *GameRegionStrings[] = {"Japan", "US", "Export"};

const char *GpuTypeStrings[] = {"Auto Detection", "NVIDIA", "AMD", "ATI", "INTEL", "Unknown", "ERROR_GPU"};

static int detectGame(uint32_t elf_crc)
{
    const GameData *gameData = getGameData(elf_crc);

    if (gameData)
    {
        config.gameTitle = (char *)gameData->gameTitle;
        config.gameShortTitle = (char *)gameData->gameShortTitle;
        config.gameDVP = (char *)gameData->gameDVP;
        config.gameID = (char *)gameData->gameID;
        config.gameReleaseYear = (char *)gameData->gameReleaseYear;
        config.gameNativeResolutions = (char *)gameData->gameNativeResolutions;
        config.gameStatus = gameData->gameStatus;
        config.jvsIOType = gameData->jvsIOType;
        config.gameType = gameData->gameType;
        config.width = gameData->width;
        config.height = gameData->height;
        config.gameGroup = gameData->gameGroup;
        config.emulateRideboard = gameData->emulateRideboard;
        config.emulateDriveboard = gameData->emulateDriveboard;
        config.emulateMotionboard = gameData->emulateMotionboard;
        config.emulateHW210CardReader = gameData->emulateHW210CardReader;
        config.emulateTouchscreen = gameData->emulateTouchscreen;
        config.gameLindberghColour = gameData->gameLindberghColour;
        return 0;
    }

    config.crc32 = UNKNOWN;
    return 1;
}

char *getGameName()
{
    return config.gameTitle;
}

char *getDvpName()
{
    return config.gameDVP;
}

char *getGameId()
{
    return config.gameID;
}

int getGameLindberghColour()
{
    return config.gameLindberghColour;
}

char *getGameReleaseYear()
{
    return config.gameReleaseYear;
}

char *getGameNativeResolutions()
{
    return config.gameNativeResolutions;
}

const char *getLindberghColourString(Colour lindberghColour)
{
    return LindbergColourStrings[lindberghColour];
}

const char *getGameRegionString(GameRegion region)
{
    return GameRegionStrings[region];
}

const char *getGpuTypeString(GpuType gpuType)
{
    return GpuTypeStrings[gpuType];
}

void toLowerCase(char *str)
{
    while (*str)
    {
        *str = tolower((unsigned char)*str); 
        str++;                               
    }
}

void setDefaultValues(EmulatorConfig *cfg)
{
    cfg->emulateRideboard = 0;
    cfg->emulateDriveboard = 0;
    cfg->emulateMotionboard = 0;
    cfg->emulateHW210CardReader = 0;
    cfg->emulateTouchscreen = 0;
    strcpy(cfg->cardFile1, "Card_01.crd");
    strcpy(cfg->cardFile2, "Card_02.crd");
    cfg->emulateJVS = 1;
    cfg->fullscreen = 0;
    cfg->lindberghColour = YELLOW;
    strcpy(cfg->eepromPath, "eeprom.bin");
    strcpy(cfg->sramPath, "sram.bin");
    strcpy(cfg->libCgPath, "");
    strcpy(cfg->jvsPath, "/dev/ttyUSB0");
    strcpy(cfg->serial1Path, "/dev/ttyS0");
    strcpy(cfg->serial2Path, "/dev/ttyS1");
    cfg->width = 640;
    cfg->height = 480;
    cfg->boostRenderRes = 1;
    cfg->region = EX;
    cfg->freeplay = -1;
    cfg->showDebugMessages = 0;
    cfg->hummerFlickerFix = 0;
    cfg->keepAspectRatio = 1;
    cfg->outrunLensGlareEnabled = 1;
    cfg->ramboGunsSwitch = 0;
    cfg->id5ChineseLanguage = 0;
    cfg->idSteeringPercentageReduction = 0.0f;
    cfg->lgjRenderWithMesa = 1;
    cfg->gameTitle = "Unknown game";
    cfg->gameID = "XXXX";
    cfg->gameDVP = "DVP-XXXX";
    cfg->gameType = SHOOTING;
    cfg->gameLindberghColour = YELLOW;
    cfg->gameReleaseYear = "";
    cfg->gameNativeResolutions = "";
    cfg->jvsIOType = SEGA_TYPE_3;
    cfg->GPUVendor = AUTO_DETECT_GPU;
    cfg->fpsLimiter = 1;
    cfg->fpsTarget = 60.0f;
    cfg->phScreenMode = 2;
    cfg->phTestScreenSingle = 1;
    cfg->disableBuiltinFont = 0;
    cfg->disableBuiltinLogos = 0;
    cfg->hideCursor = 1;
    cfg->customCursorEnabled = 0;
    strcpy(cfg->customCursor, "");
    cfg->customCursorWidth = 32;
    cfg->customCursorHeight = 32;
    strcpy(cfg->touchCursor, "");
    cfg->touchCursorWidth = 32;
    cfg->touchCursorHeight = 32;
    cfg->mj4EnabledAtT = 0;
    cfg->enableNetworkPatches = 1;
    strcpy(cfg->idIpSeat1, "");
    strcpy(cfg->idIpSeat2, "");
    strcpy(cfg->nicName, "");
    strcpy(cfg->or2IP, "");
    strcpy(cfg->or2Netmask, "");
    strcpy(cfg->IpCab1, "");
    strcpy(cfg->IpCab2, "");
    strcpy(cfg->IpCab3, "");
    strcpy(cfg->IpCab4, "");
    strcpy(cfg->tooSpicyIpCab1, "");
    strcpy(cfg->tooSpicyIpCab2, "");
    strcpy(cfg->srtvIP, "");
    cfg->cpuFreqGhz = 0.0f;
    memset(&cfg->arcadeInputs, 0, sizeof(cfg->arcadeInputs));
    cfg->crc32 = 0;
    cfg->gameGroup = -1;
    cfg->skipOutrunCabinetCheck = 0;
    cfg->borderEnabled = 0;
    cfg->whiteBorderPercentage = 0.02f;
    cfg->blackBorderPercentage = 0.0f;
    cfg->inputMode = 1;
}

static const char *getValue(const IniConfig *ini, const char *sectionName, const char *key)
{
    IniSection *section = iniGetSection(ini, sectionName);
    if (!section)
    {
        return NULL;
    }
    for (int i = 0; i < section->numPairs; i++)
    {
        if (strcmp(section->pairs[i].key, key) == 0)
        {
            return section->pairs[i].value;
        }
    }
    return NULL;
}

static char *cleanValue(const char *rawValue, char *buffer, size_t bufferSize)
{
    if (!rawValue || bufferSize == 0)
        return NULL;

    strncpy(buffer, rawValue, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';

    char *start = buffer;
    char *end = buffer + strlen(buffer) - 1;

    while (isspace((unsigned char)*start))
        start++;
    while (end > start && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';

    if (*start == '"' && *end == '"')
    {
        start++;
        *end = '\0';
    }
    return start;
}

static int getInt(const IniConfig *ini, const char *section, const char *key, int defaultValue)
{
    const char *rawValue = getValue(ini, section, key);
    if (!rawValue)
        return defaultValue;

    char cleanBuffer[256];
    char *valueStr = cleanValue(rawValue, cleanBuffer, sizeof(cleanBuffer));
    if (!valueStr)
        return defaultValue;

    char tempStr[256];
    strncpy(tempStr, valueStr, sizeof(tempStr) - 1);
    tempStr[sizeof(tempStr) - 1] = '\0';
    toLowerCase(tempStr);

    if (strcmp(tempStr, "auto") == 0)
    {
        return defaultValue;
    }
    if (strcmp(tempStr, "true") == 0)
    {
        return 1;
    }
    if (strcmp(tempStr, "false") == 0)
    {
        return 0;
    }
    if (strcmp(tempStr, "none") == 0)
    {
        return -1;
    }
    return atoi(valueStr);
}

static float getFloat(const IniConfig *ini, const char *section, const char *key, float defaultValue)
{
    const char *rawValue = getValue(ini, section, key);
    if (!rawValue)
        return defaultValue;
    char cleanBuffer[256];
    char *valueStr = cleanValue(rawValue, cleanBuffer, sizeof(cleanBuffer));
    return valueStr ? atof(valueStr) : defaultValue;
}

static void getString(const IniConfig *ini, const char *section, const char *key, char *dest, int dest_size)
{
    const char *rawValue = getValue(ini, section, key);
    if (rawValue)
    {
        char cleanBuffer[MAX_PATH_LENGTH];
        char *valueStr = cleanValue(rawValue, cleanBuffer, sizeof(cleanBuffer));
        if (valueStr)
        {
            strncpy(dest, valueStr, dest_size - 1);
            dest[dest_size - 1] = '\0';
        }
    }
}

void applyIniConfig(EmulatorConfig *config, const IniConfig *ini)
{
    // [Display]
    config->width = getInt(ini, "Display", "WIDTH", config->width);
    config->height = getInt(ini, "Display", "HEIGHT", config->height);
    config->boostRenderRes = getInt(ini, "Display", "BOOST_RENDER_RES", config->boostRenderRes);
    config->fullscreen = getInt(ini, "Display", "FULLSCREEN", config->fullscreen);
    config->borderEnabled = getInt(ini, "Display", "BORDER_ENABLED", config->borderEnabled);
    config->whiteBorderPercentage = getInt(ini, "Display", "WHITE_BORDER_PERCENTAGE", config->whiteBorderPercentage * 100) / 100.0f;
    config->blackBorderPercentage = getInt(ini, "Display", "BLACK_BORDER_PERCENTAGE", config->blackBorderPercentage * 100) / 100.0f;
    config->keepAspectRatio = getInt(ini, "Display", "KEEP_ASPECT_RATIO", config->keepAspectRatio);
    config->hideCursor = getInt(ini, "Display", "HIDE_CURSOR", config->hideCursor);

    // [Input]
    config->inputMode = getInt(ini, "Input", "INPUT_MODE", config->inputMode);

    // [Emulation]
    const char *regionRaw = getValue(ini, "Emulation", "REGION");
    if (regionRaw)
    {
        char cleanBuffer[16];
        char *regionStr = cleanValue(regionRaw, cleanBuffer, sizeof(cleanBuffer));
        if (strcmp(regionStr, "JP") == 0)
            config->region = JP;
        else if (strcmp(regionStr, "US") == 0)
            config->region = US;
        else if (strcmp(regionStr, "EX") == 0)
            config->region = EX;
    }
    config->freeplay = getInt(ini, "Emulation", "FREEPLAY", config->freeplay);
    config->emulateJVS = getInt(ini, "Emulation", "EMULATE_JVS", config->emulateJVS);
    config->emulateRideboard = getInt(ini, "Emulation", "EMULATE_RIDEBOARD", config->emulateRideboard);
    config->emulateDriveboard = getInt(ini, "Emulation", "EMULATE_DRIVEBOARD", config->emulateDriveboard);
    config->emulateMotionboard = getInt(ini, "Emulation", "EMULATE_MOTIONBOARD", config->emulateMotionboard);
    config->emulateHW210CardReader = getInt(ini, "Emulation", "EMULATE_HW210_CARDREADER", config->emulateHW210CardReader);
    config->emulateTouchscreen = getInt(ini, "Emulation", "EMULATE_TOUCHSCREEN", config->emulateTouchscreen);

    // [Cards]
    getString(ini, "Cards", "CARDFILE_01", config->cardFile1, MAX_PATH_LENGTH);
    getString(ini, "Cards", "CARDFILE_02", config->cardFile2, MAX_PATH_LENGTH);

    // [Paths]
    getString(ini, "Paths", "JVS_PATH", config->jvsPath, MAX_PATH_LENGTH);
    getString(ini, "Paths", "SERIAL_1_PATH", config->serial1Path, MAX_PATH_LENGTH);
    getString(ini, "Paths", "SERIAL_2_PATH", config->serial2Path, MAX_PATH_LENGTH);
    getString(ini, "Paths", "SRAM_PATH", config->sramPath, MAX_PATH_LENGTH);
    getString(ini, "Paths", "EEPROM_PATH", config->eepromPath, MAX_PATH_LENGTH);
    getString(ini, "Paths", "LIBCG_PATH", config->libCgPath, MAX_PATH_LENGTH);

    // [Graphics]
    config->configGPUVendor = config->GPUVendor = getInt(ini, "Graphics", "GPU_VENDOR", config->GPUVendor);
    config->hummerFlickerFix = getInt(ini, "Graphics", "HUMMER_FLICKER_FIX", config->hummerFlickerFix);
    config->outrunLensGlareEnabled = getInt(ini, "Graphics", "OUTRUN_LENS_GLARE_ENABLED", config->outrunLensGlareEnabled);
    config->fpsLimiter = getInt(ini, "Graphics", "FPS_LIMITER_ENABLED", config->fpsLimiter);
    config->fpsTarget = getFloat(ini, "Graphics", "FPS_TARGET", config->fpsTarget);
    config->lgjRenderWithMesa = getInt(ini, "Graphics", "LGJ_RENDER_WITH_MESA", config->lgjRenderWithMesa);
    config->disableBuiltinFont = getInt(ini, "Graphics", "DISABLE_BUILTIN_FONT", config->disableBuiltinFont);
    config->disableBuiltinLogos = getInt(ini, "Graphics", "DISABLE_BUILTIN_LOGOS", config->disableBuiltinLogos);

    // [Cursor]
    config->customCursorEnabled = getInt(ini, "Cursor", "CUSTOM_CURSOR_ENABLED", config->customCursorEnabled);
    getString(ini, "Cursor", "CUSTOM_CURSOR", config->customCursor, MAX_PATH_LENGTH);
    config->customCursorWidth = getInt(ini, "Cursor", "CUSTOM_CURSOR_WIDTH", config->customCursorWidth);
    config->customCursorHeight = getInt(ini, "Cursor", "CUSTOM_CURSOR_HEIGHT", config->customCursorHeight);
    getString(ini, "Cursor", "TOUCH_CURSOR", config->touchCursor, MAX_PATH_LENGTH);
    config->touchCursorWidth = getInt(ini, "Cursor", "TOUCH_CURSOR_WIDTH", config->touchCursorWidth);
    config->touchCursorHeight = getInt(ini, "Cursor", "TOUCH_CURSOR_HEIGHT", config->touchCursorHeight);

    // [GameSpecific]
    config->phScreenMode = getInt(ini, "GameSpecific", "PRIMEVAL_HUNT_SCREEN_MODE", config->phScreenMode);
    config->phTestScreenSingle = getInt(ini, "GameSpecific", "PRIMEVAL_HUNT_TEST_SCREEN_SINGLE", config->phTestScreenSingle);
    config->skipOutrunCabinetCheck = getInt(ini, "GameSpecific", "SKIP_OUTRUN_CABINET_CHECK", config->skipOutrunCabinetCheck);
    config->mj4EnabledAtT = getInt(ini, "GameSpecific", "MJ4_ENABLED_ALL_THE_TIME", config->mj4EnabledAtT);
    config->cpuFreqGhz = getFloat(ini, "GameSpecific", "CPU_FREQ_GHZ", config->cpuFreqGhz);
    config->ramboGunsSwitch = getInt(ini, "GameSpecific", "RAMBO_GUNS_SWITCH", config->ramboGunsSwitch);
    config->id5ChineseLanguage = getInt(ini, "GameSpecific", "ID5_CHINESE_LANGUAGE", config->id5ChineseLanguage);
    config->idSteeringPercentageReduction = getFloat(ini, "GameSpecific", "ID_STEERING_REDUCTION_PERCENTAGE", config->idSteeringPercentageReduction);

    config->showDebugMessages = getInt(ini, "System", "DEBUG_MSGS", config->showDebugMessages);
    const char *colourRaw = getValue(ini, "System", "LINDBERGH_COLOUR");
    if (colourRaw)
    {
        char cleanBuffer[32];
        char *colourStr = cleanValue(colourRaw, cleanBuffer, sizeof(cleanBuffer));
        for (char *p = colourStr; *p; ++p)
            *p = toupper(*p);
        if (strcmp(colourStr, "RED") == 0)
            config->lindberghColour = RED;
        else if (strcmp(colourStr, "YELLOW") == 0)
            config->lindberghColour = YELLOW;
        else if (strcmp(colourStr, "BLUE") == 0)
            config->lindberghColour = BLUE;
        else if (strcmp(colourStr, "SILVER") == 0)
            config->lindberghColour = SILVER;
        else if (strcmp(colourStr, "REDEX") == 0)
            config->lindberghColour = REDEX;
    }

    // [Network]
    config->enableNetworkPatches = getInt(ini, "Network", "ENABLE_NETWORK_PATCHES", config->enableNetworkPatches);
    getString(ini, "Network", "ID_IP_SEAT_1", config->idIpSeat1, 16);
    getString(ini, "Network", "ID_IP_SEAT_2", config->idIpSeat2, 16);
    getString(ini, "Network", "NIC_NAME", config->nicName, 20);
    getString(ini, "Network", "OR2_IPADDRESS", config->or2IP, 16); 
    getString(ini, "Network", "OR2_NETMASK", config->or2Netmask, 16);
    getString(ini, "Network", "IP_CAB1", config->IpCab1, 16);
    getString(ini, "Network", "IP_CAB2", config->IpCab2, 16);
    getString(ini, "Network", "IP_CAB3", config->IpCab3, 16);
    getString(ini, "Network", "IP_CAB4", config->IpCab4, 16);
    getString(ini, "Network", "2SPICY_IP_CAB1", config->tooSpicyIpCab1, 16);
    getString(ini, "Network", "2SPICY_IP_CAB2", config->tooSpicyIpCab2, 16);
    getString(ini, "Network", "SRTV_IPADDRESS", config->srtvIP, 16);

    // [EVDEV]
    getString(ini, "EVDEV", "TEST_BUTTON", config->arcadeInputs.test, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_COIN", config->arcadeInputs.player1_coin, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_START", config->arcadeInputs.player1_button_start, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_SERVICE", config->arcadeInputs.player1_button_service, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_UP", config->arcadeInputs.player1_button_up, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_DOWN", config->arcadeInputs.player1_button_down, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_LEFT", config->arcadeInputs.player1_button_left, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_RIGHT", config->arcadeInputs.player1_button_right, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_1", config->arcadeInputs.player1_button_1, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_2", config->arcadeInputs.player1_button_2, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_3", config->arcadeInputs.player1_button_3, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_4", config->arcadeInputs.player1_button_4, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_5", config->arcadeInputs.player1_button_5, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_6", config->arcadeInputs.player1_button_6, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_7", config->arcadeInputs.player1_button_7, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_1_BUTTON_8", config->arcadeInputs.player1_button_8, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_COIN", config->arcadeInputs.player2_coin, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_START", config->arcadeInputs.player2_button_start, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_SERVICE", config->arcadeInputs.player2_button_service, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_UP", config->arcadeInputs.player2_button_up, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_DOWN", config->arcadeInputs.player2_button_down, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_LEFT", config->arcadeInputs.player2_button_left, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_RIGHT", config->arcadeInputs.player2_button_right, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_1", config->arcadeInputs.player2_button_1, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_2", config->arcadeInputs.player2_button_2, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_3", config->arcadeInputs.player2_button_3, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_4", config->arcadeInputs.player2_button_4, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_5", config->arcadeInputs.player2_button_5, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_6", config->arcadeInputs.player2_button_6, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_7", config->arcadeInputs.player2_button_7, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "PLAYER_2_BUTTON_8", config->arcadeInputs.player2_button_8, INPUT_STRING_LENGTH);

    getString(ini, "EVDEV", "ANALOGUE_1", config->arcadeInputs.analogue_1, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "ANALOGUE_2", config->arcadeInputs.analogue_2, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "ANALOGUE_3", config->arcadeInputs.analogue_3, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "ANALOGUE_4", config->arcadeInputs.analogue_4, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "ANALOGUE_5", config->arcadeInputs.analogue_5, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "ANALOGUE_6", config->arcadeInputs.analogue_6, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "ANALOGUE_7", config->arcadeInputs.analogue_7, INPUT_STRING_LENGTH);
    getString(ini, "EVDEV", "ANALOGUE_8", config->arcadeInputs.analogue_8, INPUT_STRING_LENGTH);

    for (int i = 0; i < 8; i++)
    {
        char key[32];
        sprintf(key, "ANALOGUE_DEADZONE_%d", i + 1);
        const char *rawDz = getValue(ini, "EVDEV", key);
        if (rawDz)
        {
            char cleanBuffer[64];
            char *dzStr = cleanValue(rawDz, cleanBuffer, sizeof(cleanBuffer));
            if (dzStr)
            {
                sscanf(dzStr, "%d %d %d", &config->arcadeInputs.analogue_deadzone_start[i],
                       &config->arcadeInputs.analogue_deadzone_middle[i], &config->arcadeInputs.analogue_deadzone_end[i]);
            }
        }
    }
}

int initConfig(const char *configFilePath)
{
    setDefaultValues(&config);

    config.crc32 = elf_crc;
    if (detectGame(config.crc32) != 0)
    {
        log_warn("Unsure what game with CRC 0x%X is. Please submit this new game to the GitHub repository: "
                 "https://github.com/lindbergh-loader/lindbergh-loader/issues/"
                 "new?title=Please+add+new+game+0x%X&body=I+tried+to+launch+the+following+game:\n",
                 config.crc32, config.crc32);
    }

    config.inputMode = 0; 

    char filePath[PATH_MAX];
    if (configFilePath != NULL && configFilePath[0] != '\0')
    {
        strncpy(filePath, configFilePath, PATH_MAX - 1);
    }
    else
    {
        strncpy(filePath, CONFIG_PATH, PATH_MAX - 1);
    }
    filePath[PATH_MAX - 1] = '\0';

    IniConfig *ini = iniLoad(filePath);

    if (ini == NULL)
    {
        log_warn("Cannot open or parse %s, using default values.", filePath);
        return 1;
    }

    applyIniConfig(&config, ini);

    if (config.customCursorEnabled && (strcmp(config.customCursor, "") != 0 || strcmp(config.touchCursor, "") != 0))
        config.hideCursor = 0;

    iniFree(ini);

    return 0;
}

EmulatorConfig *getConfig()
{
    return &config;
}
