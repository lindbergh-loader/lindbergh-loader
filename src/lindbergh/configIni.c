#include "configIni.h"
#include "config.h"
#include "log.h"
#include <stdio.h>

void createDefaultIni(const char *filePath)
{
    FILE *file = fopen(filePath, "w");
    if (!file)
    {
        log_error("Could not create default INI file at %s", filePath);
        return;
    }

    EmulatorConfig defaults;
    setDefaultValues(&defaults);

    fprintf(file, "# SEGA Lindbergh Emulator Configuration File\n");
    fprintf(file, "# By the Lindbergh Development Team 2024-2025\n\n");

    // [Display]
    fprintf(file, "[Display]\n");
    fprintf(file, "# Set the width resolution here\nWIDTH = AUTO\n\n");
    fprintf(file, "# Set the height resolution here\nHEIGHT = AUTO\n\n");
    fprintf(file, "# Boost render resolution in HOD4/2Spicy/Harley/Rambo/HOD-EX/ID4/ID5 and LGJ\n");
    fprintf(file, "BOOST_RENDER_RES = %s\n\n", defaults.boostRenderRes ? "true" : "false");
    fprintf(file, "# Set to true for full screen\nFULLSCREEN = %s\n\n", defaults.fullscreen ? "true" : "false");
    fprintf(file, "# Set to true if you\'d like to add a border for optical light gun tracking\n");
    fprintf(file, "BORDER_ENABLED = %s\n\n", defaults.borderEnabled ? "true" : "false");
    fprintf(file, "# Set the thickness of the white border as a percentage of the width of the screen\n");
    fprintf(file, "WHITE_BORDER_PERCENTAGE = %.0f\n\n", defaults.whiteBorderPercentage * 100);
    fprintf(file, "# Set the thickness of the black border which sits around the\n");
    fprintf(file, "# white border as a percentage of the width of the screen\n");
    fprintf(file, "BLACK_BORDER_PERCENTAGE = %.0f\n\n", defaults.blackBorderPercentage * 100);
    fprintf(file, "# Set to keep the aspect ratio in games like Sega Race TV Primeval Hunt and LGJ-SP\n");
    fprintf(file, "KEEP_ASPECT_RATIO = %s\n\n", defaults.keepAspectRatio ? "true" : "false");
    fprintf(file, "# Set to true to enable the mouse pointer/Cursor\nHIDE_CURSOR = %s\n\n", defaults.hideCursor ? "true" : "false");

    // [Input]
    fprintf(file, "[Input]\n");
    fprintf(file, "# Sets the Input Mode (1: SDL, 2: EVDEV\n");
    fprintf(file, "INPUT_MODE = %d\n\n", defaults.inputMode);

    // [Emulation]
    fprintf(file, "[Emulation]\n");
    fprintf(file, "# Set the Region (JP/US/EX)\nREGION = EX\n\n");
    fprintf(file, "# Set to true for Free Play, none to leave as default\nFREEPLAY = none\n\n");
    fprintf(file, "# Set to true to emulate JVS and use the keyboard/mouse for controls.\n");
    fprintf(file, "# If this is set to false, the loader will route the traffic to the serial device\n");
    fprintf(file, "# defined in JVS_PATH if it has been defined.\nEMULATE_JVS = %s\n\n", defaults.emulateJVS ? "true" : "false");
    fprintf(file, "# Set to true to emulate the rideboard used in the SPECIAL games\n");
    fprintf(file, "# If this is set to false, then the emulator will route the traffic to one of the serial ports\n");
    fprintf(file, "EMULATE_RIDEBOARD = AUTO\n\n");
    fprintf(file, "# Set to true to emulate the driveboard used in driving games\n");
    fprintf(file, "# If this is set to false, then the emulator will route the traffic to one of the serial ports\n");
    fprintf(file, "EMULATE_DRIVEBOARD = AUTO\n\n");
    fprintf(file, "# Set to true to emulate the motion board from Outrun 2 SP SDX\n");
    fprintf(file, "# If this is set to false, then the emulator will route the traffic to one of the serial ports\n");
    fprintf(file, "EMULATE_MOTIONBOARD = AUTO\n\n");
    fprintf(file, "# Set to true to enable card reader emulation in Virtua Tennis 3 or R-Tuned\nEMULATE_HW210_CARDREADER = AUTO\n\n");
    fprintf(file, "# Set to true to enable touchscreen emulation  with the mouse\nEMULATE_TOUCHSCREEN = AUTO\n\n");

    // [Cards]
    fprintf(file, "[Cards]\n");
    fprintf(file, "# Card File for reader 1 in VT3 or R-Tuned\nCARDFILE_01 = \"%s\"\n\n", defaults.cardFile1);
    fprintf(file, "# Card File for reader 2 in VT3 or R-Tuned\nCARDFILE_02 = \"%s\"\n\n", defaults.cardFile2);

    // [Paths]
    fprintf(file, "[Paths]\n");
    fprintf(file, "# Define the path to pass the JVS packets to if JVS is not being emulated\nJVS_PATH = \"%s\"\n\n", defaults.jvsPath);
    fprintf(file, "# Define the path to pass the first serial port to\nSERIAL_1_PATH = \"%s\"\n\n", defaults.serial1Path);
    fprintf(file, "# Define the path to pass the second serial port to\nSERIAL_2_PATH = \"%s\"\n\n", defaults.serial2Path);
    fprintf(file, "# Define the path to the sram.bin file\nSRAM_PATH = \"%s\"\n\n", defaults.sramPath);
    fprintf(file, "# Define the path to the eeprom.bin file\nEEPROM_PATH = \"%s\"\n\n", defaults.eepromPath);
    fprintf(file, "# If set, libCG.so needed for 2Spicy, Harley, Rambo and HOD-Ex shader recompilation\n");
    fprintf(file, "# will be loaded from the specified location. (include the name of file in the location ");
    fprintf(file, "For Example: /my/file/location/myLibCg.so)\nLIBCG_PATH = \"%s\"\n\n", defaults.libCgPath);

    // [Graphics]
    fprintf(file, "[Graphics]\n");
    fprintf(file, "# Set the GPU vendor (0 = Autodetect / 1 = NVidia / 2 = AMD / 3 = ATI / 4 = Intel)\n");
    fprintf(file, "GPU_VENDOR = %d\n\n", defaults.GPUVendor);
    fprintf(file, "# Set to true if you experience flickering in hummer\n");
    fprintf(file, "HUMMER_FLICKER_FIX = %s\n\n", defaults.hummerFlickerFix ? "true" : "false");
    fprintf(file, "# Set to false if you don't want to limit the FPS\n");
    fprintf(file, "FPS_LIMITER_ENABLED = %s\n\n", defaults.fpsLimiter ? "true" : "false");
    fprintf(file, "# Set the target FPS (will only work if FPS_LIMITER_ENABLED = 1)\nFPS_TARGET = %.1f\n\n", defaults.fpsTarget);
    fprintf(file, "# Set to true if you want to render LGJ using the mesa patches\n");
    fprintf(file, "LGJ_RENDER_WITH_MESA = %s\n\n", defaults.lgjRenderWithMesa ? "true" : "false");
    fprintf(file, "# Disable to use the original fonts instead of the built in font\n");
    fprintf(file, "DISABLE_BUILTIN_FONT = %s\n\n", defaults.disableBuiltinFont ? "true" : "false");
    fprintf(file, "# Disable to use the original logos instead of the built in logos\n");
    fprintf(file, "DISABLE_BUILTIN_LOGOS = %s\n\n", defaults.disableBuiltinLogos ? "true" : "false");

    // [Cursor]
    fprintf(file, "[Cursor]\n");
    fprintf(file, "# If true, a custom mouse cursor will be used loaded from a png file set in CUSTOM_CURSOR\n");
    fprintf(file, "# Ovewrites HIDE_CURSOR\n");
    fprintf(file, "CUSTOM_CURSOR_ENABLED = %s\n\n", defaults.customCursorEnabled ? "true" : "false");
    fprintf(file, "# Set the custom mouse pointer from a PNG file (Usefull for shooting games)\n");
    fprintf(file, "CUSTOM_CURSOR = \"%s\"\n\n", defaults.customCursor);
    fprintf(file, "# Set the width of the custom cursor\nCUSTOM_CURSOR_WIDTH = %d\n\n", defaults.customCursorWidth);
    fprintf(file, "# Set the height of the custom cursor\nCUSTOM_CURSOR_HEIGHT = %d\n\n", defaults.customCursorHeight);
    fprintf(file, "# Set a custom cursor for the touch screen in Primeval Hunt\n");
    fprintf(file, "TOUCH_CURSOR = \"%s\"\n\n", defaults.touchCursor);
    fprintf(file, "# Set the width of the custom cursor\nTOUCH_CURSOR_WIDTH = %d\n\n", defaults.touchCursorWidth);
    fprintf(file, "# Set the height of the custom cursor\nTOUCH_CURSOR_HEIGHT = %d\n\n", defaults.touchCursorHeight);

    // [GameSpecific]
    fprintf(file, "[GameSpecific]\n");
    fprintf(file, "# Set Primeval Hunt\n# Mode 0: Default (side by side)\n# Mode 1: No Touch screen\n# Mode 2: Side By Side\n");
    fprintf(file, "# Mode 3: 3ds mode 1 (Touch screen to the right)\n# Mode 4: 3ds mode 2 (Touch screen to the bottom)\n");
    fprintf(file, "PRIMEVAL_HUNT_SCREEN_MODE = %d\n\n", defaults.phScreenMode);
    fprintf(file, "# Set Primeval Hunt Test mode screen to single screen\n");
    fprintf(file, "PRIMEVAL_HUNT_TEST_SCREEN_SINGLE = %s\n\n", defaults.phTestScreenSingle ? "true" : "false");
    fprintf(file, "# Set to true to bypass cabinet checks including drive board and tower in Outrun 2 SP SDX\n");
    fprintf(file, "SKIP_OUTRUN_CABINET_CHECK = %s\n\n", defaults.skipOutrunCabinetCheck ? "true" : "false");
    fprintf(file, "# Set to false if you want to disable the Glare effect in OutRun\n");
    fprintf(file, "OUTRUN_LENS_GLARE_ENABLED = %s\n\n", defaults.outrunLensGlareEnabled ? "true" : "false");
    fprintf(file, "# Hacky way to make MJ4 and AxA work at prohibited times\nMJ4_ENABLED_ALL_THE_TIME = %s\n\n",
            defaults.mj4EnabledAtT ? "true" : "false");
    fprintf(file, "# House of the dead 4 speed fix, set the frequency of your CPU in Ghz\nCPU_FREQ_GHZ = %.1f\n\n", defaults.cpuFreqGhz);
    fprintf(file, "# Set to true if you want to chnge the way the guns are show in Rambo\n");
    fprintf(file, "RAMBO_GUNS_SWITCH = false\n\n");
    fprintf(file, "# Set to true to set the language in Chinese for ID5 DVP-0084 and DVP-0084A\n");
    fprintf(file, "ID5_CHINESE_LANGUAGE = %s\n\n", defaults.id5ChineseLanguage ? "true" : "false");
    fprintf(file, "# Set the percentage of the steering wheel travel reduction\n");
    fprintf(file, "ID_STEERING_REDUCTION_PERCENTAGE = %.1f\n\n", defaults.idSteeringPercentageReduction);

    // [System]
    fprintf(file, "[System]\n");
    fprintf(file, "# Set to true to see debug messages in the console\n");
    fprintf(file, "DEBUG_MSGS = %s\n\n", defaults.showDebugMessages ? "true" : "false");
    fprintf(file, "# Set the colour of the lindbergh (YELLOW, RED, BLUE, SILVER, REDEX)\nLINDBERGH_COLOUR = YELLOW\n\n");

    // [Network]
    fprintf(file, "[Network]\n");
    fprintf(file, "# If true, the loader will apply the following network patches depending on the game\n");
    fprintf(file, "ENABLE_NETWORK_PATCHES = %s\n\n", defaults.enableNetworkPatches ? "true" : "false");
    fprintf(file, "# Sets the name of the Network Interface Card\nNIC_NAME = \"enp0s1\"\n\n");
    fprintf(file, "# ID4 and ID5 network configuration per seat\nID_IP_SEAT_1 = \"192.168.1.2\"\nID_IP_SEAT_2 = \"192.168.1.3\"\n\n");
    fprintf(file, "# Sets the IP address and Netmask for Outrun link (you have to put your NIC ip)\n");
    fprintf(file, "OR2_IPADDRESS = \"192.168.1.2\"\nOR2_NETMASK = \"255.255.255.0\"\n\n");
    fprintf(file, "# Harley / Hummer and R-Tuned IP address for each Cabinet\n");
    fprintf(file, "IP_CAB1 = \"192.168.1.2\"\nIP_CAB2 = \"192.168.1.3\"\nIP_CAB3 = \"192.168.1.4\"\nIP_CAB4 = \"192.168.1.5\"\n\n");
    fprintf(file, "# Sets the IP address of each cabinet for network play in 2Spicy\n");
    fprintf(file, "2SPICY_IP_CAB1 = \"192.168.1.2\"\n2SPICY_IP_CAB2 = \"192.168.1.3\"\n\n");
    fprintf(file, "# Sets the IP address for SRTV\nSRTV_IPADDRESS = \"192.168.1.2\"\n\n");

    // [EVDEV]
    fprintf(file, "[EVDEV]\n");
    fprintf(file, "# EVDEV MODE (Input Mode 2)\n# To find the value pairs for these run ./lindbergh --list-controllers\n\n");
    fprintf(file, "#TEST_BUTTON = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_T\"\n\n");
    fprintf(file, "#PLAYER_1_COIN = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_5\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_START = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_1\n");
    fprintf(file, "#PLAYER_1_BUTTON_SERVICE = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_S\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_UP = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_UP\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_DOWN = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_DOWN\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_LEFT = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_LEFT\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_RIGHT = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_RIGHT\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_1 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_Q\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_2 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_W\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_3 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_E\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_4 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_R\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_5 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_Y\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_6 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_U\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_7 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_I\"\n");
    fprintf(file, "#PLAYER_1_BUTTON_8 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_O\"\n\n");
    fprintf(file, "#PLAYER_1_COIN = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_6\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_START = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_1\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_SERVICE = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_S\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_UP = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_UP\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_DOWN = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_DOWN\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_LEFT = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_LEFT\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_RIGHT = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_RIGHT\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_1 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_Q\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_2 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_W\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_3 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_E\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_4 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_R\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_5 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_Y\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_6 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_U\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_7 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_I\"\n");
    fprintf(file, "#PLAYER_2_BUTTON_8 = \"AT_TRANSLATED_SET_2_KEYBOARD_KEY_O\"\n\n");
    fprintf(file, "#ANALOGUE_1 = \"SYNPS_2_SYNAPTICS_TOUCHPAD_ABS_X\"\n");
    fprintf(file, "#ANALOGUE_2 = \"SYNPS_2_SYNAPTICS_TOUCHPAD_ABS_Y\"\n");
    fprintf(file, "#ANALOGUE_3 = \"SYNPS_2_SYNAPTICS_TOUCHPAD_ABS_Z\"\n");
    fprintf(file, "#ANALOGUE_4 = \"SYNPS_2_SYNAPTICS_TOUCHPAD_ABS_RZ\"\n");
    fprintf(file, "#ANALOGUE_5 = \"SYNPS_2_SYNAPTICS_TOUCHPAD_ABS_X2\"\n");
    fprintf(file, "#ANALOGUE_6 = \"SYNPS_2_SYNAPTICS_TOUCHPAD_ABS_Y2\"\n");
    fprintf(file, "#ANALOGUE_7 = \"SYNPS_2_SYNAPTICS_TOUCHPAD_ABS_Z2\"\n");
    fprintf(file, "#ANALOGUE_8 = \"SYNPS_2_SYNAPTICS_TOUCHPAD_ABS_RZ2\"\n\n");

    for (int x = 1; x < 9; x++)
        fprintf(file, "#ANALOGUE_DEADZONE_%d = 0 0 0\n", x);

    fprintf(file, "\n");
    fclose(file);
}
