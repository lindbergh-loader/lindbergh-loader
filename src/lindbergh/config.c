#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "gpuvendor.h"
#include "log.h"

EmulatorConfig config = {0};

extern uint32_t elf_crc;

FILE *configFile = NULL;

#define CONFIG_PATH "lindbergh.conf"
#define MAX_LINE_LENGTH 1024

const char* LindbergColourStrings [] = {
    "Lindbergh Yellow",
    "Lindbergh Red",
    "Lindbergh Blue",
    "Lindbergh Silver",
    "Lindbergh RedEX"
};

const char* GameRegionStrings [] = {
    "Japan",
    "US",
    "Export"
};

const char* GpuTypeStrings [] = {
    "Auto Detection",
    "NVIDIA",
    "AMD",
    "ATI",
    "INTEL",
    "Unknown",
    "ERROR_GPU"
};

/**
 * Function to calculate CRC32 checksum in memory.
 */
uint32_t get_crc32(const char *s, ssize_t n)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < n; i++)
    {
        char ch = s[i];
        for (size_t j = 0; j < 8; j++)
        {
            uint32_t b = (ch ^ crc) & 1;
            crc >>= 1;
            if (b)
                crc = crc ^ 0xEDB88320;
            ch >>= 1;
        }
    }
    return ~crc;
}

static char *getNextToken(char *buffer, char *seperator, char **saveptr)
{
    char *token = strtok_r(buffer, seperator, saveptr);
    if (token == NULL)
        return NULL;

    for (int i = 0; i < (int)strlen(token); i++)
    {
        if ((token[i] == '\n') || (token[i] == '\r'))
        {
            token[i] = 0;
        }
    }
    return token;
}

static int detectGame(uint32_t elf_crc)
{

    switch (elf_crc)
    {
    case AFTER_BURNER_CLIMAX:
    {
        config.gameTitle = "After Burner Climax";
        config.gameDVP = "DVP-0009";
        config.gameID = "SBLR";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "640x480";
        config.gameStatus = WORKING;
        config.jvsIOType = SEGA_TYPE_1;
        config.gameType = ABC;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case AFTER_BURNER_CLIMAX_REVA:
    {
        config.gameTitle = "After Burner Climax Rev A";
        config.gameDVP = "DVP-0009A";
        config.gameID = "SBLR";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "640x480";
        config.gameStatus = WORKING;
        config.jvsIOType = SEGA_TYPE_1;
        config.gameType = ABC;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case AFTER_BURNER_CLIMAX_REVB:
    {
        config.gameTitle = "After Burner Climax Rev B";
        config.gameDVP = "DVP-0009B";
        config.gameID = "SBLR";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "640x480";
        config.gameStatus = WORKING;
        config.jvsIOType = SEGA_TYPE_1;
        config.gameType = ABC;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case AFTER_BURNER_CLIMAX_SDX:
    {
        config.gameTitle = "After Burner Climax SDX";
        config.gameDVP = "DVP-0018-SDX";
        config.gameID = "SBMN";
        config.gameReleaseYear = "2006 ?";
        config.gameNativeResolutions = "640x480";
        config.gameStatus = WORKING;
        config.jvsIOType = SEGA_TYPE_1;
        config.gameType = ABC;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case AFTER_BURNER_CLIMAX_SDX_REVA:
    {
        config.gameTitle = "After Burner Climax SDX Rev A";
        config.gameDVP = "DVP-0018A-SDX";
        config.gameID = "SBMN";
        config.gameReleaseYear = "2006 ?";
        config.gameNativeResolutions = "640x480";
        config.gameStatus = WORKING;
        config.jvsIOType = SEGA_TYPE_1;
        config.gameType = ABC;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case AFTER_BURNER_CLIMAX_SE:
    {
        config.gameTitle = "After Burner Climax SE";
        config.gameDVP = "DVP-0031";
        config.gameID = "SBLR";
        config.gameReleaseYear = "2006 ?";
        config.gameNativeResolutions = "";
        config.gameStatus = WORKING;
        config.jvsIOType = SEGA_TYPE_1;
        config.gameType = ABC;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case AFTER_BURNER_CLIMAX_SE_REVA:
    {
        config.gameTitle = "After Burner Climax SE Rev A";
        config.gameDVP = "DVP-0031A";
        config.gameID = "SBLR";
        config.gameReleaseYear = "2006 ?";
        config.gameNativeResolutions = "";
        config.gameStatus = WORKING;
        config.jvsIOType = SEGA_TYPE_1;
        config.gameType = ABC;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case GHOST_SQUAD_EVOLUTION:
    {
        config.gameTitle = "Ghost Squad Evolution";
        config.gameStatus = WORKING;
        config.gameDVP = "DVP-0029A";
        config.gameID = "SBNJ";
        config.gameReleaseYear = "2007";
        config.gameNativeResolutions = "640x480";
        config.gameLindberghColour = RED;
        config.gameType = SHOOTING;
        config.jvsIOType = SEGA_TYPE_1;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case HARLEY_DAVIDSON:
    {
        config.gameTitle = "Harley-Davidson: King of the Road";
        config.gameDVP = "DVP-5007";
        config.gameID = "SBRG";
        config.gameReleaseYear = "2009";
        config.gameNativeResolutions = "1360x768";
        config.gameLindberghColour = RED;
        config.gameType = HARLEY;
        config.gameStatus = WORKING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case HUMMER:
    {
        config.gameTitle = "Hummer";
        config.gameID = "SBQN";
        config.gameDVP = "DVP-0057B";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "1280x768";
        config.gameType = DRIVING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case HUMMER_EXTREME:
    {
        config.gameTitle = "Hummer Extreme";
        config.gameID = "SBST";
        config.gameDVP = "DVP-0079";
        config.gameReleaseYear = "2009";
        config.gameNativeResolutions = "";
        config.gameType = DRIVING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case HUMMER_EXTREME_MDX:
    {
        config.gameTitle = "Hummer Extreme MDX";
        config.gameID = "SBST";
        config.gameDVP = "DVP-0083";
        config.gameReleaseYear = "2009";
        config.gameNativeResolutions = "1280x768";
        config.gameType = DRIVING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case HUMMER_SDLX:
    {
        config.gameTitle = "Hummer SDLX";
        config.gameID = "SBST";
        config.gameDVP = "DVP-0057";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "";
        config.gameType = DRIVING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_4_EXP_REVB:
    {
        config.gameTitle = "Initial D Arcade Stage 4 Export Rev B";
        config.gameID = "SBNK";
        config.gameDVP = "DVP-0030B";
        config.gameReleaseYear = "2007";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_4_EXP_REVC:
    {
        config.gameTitle = "Initial D Arcade Stage 4 Export Rev C";
        config.gameID = "SBNK";
        config.gameDVP = "DVP-0030C";
        config.gameReleaseYear = "2007";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_4_EXP_REVD:
    {
        config.gameTitle = "Initial D Arcade Stage 4 Export Rev D";
        config.gameID = "SBNK";
        config.gameDVP = "DVP-0030D";
        config.gameReleaseYear = "2007";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_4_REVA:
    {
        config.gameTitle = "Initial D Arcade Stage 4 Rev A";
        config.gameID = "SBML";
        config.gameDVP = "DVP-0019A";
        config.gameReleaseYear = "2007";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_4_REVB:
    {
        config.gameTitle = "Initial D Arcade Stage 4 Rev B";
        config.gameID = "SBML";
        config.gameDVP = "DVP-0019B";
        config.gameReleaseYear = "2007";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_4_REVC:
    {
        config.gameTitle = "Initial D Arcade Stage 4 Rev C";
        config.gameID = "SBML";
        config.gameDVP = "DVP-0019C";
        config.gameReleaseYear = "2007";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_4_REVD:
    {
        config.gameTitle = "Initial D Arcade Stage 4 Rev D";
        config.gameID = "SBML";
        config.gameDVP = "DVP-0019D";
        config.gameReleaseYear = "2007";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_4_REVG:
    {
        config.gameTitle = "Initial D Arcade Stage 4 Rev G";
        config.gameID = "SBML";
        config.gameDVP = "DVP-0019G";
        config.gameReleaseYear = "2007";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_5_EXP:
    {
        config.gameTitle = "Initial D5 EXP";
        config.gameStatus = WORKING;
        config.gameDVP = "DVP-0075";
        config.gameID = "SBRY";
        config.gameReleaseYear = "2009";
        config.gameNativeResolutions = "1360x768";
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_5_EXP_20:
    {
        config.gameTitle = "Initial D5 EXP 2.0";
        config.gameStatus = WORKING;
        config.gameDVP = "DVP-0084";
        config.gameID = "SBTS";
        config.gameReleaseYear = "2009";
        config.gameNativeResolutions = "1360x768";
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_5_EXP_20A:
    {
        config.gameTitle = "Initial D5 5 EXP 2.0";
        config.gameDVP = "DVP-0084A";
        config.gameID = "SBQN";
        config.gameReleaseYear = "2009";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_5_JAP_REVA:
    {
        config.gameTitle = "Initial D Arcade Stage 5 Rev A";
        config.gameDVP = "DVP-0070A";
        config.gameID = "SBQZ";
        config.gameReleaseYear = "2009";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case INITIALD_5_JAP_REVF:
    {
        config.gameTitle = "Initial D Arcade Stage 5 Rev F";
        config.gameDVP = "DVP-0070F";
        config.gameID = "SBQZ";
        config.gameReleaseYear = "2009";
        config.gameNativeResolutions = "1360x768";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case LETS_GO_JUNGLE:
    {
        config.gameTitle = "Let's Go Jungle! Lost on the Island of Spice!";
        config.gameDVP = "DVP-0011";
        config.gameID = "SBLU";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "1360x768";
        config.gameType = SHOOTING;
        config.gameStatus = WORKING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case LETS_GO_JUNGLE_REVA:
    {
        config.gameTitle = "Let's Go Jungle! Lost on the Island of Spice! Rev A";
        config.gameDVP = "DVP-0011A";
        config.gameID = "SBLU";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "1360x768";
        config.gameType = SHOOTING;
        config.gameStatus = WORKING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case LETS_GO_JUNGLE_SPECIAL:
    {
        config.gameTitle = "Let's Go Jungle! Special!";
        config.emulateRideboard = 1;
        config.gameID = "SBNR";
        config.gameDVP = "DVP-0036";
        config.gameReleaseYear = "2007";
        config.gameNativeResolutions = "1024x768";
        config.gameType = SHOOTING;
        config.gameStatus = WORKING;
        config.width = 1024;
        config.height = 768;
        return 0;
    }
    break;

    case MJ4_EVO:
    {
        config.gameTitle = "SEGA Network Taisen Mahjong MJ4 Evolution";
        config.gameDVP = "DVP-0081";
        config.gameID = "SBTA";
        config.gameStatus = WORKING;
        config.gameType = MAHJONG;
        return 0;
    }
    break;

    case MJ4_REVG:
    {
        config.gameTitle = "SEGA Network Taisen Mahjong MJ4";
        config.gameDVP = "DVP-0049G";
        config.gameID = "SBPN";
        config.gameStatus = WORKING;
        config.gameType = MAHJONG;
        return 0;
    }
    break;

    case OUTRUN_2_SP_SDX:
    case OUTRUN_2_SP_SDX_TEST:
    {
        config.gameTitle = "Outrun 2 SP SDX";
        config.gameDVP = "DVP-0015";
        config.gameID = "SBMB";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "800x480";
        config.emulateDriveboard = 1;
        config.emulateMotionboard = 1;
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 800;
        config.height = 480;
        return 0;
    }
    break;

    case OUTRUN_2_SP_SDX_REVA:
    case OUTRUN_2_SP_SDX_REVA_TEST:
    case OUTRUN_2_SP_SDX_REVA_TEST2:
    {
        config.gameTitle = "Outrun 2 SP SDX Rev A";
        config.gameDVP = "DVP-0015A";
        config.gameID = "SBMB";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "800x480";
        config.emulateDriveboard = 1;
        config.emulateMotionboard = 1;
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 800;
        config.height = 480;
        return 0;
    }
    break;

    case PRIMEVAL_HUNT:
    {
        config.gameTitle = "Primeval Hunt";
        config.gameDVP = "DVP-0048A";
        config.gameID = "SBPP";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "640x480";
        config.gameLindberghColour = RED;
        config.gameStatus = WORKING;
        config.gameType = SHOOTING;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case RAMBO:
    {
        config.gameTitle = "Rambo";
        config.gameDVP = "DVP-0069";
        config.gameID = "SBQL";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "640x480, 1280x768";
        config.gameLindberghColour = REDEX;
        config.gameType = SHOOTING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case R_TUNED:
    {
        config.gameTitle = "R-Tuned: Ultimate Street Racing";
        config.gameDVP = "DVP-0060";
        config.gameID = "SBQW";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "640x480";
        config.emulateDriveboard = 1;
        config.emulateCardreader = 1;
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case SEGABOOT_2_4:
    {
        config.gameTitle = "Segaboot 2.4";
        config.gameStatus = WORKING;
        return 0;
    }
    break;

    case SEGABOOT_2_4_SYM:
    {
        config.gameTitle = "Segaboot 2.4 with symbols";
        config.gameStatus = WORKING;
        return 0;
    }
    break;

    case SEGA_RACE_TV:
    {
        config.gameTitle = "SEGA Race TV";
        config.emulateDriveboard = 1;
        config.gameDVP = "DVP-0044";
        config.gameID = "SBPF";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "640x480";
        config.gameStatus = WORKING;
        config.gameType = DRIVING;
        config.width = 640;
        config.height = 480;
        return 0;
    }
    break;

    case THE_HOUSE_OF_THE_DEAD_4_REVA:
    case THE_HOUSE_OF_THE_DEAD_4_REVA_TEST:
    {
        config.gameTitle = "The House of the Dead 4 Rev A";
        config.gameID = "SBLC";
        config.gameDVP = "DVP-0003A";
        config.gameReleaseYear = "2005";
        config.gameNativeResolutions = "640x480, 1280x768";
        config.gameType = SHOOTING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case THE_HOUSE_OF_THE_DEAD_4_REVB:
    case THE_HOUSE_OF_THE_DEAD_4_REVB_TEST:
    {
        config.gameTitle = "The House of the Dead 4 Rev B";
        config.gameID = "SBLC";
        config.gameDVP = "DVP-0003B";
        config.gameReleaseYear = "2005";
        config.gameNativeResolutions = "640x480, 1280x768";
        config.gameType = SHOOTING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case THE_HOUSE_OF_THE_DEAD_4_REVC:
    case THE_HOUSE_OF_THE_DEAD_4_REVC_TEST:
    {
        config.gameTitle = "The House of the Dead 4 Rev C";
        config.gameID = "SBLC";
        config.gameDVP = "DVP-0003C";
        config.gameReleaseYear = "2005";
        config.gameNativeResolutions = "640x480, 1280x768";
        config.gameType = SHOOTING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL:
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_TEST:
    {
        config.gameTitle = "The House of the Dead 4 Special";
        config.emulateRideboard = 1;
        config.gameID = "SBLS";
        config.gameDVP = "DVP-0010";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "1024x768";
        config.gameStatus = WORKING;
        config.gameType = SHOOTING;
        config.width = 1024;
        config.height = 768;
        return 0;
    }
    break;

    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB:
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB_TEST:
    {
        config.gameTitle = "The House of the Dead 4 Special Rev B";
        config.emulateRideboard = 1;
        config.gameID = "SBLS";
        config.gameDVP = "DVP-0010B";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "1024x768";
        config.gameStatus = WORKING;
        config.gameType = SHOOTING;
        config.width = 1024;
        config.height = 768;
        return 0;
    }
    break;

    case THE_HOUSE_OF_THE_DEAD_EX:
    case THE_HOUSE_OF_THE_DEAD_EX_TEST:
    {
        config.gameTitle = "Ai Sareruyori Ai Shitai: The House of the Dead EX";
        config.gameStatus = WORKING;
        config.gameType = SHOOTING;
        config.gameID = "SBRC";
        config.gameDVP = "DVP-0063";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "640x480, 1280x768";
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case TOO_SPICY:
    case TOO_SPICY_TEST:
    {
        config.gameTitle = "Too Spicy";
        config.gameDVP = "DVP-0027A";
        config.gameID = "SBMV";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "640x480, 1280x768";
        config.gameLindberghColour = RED;
        config.gameStatus = WORKING;
        config.gameType = SHOOTING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5:
    {
        config.gameTitle = "Virtua Fighter 5";
        config.gameDVP = "DVP-0008";
        config.gameID = "SBLM";
        config.gameReleaseYear = "2005";
        config.gameNativeResolutions = "1280x768";
        config.gameStatus = WORKING;
        config.gameType = FIGHTING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5_EXPORT:
    {
        config.gameTitle = "Virtua Fighter 5 Export";
        config.gameDVP = "DVP-0043";
        config.gameID = "SBLM";
        config.gameReleaseYear = "2005";
        config.gameNativeResolutions = "1280x768";
        config.gameStatus = WORKING;
        config.gameType = FIGHTING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA:
    {
        config.gameTitle = "Virtua Fighter 5 Final Showdown REV A";
        config.gameDVP = "DVP-5019A";
        config.gameID = "SBUV";
        config.gameReleaseYear = "2010";
        config.gameNativeResolutions = "1280x768";
        config.gameType = FIGHTING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB:
    {
        config.gameTitle = "Virtua Fighter 5 Final Showdown REV B";
        config.gameDVP = "DVP-5020";
        config.gameID = "SBXX";
        config.gameReleaseYear = "2010";
        config.gameNativeResolutions = "1280x768";
        config.gameType = FIGHTING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000:
    {
        config.gameTitle = "Virtua Fighter 5 Final Showdown REV B ver 6.0000";
        config.gameDVP = "DVP-5020";
        config.gameID = "SBXX";
        config.gameReleaseYear = "2010";
        config.gameNativeResolutions = "1280x768";
        config.gameType = FIGHTING;
        config.gameStatus = WORKING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5_R:
    {
        config.gameTitle = "Virtua Fighter 5 R";
        config.gameDVP = "DVP-XXXX";
        config.gameID = "SBQU";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "1280x768";
        config.gameStatus = WORKING;
        config.gameType = FIGHTING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5_R_REVD:
    {
        config.gameTitle = "Virtua Fighter 5 R Rev D";
        config.gameDVP = "DVP-XXXX";
        config.gameID = "SBQU";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "1280x768";
        config.gameStatus = WORKING;
        config.gameType = FIGHTING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5_R_REVG:
    {
        config.gameTitle = "Virtua Fighter 5 R Rev G";
        config.gameDVP = "DVP-XXXX";
        config.gameID = "SBQU";
        config.gameReleaseYear = "2008";
        config.gameNativeResolutions = "1280x768";
        config.gameStatus = WORKING;
        config.gameType = FIGHTING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5_REVA:
    {
        config.gameTitle = "Virtua Fighter 5 Rev A";
        config.gameDVP = "DVP-0008A";
        config.gameID = "SBLM";
        config.gameReleaseYear = "2005";
        config.gameNativeResolutions = "1280x768";
        config.gameStatus = WORKING;
        config.gameType = FIGHTING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5_REVB:
    {
        config.gameTitle = "Virtua Fighter 5 Rev B";
        config.gameDVP = "DVP-0008B";
        config.gameID = "SBLM";
        config.gameReleaseYear = "2005";
        config.gameNativeResolutions = "1280x768";
        config.gameStatus = WORKING;
        config.gameType = FIGHTING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_FIGHTER_5_REVE:
    {
        config.gameTitle = "Virtua Fighter 5 Rev E (Public Version C)";
        config.gameDVP = "DVP-0008E";
        config.gameID = "SBLM";
        config.gameReleaseYear = "2005";
        config.gameNativeResolutions = "1280x768";
        config.gameStatus = WORKING;
        config.gameType = FIGHTING;
        config.width = 1280;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_TENNIS_3:
    case VIRTUA_TENNIS_3_TEST:
    {
        config.gameTitle = "Virtua Tennis 3";
        config.gameDVP = "DVP-0005";
        config.gameID = "SBKX";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "640x480, 1360x768";
        config.emulateCardreader = 1;
        config.gameStatus = WORKING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_TENNIS_3_REVA:
    case VIRTUA_TENNIS_3_REVA_TEST:
    {
        config.gameTitle = "Virtua Tennis 3 Rev A";
        config.gameDVP = "DVP-0005A";
        config.gameID = "SBKX";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "640x480, 1360x768";
        config.emulateCardreader = 1;
        config.gameType = FIGHTING;
        config.gameStatus = WORKING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_TENNIS_3_REVB:
    case VIRTUA_TENNIS_3_REVB_TEST:
    {
        config.gameTitle = "Virtua Tennis 3 Rev B";
        config.gameDVP = "DVP-0005B";
        config.gameID = "SBKX";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "640x480, 1360x768";
        config.emulateCardreader = 1;
        config.gameType = FIGHTING;
        config.gameStatus = WORKING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;

    case VIRTUA_TENNIS_3_REVC:
    case VIRTUA_TENNIS_3_REVC_TEST:
    {
        config.gameTitle = "Virtua Tennis 3 Rev C";
        config.gameDVP = "DVP-0005C";
        config.gameID = "SBKX";
        config.gameReleaseYear = "2006";
        config.gameNativeResolutions = "640x480, 1360x768";
        config.emulateCardreader = 1;
        config.gameType = FIGHTING;
        config.gameStatus = WORKING;
        config.width = 1360;
        config.height = 768;
        return 0;
    }
    break;
    default:
    {
        config.crc32 = UNKNOWN;
    }
    break;
    }

    return 1;
}

char *getGameName()
{
    return config.gameTitle;
}

char *getDVPName()
{
    return config.gameDVP;
}

char *getGameID()
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

void toLowerCase(char *str) {
    while (*str) {  // Iterate through the string until the null terminator
        *str = tolower((unsigned char)*str);  // Convert each character to lowercase
        str++;  // Move to the next character
    }
}

int getNextIntOrAuto(char *saveptr, int defaultValue) {
    char nextToken[256];
    strcpy(nextToken, getNextToken(NULL, " ", &saveptr));
    toLowerCase(nextToken);

    if (strcmp(nextToken, "auto") == 0)
        return defaultValue;

    return atoi(nextToken);
}

int readConfig(FILE *configFile, EmulatorConfig *config)
{
    char buffer[MAX_LINE_LENGTH];
    char *saveptr = NULL;

    while (fgets(buffer, MAX_LINE_LENGTH, configFile))
    {

        /* Check for comments */
        if (buffer[0] == '#' || buffer[0] == 0 || buffer[0] == ' ' || buffer[0] == '\r' || buffer[0] == '\n')
            continue;

        char *command = getNextToken(buffer, " ", &saveptr);

        if (strcmp(command, "WIDTH") == 0) {
            int defaultValue = config->width;
            config->width = getNextIntOrAuto(saveptr, defaultValue);
        }

        else if (strcmp(command, "HEIGHT") == 0) {
            int defaultValue = config->height;
            config->height = getNextIntOrAuto(saveptr, defaultValue);
        }

        else if (strcmp(command, "BOOST_RENDER_RES") == 0)
            config->boostRenderRes = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "EEPROM_PATH") == 0)
            strcpy(config->eepromPath, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "SRAM_PATH") == 0)
            strcpy(config->sramPath, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "LIBCG_PATH") == 0)
            strcpy(config->libCgPath, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "EMULATE_RIDEBOARD") == 0) {
            int defaultValue = config->emulateRideboard;
            config->emulateRideboard = getNextIntOrAuto(saveptr, defaultValue);
        }

        else if (strcmp(command, "EMULATE_DRIVEBOARD") == 0) {
            int defaultValue = config->emulateDriveboard;
            config->emulateDriveboard = getNextIntOrAuto(saveptr, defaultValue);
        }

        else if (strcmp(command, "EMULATE_MOTIONBOARD") == 0) {
            int defaultValue = config->emulateMotionboard;
            config->emulateMotionboard = getNextIntOrAuto(saveptr, defaultValue);
        }

        else if (strcmp(command, "EMULATE_CARDREADER") == 0)
        {
            int defaultValue = config->emulateCardreader;
            config->emulateCardreader = getNextIntOrAuto(saveptr, defaultValue);
        }

        else if (strcmp(command, "EMULATE_TOUCHSCREEN") == 0)
            config->emulateTouchscreen = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "CARDFILE_01") == 0)
            strcpy(config->cardFile1, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "CARDFILE_02") == 0)
            strcpy(config->cardFile2, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "FULLSCREEN") == 0)
            config->fullscreen = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "NO_SDL") == 0)
            config->noSDL = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "EMULATE_JVS") == 0)
            config->emulateJVS = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "JVS_PATH") == 0)
            strcpy(config->jvsPath, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "SERIAL_1_PATH") == 0)
            strcpy(config->serial1Path, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "SERIAL_2_PATH") == 0)
            strcpy(config->serial2Path, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "FREEPLAY") == 0)
            config->freeplay = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "LINDBERGH_COLOUR") == 0)
        {
            char colour[256];
            strcpy(colour, getNextToken(NULL, " ", &saveptr));

            // Convert colour to uppercase just to avoid case-sensitivity issues
            for (char *p = colour; *p; ++p) *p = toupper(*p);

            if (strcmp(colour, "RED") == 0)
                config->lindberghColour = RED;
            else if (strcmp(colour, "YELLOW") == 0)
                config->lindberghColour = YELLOW;
            else if (strcmp(colour, "BLUE") == 0)
                config->lindberghColour = BLUE;
            else if (strcmp(colour, "SILVER") == 0)
                config->lindberghColour = SILVER;
            else if (strcmp(colour, "REDEX") == 0)
                config->lindberghColour = REDEX;
            else
            {
                // Print a warning and keep the default colour
                log_warn("Unknown Lindbergh colour '%s'. Keeping default value.\n", colour);
            }
        }

        else if (strcmp(command, "REGION") == 0)
        {
            char region[256];
            strcpy(region, getNextToken(NULL, " ", &saveptr));
            if (strcmp(region, "JP") == 0)
                config->region = JP;
            else if (strcmp(region, "US") == 0)
                config->region = US;
            else if (strcmp(region, "EX") == 0)
                config->region = EX;
            else
            {
                // Print a warning and keep the default region
                log_warn("Unknown Region '%s'. Keeping default value.\n", region);
            }
        }

        else if (strcmp(command, "DEBUG_MSGS") == 0)
            config->showDebugMessages = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "HUMMER_FLICKER_FIX") == 0)
            config->hummerFlickerFix = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "KEEP_ASPECT_RATIO") == 0)
            config->keepAspectRatio = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "GPU_VENDOR") == 0)
            config->configGPUVendor = config->GPUVendor = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "OUTRUN_LENS_GLARE_ENABLED") == 0)
            config->outrunLensGlareEnabled = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "FPS_LIMITER_ENABLED") == 0)
            config->fpsLimiter = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "FPS_TARGET") == 0)
            config->fpsTarget = atof(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "LGJ_RENDER_WITH_MESA") == 0)
            config->lgjRenderWithMesa = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PRIMEVAL_HUNT_MODE") == 0)
            config->phMode = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "DISABLE_BUILTIN_FONT") == 0)
            config->disableBuiltinFont = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "DISABLE_BUILTIN_LOGOS") == 0)
            config->disableBuiltinLogos = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "HIDE_CURSOR") == 0)
            config->hideCursor = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "CUSTOM_CURSOR") == 0)
            strcpy(config->customCursor, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "CUSTOM_CURSOR_WIDTH") == 0)
            config->customCursorWidth = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "CUSTOM_CURSOR_HEIGHT") == 0)
            config->customCursorHeight = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PH_TOUCH_CURSOR") == 0)
            strcpy(config->phTouchCursor, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PH_TOUCH_CURSOR_WIDTH") == 0)
            config->phTouchCursorWidth = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PH_TOUCH_CURSOR_HEIGHT") == 0)
            config->phTouchCursorHeight = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "MJ4_ENABLED_ALL_THE_TIME") == 0)
            config->mj4EnabledAtT = atoi(getNextToken(NULL, " ", &saveptr));
 
        else if (strcmp(command, "OR2_IP") == 0)
            strcpy(config->or2IP, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "OR2_NETMASK") == 0)
            strcpy(config->or2Netmask, getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "CPU_FREQ_GHZ") == 0)
            config->cpuFreqGhz = atof(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "TEST_KEY") == 0)
            config->keymap.test = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_START_KEY") == 0)
            config->keymap.player1.start = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_SERVICE_KEY") == 0)
            config->keymap.player1.service = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_COIN_KEY") == 0)
            config->keymap.player1.coin = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_UP_KEY") == 0)
            config->keymap.player1.up = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_DOWN_KEY") == 0)
            config->keymap.player1.down = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_LEFT_KEY") == 0)
            config->keymap.player1.left = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_RIGHT_KEY") == 0)
            config->keymap.player1.right = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_BUTTON_1_KEY") == 0)
            config->keymap.player1.button1 = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_BUTTON_2_KEY") == 0)
            config->keymap.player1.button2 = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_BUTTON_3_KEY") == 0)
            config->keymap.player1.button3 = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_BUTTON_4_KEY") == 0)
            config->keymap.player1.button4 = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_BUTTON_5_KEY") == 0)
            config->keymap.player1.button5 = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_BUTTON_6_KEY") == 0)
            config->keymap.player1.button6 = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_BUTTON_7_KEY") == 0)
            config->keymap.player1.button7 = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "PLAYER_1_BUTTON_8_KEY") == 0)
            config->keymap.player1.button8 = atoi(getNextToken(NULL, " ", &saveptr));

        // Evdev TEST button
        else if (strcmp(command, "TEST_BUTTON") == 0)
            strncpy(config->arcadeInputs.test, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);

        // Player 1 controls
        else if (strcmp(command, "PLAYER_1_BUTTON_START") == 0)
            strncpy(config->arcadeInputs.player1_button_start, getNextToken(NULL, " ", &saveptr),
                    INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_SERVICE") == 0)
            strncpy(config->arcadeInputs.player1_button_service, getNextToken(NULL, " ", &saveptr),
                    INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_UP") == 0)
            strncpy(config->arcadeInputs.player1_button_up, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_DOWN") == 0)
            strncpy(config->arcadeInputs.player1_button_down, getNextToken(NULL, " ", &saveptr),
                    INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_LEFT") == 0)
            strncpy(config->arcadeInputs.player1_button_left, getNextToken(NULL, " ", &saveptr),
                    INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_RIGHT") == 0)
            strncpy(config->arcadeInputs.player1_button_right, getNextToken(NULL, " ", &saveptr),
                    INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_1") == 0)
            strncpy(config->arcadeInputs.player1_button_1, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_2") == 0)
            strncpy(config->arcadeInputs.player1_button_2, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_3") == 0)
            strncpy(config->arcadeInputs.player1_button_3, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_4") == 0)
            strncpy(config->arcadeInputs.player1_button_4, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_5") == 0)
            strncpy(config->arcadeInputs.player1_button_5, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_6") == 0)
            strncpy(config->arcadeInputs.player1_button_6, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_7") == 0)
            strncpy(config->arcadeInputs.player1_button_7, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_8") == 0)
            strncpy(config->arcadeInputs.player1_button_8, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_9") == 0)
            strncpy(config->arcadeInputs.player1_button_9, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_BUTTON_10") == 0)
            strncpy(config->arcadeInputs.player1_button_10, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_1_COIN") == 0)
            strncpy(config->arcadeInputs.player1_coin, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);

        // Player 2 controls
        else if (strcmp(command, "PLAYER_2_BUTTON_START") == 0)
            strncpy(config->arcadeInputs.player2_button_start, getNextToken(NULL, " ", &saveptr),
                    INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_SERVICE") == 0)
            strncpy(config->arcadeInputs.player2_button_service, getNextToken(NULL, " ", &saveptr),
                    INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_UP") == 0)
            strncpy(config->arcadeInputs.player2_button_up, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_DOWN") == 0)
            strncpy(config->arcadeInputs.player2_button_down, getNextToken(NULL, " ", &saveptr),
                    INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_LEFT") == 0)
            strncpy(config->arcadeInputs.player2_button_left, getNextToken(NULL, " ", &saveptr),
                    INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_RIGHT") == 0)
            strncpy(config->arcadeInputs.player2_button_right, getNextToken(NULL, " ", &saveptr),
                    INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_1") == 0)
            strncpy(config->arcadeInputs.player2_button_1, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_2") == 0)
            strncpy(config->arcadeInputs.player2_button_2, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_3") == 0)
            strncpy(config->arcadeInputs.player2_button_3, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_4") == 0)
            strncpy(config->arcadeInputs.player2_button_4, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_5") == 0)
            strncpy(config->arcadeInputs.player2_button_5, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_6") == 0)
            strncpy(config->arcadeInputs.player2_button_6, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_7") == 0)
            strncpy(config->arcadeInputs.player2_button_7, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_8") == 0)
            strncpy(config->arcadeInputs.player2_button_8, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_9") == 0)
            strncpy(config->arcadeInputs.player2_button_9, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_BUTTON_10") == 0)
            strncpy(config->arcadeInputs.player2_button_10, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "PLAYER_2_COIN") == 0)
            strncpy(config->arcadeInputs.player2_coin, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);

        // Analogue inputs
        else if (strcmp(command, "ANALOGUE_1") == 0)
            strncpy(config->arcadeInputs.analogue_1, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "ANALOGUE_2") == 0)
            strncpy(config->arcadeInputs.analogue_2, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "ANALOGUE_3") == 0)
            strncpy(config->arcadeInputs.analogue_3, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "ANALOGUE_4") == 0)
            strncpy(config->arcadeInputs.analogue_4, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "ANALOGUE_5") == 0)
            strncpy(config->arcadeInputs.analogue_5, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "ANALOGUE_6") == 0)
            strncpy(config->arcadeInputs.analogue_6, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "ANALOGUE_7") == 0)
            strncpy(config->arcadeInputs.analogue_7, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);
        else if (strcmp(command, "ANALOGUE_8") == 0)
            strncpy(config->arcadeInputs.analogue_8, getNextToken(NULL, " ", &saveptr), INPUT_STRING_LENGTH - 1);

        // Analogue deadzone
        else if (strcmp(command, "ANALOGUE_DEADZONE_1") == 0) {
            config->arcadeInputs.analogue_deadzone_start[0] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_middle[0] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_end[0] = atoi(getNextToken(NULL, " ", &saveptr));
        } else if (strcmp(command, "ANALOGUE_DEADZONE_2") == 0) {
            config->arcadeInputs.analogue_deadzone_start[1] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_middle[1] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_end[1] = atoi(getNextToken(NULL, " ", &saveptr));
        } else if (strcmp(command, "ANALOGUE_DEADZONE_3") == 0) {
            config->arcadeInputs.analogue_deadzone_start[2] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_middle[2] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_end[2] = atoi(getNextToken(NULL, " ", &saveptr));
        } else if (strcmp(command, "ANALOGUE_DEADZONE_4") == 0) {
            config->arcadeInputs.analogue_deadzone_start[3] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_middle[3] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_end[3] = atoi(getNextToken(NULL, " ", &saveptr));
        } else if (strcmp(command, "ANALOGUE_DEADZONE_5") == 0) {
            config->arcadeInputs.analogue_deadzone_start[4] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_middle[4] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_end[4] = atoi(getNextToken(NULL, " ", &saveptr));
        } else if (strcmp(command, "ANALOGUE_DEADZONE_6") == 0) {
            config->arcadeInputs.analogue_deadzone_start[5] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_middle[5] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_end[5] = atoi(getNextToken(NULL, " ", &saveptr));
        } else if (strcmp(command, "ANALOGUE_DEADZONE_7") == 0) {
            config->arcadeInputs.analogue_deadzone_start[6] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_middle[6] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_end[6] = atoi(getNextToken(NULL, " ", &saveptr));
        } else if (strcmp(command, "ANALOGUE_DEADZONE_8") == 0) {
            config->arcadeInputs.analogue_deadzone_start[7] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_middle[7] = atoi(getNextToken(NULL, " ", &saveptr));
            config->arcadeInputs.analogue_deadzone_end[7] = atoi(getNextToken(NULL, " ", &saveptr));
        }

        else if (strcmp(command, "SKIP_OUTRUN_CABINET_CHECK") == 0)
            config->skipOutrunCabinetCheck = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "BORDER_ENABLED") == 0)
            config->borderEnabled = atoi(getNextToken(NULL, " ", &saveptr));

        else if (strcmp(command, "WHITE_BORDER_PERCENTAGE") == 0)
            config->whiteBorderPercentage = (float) atoi(getNextToken(NULL, " ", &saveptr)) / 100.0;

        else if (strcmp(command, "BLACK_BORDER_PERCENTAGE") == 0)
            config->blackBorderPercentage = (float) atoi(getNextToken(NULL, " ", &saveptr)) / 100.0;

        else if (strcmp(command, "INPUT_MODE") == 0)
            config->inputMode = atoi(getNextToken(NULL, " ", &saveptr));

        else
            printf("Error: Unknown settings command %s\n", command);
    }

    return 0;
}

KeyMapping getDefaultKeymap()
{
    KeyMapping defaultKeyMapping;
    defaultKeyMapping.test = 28;
    defaultKeyMapping.player1.start = 10;
    defaultKeyMapping.player1.service = 39;
    defaultKeyMapping.player1.coin = 14;
    defaultKeyMapping.player1.up = 111;
    defaultKeyMapping.player1.down = 116;
    defaultKeyMapping.player1.left = 113;
    defaultKeyMapping.player1.right = 114;
    defaultKeyMapping.player1.button1 = 24;
    defaultKeyMapping.player1.button2 = 25;
    defaultKeyMapping.player1.button3 = 26;
    defaultKeyMapping.player1.button4 = 27; // R
    defaultKeyMapping.player1.button5 = 40; // D
    defaultKeyMapping.player1.button6 = 41;
    defaultKeyMapping.player1.button7 = 42;
    defaultKeyMapping.player1.button8 = 43;
    defaultKeyMapping.player2.start = 60;
    defaultKeyMapping.player2.service = 61;
    defaultKeyMapping.player2.coin = 15;
    defaultKeyMapping.player2.up = 54;
    defaultKeyMapping.player2.down = 53;
    defaultKeyMapping.player2.left = 55;
    defaultKeyMapping.player2.right = 56;
    defaultKeyMapping.player2.button1 = 52;
    defaultKeyMapping.player2.button2 = 57;
    defaultKeyMapping.player2.button3 = 58;
    defaultKeyMapping.player2.button4 = 59; // ,
    defaultKeyMapping.player2.button5 = 37; // CTRL
    return defaultKeyMapping;
}

int initConfig(const char* configFilePath)
{
    config.emulateRideboard = 0;
    config.emulateDriveboard = 0;
    config.emulateMotionboard = 0;
    config.emulateCardreader = 0;
    config.emulateTouchscreen = 0;
    strcpy(config.cardFile1, "Card_01.crd");
    strcpy(config.cardFile2, "Card_02.crd");
    config.emulateJVS = 1;
    config.fullscreen = 0;
    config.lindberghColour = YELLOW;
    strcpy(config.eepromPath, "eeprom.bin");
    strcpy(config.sramPath, "sram.bin");
    strcpy(config.libCgPath, "");
    strcpy(config.jvsPath, "/dev/ttyUSB0");
    strcpy(config.serial1Path, "/dev/ttyS0");
    strcpy(config.serial2Path, "/dev/ttyS1");
    config.width = 640;
    config.height = 480;
    config.boostRenderRes = 1;
    config.region = EX;
    config.freeplay = -1;
    config.showDebugMessages = 0;
    config.hummerFlickerFix = 0;
    config.keepAspectRatio = 0;
    config.outrunLensGlareEnabled = 1;
    config.lgjRenderWithMesa = 1;
    config.gameTitle = "Unknown game";
    config.gameID = "XXXX";
    config.gameDVP = "DVP-XXXX";
    config.gameType = SHOOTING;
    config.gameLindberghColour = YELLOW;
    config.gameReleaseYear = "";
    config.gameNativeResolutions = "";
    config.keymap = getDefaultKeymap();
    config.jvsIOType = SEGA_TYPE_3;
    config.GPUVendor = AUTO_DETECT_GPU;
    config.fpsLimiter = 0;
    config.fpsTarget = 60.0f;
    config.noSDL = 0;
    config.phMode = 2;
    config.disableBuiltinFont = 0;
    config.disableBuiltinLogos = 0;
    config.hideCursor = 1;
    strcpy(config.customCursor, "");
    config.customCursorWidth = 32;
    config.customCursorHeight = 32;
    strcpy(config.phTouchCursor, "");
    config.phTouchCursorWidth = 32;
    config.phTouchCursorHeight = 32;
    config.mj4EnabledAtT = 0;

     strcpy(config.or2IP, "");
     strcpy(config.or2Netmask, "");
    config.cpuFreqGhz = 0.0f;
    memset(&config.arcadeInputs.analogue_deadzone_start, 0, sizeof(config.arcadeInputs.analogue_deadzone_start));
    memset(&config.arcadeInputs.analogue_deadzone_middle, 0, sizeof(config.arcadeInputs.analogue_deadzone_middle));
    memset(&config.arcadeInputs.analogue_deadzone_end, 0, sizeof(config.arcadeInputs.analogue_deadzone_end));
    config.crc32 = elf_crc;
    config.skipOutrunCabinetCheck = 0;
    config.borderEnabled = 0;
    config.whiteBorderPercentage = 0.02;
    config.blackBorderPercentage = 0.0;
    if (detectGame(config.crc32) != 0)
    {
        log_warn("Unsure what game with CRC 0x%X is. Please submit this new game to the GitHub repository: "
               "https://github.com/lindbergh-loader/lindbergh-loader/issues/"
               "new?title=Please+add+new+game+0x%X&body=I+tried+to+launch+the+following+game:\n",
               config.crc32, config.crc32);
    }

    config.inputMode = 0; // Default to all inputs

    char filePath[PATH_MAX];
    strncpy(filePath, CONFIG_PATH, PATH_MAX);
    if (configFilePath != NULL && configFilePath[0] != '\0')
    {
        strncpy(filePath, configFilePath, PATH_MAX);
    }

    configFile = fopen(filePath, "r");

    if (configFile == NULL)
    {
        log_warn("Cannot open %s, using default values.\n", filePath);
        return 1;
    }

    readConfig(configFile, &config);

    fclose(configFile);

    return 0;
}

EmulatorConfig *getConfig()
{
    return &config;
}
