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
#define TEAM "bobbydilley, retrofan, dkeruza-neo, doozer, francesco, rolel, caviar-x"

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
                 "END"};

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
        strcpy(name, "./apacheTestM.elf");
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
void setEnvironmentVariables()
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
    setenv(LD_PRELOAD, PRELOAD_FILE_NAME, 1);
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
    printf("  --gdb               Runs with GDB\n");
    printf("  --list-controllers  Lists available controllers and inputs\n");
    printf("  --version           Displays the version of the loader and team's names\n");
    printf("  --help              Displays this usage text\n");
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
    // Ensure environment variables are set correctly
    setEnvironmentVariables();

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
        log_error("The preload object lindbergh.so was not found in this directory.");
        return EXIT_FAILURE;
    }

    // Build up the command to start the game

    int testMode = 0;
    int gdb = 0;
    int forceGame = 0;
    int segaboot = 0;

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

        // Treat the argument as the game name
        strcpy(forceGamePath, argv[i]);
        forceGame = 1;
    }

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

    if (gdb)
    {
        char temp[128];
        strcpy(temp, "gdb ");
        strcat(temp, command);
        strcpy(command, temp);
    }

    log_info("Starting $ %s", command);

    return system(command);
}
