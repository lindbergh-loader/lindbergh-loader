#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "controlIniGen.h"
#include "iniParser.h"
#include "sdlInput.h"

const ControlBinding gDefaultCommonBindings[] = {
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_T, AXIS_MODE_DIGITAL, 0, false, SYSTEM, LA_Test},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_5, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Coin},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_6, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Coin},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_1, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Start},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_2, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Start},
    {INPUT_TYPE_JOY_BUTTON, 0, 11, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Start},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_START, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Start},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_S, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Service},
    {INPUT_TYPE_JOY_BUTTON, 0, 10, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Service},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_BACK, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Service},
};

const size_t gDefaultCommonBindingsSize = sizeof(gDefaultCommonBindings) / sizeof(gDefaultCommonBindings[0]);

const ControlBinding gDefaultDigitalBindings[] = {
    // Keyboard
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_UP, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Up},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_DOWN, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Down},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_LEFT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Left},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_RIGHT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Right},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_Q, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Button1},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_W, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Button2},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_E, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Button3},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_F7, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Card1Insert},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_F8, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Card2Insert},

    // Controller
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_DPAD_UP, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Up},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_DPAD_DOWN, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Down},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_DPAD_LEFT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Left},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_DPAD_RIGHT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Right},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_SOUTH, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Button1},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_EAST, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Button2},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_WEST, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Button3},
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_LEFTY, AXIS_MODE_DIGITAL, -1, false, PLAYER_1, LA_Up},
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_LEFTY, AXIS_MODE_DIGITAL, 1, false, PLAYER_1, LA_Down},
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_LEFTX, AXIS_MODE_DIGITAL, 1, false, PLAYER_1, LA_Right},
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_LEFTX, AXIS_MODE_DIGITAL, -1, false, PLAYER_1, LA_Left},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_GUIDE, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Card1Insert},
    {INPUT_TYPE_GAMEPAD_BUTTON, 1, SDL_GAMEPAD_BUTTON_GUIDE, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Card2Insert},

    // Joystick
    {INPUT_TYPE_JOY_HAT, 0, 0, AXIS_MODE_DIGITAL, SDL_HAT_UP, false, PLAYER_1, LA_Up},
    {INPUT_TYPE_JOY_HAT, 0, 0, AXIS_MODE_DIGITAL, SDL_HAT_DOWN, false, PLAYER_1, LA_Down},
    {INPUT_TYPE_JOY_HAT, 0, 0, AXIS_MODE_DIGITAL, SDL_HAT_LEFT, false, PLAYER_1, LA_Left},
    {INPUT_TYPE_JOY_HAT, 0, 0, AXIS_MODE_DIGITAL, SDL_HAT_RIGHT, false, PLAYER_1, LA_Right},
    {INPUT_TYPE_JOY_BUTTON, 0, 0, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Button1},
    {INPUT_TYPE_JOY_BUTTON, 0, 1, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Button2},
    {INPUT_TYPE_JOY_BUTTON, 0, 2, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Button3},
    {INPUT_TYPE_JOY_AXIS, 0, 1, AXIS_MODE_DIGITAL, -1, false, PLAYER_1, LA_Up},
    {INPUT_TYPE_JOY_AXIS, 0, 1, AXIS_MODE_DIGITAL, 1, false, PLAYER_1, LA_Down},
    {INPUT_TYPE_JOY_AXIS, 0, 0, AXIS_MODE_DIGITAL, -1, false, PLAYER_1, LA_Left},
    {INPUT_TYPE_JOY_AXIS, 0, 0, AXIS_MODE_DIGITAL, 1, false, PLAYER_1, LA_Right},
    {INPUT_TYPE_JOY_BUTTON, 0, 11, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Card1Insert},
    {INPUT_TYPE_JOY_BUTTON, 1, 11, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Card2Insert},
};

const size_t gDefaultDigitalBindingsSize = sizeof(gDefaultDigitalBindings) / sizeof(gDefaultDigitalBindings[0]);

const ControlBinding gDefaultDrivingBindings[] = {
    // Keyboard
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_LEFT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Steer_Left},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_RIGHT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Steer_Right},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_UP, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Gas_Digital},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_DOWN, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Brake_Digital},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_V, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ViewChange},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_Q, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Boost},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_W, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_BoostRight},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_A, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_GearUp},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_Z, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_GearDown},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_M, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_MusicChange},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_I, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Up},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_K, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Down},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_J, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Left},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_L, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Right},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_F7, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_CardInsert},

    // Controller
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_LEFTX, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_Steer},
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_Gas},
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_Brake},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_SOUTH, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Boost},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_EAST, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_BoostRight},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_NORTH, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ViewChange},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_WEST, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_MusicChange},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_GearUp},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_GearDown},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_DPAD_UP, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Up},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_DPAD_DOWN, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Down},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_DPAD_LEFT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Left},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_DPAD_RIGHT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Right},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_GUIDE, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_CardInsert},

    // Joystick
    {INPUT_TYPE_JOY_AXIS, 0, 0, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_Steer},
    {INPUT_TYPE_JOY_AXIS, 0, 5, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_Gas},
    {INPUT_TYPE_JOY_AXIS, 0, 2, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_Brake},
    {INPUT_TYPE_JOY_BUTTON, 0, 0, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Boost},
    {INPUT_TYPE_JOY_BUTTON, 0, 1, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_BoostRight},
    {INPUT_TYPE_JOY_BUTTON, 0, 2, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ViewChange},
    {INPUT_TYPE_JOY_BUTTON, 0, 5, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_GearUp},
    {INPUT_TYPE_JOY_BUTTON, 0, 4, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_GearDown},
    {INPUT_TYPE_JOY_BUTTON, 0, 3, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_MusicChange},
    {INPUT_TYPE_JOY_HAT, 0, 0, AXIS_MODE_DIGITAL, SDL_HAT_UP, false, PLAYER_1, LA_Up},
    {INPUT_TYPE_JOY_HAT, 0, 0, AXIS_MODE_DIGITAL, SDL_HAT_DOWN, false, PLAYER_1, LA_Down},
    {INPUT_TYPE_JOY_HAT, 0, 0, AXIS_MODE_DIGITAL, SDL_HAT_LEFT, false, PLAYER_1, LA_Left},
    {INPUT_TYPE_JOY_HAT, 0, 0, AXIS_MODE_DIGITAL, SDL_HAT_RIGHT, false, PLAYER_1, LA_Right},
    {INPUT_TYPE_JOY_BUTTON, 0, 12, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_CardInsert},
};

const size_t gDefaultDrivingBindingsSize = sizeof(gDefaultDrivingBindings) / sizeof(gDefaultDrivingBindings[0]);

const ControlBinding gDefaultAbcBindings[] = {
    // Keyboard
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_LEFT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ABC_Left},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_RIGHT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ABC_Right},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_UP, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ABC_Up},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_DOWN, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ABC_Down},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_A, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Throttle_Accelerate},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_Z, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Throttle_Slowdown},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_Q, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_GunTrigger},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_W, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_MissileTrigger},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_E, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ClimaxSwitch},

    // Controller
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_LEFTX, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_ABC_X},
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_LEFTY, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_ABC_Y},
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_RIGHTY, AXIS_MODE_FULL, 0, true, PLAYER_1, LA_Throttle},
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, AXIS_MODE_POSITIVE_HALF, 0, false, PLAYER_1, LA_Throttle},
    {INPUT_TYPE_GAMEPAD_AXIS, 0, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, AXIS_MODE_NEGATIVE_HALF, 0, false, PLAYER_1, LA_Throttle},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_SOUTH, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_GunTrigger},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_EAST, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_MissileTrigger},
    {INPUT_TYPE_GAMEPAD_BUTTON, 0, SDL_GAMEPAD_BUTTON_NORTH, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ClimaxSwitch},

    // Joystick
    {INPUT_TYPE_JOY_AXIS, 0, 0, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_ABC_X},
    {INPUT_TYPE_JOY_AXIS, 0, 1, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_ABC_Y},
    {INPUT_TYPE_JOY_AXIS, 0, 4, AXIS_MODE_FULL, 0, true, PLAYER_1, LA_Throttle},
    {INPUT_TYPE_JOY_AXIS, 0, 5, AXIS_MODE_POSITIVE_HALF, 0, false, PLAYER_1, LA_Throttle},
    {INPUT_TYPE_JOY_AXIS, 0, 4, AXIS_MODE_NEGATIVE_HALF, 0, false, PLAYER_1, LA_Throttle},
    {INPUT_TYPE_JOY_BUTTON, 0, 0, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_GunTrigger},
    {INPUT_TYPE_JOY_BUTTON, 0, 1, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_MissileTrigger},
    {INPUT_TYPE_JOY_BUTTON, 0, 4, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ClimaxSwitch},

};

const size_t gDefaultAbcBindingsSize = sizeof(gDefaultAbcBindings) / sizeof(gDefaultAbcBindings[0]);

const ControlBinding gDefaultShootingBindings[] = {
    // Mouse / Gun
    {INPUT_TYPE_MOUSE_AXIS, 0, 0, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_GunX},
    {INPUT_TYPE_MOUSE_AXIS, 0, 1, AXIS_MODE_FULL, 0, false, PLAYER_1, LA_GunY},
    {INPUT_TYPE_MOUSE_BUTTON, 0, SDL_BUTTON_LEFT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Trigger},
    {INPUT_TYPE_MOUSE_BUTTON, 0, SDL_BUTTON_RIGHT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Reload},
    {INPUT_TYPE_MOUSE_BUTTON, 0, SDL_BUTTON_MIDDLE, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_GunButton},

    // {INPUT_TYPE_MOUSE_AXIS, 0, 0, AXIS_MODE_FULL, 0, false, PLAYER_2, LA_GunX},
    // {INPUT_TYPE_MOUSE_AXIS, 0, 1, AXIS_MODE_FULL, 0, false, PLAYER_2, LA_GunY},
    // {INPUT_TYPE_MOUSE_BUTTON, 0, SDL_BUTTON_LEFT, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Trigger},
    // {INPUT_TYPE_MOUSE_BUTTON, 0, SDL_BUTTON_RIGHT, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Reload},
    // {INPUT_TYPE_MOUSE_BUTTON, 0, SDL_BUTTON_MIDDLE, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_GunButton},

    // Keyboard
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_Q, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Trigger},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_W, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_Reload},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_E, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_GunButton},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_R, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_ActionButton},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_LEFT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_PedalLeft},
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_RIGHT, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_PedalRight},
};

const size_t gDefaultShootingBindingsSize = sizeof(gDefaultShootingBindings) / sizeof(gDefaultShootingBindings[0]);

const ControlBinding gDefaultMahjongBindings[] = {
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_Y, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_A}, // A
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_U, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_B}, // B
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_I, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_C}, // C
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_O, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_D}, // D

    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_G, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_E}, // E
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_H, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_F}, // F
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_J, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_G}, // G
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_K, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_H}, // H

    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_L, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_I},     // I
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_V, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_J},     // J
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_B, AXIS_MODE_DIGITAL, 0, false, PLAYER_1, LA_K},     // K
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_N, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_L},     // L
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_M, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_M},     // M
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_COMMA, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_N}, // N

    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_F1, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Chi},        // CHI
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_F2, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Pon},        // PON
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_F3, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Kan},        // KAN
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_F4, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Reach},      // REACH
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_F5, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Agari},      // AGARI
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_F6, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_Cancel},     // CANCEL
    {INPUT_TYPE_KEY, 0, SDL_SCANCODE_F7, AXIS_MODE_DIGITAL, 0, false, PLAYER_2, LA_CardInsert}, // Cards In
};

const size_t gDefaultMahjongBindingsSize = sizeof(gDefaultMahjongBindings) / sizeof(gDefaultMahjongBindings[0]);

char *toUpperCase(const char *str)
{
    char *result = (char *)malloc(strlen(str) + 1);

    if (result == NULL)
        return NULL;

    int i = 0;
    while (str[i] != '\0')
    {
        result[i] = toupper((unsigned char)str[i]);
        i++;
    }
    result[i] = '\0';

    return result;
}

/**
 * @brief Helper function to convert a ControlBinding struct back into an INI string.
 * This is the reverse of parseSdlSource, used for generating a default controls.ini.
 * @param binding Pointer to the ControlBinding to convert.
 * @param buffer Character buffer to write the string into.
 * @param buffer_size The size of the character buffer.
 */
static void getBindingString(const ControlBinding *binding, char *buffer, size_t buffer_size)
{
    char temp_buffer[128] = {0};
    switch (binding->type)
    {
        case INPUT_TYPE_KEY:
            snprintf(temp_buffer, sizeof(temp_buffer), "KEY_%s", SDL_GetScancodeName(binding->sdlId));
            break;
        case INPUT_TYPE_MOUSE_BUTTON:
            if (binding->sdlId == SDL_BUTTON_LEFT)
                strcpy(temp_buffer, "MOUSE_LEFT_BUTTON");
            else if (binding->sdlId == SDL_BUTTON_RIGHT)
                strcpy(temp_buffer, "MOUSE_RIGHT_BUTTON");
            else if (binding->sdlId == SDL_BUTTON_MIDDLE)
                strcpy(temp_buffer, "MOUSE_MIDDLE_BUTTON");
            else
                snprintf(temp_buffer, sizeof(temp_buffer), "MOUSE_BUTTON_%d", binding->sdlId);
            break;
        case INPUT_TYPE_MOUSE_AXIS:
            snprintf(temp_buffer, sizeof(temp_buffer), "MOUSE_AXIS_%c", binding->sdlId == 0 ? 'X' : 'Y');
            break;
        case INPUT_TYPE_JOY_BUTTON:
            snprintf(temp_buffer, sizeof(temp_buffer), "JOY%d_BUTTON_%d", binding->deviceIndex, binding->sdlId);
            break;
        case INPUT_TYPE_JOY_HAT:
            if (binding->axisThreshold == SDL_HAT_UP)
                snprintf(temp_buffer, sizeof(temp_buffer), "JOY%d_HAT%d_UP", binding->deviceIndex, binding->sdlId);
            else if (binding->axisThreshold == SDL_HAT_DOWN)
                snprintf(temp_buffer, sizeof(temp_buffer), "JOY%d_HAT%d_DOWN", binding->deviceIndex, binding->sdlId);
            else if (binding->axisThreshold == SDL_HAT_LEFT)
                snprintf(temp_buffer, sizeof(temp_buffer), "JOY%d_HAT%d_LEFT", binding->deviceIndex, binding->sdlId);
            else if (binding->axisThreshold == SDL_HAT_RIGHT)
                snprintf(temp_buffer, sizeof(temp_buffer), "JOY%d_HAT%d_RIGHT", binding->deviceIndex, binding->sdlId);
            break;
        case INPUT_TYPE_GAMEPAD_BUTTON:
            snprintf(temp_buffer, sizeof(temp_buffer), "GC%d_BUTTON_%s", binding->deviceIndex,
                     toUpperCase(SDL_GetGamepadStringForButton(binding->sdlId)));
            break;
        case INPUT_TYPE_JOY_AXIS:
        case INPUT_TYPE_GAMEPAD_AXIS:
        {
            char prefix[64];
            if (binding->type == INPUT_TYPE_JOY_AXIS)
                snprintf(prefix, sizeof(prefix), "JOY%d_AXIS_%d", binding->deviceIndex, binding->sdlId);
            else
                snprintf(prefix, sizeof(prefix), "GC%d_AXIS_%s", binding->deviceIndex,
                         toUpperCase(SDL_GetGamepadStringForAxis(binding->sdlId)));

            switch (binding->axisMode)
            {
                case AXIS_MODE_DIGITAL:
                    if (binding->axisThreshold > 0)
                        snprintf(temp_buffer, sizeof(temp_buffer), "%s_POSITIVE", prefix);
                    else
                        snprintf(temp_buffer, sizeof(temp_buffer), "%s_NEGATIVE", prefix);
                    break;
                case AXIS_MODE_POSITIVE_HALF:
                    snprintf(temp_buffer, sizeof(temp_buffer), "%s_POSITIVE_HALF", prefix);
                    break;
                case AXIS_MODE_NEGATIVE_HALF:
                    snprintf(temp_buffer, sizeof(temp_buffer), "%s_NEGATIVE_HALF", prefix);
                    break;
                case AXIS_MODE_FULL:
                default:
                    strcpy(temp_buffer, prefix);
                    break;
            }
            break;
        }
        default:
            break;
    }

    if (binding->isInverted)
    {
        strncat(temp_buffer, "_INVERTED", sizeof(temp_buffer) - strlen(temp_buffer) - 1);
    }
    strncpy(buffer, temp_buffer, buffer_size);
}

/**
 * @brief Helper to add a set of default bindings to an in-memory INI config.
 */
static void addBindingsToIni(IniConfig *cfg, const char *section_name, const ControlBinding *bindings, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        const ControlBinding *b = &bindings[i];
        char action_str[128];
        char binding_str[128];
        getLogicalActionString(b, action_str, sizeof(action_str), section_name);
        getBindingString(b, binding_str, sizeof(binding_str));

        IniSection *sec = iniGetSection(cfg, section_name);
        IniKeyValuePair *pair = NULL;
        if (sec)
        {
            for (int j = 0; j < sec->numPairs; j++)
            {
                if (strcmp(sec->pairs[j].key, action_str) == 0)
                {
                    pair = &sec->pairs[j];
                    break;
                }
            }
        }

        if (pair)
        { // Key already exists, append to it
            char new_value[1024];
            snprintf(new_value, sizeof(new_value), "%s, %s", pair->value, binding_str);
            iniSetValue(cfg, section_name, action_str, new_value);
        }
        else
        { // New key, just set it
            iniSetValue(cfg, section_name, action_str, binding_str);
        }
    }
}

/**
 * @brief Generates a default controls.ini file based on the hardcoded default binding arrays.
 */
void createDefaultControlsIni(const char *fileName)
{
    printf("Generating default controls.ini from built-in bindings...\n");

    IniConfig *ini = calloc(1, sizeof(IniConfig));
    if (!ini)
    {
        perror("Failed to allocate memory for INI config");
        return;
    }

    iniSetValue(ini, "Config", "Steer_DeadZone", "8000");
    iniSetValue(ini, "Config", "Gas_DeadZone", "500");
    iniSetValue(ini, "Config", "Brake_DeadZone", "500");
    iniSetValue(ini, "Config", "ABC_X_DeadZone", "8000");
    iniSetValue(ini, "Config", "ABC_Y_DeadZone", "8000");
    iniSetValue(ini, "Config", "Throttle_DeadZone", "8000");
    iniSetValue(ini, "Config", "ShakeIncreaseRate", "10.0");
    iniSetValue(ini, "Config", "ShakeDecayRate", "0.95");

    addBindingsToIni(ini, "Common", gDefaultCommonBindings, gDefaultCommonBindingsSize);
    addBindingsToIni(ini, "Digital", gDefaultDigitalBindings, gDefaultDigitalBindingsSize);
    addBindingsToIni(ini, "Driving", gDefaultDrivingBindings, gDefaultDrivingBindingsSize);
    addBindingsToIni(ini, "ABC", gDefaultAbcBindings, gDefaultAbcBindingsSize);
    addBindingsToIni(ini, "Shooting", gDefaultShootingBindings, gDefaultShootingBindingsSize);
    addBindingsToIni(ini, "Mahjong", gDefaultMahjongBindings, gDefaultMahjongBindingsSize);

    if (iniSave(ini, fileName) == 0)
    {
        printf("Default %s has been created successfully.\n", fileName);
    }
    else
    {
        fprintf(stderr, "ERROR: Failed to create %s.\n", fileName);
    }

    iniFree(ini);
}
