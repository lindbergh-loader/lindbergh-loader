#include <dirent.h>
#include <libgen.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "evdevinput.h"
#include "version.h"
#include "log.h"

#define LD_LIBRARY_PATH "LD_LIBRARY_PATH"
#define LD_PRELOAD "LD_PRELOAD"
#define PRELOAD_FILE_NAME "lindbergh.so"
#define TEAM "bobbydilley, retrofan, dkeruza-neo, doozer, francesco, rolel, caviar-x"
#define LINDBERGH_CONFIG_PATH "LINDBERGH_CONFIG_PATH"
#define LINDBERGH_LOADER_CURRENT_DIR "LINDBERGH_LOADER_CURRENT_DIR"
#define MAX_PATH_LEN 1024

/**
 * @brief Calculates the CRC32 of a single byte.
 *
 * This function calculates the CRC32 value for a given byte of data.
 *
 * @param crc The current CRC32 value.
 * @param data The byte of data to calculate the CRC32 for.
 * @return The updated CRC32 value.
 */
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

/**
 * @brief Looks up a CRC value in the cleanElfCRC32 table.
 *
 * This function searches for a given CRC value within the cleanElfCRC32 array.
 *
 * @param crc The CRC value to look up.
 * @return 1 if the CRC is found in the table, 0 otherwise.
 */
int lookupCrcTable(uint32_t crc)
{
    for (int x = 0; x < cleanElfCRC32Count; x++)
    {
        if (cleanElfCRC32[x] == crc)
            return 1;
    }
    return 0;
}

/**
 * @brief Removes double quotes from a string.
 *
 * This function iterates through the input string and removes any double quote characters.
 * It modifies the string in place.
 *
 * @param str The string to remove double quotes from.
 */
void removeDoubleQuotes(char *str)
{
    if (str == NULL)
    {
        return;
    }

    int j = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != '"')
        {
            str[j] = str[i];
            j++;
        }
    }
    str[j] = '\0';
}

/**
 * @brief Calculates the CRC32 of a file.
 *
 * This function calculates the CRC32 of the specified file.
 *
 * @param filename The path to the file.
 * @param crc The pointer to the variable that will contain the CRC32.
 * @return 0 if the CRC32 was calculated successfully, -1 otherwise.
 */
int calculateFileCRC32(const char *filename, uint32_t *crc)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        log_error("Could not open file to calculate the CRC32.");
        return -1;
    }

    *crc = 0xFFFFFFFF;
    uint8_t buffer[4096];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        for (size_t i = 0; i < bytesRead; i++)
        {
            *crc = calcCrc32(*crc, buffer[i]);
        }
    }
    fclose(file);
    return 0;
}

bool hasSpaces(const char *str)
{
    if (!str)
        return false;
    while (*str)
    {
        if (*str == ' ')
            return true;
        str++;
    }
    return false;
}

/**
 * @brief Checks if the ELF file is clean based on its CRC32.
 *
 * This function calculates the CRC32 of the specified ELF file and compares it
 * against a list of known clean ELF CRC32 values.
 * @param command The command line string containing the path to the ELF file.
 */
void checkIfElfisClean(char *command)
{
    char *last_space = strrchr(command, ' ');
    size_t length;
    if (last_space != NULL && strcmp(last_space, " -t") == 0)
        length = (size_t)(last_space - command);
    else
        length = strlen(command);

    char elfName[256];
    strncpy(elfName, command, length);
    elfName[length] = '\0';

    removeDoubleQuotes(elfName);

    uint32_t crc;
    if (calculateFileCRC32(elfName, &crc) != 0)
        return;

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
    printf("name: %s\n", name);
    char *pos;
    // Check if a different testmode elf is used
    pos = strstr(name, "hod4M.elf");
    if (pos != NULL)
    {
        strcpy(pos, "hod4testM.elf");
        return;
    }

    pos = strstr(name, "hodexRI.elf");
    if (pos != NULL)
    {
        strcpy(pos, "hodextestR.elf");
        return;
    }

    pos = strstr(name, "/Jennifer/\"Jennifer");
    if (pos != NULL)
    {
        strcpy(pos, "/Jennifer/\"../JenTest/JenTest");
        return;
    }

    pos = strstr(name, "/Jennifer");
    if (pos != NULL)
    {
        strcpy(pos, "/../JenTest/JenTest");
        return;
    }

    pos = strstr(name, "apacheM.elf");
    if (pos != NULL)
    {
        strcpy(pos, "apachetestM.elf");
        return;
    }

    pos = strstr(name, "/vt3_Lindbergh/\"vt3_Lindbergh");
    if (pos != NULL)
    {
        strcpy(pos, "/vt3_Lindbergh/\"vt3_testmode");
        return;
    }

    pos = strstr(name, "/vt3_Lindbergh");
    if (pos != NULL)
    {
        strcpy(pos, "/vt3_testmode");
        return;
    }

    // Otherwise add the standard -t to the end
    strcat(name, " -t");
}

/**
 * @brief Checks if a given path is a valid directory.
 *
 * This function uses the stat system call to determine if the provided path
 * corresponds to a directory.
 * @param path The path to check.
 */
bool isValidDirectory(const char *path)
{
    struct stat statbuf;

    if (stat(path, &statbuf) != 0)
        return false;

    return S_ISDIR(statbuf.st_mode);
}

/**
 * @brief Finds a suitable 32-bit library folder.
 *
 * This function searches for a directory that is likely to contain 32-bit
 * libraries, based on a predefined list of candidate paths.
 * @return A dynamically allocated string containing the path to the found
 *         directory, or NULL if no suitable directory is found.
 */
char *find32bLibFolder(void)
{
    const char *folderCandidates[] = {"/app/lib32", "/usr/lib/i386-linux-gnu", "/usr/lib/i686-linux-gnu", "/usr/lib32", "/usr/lib"};
    int numCandidates = sizeof(folderCandidates) / sizeof(folderCandidates[0]);

    char currentPath[PATH_MAX];
    struct stat st;
    char *resultPath = NULL;

    for (int i = 0; i < numCandidates; ++i)
    {
        int written = snprintf(currentPath, sizeof(currentPath), "%s", folderCandidates[i]);

        if (written < 0 || (size_t)written >= sizeof(currentPath))
            continue;

        if (stat(currentPath, &st) == 0 && S_ISDIR(st.st_mode))
        {
            size_t pathLen = strlen(currentPath);
            resultPath = (char *)malloc(pathLen + 1);

            if (resultPath == NULL)
            {
                log_fatal("Failed to allocate memory for result path.");
                return NULL;
            }

            strcpy(resultPath, currentPath);
            return resultPath;
        }
    }
    return NULL;
}

/**
 * @brief Finds the path to the LD_PRELOAD library.
 *
 * This function searches for the lindbergh.so library in various locations,
 * including the current directory and system library folders.
 * @return A dynamically allocated string containing the path to the library, or an empty string if not found.
 */
char *findLDPreloadLibrary()
{
    char *foundPathPtr = NULL;

    char *systemLibFolder = find32bLibFolder();

    if (systemLibFolder == NULL || systemLibFolder[0] == '\0')
    {
        log_error("Error: library folder not found.\n");
        return "";
    }

    char fullLibPath[PATH_MAX];

    int lenWritten = snprintf(fullLibPath, sizeof(fullLibPath), "%s/%s", systemLibFolder, PRELOAD_FILE_NAME);

    if (lenWritten < 0 || (size_t)lenWritten >= sizeof(fullLibPath))
    {
        log_error("Warning: Constructed path is too long or invalid: %s/%s\n", systemLibFolder, PRELOAD_FILE_NAME);
        return "";
    }

    if (access(PRELOAD_FILE_NAME, F_OK) == 0) // lindbergh.so in current folder
    {
        char curDir[MAX_PATH_LEN];
        if (getcwd(curDir, sizeof(curDir)) == NULL)
            return "";
        size_t len = strlen(curDir) + strlen(PRELOAD_FILE_NAME) + 1;
        foundPathPtr = (char *)malloc(len + 1);
        if (foundPathPtr == NULL)
            return "";
        else
            snprintf(foundPathPtr, len + 1, "%s/%s", curDir, PRELOAD_FILE_NAME);
    }
    else if (access(fullLibPath, F_OK) == 0) // lindbergh.so in system lib folder
    {
        foundPathPtr = (char *)malloc(strlen(fullLibPath) + 1);
        if (foundPathPtr == NULL)
            return "";
        else
            strcpy(foundPathPtr, fullLibPath);
    }
    else // lindbergh.so in the ELF folder.
    {
        return PRELOAD_FILE_NAME;
    }
    return foundPathPtr;
}

/**
 * Makes sure the environment variables are set correctly
 *
 * This function sets the LD_LIBRARY_PATH and LD_PRELOAD environment variables
 * to ensure that the game can find the necessary libraries and preload the
 * lindbergh.so library.
 *
 * @param ldLibPath The path to the LD_PRELOAD library.
 * @param curDir The current working directory.
 * @param gameDir The directory where the game is located.
 * @param zink Flag indicating whether to use Zink.
 * @param nvidia Flag indicating whether to use NVIDIA GPU.
 * to run the game.
 */
void setEnvironmentVariables(char *ldLibPath, char *curDir, char *gameDir, int zink, int nvidia)
{
    // Ensure the library path is set correctly
    char libraryPath[MAX_PATH_LEN] = {0};

    const char *currentLibraryPath = getenv(LD_LIBRARY_PATH);
    if (currentLibraryPath != NULL)
    {
        strcat(libraryPath, currentLibraryPath);
        strcat(libraryPath, ":");
    }

    strcat(libraryPath, ".:lib:../lib");

    char *tmpLibPath = dirname(strdup(ldLibPath));

    if (strcmp(tmpLibPath, ".") != 0)
    {
        strcat(libraryPath, ":");
        strcat(libraryPath, tmpLibPath);
    }

    if (curDir != NULL && curDir[0] != '\0' && strcmp(curDir, tmpLibPath) != 0)
    {
        strcat(libraryPath, ":");
        strcat(libraryPath, curDir);
    }

    if (gameDir != NULL && gameDir[0] != '\0')
    {
        strcat(libraryPath, ":");
        strcat(libraryPath, gameDir);
    }

    setenv(LD_LIBRARY_PATH, libraryPath, 1);

    // Ensure the preload path is set correctly
    if (strcmp(ldLibPath, "") != 0)
        setenv(LD_PRELOAD, ldLibPath, 1);
    else
        setenv(LD_PRELOAD, PRELOAD_FILE_NAME, 1); // Should not reach here

    if (zink)
        setenv("MESA_LOADER_DRIVER_OVERRIDE", "zink", 1);

    if (nvidia)
    {
        setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
        setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);
    }
    setenv("LINDBERGH_LOADER_CURRENT_DIR", curDir, 1);
}

/**
 * @brief Prints the usage information for the Lindbergh loader.
 *
 * This function displays the command-line options and usage instructions for
 * the Lindbergh loader.
 * @param argv The command-line arguments array.
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
    printf("  --gamepath | -g     Specifies game path without ELF name\n");
}

/**
 * @brief Lists available evdev controllers and their inputs.
 *
 * This function initializes the controller system and then iterates through
 * the available controllers, printing their names and the names of their
 * inputs.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
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
 * @brief Prints the version information of the Lindbergh loader.
 *
 * This function displays the version number of the Lindbergh loader and the
 * names of the team members.
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
    char gameELF[128] = {0};
    char gameDir[MAX_PATH_LEN] = {0};

    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        printVersion();
        return EXIT_SUCCESS;
    }

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
    {
        printUsage(argv);
        return EXIT_SUCCESS;
    }

    if (argc > 1 && strcmp(argv[1], "--list-controllers") == 0)
    {
        return listControllers();
    }
    // Build up the command to start the game
    int testMode = false;
    int gdb = false;
    int zink = false;
    int nvidia = false;
    int forceGame = false;
    int segaboot = false;
    char *extConfigPath = NULL;
    char *passedGamePath = NULL;
    char *forcedGamePath = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--test") == 0)
        {
            testMode = true;
            continue;
        }

        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--segaboot") == 0)
        {
            segaboot = true;
            continue;
        }

        if (strcmp(argv[i], "--gdb") == 0)
        {
            gdb = true;
            continue;
        }

        if (strcmp(argv[i], "-z") == 0 || strcmp(argv[i], "--zink") == 0)
        {
            zink = true;
            continue;
        }

        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nvidia") == 0)
        {
            nvidia = true;
            continue;
        }

        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0)
        {
            if (i + 1 >= argc)
            {
                break;
            }
            extConfigPath = strdup(argv[i + 1]);
            i += 1;
            continue;
        }

        if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--gamepath") == 0)
        {
            if (i + 1 >= argc)
            {
                break;
            }
            passedGamePath = strdup(argv[i + 1]);

            i += 1;
            continue;
        }

        // Treat the argument as the game name
        forcedGamePath = strdup(argv[i]);
        forceGame = true;
    }

    // We save the current dir to return to it later
    char curDir[MAX_PATH_LEN];
    if (getcwd(curDir, sizeof(curDir)) == NULL)
    {
        log_error("Error: could not get the current directory.");
        return EXIT_FAILURE;
    }

    if (hasSpaces(curDir))
    {
        log_warn("The current path contains spaces, this most likely will cause issues.");
        log_warn("Please, make sure you don't use spaces in the path.");
    }

    char command[PATH_MAX] = {0};
    char *forcedGameDir;
    char *forcedGameElf;
    if (forceGame)
    {
        if (forcedGamePath != NULL)
        {
            forcedGameDir = dirname(strdup(forcedGamePath));
            forcedGameElf = basename(strdup(forcedGamePath));

            if (strcmp(forcedGameDir, ".") == 0) // Only the ELF file name was passed.
            {
                if (passedGamePath[0] != '\0')
                    snprintf(gameDir, MAX_PATH_LEN, "%s", passedGamePath);
                else
                    snprintf(gameDir, MAX_PATH_LEN, "./");
            }
            else
            {
                if (passedGamePath != NULL) // ELF with path and also -g option pointing to a folder
                    log_warn("Warning: elf passed with a path, and -g option enabled. Ignoring -g option.");

                snprintf(gameDir, MAX_PATH_LEN, "%s", forcedGameDir);
            }

            if (strcmp(forcedGameElf, ".") != 0)
                strcpy(gameELF, forcedGameElf);
            // Check if the passed folder is valid
            if (!isValidDirectory(gameDir))
            {
                log_error("%s is not a valid directory here.", gameDir);
                return EXIT_FAILURE;
            }
        }
    }
    else
    {
        // Look for the game ELF's
        struct dirent *ent;
        DIR *dir;

        if (passedGamePath != NULL && passedGamePath[0] != '\0')
        {
            snprintf(gameDir, MAX_PATH_LEN, "%s", passedGamePath);

            // Check if the passed folder is valid
            if (!isValidDirectory(gameDir))
            {
                log_error("%s is not a valid directory.", gameDir);
                return EXIT_FAILURE;
            }
            dir = opendir(gameDir);
        }
        else
        {
            snprintf(gameDir, MAX_PATH_LEN, "%s", curDir);
            dir = opendir(curDir);
        }

        if (dir == NULL)
        {
            log_error("Could not list files in current directory.");
            return EXIT_FAILURE;
        }

        bool found = false;
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_type != DT_REG)
                continue;

            int index = 0;
            while (!found)
            {
                if (strcmp(games[index], "END") == 0)
                    break;

                if (strcmp(ent->d_name, games[index]) == 0)
                {
                    strcpy(gameELF, games[index]);
                    found = true;
                    break;
                }
                index++;
            }
        }
        closedir(dir);
    }

    if (segaboot)
    {
        strcpy(gameELF, "segaboot");
        testMode = true;
    }

    if (gameELF[0] == '\0')
    {
        log_error("No lindbergh game found in this directory.");
        printUsage(argv);
        return EXIT_FAILURE;
    }

    sprintf(command, "\"%s/\"%s", gameDir, gameELF);

    char *ldPreloadLibrary = findLDPreloadLibrary();

    if (strcmp(curDir, gameDir) == 0)
        ldPreloadLibrary = PRELOAD_FILE_NAME;

    // Ensure environment variables are set correctly
    setEnvironmentVariables(ldPreloadLibrary, curDir, gameDir, zink, nvidia);

    if (testMode)
        testModePath(command);

    checkIfElfisClean(command);

    if (gdb)
    {
        char temp[128];
        strcpy(temp, "gdb ");
        strcat(temp, command);
        strcpy(command, temp);
    }

    if (extConfigPath != NULL && extConfigPath[0] != '\0')
    {
        if (hasSpaces(extConfigPath))
        {
            log_warn("The config path contains spaces, this most likely will cause issues.");
            log_warn("Please, make sure you don't use spaces in the path.");
        }
        setenv(LINDBERGH_CONFIG_PATH, extConfigPath, 1);
    }

    log_info("Starting $ %s", command);

    if (gameDir[0] != '\0')
    {
        if (chdir(gameDir) != 0)
        {
            log_error("Could not change to game path.");
            return EXIT_FAILURE;
        }
    }
    int sysCmd = system(command);

    if (gameDir[0] != '\0')
    {
        if (chdir(curDir) != 0)
        {
            log_error("Could not return to the original directory.");
            return EXIT_FAILURE;
        }
    }

    return sysCmd;
}
