#include <GL/gl.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <libgen.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "blitStretching.h"
#include "config.h"
#include "gameControllerMappings.h"
#include "iniParser.h"
#include "sdlInput.h"
#include "jvs.h"
#include "log.h"
#include "touchScreen.h"

// --- GLOBAL STATE AND MAPPINGS ---
ActionState gActionStates[MAX_ENTITIES][NUM_LOGICAL_ACTIONS] = {0};
JVSActionMapping gJvsMap[MAX_ENTITIES][NUM_LOGICAL_ACTIONS];
ChangedAction gChangedActions[NUM_LOGICAL_ACTIONS * MAX_ENTITIES];
LogicalActionProperties gActionProperties[MAX_ENTITIES][NUM_LOGICAL_ACTIONS] = {0};
int gNumChangedActions = 0;
SDLControllers sdlJoysticks = {0};
int gLastGunX[MAX_ENTITIES];
int gLastGunY[MAX_ENTITIES];
int gLastGunXDir[MAX_ENTITIES];
int gLastGunYDir[MAX_ENTITIES];
float gShakeValue[MAX_ENTITIES];
float gShakeIncreaseRate = 10.0f;
float gShakeDecayRate = 0.95f;

// --- NEW GLOBALS FOR GUID MAPPING ---
char gPlayerGUIDs[MAX_PLAYERS + 1][33]; // +1 for SYSTEM, 33 for 32 chars + null terminator
bool gPlayerGUIDsDirty = false;         // Flag to check if we need to save the INI

// Direct lookup tables for high-performance event handling
ControlBinding gKeyBindings[SDL_SCANCODE_COUNT] = {0};
ControlBinding gMouseButtonBindings[MAX_MOUSE_BUTTONS] = {0};
ControlBinding gMouseAxisBindings[2] = {0}; // 0=X, 1=Y
BindingPair gJoyAxisBindings[MAX_JOYSTICKS][MAX_JOY_AXES] = {0};
ControlBinding gJoyButtonBindings[MAX_JOYSTICKS][MAX_JOY_BUTTONS] = {0};
BindingPair gControllerAxisBindings[MAX_JOYSTICKS][SDL_GAMEPAD_AXIS_COUNT] = {0};
ControlBinding gControllerButtonBindings[MAX_JOYSTICKS][SDL_GAMEPAD_BUTTON_COUNT] = {0};
HatBinding gJoyHatBindings[MAX_JOYSTICKS][MAX_JOY_HATS] = {0};

extern uint32_t gId;
extern int gGrp;
extern int gWidth;
extern int gHeight;
int widthOri;
int heightOri;
extern SDL_Window *sdlWindow;
extern bool sdlInputInitialized;

extern Dest dest;

extern int phX, phY, phW, phH;
int phIsDragging = 0;

extern int drawableW;
extern int drawableH;

int jvsAnalogueMaxValue;
int jvsAnalogueCenterValue;

GameType gameType;

// Map between string names and LogicalAction enums for INI parsing.
const struct
{
    const char *name;
    LogicalAction action;
} gActionNameMap[] = {{"Test", LA_Test},
                      {"Coin", LA_Coin},
                      {"GearUp", LA_GearUp},
                      {"GearDown", LA_GearDown},
                      {"ViewChange", LA_ViewChange},
                      {"MusicChange", LA_MusicChange},
                      {"Boost", LA_Boost},
                      {"BoostRight", LA_BoostRight},
                      {"Start", LA_Start},
                      {"Service", LA_Service},
                      {"Up", LA_Up},
                      {"Down", LA_Down},
                      {"Left", LA_Left},
                      {"Right", LA_Right},
                      {"Button1", LA_Button1},
                      {"Button2", LA_Button2},
                      {"Button3", LA_Button3},
                      {"Button4", LA_Button4},
                      {"Button5", LA_Button5},
                      {"Button6", LA_Button6},
                      {"Button7", LA_Button7},
                      {"Button8", LA_Button8},
                      {"Button9", LA_Button9},
                      {"Button10", LA_Button10},
                      {"Trigger", LA_Trigger},
                      {"OutOfScreen", LA_OutOfScreen},
                      {"Reload", LA_Reload},
                      {"GunButton", LA_GunButton},
                      {"ActionButton", LA_ActionButton},
                      {"PedalLeft", LA_PedalLeft},
                      {"PedalRight", LA_PedalRight},
                      {"Steer", LA_Steer},
                      {"Gas", LA_Gas},
                      {"Brake", LA_Brake},
                      {"GunX", LA_GunX},
                      {"GunY", LA_GunY},
                      {"Steer_Left", LA_Steer_Left},
                      {"Steer_Right", LA_Steer_Right},
                      {"Gas_Digital", LA_Gas_Digital},
                      {"Brake_Digital", LA_Brake_Digital},
                      {"ABC_X", LA_ABC_X},
                      {"ABC_Left", LA_ABC_Left},
                      {"ABC_Right", LA_ABC_Right},
                      {"ABC_Y", LA_ABC_Y},
                      {"ABC_Up", LA_ABC_Up},
                      {"ABC_Down", LA_ABC_Down},
                      {"GunTrigger", LA_GunTrigger},
                      {"MissileTrigger", LA_MissileTrigger},
                      {"ClimaxSwitch", LA_ClimaxSwitch},
                      {"Throttle", LA_Throttle},
                      {"Throttle_Accelerate", LA_Throttle_Accelerate},
                      {"Throttle_Slowdown", LA_Throttle_Slowdown},
                      {"CardInsert", LA_CardInsert},
                      {"Card1Insert", LA_Card1Insert},
                      {"Card2Insert", LA_Card2Insert},
                      {"ButtonA", LA_A},
                      {"ButtonB", LA_B},
                      {"ButtonC", LA_C},
                      {"ButtonD", LA_D},
                      {"ButtonE", LA_E},
                      {"ButtonF", LA_F},
                      {"ButtonG", LA_G},
                      {"ButtonH", LA_H},
                      {"ButtonI", LA_I},
                      {"ButtonJ", LA_J},
                      {"ButtonK", LA_K},
                      {"ButtonL", LA_L},
                      {"ButtonM", LA_M},
                      {"ButtonN", LA_N},
                      {"ButtonReach", LA_Reach},
                      {"ButtonChi", LA_Chi},
                      {"ButtonPon", LA_Pon},
                      {"ButtonKan", LA_Kan},
                      {"ButtonAgari", LA_Agari},
                      {"ButtonCancel", LA_Cancel}};

const int NUM_ACTION_NAMES = sizeof(gActionNameMap) / sizeof(gActionNameMap[0]);

// --- Default Bindings ---
// These arrays define the built-in control schemes that are used when controls.ini is not found.
// They are also used by createDefaultControlsIni() to generate a fresh INI file.
extern const ControlBinding gDefaultCommonBindings[];
extern const ControlBinding gDefaultDigitalBindings[];
extern const ControlBinding gDefaultDrivingBindings[];
extern const ControlBinding gDefaultAbcBindings[];
extern const ControlBinding gDefaultShootingBindings[];
extern const ControlBinding gDefaultMahjongBindings[];

extern const size_t gDefaultCommonBindingsSize;
extern const size_t gDefaultDigitalBindingsSize;
extern const size_t gDefaultDrivingBindingsSize;
extern const size_t gDefaultAbcBindingsSize;
extern const size_t gDefaultShootingBindingsSize;
extern const size_t gDefaultMahjongBindingsSize;

#ifndef COMPILING_LINDBERGH_ELF

// Forward declaration
void saveGuidsToIni();

/**
 * @brief Initializes the entire SDL input system.
 * This function initializes SDL subsystems, detects attached controllers,
 * loads control schemes from controls.ini, or applies hardcoded defaults.
 * It is the main entry point for setting up all input handling.
 * @return 0 on success, non-zero on failure.
 */
int initSdlInput(char *controlsPath)
{
    if (getConfig()->inputMode == 2)
        return 0;

    // Initialize GameController subsystem
    if (!SDL_Init(SDL_INIT_GAMEPAD))
        log_warn("Could not initialize SDL_GameController: %s\n", SDL_GetError());

    // Loads the built-in database of controller
    loadSDLControllerMappings();

    // Load official and community-sourced controller mappings from a database file.
    char *envDbPath = getenv("LINDBERGH_CONTROLS_DB_PATH");
    if (envDbPath)
    {
        SDL_AddGamepadMappingsFromFile(envDbPath);
        printf("%s loaded as a controller database.\n", basename(envDbPath));
    }

    gameType = getConfig()->gameType;
    jvsAnalogueMaxValue = (1 << getJVSIO()->capabilities.analogueInBits) - 1;
    jvsAnalogueCenterValue = jvsAnalogueMaxValue / 2;
    memset(gPlayerGUIDs, 0, sizeof(gPlayerGUIDs));

    // Set initial analog values for specific games that require it.
    if (gGrp == GROUP_HOD4 || gGrp == GROUP_HOD4_TEST)
    {
        setAnalogue(ANALOGUE_5, jvsAnalogueCenterValue);
        setAnalogue(ANALOGUE_6, jvsAnalogueCenterValue);
        setAnalogue(ANALOGUE_7, jvsAnalogueCenterValue);
        setAnalogue(ANALOGUE_8, jvsAnalogueCenterValue);
    }
    else if (gId == HARLEY_DAVIDSON)
    {
        setAnalogue(ANALOGUE_2, jvsAnalogueCenterValue);
    }
    else if (gameType == DRIVING)
    {
        setAnalogue(ANALOGUE_1, jvsAnalogueCenterValue);
        setAnalogue(ANALOGUE_5, jvsAnalogueCenterValue);
    }

    // Initialize all mappings and properties.
    initJvsMappings();
    initActionProperties();

    // Load control bindings from specified file on tries to load it from the current folder, otherwise use defaults.
    struct stat buffer;
    IniConfig *ini;
    int isProfileLoaded = 0;
    if (stat(controlsPath, &buffer) == 0)
    {
        log_debug("controls config file loaded from %s\n", controlsPath);
        ini = iniLoad(controlsPath);
    }
    else
    {
        log_debug("controls config file \"controls.ini\" loaded from current folder\n");
        ini = iniLoad("controls.ini");
    }

    if (ini)
    {
        printf("Found controls.ini, loading custom configuration...\n");
        loadGlobalConfig(ini);
        loadProfileFromIni(iniGetSection(ini, "Common"));
        if (gameType == DRIVING)
            isProfileLoaded = loadProfileFromIni(iniGetSection(ini, "Driving"));
        else if (gameType == DIGITAL)
            isProfileLoaded = loadProfileFromIni(iniGetSection(ini, "Digital"));
        else if (gameType == SHOOTING)
            isProfileLoaded = loadProfileFromIni(iniGetSection(ini, "Shooting"));
        else if (gameType == ABC)
            isProfileLoaded = loadProfileFromIni(iniGetSection(ini, "ABC"));
        else if (gameType == MAHJONG)
            isProfileLoaded = loadProfileFromIni(iniGetSection(ini, "Mahjong"));

        iniFree(ini);
    }

    if (!isProfileLoaded)
    {
        setDefaultMappings();
    }

    // --- NEW GUID-BASED CONTROLLER MAPPING ---
    int numJoysticks;
    SDL_JoystickID *joystics = SDL_GetJoysticks(&numJoysticks);
    int playerToDeviceIndex[MAX_PLAYERS + 1];
    for (int i = 0; i <= MAX_PLAYERS; i++)
        playerToDeviceIndex[i] = -1; // -1 means unassigned

    if (joystics)
    {
        bool deviceIsClaimed[MAX_JOYSTICKS] = {false};
        SDL_JoystickID instanceId;
        // First Pass: Match saved GUIDs to available hardware
        printf("Attempting to map controllers by saved GUIDs...\n");
        for (int player = 1; player <= MAX_PLAYERS; player++)
        {
            if (strlen(gPlayerGUIDs[player]) > 0 && isProfileLoaded)
            {
                for (int i = 0; i < numJoysticks; i++)
                {
                    if (deviceIsClaimed[i])
                        continue;

                    instanceId = joystics[i];
                    SDL_GUID guid = SDL_GetJoystickGUIDForID(instanceId);
                    char device_guid_str[33];
                    SDL_GUIDToString(guid, device_guid_str, sizeof(device_guid_str));
                    device_guid_str[4] = '0';
                    device_guid_str[5] = '0';
                    device_guid_str[6] = '0';
                    device_guid_str[7] = '0';
                    if (strcmp(gPlayerGUIDs[player], device_guid_str) == 0)
                    {
                        printf("  - Matched P%d to device %d ('%s') via GUID.\n", player, i, SDL_GetJoystickNameForID(instanceId));
                        playerToDeviceIndex[player] = instanceId;
                        deviceIsClaimed[i] = true;
                        break;
                    }
                }
            }
        }

        // Second Pass: Assign remaining players to unclaimed controllers
        printf("Assigning remaining players to available controllers...\n");
        for (int player = 1; player <= MAX_PLAYERS; player++)
        {
            if (playerToDeviceIndex[player] == -1) // If this player is still unassigned
            {
                for (int i = 0; i < numJoysticks; i++)
                {
                    if (!deviceIsClaimed[i]) // Find the first unclaimed device
                    {
                        instanceId = joystics[i];
                        printf("  - Assigned P%d to first available device %d ('%s').\n", player, i, SDL_GetJoystickNameForID(instanceId));
                        playerToDeviceIndex[player] = instanceId;
                        deviceIsClaimed[i] = true;

                        // Get its GUID and mark for saving.
                        SDL_GUID guid = SDL_GetJoystickGUIDForID(instanceId);
                        SDL_GUIDToString(guid, gPlayerGUIDs[player], sizeof(gPlayerGUIDs[player]));
                        gPlayerGUIDs[player][4] = '0';
                        gPlayerGUIDs[player][5] = '0';
                        gPlayerGUIDs[player][6] = '0';
                        gPlayerGUIDs[player][7] = '0';
                        gPlayerGUIDsDirty = true;
                        break;
                    }
                }
            }
        }

        // Now, open devices in player-centric order.
        // The index `i` in sdlJoysticks (0..MAX_JOYSTICKS-1) now corresponds to Player (i+1).
        // This makes JOY0_, GC0_ always refer to P1, etc.
        sdlJoysticks.joysticksCount = 0;
        for (int player = 1; player <= MAX_PLAYERS; player++)
        {
            int deviceIndex = playerToDeviceIndex[player];
            int internal_index = player - 1;

            if (deviceIndex != -1)
            {
                if (SDL_IsGamepad(deviceIndex))
                {
                    sdlJoysticks.controllers[internal_index] = SDL_OpenGamepad(deviceIndex);
                }
                else
                {
                    sdlJoysticks.joysticks[internal_index] = SDL_OpenJoystick(deviceIndex);
                }

                if (sdlJoysticks.controllers[internal_index] || sdlJoysticks.joysticks[internal_index])
                {
                    sdlJoysticks.joysticksCount++;
                }
            }
        }
    }

    // Save any new GUID assignments back to the INI file.
    if (isProfileLoaded)
        saveGuidsToIni();

    // After loading all bindings, scan them to identify combined axes.
    detectCombinedAxes();

    // Apply any final game-specific mapping overrides.
    remapPerGame();

    sdlInputInitialized = true;
    return 0;
}

/**
 * @brief Parses a logical action key from the INI file (e.g., "P1_Start").
 * @param key The string key from the INI file.
 * @param out_player Pointer to store the parsed JVSPlayer.
 * @param out_action Pointer to store the parsed LogicalAction.
 * @return True if parsing was successful, false otherwise.
 */
bool parseActionKey(const char *key, JVSPlayer *out_player, LogicalAction *out_action)
{
    char genericKey[64];
    int p_num;
    if (sscanf(key, "P%d_%s", &p_num, genericKey) == 2)
    {
        if (p_num >= 1 && p_num <= MAX_PLAYERS)
            *out_player = (JVSPlayer)p_num;
        else
            return false;
    }
    else
    {
        *out_player = PLAYER_1;
        strcpy(genericKey, key);
        if (strcmp(genericKey, "Test") == 0)
            *out_player = SYSTEM;
    }
    for (int i = 0; i < NUM_ACTION_NAMES; i++)
    {
        if (strcmp(gActionNameMap[i].name, genericKey) == 0)
        {
            *out_action = gActionNameMap[i].action;
            return true;
        }
    }
    return false;
}

/**
 * @brief Initializes the mapping from LogicalActions to JVS inputs.
 * This function sets up the default JVS configuration for all players and actions,
 * defining whether an action corresponds to a switch, an analog input, or a coin drop.
 */
void initJvsMappings()
{
    for (int p = 0; p < MAX_ENTITIES; p++)
    {
        for (int i = 0; i < NUM_LOGICAL_ACTIONS; i++)
            gJvsMap[p][i] = (JVSActionMapping){JVS_CALL_NONE, NONE};
    }

    gJvsMap[SYSTEM][LA_Test] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_TEST};

    for (int p = 1; p <= MAX_PLAYERS; p++)
    {
        gJvsMap[p][LA_Start] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_START};
        gJvsMap[p][LA_Service] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_SERVICE};
        gJvsMap[p][LA_Coin] = (JVSActionMapping){JVS_CALL_COIN, COIN};

        int shootingPlayerIdx = (p - 1) * 2;

        if (gameType == DIGITAL)
        {
            gJvsMap[p][LA_Up] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_UP};
            gJvsMap[p][LA_Down] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_DOWN};
            gJvsMap[p][LA_Left] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_LEFT};
            gJvsMap[p][LA_Right] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_RIGHT};
            gJvsMap[p][LA_Button1] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_1};
            gJvsMap[p][LA_Button2] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_2};
            gJvsMap[p][LA_Button3] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_3};
        }
        else if (gameType == SHOOTING)
        {
            gJvsMap[p][LA_GunX] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1 + shootingPlayerIdx};
            gJvsMap[p][LA_GunY] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2 + shootingPlayerIdx};
            gJvsMap[p][LA_Trigger] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_1};
            gJvsMap[p][LA_Reload] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_2};
            gJvsMap[p][LA_GunButton] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_3};
            gJvsMap[p][LA_ActionButton] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_4};
            gJvsMap[p][LA_ChangeButton] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_5};
            gJvsMap[p][LA_PedalLeft] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_LEFT};
            gJvsMap[p][LA_PedalRight] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_RIGHT};
        }
    }

    switch ((int)gameType)
    {
        case DRIVING:
        {
            gJvsMap[PLAYER_1][LA_ViewChange] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_DOWN};
            gJvsMap[PLAYER_2][LA_GearUp] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_UP};
            gJvsMap[PLAYER_2][LA_GearDown] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_DOWN};
            gJvsMap[PLAYER_2][LA_Boost] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_1};
            gJvsMap[PLAYER_1][LA_CardInsert] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_UP};

            gJvsMap[PLAYER_1][LA_Steer] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1};
            gJvsMap[PLAYER_1][LA_Gas] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2};
            gJvsMap[PLAYER_1][LA_Brake] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_3};
            gJvsMap[PLAYER_1][LA_Gas_Digital] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2};
            gJvsMap[PLAYER_1][LA_Brake_Digital] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_3};
            gJvsMap[PLAYER_1][LA_Steer_Left] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1};
            gJvsMap[PLAYER_1][LA_Steer_Right] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1};
            gJvsMap[PLAYER_1][LA_Up] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_UP};
            gJvsMap[PLAYER_1][LA_Down] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_DOWN};
            gJvsMap[PLAYER_1][LA_Left] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_LEFT};
            gJvsMap[PLAYER_1][LA_Right] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_RIGHT};

            gJvsMap[PLAYER_2][LA_Steer] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_5};
            gJvsMap[PLAYER_2][LA_Gas] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_6};
            gJvsMap[PLAYER_2][LA_Brake] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_7};
            gJvsMap[PLAYER_2][LA_Gas_Digital] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_6};
            gJvsMap[PLAYER_2][LA_Brake_Digital] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_7};
            gJvsMap[PLAYER_2][LA_Steer_Left] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_5};
            gJvsMap[PLAYER_2][LA_Steer_Right] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_5};
        }
        break;
        case ABC:
        {
            gJvsMap[PLAYER_1][LA_ABC_X] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1};
            gJvsMap[PLAYER_1][LA_ABC_Left] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1};
            gJvsMap[PLAYER_1][LA_ABC_Right] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1};
            gJvsMap[PLAYER_1][LA_ABC_Y] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2};
            gJvsMap[PLAYER_1][LA_ABC_Up] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2};
            gJvsMap[PLAYER_1][LA_ABC_Down] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2};
            gJvsMap[PLAYER_1][LA_Throttle] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_3};
            gJvsMap[PLAYER_1][LA_Throttle_Accelerate] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_3};
            gJvsMap[PLAYER_1][LA_Throttle_Slowdown] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_3};
            gJvsMap[PLAYER_1][LA_GunTrigger] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_1};
            gJvsMap[PLAYER_1][LA_MissileTrigger] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_2};
            gJvsMap[PLAYER_1][LA_ClimaxSwitch] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_3};
        }
        break;
        case MAHJONG:
        {
            gJvsMap[PLAYER_1][LA_A] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_RIGHT};
            gJvsMap[PLAYER_1][LA_B] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_LEFT};
            gJvsMap[PLAYER_1][LA_C] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_UP};
            gJvsMap[PLAYER_1][LA_D] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_DOWN};
            gJvsMap[PLAYER_1][LA_E] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_1};
            gJvsMap[PLAYER_1][LA_F] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_2};
            gJvsMap[PLAYER_1][LA_G] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_3};
            gJvsMap[PLAYER_1][LA_H] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_4};
            gJvsMap[PLAYER_1][LA_I] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_5};
            gJvsMap[PLAYER_1][LA_J] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_6};
            gJvsMap[PLAYER_1][LA_K] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_7};
            gJvsMap[PLAYER_2][LA_L] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_RIGHT};
            gJvsMap[PLAYER_2][LA_M] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_LEFT};
            gJvsMap[PLAYER_2][LA_N] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_UP};
            gJvsMap[PLAYER_2][LA_Chi] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_DOWN};
            gJvsMap[PLAYER_2][LA_Pon] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_1};
            gJvsMap[PLAYER_2][LA_Kan] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_2};
            gJvsMap[PLAYER_2][LA_Reach] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_3};
            gJvsMap[PLAYER_2][LA_Agari] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_4};
            gJvsMap[PLAYER_2][LA_Cancel] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_5};
            gJvsMap[PLAYER_2][LA_CardInsert] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_7};
        }
        break;
    }
    gJvsMap[PLAYER_1][LA_Card1Insert] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_4};
    gJvsMap[PLAYER_2][LA_Card2Insert] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_4};
}

/**
 * @brief Applies specific JVS mapping overrides for certain games.
 * After the default mappings are set up, this function can patch them with
 * game-specific exceptions (e.g., if a game uses an unusual button layout).
 */
void remapPerGame()
{
    if (gId == R_TUNED)
    {
        gJvsMap[PLAYER_1][LA_BoostRight] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_RIGHT};
        gJvsMap[PLAYER_1][LA_CardInsert] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_UP};
    }
    else if (gGrp == GROUP_HUMMER)
        gJvsMap[PLAYER_2][LA_Boost] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_DOWN};
    else if (gGrp == GROUP_ID4_EXP || gGrp == GROUP_ID4_JAP || gGrp == GROUP_ID5)
        gJvsMap[PLAYER_1][LA_ViewChange] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_1};
    else if (gId == LETS_GO_JUNGLE || gId == LETS_GO_JUNGLE_REVA)
    {
        gJvsMap[PLAYER_1][LA_GunX] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2};
        gJvsMap[PLAYER_1][LA_GunY] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1};
        gJvsMap[PLAYER_2][LA_GunX] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2};
        gJvsMap[PLAYER_2][LA_GunY] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1};
    }
    else if (gId == HARLEY_DAVIDSON)
    {
        gJvsMap[PLAYER_1][LA_Steer] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2};
        gJvsMap[PLAYER_1][LA_Steer_Left] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2};
        gJvsMap[PLAYER_1][LA_Steer_Right] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_2};
        gJvsMap[PLAYER_1][LA_Gas] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1};
        gJvsMap[PLAYER_1][LA_Gas_Digital] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_1};
        gJvsMap[PLAYER_1][LA_Brake] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_4};
        gJvsMap[PLAYER_1][LA_Brake_Digital] = (JVSActionMapping){JVS_CALL_ANALOGUE, ANALOGUE_4};
        gJvsMap[PLAYER_1][LA_MusicChange] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_1};
        gJvsMap[PLAYER_1][LA_ViewChange] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_2};
        gJvsMap[PLAYER_1][LA_GearUp] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_3};
        gJvsMap[PLAYER_1][LA_GearDown] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_4};
    }
    else if (gId == PRIMEVAL_HUNT)
    {
        gJvsMap[PLAYER_1][LA_Reload] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_2};
        gJvsMap[PLAYER_1][LA_GunButton] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_3};
        gJvsMap[PLAYER_2][LA_Reload] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_2};
        gJvsMap[PLAYER_2][LA_GunButton] = (JVSActionMapping){JVS_CALL_SWITCH, BUTTON_3};
    }
}

/**
 * @brief Initializes special properties for certain logical actions.
 * For example, it defines which analog actions should auto-center (like a joystick)
 * and which should not (like a gas pedal).
 */
void initActionProperties()
{
    for (int p = 0; p < MAX_ENTITIES; p++)
    {
        for (int i = 0; i < NUM_LOGICAL_ACTIONS; i++)
        {
            gActionProperties[p][i].isCentering = false;
            gActionProperties[p][i].isCombinedAxis = false;
            // Default deadzone for most actions is a sensible 8000
            gActionProperties[p][i].deadzone = 8000;
        }
        // Explicitly define which actions are centering (like sticks)
        gActionProperties[p][LA_Steer].isCentering = true;

        // Set smaller default deadzones for non-centering actions
        gActionProperties[p][LA_Gas].deadzone = 500;
        gActionProperties[p][LA_Brake].deadzone = 500;
    }
    gActionProperties[PLAYER_1][LA_ABC_X].isCentering = true;
    gActionProperties[PLAYER_1][LA_ABC_Y].isCentering = true;
    gActionProperties[PLAYER_1][LA_Throttle].isCentering = true;
}

/**
 * @brief Adds a new control binding to the appropriate lookup table.
 * It also checks for and reports any conflicts where a single physical input
 * is mapped to two different logical actions. For axes, it allows up to two
 * bindings to support split-axis configurations.
 * @param binding The ControlBinding to add.
 */
void addBinding(ControlBinding binding)
{
    char name1[128], name2[128];
    char inputName[64] = "Unknown";

    // Handle non-axis types first, as they only allow one binding per input.
    if (binding.type != INPUT_TYPE_JOY_AXIS && binding.type != INPUT_TYPE_GAMEPAD_AXIS)
    {
        ControlBinding *existing = NULL;
        switch (binding.type)
        {
            case INPUT_TYPE_KEY:
                if (binding.sdlId < SDL_SCANCODE_COUNT)
                {
                    existing = &gKeyBindings[binding.sdlId];
                    snprintf(inputName, sizeof(inputName), "KEY_%s", SDL_GetScancodeName(binding.sdlId));
                }
                break;
            case INPUT_TYPE_MOUSE_BUTTON:
                if (binding.sdlId < MAX_MOUSE_BUTTONS)
                {
                    existing = &gMouseButtonBindings[binding.sdlId];
                    snprintf(inputName, sizeof(inputName), "MOUSE_BUTTON_%d", binding.sdlId);
                }
                break;
            case INPUT_TYPE_MOUSE_AXIS:
                if (binding.sdlId < 2)
                {
                    existing = &gMouseAxisBindings[binding.sdlId];
                    snprintf(inputName, sizeof(inputName), "MOUSE_AXIS_%d", binding.sdlId);
                }
                break;
            case INPUT_TYPE_JOY_BUTTON:
                if (binding.deviceIndex < MAX_JOYSTICKS && binding.sdlId < MAX_JOY_BUTTONS)
                {
                    existing = &gJoyButtonBindings[binding.deviceIndex][binding.sdlId];
                    snprintf(inputName, sizeof(inputName), "JOY%d_BUTTON_%d", binding.deviceIndex, binding.sdlId);
                }
                break;
            case INPUT_TYPE_GAMEPAD_BUTTON:
                if (binding.deviceIndex < MAX_JOYSTICKS && binding.sdlId < SDL_GAMEPAD_BUTTON_COUNT)
                {
                    existing = &gControllerButtonBindings[binding.deviceIndex][binding.sdlId];
                    snprintf(inputName, sizeof(inputName), "GC%d_BUTTON_%s", binding.deviceIndex,
                             SDL_GetGamepadStringForButton(binding.sdlId));
                }
                break;
            default:
                break;
        }

        if (existing)
        {
            if (existing->type != INPUT_TYPE_NONE && existing->action != binding.action)
            {
                getLogicalActionString(existing, name1, sizeof(name1), "");
                getLogicalActionString(&binding, name2, sizeof(name2), "");
                fprintf(stderr, "ERROR: Input assignment conflict for '%s'.\n", inputName);
                fprintf(stderr, "       It is mapped to both '%s' and '%s'. Please fix controls.ini.\n", name1, name2);
            }
            else
            {
                *existing = binding;
            }
        }
        return;
    }

    if (binding.type == INPUT_TYPE_JOY_HAT)
    {
        if (binding.deviceIndex < MAX_JOYSTICKS && binding.sdlId < MAX_JOY_HATS)
        {
            HatBinding *hat = &gJoyHatBindings[binding.deviceIndex][binding.sdlId];
            ControlBinding *targetSlot = NULL;

            // Determine which directional slot to use
            if (binding.axisThreshold == SDL_HAT_UP)
                targetSlot = &hat->up;
            else if (binding.axisThreshold == SDL_HAT_DOWN)
                targetSlot = &hat->down;
            else if (binding.axisThreshold == SDL_HAT_LEFT)
                targetSlot = &hat->left;
            else if (binding.axisThreshold == SDL_HAT_RIGHT)
                targetSlot = &hat->right;

            if (targetSlot)
            {
                if (targetSlot->type != INPUT_TYPE_NONE && targetSlot->action != binding.action)
                    printf("Conflict detection");
                else
                    *targetSlot = binding;
            }
        }
        return;
    }

    // Special handling for axis types to allow two bindings per physical axis.
    BindingPair *pair = NULL;
    int devIdx = binding.deviceIndex;
    int axisId = binding.sdlId;

    if (binding.type == INPUT_TYPE_JOY_AXIS)
    {
        if (devIdx < MAX_JOYSTICKS && axisId < MAX_JOY_AXES)
        {
            pair = &gJoyAxisBindings[devIdx][axisId];
            snprintf(inputName, sizeof(inputName), "JOY%d_AXIS_%d", devIdx, axisId);
        }
    }
    else if (binding.type == INPUT_TYPE_GAMEPAD_AXIS)
    {
        if (devIdx < MAX_JOYSTICKS && axisId < SDL_GAMEPAD_AXIS_COUNT)
        {
            pair = &gControllerAxisBindings[devIdx][axisId];
            snprintf(inputName, sizeof(inputName), "GC%d_AXIS_%s", devIdx, SDL_GetGamepadStringForAxis(axisId));
        }
    }

    if (pair)
    {
        // Find an empty slot in the pair to place the binding.
        if (pair->bindings[0].type == INPUT_TYPE_NONE)
        {
            pair->bindings[0] = binding;
        }
        else if (pair->bindings[1].type == INPUT_TYPE_NONE)
        {
            // Check for redundant bindings before adding.
            if (pair->bindings[0].action == binding.action)
            {
                getLogicalActionString(&binding, name1, sizeof(name1), "");
                fprintf(stderr, "WARNING: Redundant binding for '%s' on action '%s'. Check controls.ini.\n", inputName, name1);
            }
            pair->bindings[1] = binding;
        }
        else
        {
            fprintf(stderr, "ERROR: Cannot bind more than two actions to axis '%s'. Please fix controls.ini.\n", inputName);
        }
    }
}

/**
 * @brief Applies the default control mappings if controls.ini is not found.
 * Iterates through the default binding arrays and adds them to the system.
 */
void setDefaultMappings()
{
    log_warn("controls.ini not found. Applying default mappings...\n");

    // Add common bindings
    for (size_t i = 0; i < gDefaultCommonBindingsSize; i++)
    {
        addBinding(gDefaultCommonBindings[i]);
    }

    // Add game-specific bindings
    const ControlBinding *game_bindings = NULL;
    size_t bindingsCount = 0;

    if (gameType == DIGITAL)
    {
        game_bindings = gDefaultDigitalBindings;
        bindingsCount = gDefaultDigitalBindingsSize;
    }
    else if (gameType == DRIVING)
    {
        game_bindings = gDefaultDrivingBindings;
        bindingsCount = gDefaultDrivingBindingsSize;
    }
    else if (gameType == ABC)
    {
        game_bindings = gDefaultAbcBindings;
        bindingsCount = gDefaultAbcBindingsSize;
    }
    else if (gameType == SHOOTING)
    {
        game_bindings = gDefaultShootingBindings;
        bindingsCount = gDefaultShootingBindingsSize;
    }
    else if (gameType == MAHJONG)
    {
        game_bindings = gDefaultMahjongBindings;
        bindingsCount = gDefaultMahjongBindingsSize;
    }

    if (game_bindings)
    {
        for (size_t i = 0; i < bindingsCount; i++)
        {
            addBinding(game_bindings[i]);
        }
    }
}

/**
 * @brief Parses a single input source string (e.g., "KEY_A", "JOY0_AXIS_0_NEGATIVE")
 * and creates a ControlBinding for it.
 * @param token The string token representing the physical input.
 * @param player The player this binding belongs to.
 * @param action The logical action this input should trigger.
 */
void parseSdlSource(const char *token, JVSPlayer player, LogicalAction action)
{
    ControlBinding map = {0};
    map.action = action;
    map.player = player;
    map.type = INPUT_TYPE_NONE;
    char mutableToken[128];
    strncpy(mutableToken, token, 127);
    mutableToken[127] = '\0';
    char *invertedStr = strstr(mutableToken, "_INVERTED");
    if (invertedStr)
    {
        map.isInverted = true;
        *invertedStr = '\0';
    }
    else
    {
        map.isInverted = false;
    }

    if (strncmp(mutableToken, "KEY_", 4) == 0)
    {
        map.type = INPUT_TYPE_KEY;
        if (strcmp(mutableToken + 4, "Comma") == 0 || strcmp(mutableToken + 4, "COMMA") == 0)
            map.sdlId = SDL_SCANCODE_COMMA;
        else
            map.sdlId = SDL_GetScancodeFromName(mutableToken + 4);
    }
    else if (strncmp(mutableToken, "GC", 2) == 0)
    {
        int devId;
        char typeStr[32], nameStr[64];
        if (sscanf(mutableToken, "GC%d_%[^_]_%[^_]", &devId, typeStr, nameStr) == 3)
        {
            map.deviceIndex = devId;
            if (strcmp(typeStr, "BUTTON") == 0)
            {
                map.type = INPUT_TYPE_GAMEPAD_BUTTON;
                map.sdlId = SDL_GetGamepadButtonFromString(nameStr);
            }
            else if (strcmp(typeStr, "AXIS") == 0)
            {
                map.type = INPUT_TYPE_GAMEPAD_AXIS;
                map.sdlId = SDL_GetGamepadAxisFromString(nameStr);
                // Check for axis mode suffixes
                if (strstr(mutableToken, "_POSITIVE_HALF"))
                {
                    map.axisMode = AXIS_MODE_POSITIVE_HALF;
                }
                else if (strstr(mutableToken, "_NEGATIVE_HALF"))
                {
                    map.axisMode = AXIS_MODE_NEGATIVE_HALF;
                }
                else if (strstr(mutableToken, "_POSITIVE"))
                {
                    map.axisMode = AXIS_MODE_DIGITAL;
                    map.axisThreshold = 1;
                }
                else if (strstr(mutableToken, "_NEGATIVE"))
                {
                    map.axisMode = AXIS_MODE_DIGITAL;
                    map.axisThreshold = -1;
                }
                else
                {
                    map.axisMode = AXIS_MODE_FULL;
                }
            }
        }
    }
    else if (strncmp(mutableToken, "JOY", 3) == 0)
    {
        int devId, compId;
        char dirStr[32];
        if (sscanf(mutableToken, "JOY%d_BUTTON_%d", &devId, &compId) == 2)
        {
            map.type = INPUT_TYPE_JOY_BUTTON;
            map.deviceIndex = devId;
            map.sdlId = compId;
        }
        else if (sscanf(mutableToken, "JOY%d_AXIS_%d", &devId, &compId) == 2)
        {
            map.type = INPUT_TYPE_JOY_AXIS;
            map.deviceIndex = devId;
            map.sdlId = compId;
            if (strstr(mutableToken, "_POSITIVE_HALF"))
                map.axisMode = AXIS_MODE_POSITIVE_HALF;
            else if (strstr(mutableToken, "_NEGATIVE_HALF"))
                map.axisMode = AXIS_MODE_NEGATIVE_HALF;
            else if (strstr(mutableToken, "_POSITIVE"))
            {
                map.axisMode = AXIS_MODE_DIGITAL;
                map.axisThreshold = 1;
            }
            else if (strstr(mutableToken, "_NEGATIVE"))
            {
                map.axisMode = AXIS_MODE_DIGITAL;
                map.axisThreshold = -1;
            }
            else
                map.axisMode = AXIS_MODE_FULL;
        }
        else if (sscanf(mutableToken, "JOY%d_HAT%d_%s", &devId, &compId, dirStr) == 3)
        {
            map.type = INPUT_TYPE_JOY_HAT;
            map.deviceIndex = devId;
            map.sdlId = compId;
            if (strcmp(dirStr, "UP") == 0)
                map.axisThreshold = SDL_HAT_UP;
            else if (strcmp(dirStr, "DOWN") == 0)
                map.axisThreshold = SDL_HAT_DOWN;
            else if (strcmp(dirStr, "LEFT") == 0)
                map.axisThreshold = SDL_HAT_LEFT;
            else if (strcmp(dirStr, "RIGHT") == 0)
                map.axisThreshold = SDL_HAT_RIGHT;
        }
    }
    else if (strncmp(mutableToken, "MOUSE_", 6) == 0)
    {
        const char *mouseToken = mutableToken + 6;
        if (strcmp(mouseToken, "AXIS_X") == 0)
        {
            map.type = INPUT_TYPE_MOUSE_AXIS;
            map.sdlId = 0;
        }
        else if (strcmp(mouseToken, "AXIS_Y") == 0)
        {
            map.type = INPUT_TYPE_MOUSE_AXIS;
            map.sdlId = 1;
        }
        else if (strcmp(mouseToken, "LEFT_BUTTON") == 0)
        {
            map.type = INPUT_TYPE_MOUSE_BUTTON;
            map.sdlId = SDL_BUTTON_LEFT;
        }
        else if (strcmp(mouseToken, "RIGHT_BUTTON") == 0)
        {
            map.type = INPUT_TYPE_MOUSE_BUTTON;
            map.sdlId = SDL_BUTTON_RIGHT;
        }
        else if (strcmp(mouseToken, "MIDDLE_BUTTON") == 0)
        {
            map.type = INPUT_TYPE_MOUSE_BUTTON;
            map.sdlId = SDL_BUTTON_MIDDLE;
        }
    }
    if (map.type != INPUT_TYPE_NONE)
        addBinding(map);
}

/**
 * @brief Finds the internal player-centric index for a GameController from an SDL_JoystickID.
 * The internal index (0, 1, 2...) corresponds to the player number (P1, P2, P3...).
 * @param instance_id The SDL_JoystickID from an SDL_Event.
 * @return The internal player-centric index, or -1 if not found.
 */
int getControllerID(SDL_JoystickID instance_id)
{
    for (int i = 0; i < MAX_JOYSTICKS; i++)
    {
        if (sdlJoysticks.controllers[i])
        {
            SDL_Joystick *joy = SDL_GetGamepadJoystick(sdlJoysticks.controllers[i]);
            if (joy && SDL_GetJoystickID(joy) == instance_id)
                return i;
        }
    }
    return -1;
}

/**
 * @brief Finds the internal player-centric index for a raw Joystick from an SDL_JoystickID.
 * @param instance_id The SDL_JoystickID from an SDL_Event.
 * @return The internal player-centric index, or -1 if not found.
 */
int getJoystickID(SDL_JoystickID instance_id)
{
    for (int i = 0; i < MAX_JOYSTICKS; i++)
    {
        if (sdlJoysticks.joysticks[i])
        {
            if (SDL_GetJoystickID(sdlJoysticks.joysticks[i]) == instance_id)
                return i;
        }
    }
    return -1;
}

/**
 * @brief Applies game-specific logic to determine the correct player for an action.
 * Some actions are hardcoded to a specific player (e.g., P2_GearUp).
 * @param action The logical action being assigned.
 * @param player The player parsed from the INI file.
 * @return The corrected JVSPlayer.
 */
JVSPlayer fixPlayerForAction(LogicalAction action, int player)
{
    if (action == LA_Boost)
        return PLAYER_2;

    if ((action == LA_GearUp || action == LA_GearDown) && gId != HARLEY_DAVIDSON)
        return PLAYER_2;

    return player;
}

/**
 * @brief Determines if a Mahjong action belongs to Player 2.
 * @param action The logical action to check.
 * @return True if it's a Player 2 mahjong action, false otherwise.
 */
bool isMahjongP2(LogicalAction action)
{
    switch (action)
    {
        case LA_L:
        case LA_M:
        case LA_N:
        case LA_Chi:
        case LA_Pon:
        case LA_Kan:
        case LA_Reach:
        case LA_Agari:
        case LA_Cancel:
        case LA_CardInsert:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Loads and applies all control bindings from a given INI section.
 * @param section Pointer to the IniSection to parse.
 */
int loadProfileFromIni(const IniSection *section)
{
    if (!section)
        return 0;
    for (int i = 0; i < section->numPairs; i++)
    {
        JVSPlayer player;
        LogicalAction action;
        if (!parseActionKey(section->pairs[i].key, &player, &action))
        {
            printf("Warning: Unknown action '%s'\n", section->pairs[i].key);
            continue;
        }

        if (player != SYSTEM && (action != LA_Card1Insert || action != LA_Card2Insert))
            player = fixPlayerForAction(action, player);

        if (action == LA_Card2Insert && gameType == DIGITAL)
            player = PLAYER_2;

        if (strcmp(section->name, "Mahjong") == 0 && isMahjongP2(action))
            player = PLAYER_2;

        char *value_copy = strdup(section->pairs[i].value);
        char *token = strtok(value_copy, ", ");
        while (token != NULL)
        {
            parseSdlSource(token, player, action);
            token = strtok(NULL, ", ");
        }
        free(value_copy);
    }
    return 1;
}

/**
 * @brief Loads global configuration settings from the [Config] section of the INI.
 * This now includes loading saved controller GUIDs for player mapping.
 * @param ini Pointer to the loaded IniConfig.
 */
void loadGlobalConfig(const IniConfig *ini)
{
    const IniSection *configSection = iniGetSection(ini, "Config");
    if (configSection)
    {
        printf("Loading global settings from [Config] section...\n");
        for (int i = 0; i < configSection->numPairs; i++)
        {
            const char *key = configSection->pairs[i].key;
            const char *value = configSection->pairs[i].value;
            int p_num = -1;
            char actionKeyBuffer[64];

            // Check for player-specific prefix like "P1_"
            if (sscanf(key, "P%d_%s", &p_num, actionKeyBuffer) == 2)
            {
                // Player-specific key found (e.g., "P1_Steer_DeadZone")
                char *deadzoneSuffix = strstr(actionKeyBuffer, "_DeadZone");
                if (deadzoneSuffix)
                {
                    *deadzoneSuffix = '\0'; // Trim to just the action name
                    for (int j = 0; j < NUM_ACTION_NAMES; j++)
                    {
                        if (strcmp(gActionNameMap[j].name, actionKeyBuffer) == 0)
                        {
                            LogicalAction action = gActionNameMap[j].action;
                            if (p_num >= 1 && p_num <= MAX_PLAYERS)
                            {
                                gActionProperties[p_num][action].deadzone = atoi(value);
                                printf("  Set P%d_%s DeadZone to %d\n", p_num, actionKeyBuffer, gActionProperties[p_num][action].deadzone);
                            }
                            break;
                        }
                    }
                }
            }
            else
            {
                // Handle generic keys and other settings
                char *deadzonePos = strstr(key, "_DeadZone");
                if (deadzonePos != NULL)
                {
                    // Generic key found (e.g., "Steer_DeadZone")
                    // Apply this setting to ALL players as a default
                    size_t actionNameLen = deadzonePos - key;
                    strncpy(actionKeyBuffer, key, actionNameLen);
                    actionKeyBuffer[actionNameLen] = '\0';

                    for (int j = 0; j < NUM_ACTION_NAMES; j++)
                    {
                        if (strcmp(gActionNameMap[j].name, actionKeyBuffer) == 0)
                        {
                            LogicalAction action = gActionNameMap[j].action;
                            int deadzoneVal = atoi(value);
                            for (int p = 1; p <= MAX_PLAYERS; p++)
                            {
                                // Set this only if a player-specific one wasn't already set
                                // (This requires ordering in the INI, or a second pass - for simplicity, we just set it)
                                gActionProperties[p][action].deadzone = deadzoneVal;
                            }
                            printf("  Set %s DeadZone to %d for all players (default)\n", actionKeyBuffer, deadzoneVal);
                            break;
                        }
                    }
                }
                else if (strcmp(key, "ShakeIncreaseRate") == 0)
                {
                    gShakeIncreaseRate = atof(value);
                    printf("  Set ShakeIncreaseRate to %f\n", gShakeIncreaseRate);
                }
                else if (strcmp(key, "ShakeDecayRate") == 0)
                {
                    gShakeDecayRate = atof(value);
                    printf("  Set ShakeDecayRate to %f\n", gShakeDecayRate);
                }
            }
        }
    }

    // Load saved controller GUIDs
    const IniSection *guidSection = iniGetSection(ini, "ControllerGUIDs");
    if (guidSection)
    {
        printf("Loading saved controller GUIDs...\n");
        for (int i = 0; i < guidSection->numPairs; i++)
        {
            int p_num;
            if (sscanf(guidSection->pairs[i].key, "P%d_GUID", &p_num) == 1)
            {
                if (p_num >= 1 && p_num <= MAX_PLAYERS)
                {
                    strncpy(gPlayerGUIDs[p_num], guidSection->pairs[i].value, 32);
                    gPlayerGUIDs[p_num][32] = '\0';
                    printf("  - Loaded P%d GUID: %s\n", p_num, gPlayerGUIDs[p_num]);
                }
            }
        }
    }
}

/**
 * @brief Adds an action to a list of "dirty" actions that need processing.
 * This prevents redundant processing if an action's state changes multiple
 * times between frames.
 * @param player The player associated with the action.
 * @param action The action that has changed state.
 */
void addActionToDirtyList(JVSPlayer player, LogicalAction action)
{
    for (int i = 0; i < gNumChangedActions; i++)
    {
        if (gChangedActions[i].player == player && gChangedActions[i].action == action)
            return;
    }
    if (gNumChangedActions < (NUM_LOGICAL_ACTIONS * MAX_ENTITIES))
    {
        gChangedActions[gNumChangedActions++] = (ChangedAction){player, action};
    }
}

/**
 * @brief Processes a single SDL_Event from the event queue.
 * This is the main event handling function. It checks the event type and
 * updates the internal state of the corresponding logical action.
 * @param e Pointer to the SDL_Event to process.
 */
void processSdlEvent(const SDL_Event *e)
{
    if (!sdlInputInitialized)
        return;

    switch (e->type)
    {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            ControlBinding *binding = &gKeyBindings[e->key.scancode];
            if (binding->type != INPUT_TYPE_NONE)
            {
                bool isActive = (e->type == SDL_EVENT_KEY_DOWN);
                if (gActionStates[binding->player][binding->action].isActive != isActive)
                {
                    gActionStates[binding->player][binding->action].isActive = isActive;
                    addActionToDirtyList(binding->player, binding->action);
                }
            }
        }
        break;
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
        case SDL_EVENT_JOYSTICK_BUTTON_UP:
        {
            // printf("Joy Button: %d\n", e->jbutton.button);
            int devIdx = getJoystickID(e->jbutton.which);
            if (devIdx != -1 && e->jbutton.button < MAX_JOY_BUTTONS)
            {
                ControlBinding *binding = &gJoyButtonBindings[devIdx][e->jbutton.button];
                if (binding->type != INPUT_TYPE_NONE)
                {
                    bool is_active = (e->type == SDL_EVENT_JOYSTICK_BUTTON_DOWN);
                    if (gActionStates[binding->player][binding->action].isActive != is_active)
                    {
                        gActionStates[binding->player][binding->action].isActive = is_active;
                        addActionToDirtyList(binding->player, binding->action);
                    }
                }
            }
        }
        break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
        {
            // printf("GC Button: %s\n", SDL_GameControllerGetStringForButton(e->cbutton.button));
            int devIdx = getControllerID(e->gbutton.which);
            if (devIdx != -1 && e->gbutton.button < SDL_GAMEPAD_BUTTON_COUNT)
            {
                ControlBinding *binding = &gControllerButtonBindings[devIdx][e->gbutton.button];
                if (binding->type != INPUT_TYPE_NONE)
                {
                    bool is_active = (e->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
                    if (gActionStates[binding->player][binding->action].isActive != is_active)
                    {
                        gActionStates[binding->player][binding->action].isActive = is_active;
                        addActionToDirtyList(binding->player, binding->action);
                    }
                }
            }
        }
        break;
        case SDL_EVENT_JOYSTICK_AXIS_MOTION:
        {
            // printf("Jaxis: %d, Value: %d, Device: %d\n", e->jaxis.axis, e->jaxis.value, e->jaxis.which);
            int devIdx = getJoystickID(e->jaxis.which);
            if (devIdx != -1 && e->jaxis.axis < MAX_JOY_AXES)
            {
                BindingPair *pair = &gJoyAxisBindings[devIdx][e->jaxis.axis];
                for (int i = 0; i < 2; i++) // Loop over the pair of possible bindings
                {
                    ControlBinding *binding = &pair->bindings[i];
                    if (binding->type == INPUT_TYPE_NONE)
                        continue;

                    // UPDATED: Get deadzone from the action's properties
                    const int dead_zone = gActionProperties[binding->player][binding->action].deadzone;
                    ActionState *state = &gActionStates[binding->player][binding->action];
                    bool state_did_change = false;
                    float newVal;

                    if (binding->axisMode == AXIS_MODE_DIGITAL)
                    {
                        bool is_active = (abs(e->jaxis.value) > dead_zone)
                                             ? ((binding->axisThreshold > 0) ? (e->jaxis.value > 16384) : (e->jaxis.value < -16384))
                                             : false;

                        if (state->isActive != is_active)
                        {
                            state->isActive = is_active;
                            state_did_change = true;
                        }
                    }
                    else
                    {
                        if (gActionProperties[binding->player][binding->action].isCentering)
                        {
                            newVal = (abs(e->jaxis.value) < dead_zone) ? 0.5f : (float)(e->jaxis.value + 32768) / 65535.0f;
                        }
                        else
                        {
                            // For non-centering axes, the deadzone check is different.
                            // We check if the value has moved sufficiently from its resting point (-32768).
                            if (abs(e->jaxis.value - (-32768)) < dead_zone)
                            {
                                newVal = 0.0f;
                            }
                            else
                            {
                                newVal = (float)(e->jaxis.value + 32768) / 65535.0f;
                            }
                        }

                        if (binding->isInverted)
                            newVal = 1.0f - newVal;

                        if (binding->axisMode == AXIS_MODE_POSITIVE_HALF)
                        {
                            if (fabs(state->positiveContribution - newVal) > 0.001f)
                            {
                                state->positiveContribution = newVal;
                                state_did_change = true;
                            }
                        }
                        else if (binding->axisMode == AXIS_MODE_NEGATIVE_HALF)
                        {
                            if (fabs(state->negativeContribution - newVal) > 0.001f)
                            {
                                state->negativeContribution = newVal;
                                state_did_change = true;
                            }
                        }
                        else
                        { // AXIS_MODE_FULL
                            if (fabs(state->analogValue - newVal) > 0.001f)
                            {
                                state->analogValue = newVal;
                                state_did_change = true;
                            }
                        }
                    }
                    if (state_did_change)
                        addActionToDirtyList(binding->player, binding->action);
                }
            }
        }
        break;
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        {
            // printf("GC Axis: %s, Value: %d\n", SDL_GameControllerGetStringForAxis(e->caxis.axis), e->caxis.value);
            int devIdx = getControllerID(e->gaxis.which);
            if (devIdx != -1 && e->gaxis.axis < SDL_GAMEPAD_AXIS_COUNT)
            {
                BindingPair *pair = &gControllerAxisBindings[devIdx][e->gaxis.axis];
                for (int i = 0; i < 2; i++)
                {
                    ControlBinding *binding = &pair->bindings[i];
                    if (binding->type == INPUT_TYPE_NONE)
                        continue;

                    // UPDATED: Get deadzone from the action's properties
                    const int deadZone = gActionProperties[binding->player][binding->action].deadzone;
                    ActionState *state = &gActionStates[binding->player][binding->action];
                    bool stateDidChange = false;
                    float newVal;

                    if (binding->axisMode == AXIS_MODE_DIGITAL)
                    {
                        bool isActive = (abs(e->gaxis.value) > deadZone)
                                            ? ((binding->axisThreshold > 0) ? (e->gaxis.value > 16384) : (e->gaxis.value < -16384))
                                            : false;
                        if (state->isActive != isActive)
                        {
                            state->isActive = isActive;
                            stateDidChange = true;
                        }
                    }
                    else
                    {
                        bool isTrigger =
                            (e->gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || e->gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

                        if (gActionProperties[binding->player][binding->action].isCentering && !isTrigger)
                        {
                            newVal = (abs(e->gaxis.value) < deadZone) ? 0.5f : (float)(e->gaxis.value + 32768) / 65535.0f;
                        }
                        else // This path is for triggers and other non-centering axes
                        {
                            newVal = (abs(e->gaxis.value) < deadZone) ? 0.0f : (float)e->gaxis.value / 32767.0f;
                        }

                        if (binding->isInverted)
                            newVal = 1.0f - newVal;

                        if (binding->axisMode == AXIS_MODE_POSITIVE_HALF)
                        {
                            if (fabs(state->positiveContribution - newVal) > 0.001f)
                            {
                                state->positiveContribution = newVal;
                                stateDidChange = true;
                            }
                        }
                        else if (binding->axisMode == AXIS_MODE_NEGATIVE_HALF)
                        {
                            if (fabs(state->negativeContribution - newVal) > 0.001f)
                            {
                                state->negativeContribution = newVal;
                                stateDidChange = true;
                            }
                        }
                        else
                        { // AXIS_MODE_FULL
                            if (fabs(state->analogValue - newVal) > 0.001f)
                            {
                                state->analogValue = newVal;
                                stateDidChange = true;
                            }
                        }
                    }

                    if (stateDidChange)
                        addActionToDirtyList(binding->player, binding->action);
                }
            }
        }
        break;
        case SDL_EVENT_JOYSTICK_HAT_MOTION:
        {
            int devIdx = getJoystickID(e->jhat.which);
            if (devIdx != -1 && e->jhat.hat < MAX_JOY_HATS)
            {
                HatBinding *hat = &gJoyHatBindings[devIdx][e->jhat.hat];

                // An array of the 4 directional bindings for easy iteration
                ControlBinding *directions[] = {&hat->up, &hat->down, &hat->left, &hat->right};

                for (int i = 0; i < 4; i++)
                {
                    ControlBinding *binding = directions[i];
                    if (binding->type == INPUT_TYPE_NONE)
                        continue;

                    // Check if the current hat state activates this direction
                    bool is_active = (e->jhat.value & binding->axisThreshold) != 0;
                    ActionState *state = &gActionStates[binding->player][binding->action];

                    if (state->isActive != is_active)
                    {
                        state->isActive = is_active;
                        addActionToDirtyList(binding->player, binding->action);
                    }
                }
            }
        }
        break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            if (e->button.button < MAX_MOUSE_BUTTONS)
            {
                ControlBinding *binding = &gMouseButtonBindings[e->button.button];
                if (binding->type != INPUT_TYPE_NONE)
                {
                    bool isActive = (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN);
                    if (gActionStates[binding->player][binding->action].isActive != isActive)
                    {
                        int x, y;
                        gActionStates[binding->player][binding->action].isActive = isActive;
                        if (gId == PRIMEVAL_HUNT && getConfig()->emulateTouchscreen &&
                            phIsInsideTouchScreen(e->motion.x, e->motion.y, &x, &y))
                        {
                            phTouchClick(x, y, e->type);
                            phIsDragging = isActive;
                        }
                        else
                        {
                            if (e->button.button == SDL_BUTTON_RIGHT && (gId == RAMBO || gId == RAMBO_CHINA || gId == TOO_SPICY) &&
                                isActive)
                            {
                                ActionState *state = &gActionStates[binding->player][LA_GunX];
                                state->analogValue = -1;
                                addActionToDirtyList(binding->player, LA_GunX);
                                state = &gActionStates[binding->player][LA_GunY];
                                state->analogValue = -1;
                                addActionToDirtyList(binding->player, LA_GunY);
                                gActionStates[binding->player][LA_Reload].isActive = isActive;
                                addActionToDirtyList(binding->player, LA_Reload);
                            }
                            else
                            {
                                addActionToDirtyList(binding->player, binding->action);
                            }
                        }
                    }
                }
            }
        }
        break;
        case SDL_EVENT_MOUSE_MOTION:
        {
            float mX = e->motion.x;
            float mY = e->motion.y;
            float posX = 0.0f, posY = 0.0f;

            if (gId == PRIMEVAL_HUNT)
            {
                int x, y;
                if (phIsDragging && phIsInsideTouchScreen(mX, mY, &x, &y))
                    phTouchClick(x, y, e->type);

                int motX = mX, motY = mY;
                phTouchScreenCursor(mX, mY, &motX, &motY);
                posX = ((float)(motX - phX) / (float)phW);
                posY = ((float)(motY - phY) / (float)phH);
            }
            else
            {
                if (mX <= dest.X)
                    posX = 0;
                else if (mX >= (dest.W + dest.X - 5))
                    posX = dest.W;
                else
                    posX = mX - dest.X;

                if (mY <= dest.Y)
                    posY = 0;
                else if (mY >= (dest.H + dest.Y - 5))
                    posY = dest.H;
                else
                    posY = mY - dest.Y;

                posX /= dest.W;
                posY /= dest.H;
            }

            ControlBinding *bindingX = &gMouseAxisBindings[0]; // X-Axis
            ControlBinding *bindingY = &gMouseAxisBindings[1]; // Y-Axis
            if (gameType == SHOOTING && (posX <= 0.01 || posX >= 0.99 || posY <= 0.01 || posY >= 0.99))
            {
                if (gId == RAMBO || gId == RAMBO_CHINA || gId == TOO_SPICY)
                {
                    if (bindingX->type != INPUT_TYPE_NONE)
                    {
                        ActionState *state = &gActionStates[bindingX->player][LA_GunX];
                        state->analogValue = -1;
                        addActionToDirtyList(bindingX->player, LA_GunX);
                    }
                    if (bindingY->type != INPUT_TYPE_NONE)
                    {
                        ActionState *state = &gActionStates[bindingY->player][LA_GunY];
                        state->analogValue = -1;
                        addActionToDirtyList(bindingY->player, LA_GunY);
                    }
                }
                gActionStates[PLAYER_1][LA_Reload].isActive = true;
                addActionToDirtyList(PLAYER_1, LA_Reload);
                break;
            }
            else
            {
                gActionStates[PLAYER_1][LA_Reload].isActive = false;
                addActionToDirtyList(PLAYER_1, LA_Reload);
            }

            if (bindingX->type != INPUT_TYPE_NONE)
            {
                ActionState *state = &gActionStates[bindingX->player][bindingX->action];
                if (fabs(state->analogValue - posX) > 0.001f)
                {
                    if (bindingX->isInverted)
                        state->analogValue = 1.0 - posX;
                    else
                        state->analogValue = posX;
                    addActionToDirtyList(bindingX->player, bindingX->action);
                }
            }

            if (bindingY->type != INPUT_TYPE_NONE)
            {
                ActionState *state = &gActionStates[bindingY->player][bindingY->action];
                if (fabs(state->analogValue - posY) > 0.001f)
                {
                    if (bindingY->isInverted)
                        state->analogValue = 1.0 - posY;
                    else
                        state->analogValue = posY;
                    addActionToDirtyList(bindingY->player, bindingY->action);
                }
            }
        }
        break;
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        {
            Uint32 flags = SDL_GetWindowFlags(sdlWindow);
            if (!(flags & SDL_WINDOW_FULLSCREEN) && gId == GHOST_SQUAD_EVOLUTION)
            {
                if (e->type == SDL_EVENT_WINDOW_MOUSE_LEAVE)
                {
                    gActionStates[PLAYER_1][LA_Reload].isActive = true;
                    addActionToDirtyList(PLAYER_1, LA_Reload);
                }
                else // This case must be SDL_EVENT_WINDOW_ENTER
                {
                    gActionStates[PLAYER_1][LA_Reload].isActive = false;
                    addActionToDirtyList(PLAYER_1, LA_Reload);
                }
            }
        }
        break;
    }
}

/**
 * @brief Helper function to scan a pair of axis bindings and update detection flags.
 * @param pair The BindingPair to scan.
 * @param has_full_axis Array to track actions with full axis bindings.
 * @param has_half_axis Array to track actions with half axis bindings.
 * @param has_positive Array to track actions with positive half bindings.
 * @param has_negative Array to track actions with negative half bindings.
 */
static void scan_axis_bindings(BindingPair *pair, bool hasFullAxis[], bool hasHalfSxis[], bool hasPositive[][NUM_LOGICAL_ACTIONS],
                               bool hasNegative[][NUM_LOGICAL_ACTIONS])
{
    for (int k = 0; k < 2; k++)
    {
        ControlBinding *binding = &pair->bindings[k];
        if (binding->type == INPUT_TYPE_NONE)
            continue;

        if (binding->axisMode == AXIS_MODE_FULL)
        {
            hasFullAxis[binding->action] = true;
        }
        else if (binding->axisMode == AXIS_MODE_POSITIVE_HALF)
        {
            hasHalfSxis[binding->action] = true;
            hasPositive[binding->player][binding->action] = true;
        }
        else if (binding->axisMode == AXIS_MODE_NEGATIVE_HALF)
        {
            hasHalfSxis[binding->action] = true;
            hasNegative[binding->player][binding->action] = true;
        }
    }
}

/**
 * @brief Scans all loaded bindings to detect combined axes and potential conflicts.
 * It checks if any logical action is bound to both a full axis (like a stick)
 * and a half-axis (like a trigger), which would cause a conflict. It then flags
 * actions that are correctly configured as combined axes.
 */
void detectCombinedAxes()
{
    // --- Use local variables for detection ---
    bool hasFullAxis[NUM_LOGICAL_ACTIONS] = {false};
    bool hasHalfAxis[NUM_LOGICAL_ACTIONS] = {false};
    bool hasPositive[MAX_ENTITIES][NUM_LOGICAL_ACTIONS] = {false};
    bool hasNegative[MAX_ENTITIES][NUM_LOGICAL_ACTIONS] = {false};

    // --- Pass 1: Scan all axis bindings and flag their types ---
    for (int i = 0; i < MAX_JOYSTICKS; i++)
    {
        for (int j = 0; j < MAX_JOY_AXES; j++)
        {
            scan_axis_bindings(&gJoyAxisBindings[i][j], hasFullAxis, hasHalfAxis, hasPositive, hasNegative);
        }
        for (int j = 0; j < SDL_GAMEPAD_AXIS_COUNT; j++)
        {
            scan_axis_bindings(&gControllerAxisBindings[i][j], hasFullAxis, hasHalfAxis, hasPositive, hasNegative);
        }
    }

    // --- Pass 2: Check for conflicts and print warnings ---
    for (int i = 0; i < NUM_LOGICAL_ACTIONS; i++)
    {
        if (hasFullAxis[i] && hasHalfAxis[i])
        {
            const char *action_name = "Unknown";
            for (int j = 0; j < NUM_ACTION_NAMES; j++)
            {
                if (gActionNameMap[j].action == i)
                {
                    action_name = gActionNameMap[j].name;
                    break;
                }
            }
            printf("The action '%s' is mapped to both a full axis (e.g., a stick) and half-axes (e.g., triggers).\n", action_name);
            printf("         The half-axis bindings will take priority and may override the full axis input.\n");
        }
    }

    // --- Pass 3: Flag actions that are correctly combined ---
    for (int p = 0; p < MAX_ENTITIES; p++)
    {
        for (int i = 0; i < NUM_LOGICAL_ACTIONS; i++)
        {
            if (hasPositive[p][i] && hasNegative[p][i])
            {
                gActionProperties[p][i].isCombinedAxis = true;
            }
        }
    }
}

/**
 * @brief Updates the analog value for any action flagged as a combined axis.
 * This is called every frame to calculate a single analog value from the
 * positive and negative contributions of two separate half-axes.
 */
void updateCombinedAxes()
{
    for (int p = PLAYER_1; p <= MAX_PLAYERS; p++)
    {
        for (int i = 0; i < NUM_LOGICAL_ACTIONS; i++)
        {
            if (gActionProperties[p][i].isCombinedAxis)
            {
                ActionState *state = &gActionStates[p][i];
                // Combine the two halves into a single -1 to 1 value, represented as 0.0 to 1.0
                float finalVal = 0.5f + (state->positiveContribution * 0.5f) - (state->negativeContribution * 0.5f);
                if (finalVal > 1.0f)
                    finalVal = 1.0f;
                if (finalVal < 0.0f)
                    finalVal = 0.0f;
                if (fabs(state->analogValue - finalVal) > 0.001f)
                {
                    state->analogValue = finalVal;
                    addActionToDirtyList((JVSPlayer)p, (LogicalAction)i);
                }
            }
        }
    }
}

/**
 * @brief Processes all actions that have changed state since the last frame.
 * It iterates through the "dirty list", reads the current state of each action,
 * and sends the appropriate update to the JVS IO board.
 */
void processChangedActions()
{
    for (int i = 0; i < gNumChangedActions; i++)
    {
        JVSPlayer player = gChangedActions[i].player;
        LogicalAction actionId = gChangedActions[i].action;
        JVSActionMapping *map = &gJvsMap[player][actionId];
        ActionState *state = &gActionStates[player][actionId];
        switch (map->call_type)
        {
            case JVS_CALL_SWITCH:
                setSwitch(player, map->jvsInput, state->isActive);
                break;
            case JVS_CALL_ANALOGUE:
                // Special handling for digital actions that control an analog JVS input
                switch ((int)actionId)
                {
                    case LA_Gas_Digital:
                    case LA_Brake_Digital:
                        state->analogValue = state->isActive ? 1.0f : 0.0f;
                        break;
                    case LA_Steer_Left:
                    case LA_ABC_Left:
                    case LA_ABC_Up:
                    case LA_Throttle_Slowdown:
                        state->analogValue = state->isActive ? 0.0f : 0.5f;
                        break;
                    case LA_Steer_Right:
                    case LA_ABC_Right:
                    case LA_ABC_Down:
                    case LA_Throttle_Accelerate:
                        state->analogValue = state->isActive ? 1.0f : 0.5f;
                        break;
                }

                if (getConfig()->idSteeringPercentageReduction > 0.0f && map->jvsInput == ANALOGUE_1 &&
                    (gGrp == GROUP_ID4_EXP || gGrp == GROUP_ID4_JAP || gGrp == GROUP_ID5))
                    state->analogValue = (state->analogValue - 0.5f) * (getConfig()->idSteeringPercentageReduction / 100.0f) + 0.5f;

                setAnalogue(map->jvsInput, (int)(state->analogValue * jvsAnalogueMaxValue));
                break;
            case JVS_CALL_COIN:
                if (state->isActive)
                    incrementCoin(player, 1);
                break;
            case JVS_CALL_NONE:
                break;
        }
    }
    gNumChangedActions = 0; // Clear the list for the next frame
}

/**
 * @brief Calculates and updates the gun shake effect for HOD4.
 * This function is called every frame for games that support it.
 */
void updateGunShake()
{
    for (int p = PLAYER_1; p <= PLAYER_2; p++)
    {
        int currentX = jvsAnalogueMaxValue * gActionStates[p][LA_GunX].analogValue;
        int currentY = jvsAnalogueMaxValue * gActionStates[p][LA_GunY].analogValue;

        int deltaX = currentX - gLastGunX[p];
        int deltaY = currentY - gLastGunY[p];

        int currentDirX = (deltaX > 2) ? 1 : ((deltaX < -2) ? -1 : 0);
        int currentDirY = (deltaY > 2) ? 1 : ((deltaY < -2) ? -1 : 0);

        if (currentDirX != 0 && currentDirX == -gLastGunXDir[p])
            gShakeValue[p] += abs(deltaX) * gShakeIncreaseRate;

        if (currentDirY != 0 && currentDirY == -gLastGunYDir[p])
            gShakeValue[p] += abs(deltaY) * gShakeIncreaseRate;

        // Apply decay
        gShakeValue[p] *= gShakeDecayRate;
        if (gShakeValue[p] < 1.0f)
            gShakeValue[p] = 0.0f;

        // Clamp the shake value to the max internal range (0-512)
        if (gShakeValue[p] > 512.0f)
            gShakeValue[p] = 512.0f;

        // Map the 0-512 shake value to the final 512-1024 JVS output range
        int jvsOutputValue = 512 + (int)gShakeValue[p];

        // Send the analog values
        if (p == PLAYER_1)
        {
            setAnalogue(ANALOGUE_5, jvsOutputValue);
            setAnalogue(ANALOGUE_6, jvsOutputValue);
        }
        else if (p == PLAYER_2)
        {
            setAnalogue(ANALOGUE_7, jvsOutputValue);
            setAnalogue(ANALOGUE_8, jvsOutputValue);
        }

        // Update last known positions and directions for the next frame
        gLastGunX[p] = currentX;
        gLastGunY[p] = currentY;
        if (currentDirX != 0)
            gLastGunXDir[p] = currentDirX;
        if (currentDirY != 0)
            gLastGunYDir[p] = currentDirY;
    }
}

/**
 * @brief Saves the controller GUIDs back to controls.ini if they have changed.
 * This is called during initialization if a new controller was assigned to a player.
 */
void saveGuidsToIni()
{
    if (!gPlayerGUIDsDirty)
    {
        return;
    }

    printf("Saving updated controller GUIDs to controls.ini...\n");
    IniConfig *ini = iniLoad("controls.ini");
    if (!ini)
    {
        // If the file doesn't exist, create an empty config in memory to save.
        ini = calloc(1, sizeof(IniConfig));
        if (!ini)
        {
            fprintf(stderr, "ERROR: Failed to allocate memory for INI config.\n");
            return;
        }
    }

    // Set the GUID value for each player. iniSetValue will create the section/key if needed.
    for (int player = 1; player <= MAX_PLAYERS; player++)
    {
        if (strlen(gPlayerGUIDs[player]) > 0)
        {
            char key[16];
            snprintf(key, sizeof(key), "P%d_GUID", player);
            iniSetValue(ini, "ControllerGUIDs", key, gPlayerGUIDs[player]);
        }
    }

    if (iniSave(ini, "controls.ini") == 0)
    {
        printf("Successfully saved controls.ini with updated GUIDs.\n");
    }
    else
    {
        fprintf(stderr, "ERROR: Failed to save controls.ini.\n");
    }

    iniFree(ini);
    gPlayerGUIDsDirty = false;
}

#endif

bool needsPlayer(LogicalAction action, const char *name)
{
    switch ((int)action)
    {
        case LA_Boost:
        case LA_BoostRight:
        case LA_GearUp:
        case LA_GearDown:
        case LA_ViewChange:
        case LA_MusicChange:
        case LA_ABC_X:
        case LA_ABC_Y:
        case LA_ABC_Left:
        case LA_ABC_Right:
        case LA_ABC_Up:
        case LA_ABC_Down:
        case LA_Throttle:
        case LA_Throttle_Accelerate:
        case LA_Throttle_Slowdown:
        case LA_GunTrigger:
        case LA_MissileTrigger:
        case LA_ClimaxSwitch:
        case LA_CardInsert:
            return false;
        case LA_Up:
        case LA_Down:
        case LA_Left:
        case LA_Right:
            if (strcmp(name, "Driving") == 0)
                return false;
            else
                return true;
        default:
            if (strcmp(name, "Mahjong") == 0)
                return false;
            else
                return true;
    }
}

/**
 * @brief Helper to get the full "P1_Action" string for a binding.
 * Used for creating user-friendly error messages.
 * @param binding Pointer to the ControlBinding.
 * @param out_str Buffer to store the resulting string.
 * @param str_size Size of the output buffer.
 */
void getLogicalActionString(const ControlBinding *binding, char *outStr, size_t strSize, const char *name)
{
    const char *actionName = "Unknown";
    for (int i = 0; i < NUM_ACTION_NAMES; i++)
    {
        if (gActionNameMap[i].action == binding->action)
        {
            actionName = gActionNameMap[i].name;
            break;
        }
    }

    if (binding->player == SYSTEM)
    {
        snprintf(outStr, strSize, "%s", actionName);
    }
    else
    {
        if (needsPlayer(binding->action, name))
            snprintf(outStr, strSize, "P%d_%s", binding->player, actionName);
        else
            snprintf(outStr, strSize, "%s", actionName);
    }
}

int listSdlControllers(void)
{
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD))
    {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    int numJoysticks;
    SDL_JoystickID *joysticks = SDL_GetJoysticks(&numJoysticks);
    printf("Found %d controller(s)\n", numJoysticks);

    if (joysticks)
    {
        for (int i = 0; i < numJoysticks; i++)
        {
            SDL_JoystickID instanceId = joysticks[i];
            const char *name = SDL_GetJoystickNameForID(instanceId);
            SDL_GUID guid = SDL_GetJoystickGUIDForID(instanceId);
            char guidStr[33];
            SDL_GUIDToString(guid, guidStr, sizeof(guidStr));

            int count = 0;
            int index = 0;
            for (int j = 0; j < numJoysticks; j++)
            {
                instanceId = joysticks[j];
                const char *otherName = SDL_GetJoystickNameForID(instanceId);
                SDL_GUID other_guid = SDL_GetJoystickGUIDForID(instanceId);
                char other_guid_str[33];
                SDL_GUIDToString(other_guid, other_guid_str, sizeof(other_guid_str));
                if (strcmp(name, otherName) == 0 && strcmp(guidStr, other_guid_str) == 0)
                {
                    if (j < i)
                    {
                        index++;
                    }
                    count++;
                }
            }

            guidStr[4] = '0';
            guidStr[5] = '0';
            guidStr[6] = '0';
            guidStr[7] = '0';
            if (count > 1)
            {
                printf("  - %s %d: %s", name, index, guidStr);
            }
            else
            {
                printf("  - %s: %s", name, guidStr);
            }
            printf("\n");
        }
    }
    SDL_Quit();
    return 0;
}
