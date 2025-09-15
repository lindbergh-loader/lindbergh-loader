#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "iniParser.h"
#include "jvs.h"


// --- CORE DATA STRUCTURES ---
#define MAX_PLAYERS 4
#define MAX_ENTITIES (MAX_PLAYERS + 1)
#define MAX_JOYSTICKS 8
#define MAX_JOY_BUTTONS 32
#define MAX_JOY_AXES 32
#define MAX_JOY_HATS 4
#define MAX_MOUSE_BUTTONS 16

typedef enum
{
    LA_INVALID = -1,
    LA_Test ,
    LA_Coin,
    LA_GearUp,
    LA_GearDown,
    LA_ViewChange,
    LA_MusicChange,
    LA_Boost,
    LA_BoostRight,
    LA_Start,
    LA_Service,
    LA_Up,
    LA_Down,
    LA_Left,
    LA_Right,
    LA_Button1,
    LA_Button2,
    LA_Button3,
    LA_Button4,
    LA_Button5,
    LA_Button6,
    LA_Button7,
    LA_Button8,
    LA_Button9,
    LA_Button10,
    LA_Trigger,
    LA_OutOfScreen,
    LA_Reload,
    LA_GunButton,
    LA_ActionButton,
    LA_ChangeButton,
    LA_PedalLeft,
    LA_PedalRight,
    LA_Steer,
    LA_Gas,
    LA_Brake,
    LA_GunX,
    LA_GunY,
    LA_Steer_Left,
    LA_Steer_Right,
    LA_Gas_Digital,
    LA_Brake_Digital,
    LA_ABC_X,
    LA_ABC_Left,
    LA_ABC_Right,
    LA_ABC_Y,
    LA_ABC_Up,
    LA_ABC_Down,
    LA_Throttle,
    LA_Throttle_Accelerate,
    LA_Throttle_Slowdown,
    LA_GunTrigger,
    LA_MissileTrigger,
    LA_ClimaxSwitch,
    LA_CardInsert,
    LA_Card1Insert,
    LA_Card2Insert,
    LA_A,
    LA_B,
    LA_C,
    LA_D,
    LA_E,
    LA_F,
    LA_G,
    LA_H,
    LA_I,
    LA_J,
    LA_K,
    LA_L,
    LA_M,
    LA_N,
    LA_Reach,
    LA_Chi,
    LA_Pon,
    LA_Kan,
    LA_Agari,
    LA_Cancel,
    NUM_LOGICAL_ACTIONS
} LogicalAction;

typedef enum
{
    INPUT_TYPE_NONE,
    INPUT_TYPE_KEY,
    INPUT_TYPE_JOY_AXIS,
    INPUT_TYPE_JOY_BUTTON,
    INPUT_TYPE_JOY_HAT,
    INPUT_TYPE_MOUSE_AXIS,
    INPUT_TYPE_MOUSE_BUTTON,
    INPUT_TYPE_GAMEPAD_AXIS,
    INPUT_TYPE_GAMEPAD_BUTTON
} SDLInputType;

typedef enum
{
    AXIS_MODE_FULL,
    AXIS_MODE_POSITIVE_HALF,
    AXIS_MODE_NEGATIVE_HALF,
    AXIS_MODE_DIGITAL
} AxisMode;

typedef struct
{
    SDLInputType type;
    int deviceIndex;
    int sdlId;
    AxisMode axisMode;
    int axisThreshold;
    bool isInverted;
    JVSPlayer player;
    LogicalAction action;
} ControlBinding;

typedef struct
{
    ControlBinding bindings[2];
} BindingPair;

typedef struct
{
    ControlBinding up;
    ControlBinding down;
    ControlBinding left;
    ControlBinding right;
} HatBinding;

typedef struct
{
    bool isActive;
    float analogValue;
    float positiveContribution;
    float negativeContribution;
} ActionState;

typedef struct
{
    enum
    {
        JVS_CALL_SWITCH,
        JVS_CALL_ANALOGUE,
        JVS_CALL_COIN,
        JVS_CALL_NONE
    } call_type;
    JVSInput jvsInput;
} JVSActionMapping;

typedef struct
{
    JVSPlayer player;
    LogicalAction action;
} ChangedAction;

typedef struct
{
    bool isCentering;
    bool isCombinedAxis;
    int deadzone; // Added to store per-action deadzone
} LogicalActionProperties;

typedef struct
{
    SDL_Joystick *joysticks[MAX_JOYSTICKS];
    SDL_Gamepad *controllers[MAX_JOYSTICKS];
    SDL_Haptic *haptics[MAX_JOYSTICKS];
    int joysticksCount;
    // int hapticsCount;
    // int numButtons[MAX_JOYSTICKS];
    // int numAxes[MAX_JOYSTICKS];
    // int numHats[MAX_JOYSTICKS];
} SDLControllers;

// Ini Creation
#define MAX_INI_KEYS_PER_SECTION 100
#define MAX_INI_VALUE_LENGTH 256

typedef struct
{
    char key[64];
    char value[MAX_INI_VALUE_LENGTH];
} IniFileEntry;

typedef struct
{
    const char *name;
    IniFileEntry entries[MAX_INI_KEYS_PER_SECTION];
    int num_entries;
} IniFileSection;

int initSdlInput(char *controlsPath);
int loadProfileFromIni(const IniSection *section);
void loadGlobalConfig(const IniConfig *ini);
void processChangedActions();
void processSdlEvent(const SDL_Event *e);
void initJvsMappings();
void setDefaultMappings();
void remapPerGame();
void initActionProperties();
void updateGunShake();
void updateCombinedAxes();
void detectCombinedAxes();
void getLogicalActionString(const ControlBinding *binding, char *out_str, size_t str_size, const char *name);
bool needsPlayer(LogicalAction action, const char *name);
int listSdlControllers(void);
