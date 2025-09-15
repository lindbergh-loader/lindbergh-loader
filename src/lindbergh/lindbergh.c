#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>

#include "configIni.h"
#include "evdevInput.h"
#include "log.h"
#include "version.h"
#include "controlIniGen.h"
#include "sdlInput.h"

#define LD_LIBRARY_PATH "LD_LIBRARY_PATH"
#define LD_PRELOAD "LD_PRELOAD"
#define PRELOAD_FILE_NAME "lindbergh.so"
#define PRELOAD_OPENAL "libopenal.so.0"
#define TEAM "bobbydilley, retrofan, dkeruza-neo, francesco"
#define COLLABORATORS "doozer, rolel, caviar-X"
#define SPECIAL_THANKS "Francesco"
#define LINDBERGH_CONFIG_PATH "LINDBERGH_CONFIG_PATH"
#define LINDBERGH_CONTROLS_PATH "LINDBERGH_CONTROLS_PATH"
#define LINDBERGH_CONTROLS_DB_PATH "LINDBERGH_CONTROLS_DB_PATH"
#define LINDBERGH_LOADER_CURRENT_DIR "LINDBERGH_LOADER_CURRENT_DIR"
#define MAX_PATH_LENGTH 1024

uint32_t elf_crc = 0;

// List of all lindbergh executables known, not including the test executables
char *games[] = {"a.elf",    "abc",     "apacheM.elf",   "chopperM.elf", "drive.elf",
                 "dsr",      "gsevo",   "hod4M.elf",     "hodexRI.elf",  "hummer_Master.elf",
                 "id4.elf",  "id5.elf", "Jennifer",      "lgj_final",    "lgjsp_app",
                 "main.exe", "mj4",     "q2satl_lind",   "ramboM.elf",   "vf5",
                 "vsg",      "vt3",     "vt3_Lindbergh", "END"};

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
    0xDCAD8ABA, // DVP-0025H | q2satl_lind
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
    0x65489691, // DVP-0049  | mj4
    0x653BC83B, // DVP-0057  | a.elf
    0x04D88552, // DVP-0057B | a.elf
    0x089D6051, // DVP-0060  | dsr
    0x317F3B90, // DVP-0063  | hodexRI.elf
    0x3A5EEC69, // DVP-0063  | hodextestR.elf
    0x81E02850, // DVP-0069  | ramboM.elf
    0xE4F202BB, // DVP-0070A | id5.elf
    0x400C09CD, // DVP-0070C | id5.elf
    0x2E6732A3, // DVP-0070F | id5.elf
    0xF99A3CDB, // DVP-0075  | id5.elf
    0xDCC1f8E7, // DVP-0078  | ramboM.elf
    0x05647A8E, // DVP-0079  | hummer_Master.elf
    0x0AD7CF0F, // DVP-0081  | mj4
    0x4442EA15, // DVP-0083  | hummer_Master.elf
    0x8DF6BBF9, // DVP-0084  | id5.elf
    0x2AF8004E, // DVP-0084A | id5.elf
    0xCB663DD0, // DVP-0087D | q2satl_lind
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
    crc ^= data;
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

int calculateCRC32inChunks(const char *filename, uint32_t *crc)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        log_error("Could not open file to calculate the CRC32.");
        return EXIT_FAILURE;
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

int lookupCrcTable(uint32_t crc)
{
    for (int x = 0; x < cleanElfCRC32Count; x++)
    {
        if (cleanElfCRC32[x] == crc)
            return 1;
    }
    return 0;
}

int fileExists(const char *path)
{
    struct stat buffer;
    return stat(path, &buffer) == 0;
}

int dirExists(const char *path)
{
    struct stat buffer;
    return stat(path, &buffer) == 0 && S_ISDIR(buffer.st_mode);
}

void isCleanElf(char *command)
{
    char *lastSpace = strrchr(command, ' ');
    size_t length;
    if (lastSpace != NULL && strcmp(lastSpace, " -t") == 0)
        length = (size_t)(lastSpace - command);
    else
        length = strlen(command);

    char elfName[256];
    strncpy(elfName, command, length);
    elfName[length] = '\0';

    if (dirExists(elfName))
    {
        log_error("There is a folder named \'%s\' in the current directory instead of a game's ELF.", elfName + 2);
        exit(EXIT_FAILURE);
    }

    uint32_t crc;
    if (calculateCRC32inChunks(elfName, &crc) != 0)
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

void extractPathFromProg(const char *input, char *out_path, char *out_prog)
{
    char tmp1[MAX_PATH_LENGTH], tmp2[MAX_PATH_LENGTH];
    strncpy(tmp1, input, MAX_PATH_LENGTH);
    strncpy(tmp2, input, MAX_PATH_LENGTH);
    strncpy(out_prog, basename(tmp1), MAX_PATH_LENGTH);
    strncpy(out_path, dirname(tmp2), MAX_PATH_LENGTH);
}

void testModePath(char *program)
{
    if (strcmp(program, "hod4M.elf") == 0)
        strcpy(program, "hod4testM.elf");
    else if (strcmp(program, "hodexRI.elf") == 0)
        strcpy(program, "hodextestR.elf");
    else if (strcmp(program, "apacheM.elf") == 0)
        strcpy(program, "apachetestM.elf");
    else if (strcmp(program, "vt3_Lindbergh") == 0)
        strcpy(program, "vt3_testmode");
    else if (strcmp(program, "Jennifer") == 0)
        strcpy(program, "../JenTest/JenTest");
    else
        strcat(program, " -t");
}

char *findPreloadLibrary(const char *originalDir, char *preloadFileName)
{
    static char result[MAX_PATH_LENGTH];
    char appImageLib[MAX_PATH_LENGTH];
    snprintf(appImageLib, MAX_PATH_LENGTH, "%s/usr/lib32", getenv("APP_IMG_ROOT"));
    const char *folderCandidates[] = {"/app/lib32", appImageLib, originalDir, "/usr/lib/i386-linux-gnu", "/usr/lib/i686-linux-gnu",
                                      "/usr/lib32", "/usr/lib",  NULL};

    for (int i = 0; i < sizeof(folderCandidates) / sizeof(folderCandidates[0]); i++)
    {
        snprintf(result, MAX_PATH_LENGTH, "%s/%s", folderCandidates[i], preloadFileName);
        if (fileExists(result))
            return result;
    }

    if (fileExists(preloadFileName))
        return preloadFileName;

    return "";
}

int pathsDiffer(const char *p1, const char *p2)
{
    char real1[MAX_PATH_LENGTH], real2[MAX_PATH_LENGTH];
    if (!realpath(p1, real1) || !realpath(p2, real2))
        return 1;
    return strcmp(real1, real2) != 0;
}

bool hasSpaces(const char *path)
{
    if (strchr(path, ' '))
        return true;

    return false;
}

void setEnvironmentVariables(const char *ldLibPath, const char *originalDir, const char *gameDir, int zink, int nvidia,
                             const char *confFilePath, const char *contFilePath, const char *contDbFilePath, char *libOpenal)
{

    if (libOpenal != NULL)
    {
        char ldpreloadEnv[MAX_PATH_LENGTH];
        snprintf(ldpreloadEnv, sizeof(ldpreloadEnv), "%s %s", ldLibPath, libOpenal);
        setenv("LD_PRELOAD", ldpreloadEnv, 1);
    }
    else
    {
        setenv("LD_PRELOAD", ldLibPath, 1);
    }

    if (hasSpaces(ldLibPath))
    {
        log_error("The path \'%s\' where lindbergh.so is located cannot contain spaces.", ldLibPath);
        exit(EXIT_FAILURE);
    }

    char *currentLibraryPath = getenv("LD_LIBRARY_PATH");
    char newLdLibPath[MAX_PATH_LENGTH * 3] = "";

    if (currentLibraryPath && strlen(currentLibraryPath) > 0)
        snprintf(newLdLibPath, sizeof(newLdLibPath), "%s:", currentLibraryPath);

    char *tmpLibPath = dirname(strdup(ldLibPath));

    if (strcmp(tmpLibPath, ".") != 0)
    {
        strcat(newLdLibPath, ":");
        strcat(newLdLibPath, tmpLibPath);
    }

    if (originalDir != NULL && originalDir[0] != '\0' && pathsDiffer(originalDir, tmpLibPath) != 0)
    {
        strcat(newLdLibPath, ":");
        strcat(newLdLibPath, originalDir);
    }

    if (gameDir && gameDir[0] != '\0' && pathsDiffer(originalDir, gameDir) && pathsDiffer(tmpLibPath, gameDir))
    {
        strcat(newLdLibPath, ":");
        strcat(newLdLibPath, gameDir);
    }

    strcat(newLdLibPath, ":.:lib:../lib");

    if (newLdLibPath[0] == ':')
        memmove(newLdLibPath, newLdLibPath + 1, strlen(newLdLibPath));

    setenv("LD_LIBRARY_PATH", newLdLibPath, 1);

    if (strlen(confFilePath) > 0)
    {
        if (hasSpaces(confFilePath))
        {
            log_error("The path \'%s\' for the config file cannot contain spaces.", confFilePath);
            log_error("The loader will use the default configuration.");
            confFilePath = "";
        }
        else if(!fileExists(confFilePath))
        {
            log_error("The file \'%s\' does not exist, will be using default config.", confFilePath);
            confFilePath = "";
        }

        setenv(LINDBERGH_CONFIG_PATH, confFilePath, 1);
    }

    if (strlen(contFilePath) > 0)
    {
        if (hasSpaces(contFilePath))
        {
            log_error("The path \'%s\' for the controls file cannot contain spaces.", contFilePath);
            log_error("Controls will use the default controls config.");
            contFilePath = "";
        }
        else if (!fileExists(contFilePath))
        {
            log_error("The file \'%s\' does not exist, will try to load controls.ini from the game folder or", contFilePath);
            log_error("will be using the default controls config.");
            contFilePath = "";
        }

        setenv(LINDBERGH_CONTROLS_PATH, contFilePath, 1);
    }

    if (strlen(contDbFilePath) > 0)
    {
        if (hasSpaces(contDbFilePath))
        {
            log_error("The path \'%s\' for the gamecontrollerdb.txt file cannot contain spaces.", contDbFilePath);
            log_error("Additional game controller mappings will not be added.");
            contDbFilePath = "";
        }
        else if (!fileExists(contDbFilePath))
        {
            log_error("The file \'%s\' does not exist, additional mapping db will be omitted.", contDbFilePath);
            contDbFilePath = "";
        }

        setenv(LINDBERGH_CONTROLS_DB_PATH, contDbFilePath, 1);
    }

    if (zink && nvidia)
    {
        log_error("Cannot pass both, zink and nvidia options at the same time.");
        exit(EXIT_FAILURE);
    }

    if (zink)
    {
        setenv("MESA_LOADER_DRIVER_OVERRIDE", "zink", 1);
    }

    if (nvidia)
    {
        setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
        setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);
    }

    char *appImgRoot = getenv("APP_IMG_ROOT");
    if (appImgRoot != NULL)
    {
        char libglDriPath[MAX_PATH_LENGTH];
        sprintf(libglDriPath, "%s/usr/lib32/dri", appImgRoot);
        setenv("LIBGL_DRIVERS_PATH", libglDriPath, 1);
    }
}

void printUsage(char *programName)
{
    printf("%s [GAME_PATH] [OPTIONS]\n", basename(programName));
    printf("Options:\n");
    printf("  --test             | -t  Runs the test mode\n");
    printf("  --segaboot         | -s  Runs segaboot\n");
    printf("  --zink             | -z  Runs with Zink\n");
    printf("  --nvidia           | -n  Runs with nVidia GPU when is as a secondary GPU in a laptop\n");
    printf("  --gdb                    Runs with GDB\n");
    printf("  --list-controllers | -l  Lists available controllers and inputs for EVDEV.\n");
    printf("  --list-guids       | -lg Lists available SDL controllers GUIDs.\n");
    printf("  --version          | -v  Displays the version of the loader and team's names\n");
    printf("  --help             | -h  Displays this usage text\n");
    printf("  --config           | -c  Specifies configuration ini file path\n");
    printf("  --controls         | -o  Specifies controls ini file path\n");
    printf("  --controlsdb       | -d  Specifies gamecontrollerdb.txt file path\n");
    printf("  --gamepath         | -g  Specifies game path without ELF name\n");
    printf("  --create           | -C  Creates a default config or controls file. Use '--create --help' for more info.\n");
}

void printCreateUsage(char *programName)
{
    printf("Usage for the --create option:\n");
    printf("  %s --create <sub-option> [output_path] [output_filename]\n\n", basename(programName));
    printf("Arguments:\n");
    printf("  sub-option          The type of file to create. Can be one of:\n");
    printf("                        config   (for lindbergh.ini)\n");
    printf("                        controls (for controls.ini)\n");
    printf("  output_path         (Optional) The directory where the file will be created.\n");
    printf("                      If not provided, the file is created in the current directory.\n");
    printf("  output_filename     (Optional) The name of the file to be created. Must end with .ini.\n");
    printf("                      If not provided, the default name will be used (lindbergh.ini or controls.ini).\n\n");
    printf("Examples:\n");
    printf("  %s --create config\n", basename(programName));
    printf("  %s --create controls ./my_game_config\n", basename(programName));
    printf("  %s --create config ./my_game_config myconfig.ini\n", basename(programName));
    printf("  %s --create --help\n", basename(programName));
}

void printVersion()
{
    printf("Lindbergh Loader v%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, UPDATE_VERSION);
    printf("Lead Programming/Reversing: %s\n", TEAM);
    printf("Collaborators: %s\n", COLLABORATORS);
    printf("Special Thanks to: %s\n", SPECIAL_THANKS);
}

int listEvDevControllers()
{
    Controllers controllers;

    ControllerStatus status = loadEvdevControllers(&controllers);

    if (status != CONTROLLER_STATUS_SUCCESS)
    {
        log_error("Failed to list controllers\n");
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

    stopEvdevControllers(&controllers);

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    if (argc > 1 && (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0))
    {
        printVersion();
        return EXIT_SUCCESS;
    }

    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0))
    {
        printUsage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (argc > 1 && (strcmp(argv[1], "--list-controllers") == 0 || strcmp(argv[1], "-l") == 0))
    {
        return listEvDevControllers();
    }

    if (argc > 1 && (strcmp(argv[1], "--list-guids") == 0 || strcmp(argv[1], "-lg") == 0))
    {
        return listSdlControllers();
    }
    
    if (argc > 1 && (strcmp(argv[1], "--create") == 0 || strcmp(argv[1], "-C") == 0))
    {
        if (argc < 3)
        {
            log_error("Missing arguments for --create. See usage below.\n");
            printCreateUsage(argv[0]);
            return EXIT_FAILURE;
        }

        if (strcmp(argv[2], "--help") == 0)
        {
            printCreateUsage(argv[0]);
            return EXIT_SUCCESS;
        }

        char *subOption = argv[2];
        char *path = ".";
        char *filename = NULL;
        char *argCopyForDirname = NULL;
        char *argCopyForBasename = NULL;


        if (strcmp(subOption, "config") != 0 && strcmp(subOption, "controls") != 0)
        {
            log_error("Invalid sub-option for --create: '%s'. Must be 'config' or 'controls'.\n", subOption);
            printCreateUsage(argv[0]);
            return EXIT_FAILURE;
        }

        if (argc > 3)
        {
            if (argc > 4)
            {
                path = argv[3];
                filename = argv[4];
            }
            else
            {
                char *arg = argv[3];
                const char *dot = strrchr(arg, '.');
                if (dot && strcmp(dot, ".ini") == 0)
                {
                    argCopyForDirname = strdup(arg);
                    argCopyForBasename = strdup(arg);
                    path = dirname(argCopyForDirname);
                    filename = basename(argCopyForBasename);
                }
                // if arg is a path
                else
                {
                    path = arg;
                }
            }
        }

        if (filename)
        {
            const char *dot = strrchr(filename, '.');
            if (!dot || strcmp(dot, ".ini") != 0)
            {
                log_error("Invalid filename: '%s'. Must have .ini extension.\n", filename);
                if(argCopyForDirname) free(argCopyForDirname);
                if(argCopyForBasename) free(argCopyForBasename);
                return EXIT_FAILURE;
            }
        }

        if (!dirExists(path))
        {
            log_error("Output directory does not exist: %s\n", path);
            if(argCopyForDirname) free(argCopyForDirname);
            if(argCopyForBasename) free(argCopyForBasename);
            return EXIT_FAILURE;
        }

        char finalPath[MAX_PATH_LENGTH];

        if (strcmp(subOption, "config") == 0)
        {
            if (!filename)
            {
                filename = "lindbergh.ini";
            }
            snprintf(finalPath, sizeof(finalPath), "%s/%s", path, filename);
            createDefaultIni(finalPath);
            log_info("Successfully created configuration file at %s\n", finalPath);
        }
        else if (strcmp(subOption, "controls") == 0)
        {
            if (!filename)
            {
                filename = "controls.ini";
            }
            snprintf(finalPath, sizeof(finalPath), "%s/%s", path, filename);
            createDefaultControlsIni(finalPath);
            log_info("Successfully created controls file at %s\n", finalPath);
        }

        if(argCopyForDirname) free(argCopyForDirname);
        if(argCopyForBasename) free(argCopyForBasename);

        return EXIT_SUCCESS;
    }

    char passedGamePath[MAX_PATH_LENGTH] = "";
    char forcedGamePath[MAX_PATH_LENGTH] = "";
    char forcedGameDir[MAX_PATH_LENGTH] = "";
    char gameELF[MAX_PATH_LENGTH] = "";
    char extConfigPath[MAX_PATH_LENGTH] = "";
    char extControlsPath[MAX_PATH_LENGTH] = "";
    char extControlsDbPath[MAX_PATH_LENGTH] = "";
    char originalDir[MAX_PATH_LENGTH] = "";
    bool gdb = false;
    bool testMode = false;
    bool segaboot = false;
    bool zink = false;
    bool nvidia = false;

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
            strncpy(extConfigPath, argv[i + 1], MAX_PATH_LENGTH);
            i += 1;
            continue;
        }

        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--controls") == 0)
        {
            if (i + 1 >= argc)
            {
                break;
            }
            strncpy(extControlsPath, argv[i + 1], MAX_PATH_LENGTH);
            i += 1;
            continue;
        }

        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--controlsdb") == 0)
        {
            if (i + 1 >= argc)
            {
                break;
            }
            strncpy(extControlsDbPath, argv[i + 1], MAX_PATH_LENGTH);
            i += 1;
            continue;
        }

        if ((strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--gamepath") == 0) && i + 1 < argc)
        {
            strncpy(passedGamePath, argv[i + 1], MAX_PATH_LENGTH);
            i++;
            continue;
        }

        strncpy(forcedGamePath, argv[i], MAX_PATH_LENGTH);
    }

    if (!getcwd(originalDir, sizeof(originalDir)))
    {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    int useForceCommandPath = strlen(forcedGamePath) > 0;
    bool commandOnlyElf = useForceCommandPath && strchr(forcedGamePath, '/') == NULL;

    if (useForceCommandPath && !commandOnlyElf)
    {
        if (!fileExists(forcedGamePath))
        {
            log_error("File does not exist: %s\n", forcedGamePath);
            return EXIT_FAILURE;
        }

        extractPathFromProg(forcedGamePath, forcedGameDir, gameELF);
        if (hasSpaces(forcedGameDir))
        {
            log_warn("The path contains spaces, this most likely will cause issues.");
            log_warn("Please, make sure you don't use spaces in the path.");
        }

        if (!dirExists(forcedGameDir))
        {
            log_error("Directory does not exist: %s\n", forcedGameDir);
            return EXIT_FAILURE;
        }

        chdir(forcedGameDir);
    }
    else if (strlen(passedGamePath) > 0)
    {
        if (hasSpaces(passedGamePath))
        {
            log_warn("The path contains spaces, this most likely will cause issues.");
            log_warn("Please, make sure you don't use spaces in the path.");
        }

        if (!dirExists(passedGamePath))
        {
            log_error("Directory does not exist: %s\n", passedGamePath);
            return EXIT_FAILURE;
        }

        chdir(passedGamePath);

        if (commandOnlyElf)
        {
            strncpy(gameELF, forcedGamePath, MAX_PATH_LENGTH);
            if (!fileExists(gameELF))
            {
                log_error("Program '%s' not found in %s\n", gameELF, passedGamePath);
                return EXIT_FAILURE;
            }
        }
        else
        {
            for (int i = 0; games[i] && strcmp(games[i], "END") != 0; i++)
            {
                if (fileExists(games[i]))
                {
                    strncpy(gameELF, games[i], MAX_PATH_LENGTH);
                    break;
                }
            }

            if (strlen(gameELF) == 0)
            {
                log_error("No known game file found in %s\n", passedGamePath);
                return EXIT_FAILURE;
            }
        }
    }
    else if (commandOnlyElf)
    {
        strncpy(gameELF, forcedGamePath, MAX_PATH_LENGTH);
        if (!fileExists(gameELF))
        {
            log_error("'%s' not found in current directory\n", gameELF);
            return EXIT_FAILURE;
        }
    }
    else
    {
        for (int i = 0; games[i] && strcmp(games[i], "END") != 0; i++)
        {
            if (access(games[i], F_OK) == 0)
            {
                strncpy(gameELF, games[i], sizeof(gameELF) - 1);
                gameELF[sizeof(gameELF) - 1] = '\0';
                printf("Auto-detected ELF: %s\n", gameELF);
                break;
            }
        }

        if (gameELF[0] == '\0')
        {
            log_error("No game ELF found in current directory.\n");
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (testMode)
    {
        testModePath(gameELF);
    }

    char *targetedGameDir = strlen(forcedGameDir) ? forcedGameDir : passedGamePath;
    char *libPath = strdup(findPreloadLibrary(originalDir, PRELOAD_FILE_NAME));
    if (strcmp(libPath, "") == 0)
    {
        log_error("Error: %s not found in known locations.\n", PRELOAD_FILE_NAME);
        return 1;
    }

    char *libOpenal = NULL;
    if (strstr(gameELF, "q2satl_lind") != NULL)
    {
        libOpenal = strdup(findPreloadLibrary(originalDir, PRELOAD_OPENAL));
        if (strcmp(libOpenal, "") == 0)
        {
            printf("You might not get sound because libopenal.so.0 was not found.\n");
        }
    }
    
    setEnvironmentVariables(libPath, originalDir, targetedGameDir, zink, nvidia, extConfigPath, extControlsPath, extControlsDbPath,
                            libOpenal);

    if (segaboot)
    {
        strcpy(gameELF, "segaboot -t");
    }

    // Final command
    char command[MAX_PATH_LENGTH];
    if (strcmp(gameELF, "../JenTest/JenTest") == 0)
    {
        snprintf(command, MAX_PATH_LENGTH, "%s", gameELF);
    }
    else
    {
        snprintf(command, MAX_PATH_LENGTH, "./%s", gameELF);
    }

    isCleanElf(command);

    if (gdb)
    {
        char temp[128];
        if (testMode)
            strcpy(temp, "gdb --args ");
        else
            strcpy(temp, "gdb ");
        strcat(temp, command);
        strcpy(command, temp);
    }

    log_info("Starting $ %s", command);

    int sysCmd = system(command);

    if (chdir(originalDir) != 0)
    {
        log_error("Could not return to the original directory.");
        return EXIT_FAILURE;
    }

    return sysCmd;
}
