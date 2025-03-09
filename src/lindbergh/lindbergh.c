#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "evdevinput.h"
#include "version.h"
#include "log.h"

#define LD_LIBRARY_PATH "LD_LIBRARY_PATH"
#define LD_PRELOAD "LD_PRELOAD"
#define PRELOAD_FILE_NAME "lindbergh.so"
#define PRELOAD_FILE_FLATPAK "/app/lib32/lindbergh.so"
#define TEAM "bobbydilley, retrofan, dkeruza-neo, doozer, francesco, rolel, caviar-x"
#define LINDBERGH_CONFIG_PATH "LINDBERGH_CONFIG_PATH"

uint32_t elf_crc = 0;

// List of all lindbergh executables known, not including the test executables
char *games[] = {"main.exe",
                 "ramboM.elf",
                 "vt3_Lindbergh",
                 "hummer_Master.elf",
                 "drive.elf",
                 "chopperM.elf",
                 "vsg",
                 "Jennifer",
                 "dsr",
                 "abc",
                 "hod4M.elf",
                 "lgj_final",
                 "vt3",
                 "id4.elf",
                 "id5.elf",
                 "lgjsp_app",
                 "gsevo",
                 "vf5",
                 "apacheM.elf",
                 "hodexRI.elf",
                  "a.elf",
                 "END"};

/**
 * An array containin clean games elf's CRC32
 */
uint32_t cleanElfCRC32[] = {
    0x51C4D2F6, // DVP-0003A | hod4M.elf
    0x1348BCA8, // DVP-0003A | hod4testM.elf
    0x0AAE384E, // DVP-0003B | hod4M.elf
    0x352AA797, // DVP-0003B | hod4testM.elf
    0x42EED61A, // DVP-0003C | hod4M.elf
    0x6DA6E511, // DVP-0003C | hod4testM.elf
    0x0E4BF4B1, // DVP-0005  | vt3_Lindbergh
    0x9E48AB5B, // DVP-0005  | vt3_testmode
    0xE4C64D01, // DVP-0005A | vt3_Lindbergh
    0x9C0E77E5, // DVP-0005A | vt3_testmode
    0xA4BDB9E2, // DVP-0005B | vt3_Lindbergh
    0x74E25472, // DVP-0005B | vt3_testmode
    0x987AE3FF, // DVP-0005C | vt3_Lindbergh
    0x1E4271A4, // DVP-0005C | vt3_testmode
    0xD409B70C, // DVP-0008  | vf5
    0x08EBC0DB, // DVP-0008A | vf5
    0xA47FBA2D, // DVP-0008B | vf5
    0x8CA46167, // DVP-0008C | vf5
    0x75946796, // DVP-0008E | vf5
    0x2C8F5D57, // DVP-0009  | abc
    0x13D90755, // DVP-0009A | abc
    0x633AD6FB, // DVP-0009B | abc
    0xD39825A8, // DVP-0010  | hod4M.elf
    0x0745CF0A, // DVP-0010  | hod4testM.elf
    0x13E59583, // DVP-0010B | hod4M.elf
    0x302FEB00, // DVP-0010B | hod4testM.elf
    0x04E08C99, // DVP-0011  | lgj_final
    0x0C3D3CC3, // DVP-0011A | lgj_final
    0xD9660B2E, // DVP-0015  | JenTest
    0x821C3404, // DVP-0015  | Jennifer
    0x13AF8581, // DVP-0015A | JenTest
    0xB2CE9B23, // DVP-0015A | Jennifer
    0xCC32DEAE, // DVP-0018  | abc
    0x17114BC1, // DVP-0018A | abc
    0x22905D60, // DVP-0019A | id4.elf
    0x43582D48, // DVP-0019B | id4.elf
    0x2D2A18C1, // DVP-0019C | id4.elf
    0x9BFD0D98, // DVP-0019D | id4.elf
    0x9CF9BBCC, // DVP-0019G | id4.elf
    0xFA0F6AB0, // DVP-0027A | apacheM.elf
    0x5A7F315E, // DVP-0027A | apachetestM.elf
    0x9D414D18, // DVP-0029A | vsg
    0xC345E213, // DVP-0030B | id4.elf
    0x98E6A516, // DVP-0030C | id4.elf
    0xF67365C9, // DVP-0030D | id4.elf
    0x8BDD31BA, // DVP-0031  | abc
    0x3DF37873, // DVP-0031A | abc
    0xDD8BB792, // DVP-0036A | lgjsp_app
    0xB0A96E34, // DVP-0043  | vf5
    0xF99E5635, // DVP-0044  | drive.elf
    0x4143F6B4, // DVP-0048A | main.exe
    0x653BC83B, // DVP-0057  | a.elf
    0x04D88552, // DVP-0057B | a.elf
    0x089D6051, // DVP-0060  | dsr
    0x317F3B90, // DVP-0063  | hodexRI.elf
    0x3A5EEC69, // DVP-0063  | hodextestR.elf
    0x81E02850, // DVP-0069  | ramboM.elf
    0xE4F202BB, // DVP-0070A | id5.elf
    0x2E6732A3, // DVP-0070F | id5.elf
    0xF99A3CDB, // DVP-0075  | id5.elf
    0x05647A8E, // DVP-0079  | hummer_Master.elf
    0x4442EA15, // DVP-0083  | hummer_Master.elf
    0x8DF6BBF9, // DVP-0084  | id5.elf
    0x2AF8004E, // DVP-0084A | id5.elf
    0xB95528F4, // DVP-5004  | vf5
    0x012E4898, // DVP-5004D | vf5
    0x74465F9F, // DVP-5004G | vf5
    0x75B48E22, // DVP-5007  | chopperM.elf
    0xFCB9D941, // DVP-5019A | vf5
    0xAB70901C, // DVP-5020  | vf5
    0x6BAA510D, // DVP-5020  | vf5 | Ver 6.000
};

int cleanElfCRC32Count = sizeof(cleanElfCRC32) / sizeof(uint32_t);

uint32_t calcCrc32(uint32_t crc, uint8_t data)
{
    crc ^= data; // No shift needed; working in LSB-first order
    for (int i = 0; i < 8; i++)
    {
        if (crc & 1)
        {
            crc = (crc >> 1) ^ 0xEDB88320;
        }
        else
        {
            crc = (crc >> 1);
        }
    }
    return crc;
}

int lookupCrcTable(uint32_t crc)
{
    for (int x = 0; x < cleanElfCRC32Count; x++)
    {
        if (cleanElfCRC32[x] == crc)
            return 1;
    }
    return 0;
}

void isCleanElf(char *command)
{
    const char *space_pos = strchr(command, ' ');
    size_t length = space_pos ? (size_t)(space_pos - command) : strlen(command);

    char elfName[256];
    strncpy(elfName, command, length);
    elfName[length] = '\0';

    FILE *file = fopen(elfName, "rb");
    if (!file)
    {
        log_error("Could not calculate the Elf CRC32.");
        return;
    }

    uint32_t crc = 0xFFFFFFFF;
    uint8_t buffer[4096];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        for (size_t i = 0; i < bytesRead; i++)
        {
            crc = calcCrc32(crc, buffer[i]);
        }
    }

    fclose(file);
    crc = crc ^ 0xFFFFFFFF;

    if (!lookupCrcTable(crc))
    {
        printf("\033[1;31m");
        printf("Warning: The ELF you are running is not Clean and might cause unwanted behavior.\n");
        printf("         Make sure you ELF and game dump are clean before reporting issues.\n");
        printf("         If you are sure the ELF is clean, please report it to us.\n");
        printf("\033[0m");
    }
}

/**
 * Tests if the game uses a seperate elf for test mode
 * and updates the command line to this elf.
 *
 * @param name The command path to run
 */
void testModePath(char *name)
{
    // Check if a different testmode elf is used
    if (strcmp(name, "./hod4M.elf") == 0)
    {
        strcpy(name, "./hod4testM.elf");
        return;
    }

    if (strcmp(name, "./hodexRI.elf") == 0)
    {
        strcpy(name, "./hodextestR.elf");
        return;
    }

    if (strcmp(name, "./Jennifer") == 0)
    {
        strcpy(name, "../JenTest/JenTest");
        return;
    }

    if (strcmp(name, "./apacheM.elf") == 0)
    {
        strcpy(name, "./apachetestM.elf");
        return;
    }

    if (strcmp(name, "./vt3_Lindbergh") == 0)
    {
        strcpy(name, "./vt3_testmode");
        return;
    }

    // Otherwise add the standard -t to the end
    strcat(name, " -t");
}

/**
 * Makes sure the environment variables are set correctly
 * to run the game.
 */
void setEnvironmentVariables(int isFlatpak, int zink, int nvidia)
{
    // Ensure the library path is set correctly
    char libraryPath[128] = {0};

    const char *currentLibraryPath = getenv(LD_LIBRARY_PATH);
    if (currentLibraryPath != NULL)
    {
        strcat(libraryPath, currentLibraryPath);
        strcat(libraryPath, ":");
    }

    strcat(libraryPath, ".:lib:../lib");

    setenv(LD_LIBRARY_PATH, libraryPath, 1);

    // Ensure the preload path is set correctly
    if (!isFlatpak)
        setenv(LD_PRELOAD, PRELOAD_FILE_NAME, 1);
    else
        setenv(LD_PRELOAD, PRELOAD_FILE_FLATPAK, 1);

    if (zink)
        setenv("MESA_LOADER_DRIVER_OVERRIDE", "zink", 1);

    if (nvidia)
    {
        setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
        setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);
    }
}

/**
 * Prints the usage for the loader
 */
void printUsage(char *argv[])
{
    printf("%s [GAME_PATH] [OPTIONS]\n", argv[0]);
    printf("Options:\n");
    printf("  --test | -t         Runs the test mode\n");
    printf("  --segaboot | -s     Runs segaboot\n");
    printf("  --zink | -z         Runs with Zink\n");
    printf("  --nvidia | -n       Runs with nVidia GPU when is as a secondary GPU in a laptop\n");
    printf("  --gdb               Runs with GDB\n");
    printf("  --list-controllers  Lists available controllers and inputs\n");
    printf("  --version           Displays the version of the loader and team's names\n");
    printf("  --help              Displays this usage text\n");
    printf("  --config | -c       Specifies configuration path\n");
}

/**
 * Lists available evdev controllers and their inputs
 */
int listControllers()
{
    Controllers controllers;

    ControllerStatus status = initControllers(&controllers);
    if (status != CONTROLLER_STATUS_SUCCESS)
    {
        printf("Failed to list controllers\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < controllers.count; i++)
    {
        Controller controller = controllers.controller[i];
        if (!controller.enabled)
            continue;

        printf("%s\n", controller.name);

        for (int i = 0; i < controller.inputCount; i++)
        {
            printf("  - %s\n", controller.inputs[i].inputName);
        }
    }
    
    stopControllers(&controllers);

    return EXIT_SUCCESS;
}

/**
 * Small utility to automatically detect the game and run it without
 * having to type a long string in.
 */
void printVersion() {
    printf("Lindbergh Loader v%d.%d\n", MAJOR_VERSION, MINOR_VERSION);
    printf("Created by: %s\n", TEAM);
}

/**
 * Small utility to automatically detect the game and run it without
 * having to type a long string in.
 */
int main(int argc, char *argv[])
{
    // Check for --version before directory operations
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        printVersion();
        return EXIT_SUCCESS;
    }

    // Look for the games
    struct dirent *ent;
    DIR *dir = opendir(".");

    if (dir == NULL)
    {
        log_error("Could not list files in current directory.");
        return EXIT_FAILURE;
    }

    char *game = NULL;

    int lindberghSharedObjectFound = 0;
    int isFlatpak = 0;

    while ((ent = readdir(dir)) != NULL)
    {

        if (ent->d_type != DT_REG)
            continue;

        if (strcmp(ent->d_name, PRELOAD_FILE_NAME) == 0)
        {
            lindberghSharedObjectFound = 1;
            continue;
        }

        int index = 0;
        while (game == NULL)
        {
            if (strcmp(games[index], "END") == 0)
                break;

            if (strcmp(ent->d_name, games[index]) == 0)
            {
                game = games[index];
                break;
            }

            index++;
        }

        if (game != NULL && lindberghSharedObjectFound)
            break;
    }

    closedir(dir);

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
    {
        printUsage(argv);
        return EXIT_SUCCESS;
    }

    if (argc > 1 && strcmp(argv[1], "--list-controllers") == 0)
    {
        return listControllers();
    }

    if (!lindberghSharedObjectFound)
    {
        FILE *file = fopen(PRELOAD_FILE_FLATPAK, "r");
        if (file)
        {
            fclose(file);
            isFlatpak = 1;
        }
        else
        {
            log_error("The preload object lindbergh.so was not found in this directory.");
            return EXIT_FAILURE;
        }
    }

    // Build up the command to start the game

    int testMode = 0;
    int gdb = 0;
    int zink = 0;
    int nvidia = 0;
    int forceGame = 0;
    int segaboot = 0;
    char extConfigPath[PATH_MAX] = {0};
    char forceGamePath[128] = {0};

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--test") == 0)
        {
            testMode = 1;
            continue;
        }

        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--segaboot") == 0)
        {
            segaboot = 1;
            continue;
        }

        if (strcmp(argv[i], "--gdb") == 0)
        {
            gdb = 1;
            continue;
        }

        if (strcmp(argv[i], "-z") == 0 || strcmp(argv[i], "--ziink") == 0)
        {
            zink = 1;
            continue;
        }

        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nvidia") == 0)
        {
            nvidia = 1;
            continue;
        }

        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0)
        {
            if (i + 1 >= argc)
            {
                break;
            }
            strncpy(extConfigPath, argv[i + 1], PATH_MAX);
            i += 1;
            continue;
        }
        // Treat the argument as the game name
        strcpy(forceGamePath, argv[i]);
        forceGame = 1;
    }

    // Ensure environment variables are set correctly
    setEnvironmentVariables(isFlatpak, zink, nvidia);

    char command[128] = {0};
    strcpy(command, "./");
    if (forceGame)
    {
        strcat(command, forceGamePath);
    }
    else if (segaboot)
    {
       strcat(command, "segaboot -t");
    }
    else
    {
        if (game == NULL)
        {
            log_error("No lindbergh game found in this directory.");
            return EXIT_FAILURE;
        }
        strcat(command, game);
    }

    if (testMode)
        testModePath(command);

    isCleanElf(command);

    if (gdb)
    {
        char temp[128];
        strcpy(temp, "gdb ");
        strcat(temp, command);
        strcpy(command, temp);
    }

    if (extConfigPath[0] != '\0')
    {
        setenv(LINDBERGH_CONFIG_PATH, extConfigPath, 1);
    }

    log_info("Starting $ %s", command);

    return system(command);
}
