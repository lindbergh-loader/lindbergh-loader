#pragma once

#include <stdint.h>
#include <sys/types.h>

#define MAX_PATH_LENGTH 1024
#define INPUT_STRING_LENGTH 256

#define AFTER_BURNER_CLIMAX 0xa3c246e2                       // DVP-0009
#define AFTER_BURNER_CLIMAX_REVA 0xe657b1c0                  // DVP-0009A
#define AFTER_BURNER_CLIMAX_REVB 0xdb9d8396                  // DVP-0009B
#define AFTER_BURNER_CLIMAX_SDX 0xc042a0a2                   // DVP-0018
#define AFTER_BURNER_CLIMAX_SDX_REVA 0xcb768831              // DVP-0018A
#define AFTER_BURNER_CLIMAX_SE 0x86df4e64                    // DVP-0031
#define AFTER_BURNER_CLIMAX_SE_REVA 0x684352f4               // DVP-0031A
#define GHOST_SQUAD_EVOLUTION 0x455edaec                     // DVP-0029A
#define HARLEY_DAVIDSON 0xb1dd1c12                           // DVP-5007
#define HUMMER 0x4e6d5c64                                    // DVP-0057B
#define HUMMER_SDLX 0xf3778f44                               // DVP-0057
#define HUMMER_EXTREME 0x7129c32b                            // DVP-0079
#define HUMMER_EXTREME_MDX 0xfeaf8484                        // DVP-0084
#define INITIALD_4_REVA 0x361d1cbe                           // DVP-0019A
#define INITIALD_4_REVB 0x606914be                           // DVP-0019B
#define INITIALD_4_REVC 0x50f1c269                           // DVP-0019C
#define INITIALD_4_REVD 0xba380f8a                           // DVP-0019D
#define INITIALD_4_REVG 0x6cb19701                           // DVP-0019G
#define INITIALD_4_EXP_REVB 0xd0c64f3                        // DVP-0030B
#define INITIALD_4_EXP_REVC 0x65ea22e3                       // DVP-0030C
#define INITIALD_4_EXP_REVD 0x62cc73a9                       // DVP-0030D
#define INITIALD_5_JAP_REVA 0xb3183112                       // DVP-0070A
#define INITIALD_5_JAP_REVF 0xfc3dc85d                       // DVP-0070F
#define INITIALD_5_EXP 0x701b88cf                            // DVP-0075
#define INITIALD_5_EXP_20 0x77c6b58b                         // DVP-0084
#define INITIALD_5_EXP_20A 0xd4910e75                        // DVP-0084A
#define LETS_GO_JUNGLE 0xb1c8c901                            // DVP-0011
#define LETS_GO_JUNGLE_REVA 0xc697c4fb                       // DVP-0011A
#define LETS_GO_JUNGLE_SPECIAL 0x240beedc                    // DVP-0036A
#define MJ4_REVG 0x57813a7                                   // DVP-0049G
#define MJ4_EVO 0xb785a0e9                                   // DVP-0081
#define OUTRUN_2_SP_SDX 0x92c196d5                           // DVP-0015
#define OUTRUN_2_SP_SDX_REVA 0x4debd5f0                      // DVP-0015A
#define OUTRUN_2_SP_SDX_REVA_TEST 0x6b2d5c46                 // DVP-0015A
#define OUTRUN_2_SP_SDX_REVA_TEST2 0x4ee20716                // DVP-0015A
#define OUTRUN_2_SP_SDX_TEST 0xf1c89eae                      // DVP-0015
#define PRIMEVAL_HUNT 0x6868215c                             // DVP-0048A
#define RAMBO 0x48f49dd                                      // DVP-0069
#define RAMBO_CHINA 0x0025                                   // DVP-????
#define R_TUNED 0xa68d053d                                   // DVP-0060
#define SEGABOOT 0x0027                                      // DVP-????
#define SEGABOOT_2_4 0x38d56318                              // DVP-????
#define SEGABOOT_2_4_SYM 0xa518b18b                          // DVP-????
#define SEGABOOT_2_6 0x0030                                  // DVP-????
#define SEGA_RACE_TV 0xb30ab16a                              // DVP-0044
#define THE_HOUSE_OF_THE_DEAD_4_REVA 0x226281ad              // DVP-0003A
#define THE_HOUSE_OF_THE_DEAD_4_REVA_TEST 0x1d1160aa         // DVP-0003A
#define THE_HOUSE_OF_THE_DEAD_4_REVB 0xad652b5               // DVP-0003B
#define THE_HOUSE_OF_THE_DEAD_4_REVB_TEST 0xbb24fef8         // DVP-0003B
#define THE_HOUSE_OF_THE_DEAD_4_REVC 0xe213414d              // DVP-0003C
#define THE_HOUSE_OF_THE_DEAD_4_REVC_TEST 0x6210582b         // DVP-0003C
#define THE_HOUSE_OF_THE_DEAD_4_SPECIAL 0x8a7c4ac7           // DVP-0010
#define THE_HOUSE_OF_THE_DEAD_4_SPECIAL_TEST 0xb896a44a      // DVP-0010
#define THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB 0x7d6ab3e3      // DVP-0010B
#define THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB_TEST 0xae47a9fc // DVP-0010B
#define THE_HOUSE_OF_THE_DEAD_EX 0x20115a92                  // DVP-0063
#define THE_HOUSE_OF_THE_DEAD_EX_TEST 0xd58a0053             // DVP-0063
#define TOO_SPICY 0x46bb306e                                 // DVP-0027A
#define TOO_SPICY_TEST 0x6510b2e6                            // DVP-0027A
#define UNKNOWN 0xFFFFFFFF                                   // ????????
#define VIRTUA_FIGHTER_5 0x71722584                          // DVP-0008
#define VIRTUA_FIGHTER_5_REVA 0x9745abb6                     // DVP-0008A
#define VIRTUA_FIGHTER_5_REVB 0x8953bd52                     // DVP-0008B
#define VIRTUA_FIGHTER_5_REVE 0x4c2edbf6                     // DVP-0008E
#define VIRTUA_FIGHTER_5_EXPORT 0xec474630                   // DVP-0043
#define VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA 0xbae2be62      // DVP-5019A
#define VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB 0x7cee1d81      // DVP-5020
#define VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000 0x34c0d02  // DVP-5020 ver 6.00 (Weird public version)
#define VIRTUA_FIGHTER_5_R 0x79db39d                         // DVP-5004
#define VIRTUA_FIGHTER_5_R_REVD 0x443b6d07                   // DVP-5004D
#define VIRTUA_FIGHTER_5_R_REVG 0x4702ae73                   // DVP-5004G
#define VIRTUA_TENNIS_3 0x7a021b5                            // DVP-0005
#define VIRTUA_TENNIS_3_TEST 0x3bfbd11e                      // DVP-0005
#define VIRTUA_TENNIS_3_REVA 0xa9a10e32                      // DVP-0005A
#define VIRTUA_TENNIS_3_REVA_TEST 0xdbcf31c1                 // DVP-0005A
#define VIRTUA_TENNIS_3_REVB 0x67776c01                      // DVP-0005B
#define VIRTUA_TENNIS_3_REVB_TEST 0xc689beeb                 // DVP-0005B
#define VIRTUA_TENNIS_3_REVC 0x55345ed0                      // DVP-0005C
#define VIRTUA_TENNIS_3_REVC_TEST 0xdd7c4fea                 // DVP-0005C

typedef enum
{
    YELLOW,
    RED,
    BLUE,
    SILVER,
    REDEX
} Colour;

typedef enum
{
    WORKING,
    NOT_WORKING
} GameStatus;

typedef enum
{
    JP = 0,
    US = 1,
    EX = 2
} GameRegion;

typedef enum
{
    SHOOTING,
    DRIVING,
    HARLEY,
    FIGHTING,
    ABC,
    MAHJONG
} GameType;

typedef enum
{
    AUTO_DETECT_GPU,
    NVIDIA_GPU,
    AMD_GPU,
    ATI_GPU,
    INTEL_GPU,
    UNKNOWN_GPU,
    ERROR_GPU
} GpuType;

typedef struct
{
    unsigned int service;
    unsigned int start;
    unsigned int coin;
    unsigned int up;
    unsigned int down;
    unsigned int left;
    unsigned int right;
    unsigned int button1;
    unsigned int button2;
    unsigned int button3;
    unsigned int button4;
    unsigned int button5;
    unsigned int button6;
    unsigned int button7;
    unsigned int button8;
} PlayerKeyMapping;

typedef struct
{
    unsigned int test;
    PlayerKeyMapping player1;
    PlayerKeyMapping player2;
} KeyMapping;

typedef enum
{
    SEGA_TYPE_1,
    SEGA_TYPE_3
} JVSIOType;

typedef struct {
    // Test button
    char test[INPUT_STRING_LENGTH];

    // Player 1 controls
    char player1_button_start[INPUT_STRING_LENGTH];
    char player1_button_service[INPUT_STRING_LENGTH];
    char player1_button_up[INPUT_STRING_LENGTH];
    char player1_button_down[INPUT_STRING_LENGTH];
    char player1_button_left[INPUT_STRING_LENGTH];
    char player1_button_right[INPUT_STRING_LENGTH];
    char player1_button_1[INPUT_STRING_LENGTH];
    char player1_button_2[INPUT_STRING_LENGTH];
    char player1_button_3[INPUT_STRING_LENGTH];
    char player1_button_4[INPUT_STRING_LENGTH];
    char player1_button_5[INPUT_STRING_LENGTH];
    char player1_button_6[INPUT_STRING_LENGTH];
    char player1_button_7[INPUT_STRING_LENGTH];
    char player1_button_8[INPUT_STRING_LENGTH];
    char player1_button_9[INPUT_STRING_LENGTH];
    char player1_button_10[INPUT_STRING_LENGTH];
    char player1_coin[INPUT_STRING_LENGTH];

    // Player 2 controls
    char player2_button_start[INPUT_STRING_LENGTH];
    char player2_button_service[INPUT_STRING_LENGTH];
    char player2_button_up[INPUT_STRING_LENGTH];
    char player2_button_down[INPUT_STRING_LENGTH];
    char player2_button_left[INPUT_STRING_LENGTH];
    char player2_button_right[INPUT_STRING_LENGTH];
    char player2_button_1[INPUT_STRING_LENGTH];
    char player2_button_2[INPUT_STRING_LENGTH];
    char player2_button_3[INPUT_STRING_LENGTH];
    char player2_button_4[INPUT_STRING_LENGTH];
    char player2_button_5[INPUT_STRING_LENGTH];
    char player2_button_6[INPUT_STRING_LENGTH];
    char player2_button_7[INPUT_STRING_LENGTH];
    char player2_button_8[INPUT_STRING_LENGTH];
    char player2_button_9[INPUT_STRING_LENGTH];
    char player2_button_10[INPUT_STRING_LENGTH];
    char player2_coin[INPUT_STRING_LENGTH];

    // Analogue inputs
    char analogue_1[INPUT_STRING_LENGTH];
    char analogue_2[INPUT_STRING_LENGTH];
    char analogue_3[INPUT_STRING_LENGTH];
    char analogue_4[INPUT_STRING_LENGTH];
    char analogue_5[INPUT_STRING_LENGTH];
    char analogue_6[INPUT_STRING_LENGTH];
    char analogue_7[INPUT_STRING_LENGTH];
    char analogue_8[INPUT_STRING_LENGTH];

    int analogue_deadzone_start[8];
    int analogue_deadzone_middle[8];
    int analogue_deadzone_end[8];
} ArcadeInputs;


typedef struct
{
    int emulateRideboard;
    int emulateDriveboard;
    int emulateMotionboard;
    int emulateCardreader;
    int emulateTouchscreen;
    char cardFile1[MAX_PATH_LENGTH];
    char cardFile2[MAX_PATH_LENGTH];
    int emulateJVS;
    int fullscreen;
    char eepromPath[MAX_PATH_LENGTH];
    char sramPath[MAX_PATH_LENGTH];
    char libCgPath[MAX_PATH_LENGTH];
    char jvsPath[MAX_PATH_LENGTH];
    char serial1Path[MAX_PATH_LENGTH];
    char serial2Path[MAX_PATH_LENGTH];
    int width;
    int height;
    int boostRenderRes;
    Colour lindberghColour;
    GameStatus gameStatus;
    GameType gameType;
    KeyMapping keymap;
    uint32_t crc32;
    GameRegion region;
    int freeplay;
    int showDebugMessages;
    char *gameID;
    char *gameTitle;
    char *gameDVP;
    Colour gameLindberghColour;
    char  *gameReleaseYear;
    char  *gameNativeResolutions;
    GpuType GPUVendor;
    GpuType configGPUVendor;
    char *GPUVendorString;
    JVSIOType jvsIOType;
    int outrunLensGlareEnabled;
    int hummerFlickerFix;
    int keepAspectRatio;
    int fpsLimiter;
    float fpsTarget;
    int lgjRenderWithMesa;
    int noSDL;
    int phMode;
    int disableBuiltinFont;
    int disableBuiltinLogos;
    int hideCursor;
    char customCursor[MAX_PATH_LENGTH];
    int customCursorWidth;
    int customCursorHeight;
    char phTouchCursor[MAX_PATH_LENGTH];
    int phTouchCursorWidth;
    int phTouchCursorHeight;
    int mj4EnabledAtT;
    char or2IP[16];
    char or2Netmask[16];
    float cpuFreqGhz;
    ArcadeInputs arcadeInputs;
    int inputMode;  // 0 = both, 1 = SDL/X11 only, 2 = EVDEV only
    int skipOutrunCabinetCheck;
    float whiteBorderPercentage;
    float blackBorderPercentage;
    int borderEnabled;
} EmulatorConfig;

uint32_t get_crc32(const char *s, ssize_t n);
KeyMapping getDefaultKeymap();
int initConfig(const char* configFilePath);
EmulatorConfig *getConfig();
char *getGameName();
char *getDVPName();
char *getGameID();
int getGameLindberghColour();
char *getGameReleaseYear();
char *getGameNativeResolutions();
const char *getLindberghColourString(Colour lindberghColour);
const char *getGameRegionString(GameRegion region);
const char *getGpuTypeString(GpuType gpuType);

