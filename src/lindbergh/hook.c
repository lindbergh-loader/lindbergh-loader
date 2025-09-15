#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __i386__
#define __i386__
#endif
#undef __x86_64__
#include <ctype.h>
#include <dirent.h>
#include <link.h>
#include <math.h>
#include <net/if.h>
#include <signal.h>
#include <SDL3/SDL.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include "cardReader.h"
#include "baseBoard.h"
#include "config.h"
#include "driveBoard.h"
#include "eeprom.h"
#include "evdevInput.h"
#include "fpsLimiter.h"
#include "gpuVendor.h"
#include "hook.h"
#include "jvs.h"
#include "log.h"
#include "patch.h"
#include "pciData.h"
#include "resolution.h"
#include "resources/logo.h"
#include "resources/font.h"
#include "rideBoard.h"
#include "sdlCalls.h"
#include "sdlInput.h"
#include "securityBoard.h"
#include "shaderCache.h"
#include "touchScreen.h"

#define HOOK_FILE_NAME "/dev/zero"

DeviceType hooks[5] = {-1, -1, -1, -1, -1};
FILE *fileHooks[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
FileTypes fileRead[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
char envpath[100];

int fontABCidx = 0;
int fontTGAidx = 0;
int logoTGAidx = 0;

uint32_t elf_crc = 0;

uint32_t gId = 0;
int gGrp = -1;

int gWidth;
int gHeight;

extern int hummerExtremeShaderFileIndex;
extern bool cachedShaderFilesLoaded;
extern char vf5StageNameAbbr[5];
bool phShowCursorInGame = false;

extern FpsLimit fpsLimit;
Controllers controllers = {0};
extern SDLControllers sdlJoysticks;

char *configFolder = {0};

static int callback(struct dl_phdr_info *info, size_t size, void *data);

uint16_t basePortAddress = 0xFFFF;

/**
 * @brief Signal handler for SIGSEGV.
 *
 * This function handles segmentation faults (SIGSEGV) and attempts to recover
 * from certain types of memory access errors, particularly those related to I/O ports.
 */
/**
 * Signal handler for the SIGSEGV signal, which is triggered when a process tries to access an illegal memory location.
 * @param signal
 * @param info
 * @param ptr
 */
static void handleSegfault(int signal, siginfo_t *info, void *ptr)
{
    ucontext_t *ctx = ptr;

    // Get the address of the instruction causing the segfault
    // uint8_t *code = (uint8_t *)ctx->uc_mcontext.gregs[REG_EIP];
    greg_t eip_value = ctx->uc_mcontext.gregs[REG_EIP];
    uint8_t *code = (uint8_t *)(size_t)eip_value; // Use uintptr_t to ensure proper alignment

    switch (*code)
    {
        case 0xED: // IN
        {
            // Get the port number from the EDX register
            uint16_t port = ctx->uc_mcontext.gregs[REG_EDX] & 0xFFFF;

            // The first port called is usually random, but everything after that
            // is a constant offset, so this is a hack to fix that.
            // When run as sudo it works fine!?

            if (basePortAddress == 0xFFFF)
                basePortAddress = port;

            // Adjust the port number if necessary
            if (port > 0x38)
                port = port - basePortAddress;

            // Call the security board input function with the port number and data
            securityBoardIn(port, (uint32_t *)&(ctx->uc_mcontext.gregs[REG_EAX]));

            ctx->uc_mcontext.gregs[REG_EIP]++;
            return;
        }
        break;

        case 0xE7: // OUT IMMEDIATE
        {
            // Increment the instruction pointer by two to skip over this instruction
            ctx->uc_mcontext.gregs[REG_EIP] += 2;
            return;
        }
        break;

        case 0xE6: // OUT IMMEDIATE
        {
            // Increment the instruction pointer by two to skip over this instruction
            ctx->uc_mcontext.gregs[REG_EIP] += 2;
            return;
        }
        break;

        case 0xEE: // OUT
        {
            uint16_t port = ctx->uc_mcontext.gregs[REG_EDX] & 0xFFFF;
            uint8_t data = ctx->uc_mcontext.gregs[REG_EAX] & 0xFF;
            ctx->uc_mcontext.gregs[REG_EIP]++;
            return;
        }
        break;

        case 0xEF: // OUT
        {
            uint16_t port = ctx->uc_mcontext.gregs[REG_EDX] & 0xFFFF;
            ctx->uc_mcontext.gregs[REG_EIP]++;
            return;
        }
        break;

        default:
            // repeat_printf("Skipping SEGFAULT %X\n", *code);
            log_warn("Skipping SEGFAULT %X\n", *code);
            ctx->uc_mcontext.gregs[REG_EIP]++;
            // abort();
    }
}

uint32_t getCrc32(const char *s, ssize_t n)
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

/**
 * @brief Trims leading and trailing whitespace and quotes from a string.
 *
 * This function removes leading and trailing whitespace characters, as well as
 * single and double quotes, from the input string.
 * @param str The string to trim.
 */
char *trimOS_ID(char *str)
{
    if (!str)
        return str;

    char *end;
    while (isspace((unsigned char)*str) || *str == '"' || *str == '\'')
        str++;

    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && (isspace((unsigned char)*end) || *end == '"' || *end == '\''))
        end--;

    *(end + 1) = '\0';

    return str;
}

/**
 * @brief Checks if the operating system is Debian-based.
 *
 * This function reads the /etc/os-release file to determine if the current
 * operating system is Debian or Ubuntu, or a derivative of them.
 */
bool checkOS_ID()
{
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp)
    {
        return false;
    }

    char line[256];
    bool found = false;

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "ID=", 3) == 0)
        {
            char *value = trimOS_ID(line + 3);
            if (strcmp(value, "debian") == 0 || strcmp(value, "ubuntu") == 0)
            {
                found = true;
                break;
            }
        }
        else if (strncmp(line, "ID_LIKE=", 8) == 0)
        {
            char value_part[256];
            strncpy(value_part, line + 8, sizeof(value_part) - 1);
            value_part[sizeof(value_part) - 1] = '\0';

            char *value_trimmed = trimOS_ID(value_part);
            char *token = strtok(value_trimmed, " \t\n");

            while (token != NULL)
            {
                char *clean_token = trimOS_ID(token);
                if (strcmp(clean_token, "debian") == 0 || strcmp(clean_token, "ubuntu") == 0)
                {
                    found = true;
                    break;
                }
                token = strtok(NULL, " \t\n");
            }

            if (found)
                break;
        }
    }

    fclose(fp);
    return found;
}

/**
 * @brief Initialization function for the hook library.
 *
 * This function is called automatically when the library is loaded. It sets up
 * signal handlers, initializes configuration, and performs other setup tasks.
 */
void __attribute__((constructor)) hook_init()
{
    // We force x11 for SDL so the window do not scale in wayland.
    setenv("SDL_VIDEODRIVER", "x11", 1);

    // Get offsets of the Game's ELF and calculate CRC32.
    dl_iterate_phdr(callback, NULL);

    // Implement SIGSEGV handler
    struct sigaction act;
    act.sa_sigaction = handleSegfault;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &act, NULL);

    char *envPath = getenv("LINDBERGH_CONFIG_PATH");
    initConfig(envPath);

    gId = getConfig()->crc32;
    gGrp = getConfig()->gameGroup;
    gWidth = getConfig()->width;
    gHeight = getConfig()->height;

    if (!checkOS_ID())
    {
        log_warn("Seems like you're not in debian-like system. There might be unexpected issues.");
    }

    initFpsLimiter();

    getGPUVendor();

    if (initPatch() != 0)
        exit(1);

    if (initSDL() != 0)
        exit(1);

    if (initResolutionPatches() != 0)
        exit(1);

    if (initEeprom() != 0)
        exit(1);

    if (initBaseboard() != 0)
        exit(1);

    if (initJVS() != 0)
        exit(1);

    if (initSecurityBoard() != 0)
        exit(1);

    if (getConfig()->emulateDriveboard)
    {
        if (initDriveboard() != 0)
            exit(1);
    }

    if (getConfig()->emulateRideboard)
    {
        if (initRideboard() != 0)
            exit(1);
    }

    if (getConfig()->emulateHW210CardReader)
    {
        if (initCardReader() != 0)
            exit(1);
    }

    envPath = getenv("LINDBERGH_CONTROLS_PATH");
    if (initSdlInput(envPath) != 0)
        exit(1);

    if (initEvdevControllers(&controllers) != 0)
        exit(1);

    securityBoardSetDipResolution(getConfig()->width, getConfig()->height);

    printf("\nSEGA Lindbergh Emulator\nBy the Lindbergh Development Team 2025\n\n");
    printf("  GAME:        %s\n", getGameName());
    printf("  GAME ID:     %s\n", getGameId());
    printf("  DVP:         %s\n", getDvpName());
    printf("  GPU VENDOR:  %s\n", getConfig()->GPUVendorString);

    for (int i = 0; i < controllers.count; i++)
    {
        if (controllers.controller[i].inUse)
        {
            printf("  CONTROLLER: %s\n", controllers.controller[i].name);
        }
    }
    printf("\n");

    for (int i = 0; i < MAX_JOYSTICKS; i++)
    {
        SDL_Joystick *joy = NULL;
        if (sdlJoysticks.controllers[i])
            joy = SDL_GetGamepadJoystick(sdlJoysticks.controllers[i]);
        else if (sdlJoysticks.joysticks[i])
            joy = sdlJoysticks.joysticks[i];

        if (joy != NULL)
            printf("  SDL CONTROLLER %d: %s\n", i, SDL_GetJoystickName(joy));
    }
    printf("\n");

    switch (gGrp)
    {
        case GROUP_LGJ:
        case GROUP_ABC:
        case GROUP_ID5:
        {
            if (getConfig()->GPUVendor == ATI_GPU)
            {
                printf("WARNING: Game %s is unsupported in AMD GPU with ATI driver\n", getGameName());
            }
        }
    }
}

/**
 * @brief Hook for the opendir function.
 *
 * This function intercepts calls to opendir and redirects them to different
 * directories based on the game ID.
 * @param dirname The name of the directory to open.
 */
DIR *opendir(const char *dirname)
{
    DIR *(*_opendir)(const char *dirname) = dlsym(RTLD_NEXT, "opendir");

    if (strcmp(dirname, "/tmp/") == 0 && gGrp == GROUP_ID5)
    {
        return _opendir(dirname + 1);
    }

    // Fix for Outrun high scores
    if (strcmp(dirname, "/home/disk1/rankingdata") == 0 && (gGrp == GROUP_OUTRUN || gGrp == GROUP_OUTRUN_TEST))
    {
        return _opendir("./rankingdata");
    }
    return _opendir(dirname);
}

/**
 * @brief Hook for the remove function.
 *
 * This function intercepts calls to remove and redirects them to different
 * paths based on the game ID.
 * @param path The path to remove.
 */
int remove(const char *path)
{
    int (*_remove)(const char *path) = dlsym(RTLD_NEXT, "remove");

    if (strcmp(path, "/home/disk1/rankingdata/%s") == 0 && (gGrp == GROUP_OUTRUN || gGrp == GROUP_OUTRUN_TEST))
    {
        return _remove("./rankingdata/%s");
    }
    return _remove(path);
}

/**
 * @brief Hook for the mkdir function.
 *
 * This function intercepts calls to mkdir and redirects them to different
 * paths based on the game ID.
 * @param path The path to create.
 */
int mkdir(const char *path, mode_t mode)
{
    int (*_mkdir)(const char *path, mode_t mode) = dlsym(RTLD_NEXT, "mkdir");

    if (strcmp(path, "/home/disk1/rankingdata") == 0 && (gGrp == GROUP_OUTRUN || gGrp == GROUP_OUTRUN_TEST))
    {
        return _mkdir("./rankingdata", mode);
    }
    return _mkdir(path, mode);
}

/**
 * @brief Hook for the __xstat64 function.
 *
 * This function intercepts calls to __xstat64 and redirects them to different
 * paths based on the game ID.
 * @param path The path to stat.
 */
int __xstat64(int ver, const char *path, struct stat64 *stat_buf)
{
    int (*___xstat64)(int ver, const char *path, struct stat64 *stat_buf) = dlsym(RTLD_NEXT, "__xstat64");

    if (strcmp("/var/tmp/warning", path) == 0)
    {
        return ___xstat64(ver, "warning", stat_buf);
    }
    return ___xstat64(ver, path, stat_buf);
}

/**
 * @brief Hook for the open function.
 *
 * This function intercepts calls to open and redirects them to different
 * files or devices based on the pathname.
 * @param pathname The path to open.
 */
int open(const char *pathname, int flags, ...)
{
    va_list args;
    va_start(args, flags);
    int mode = va_arg(args, int);
    va_end(args);

    int (*_open)(const char *pathname, int flags, ...) = dlsym(RTLD_NEXT, "open");

    if (strcmp(pathname, "/dev/lbb") == 0)
    {
        hooks[BASEBOARD] = _open(HOOK_FILE_NAME, flags, mode);
        return hooks[BASEBOARD];
    }

    if (strcmp(pathname, "/dev/i2c/0") == 0)
    {
        hooks[EEPROM] = _open(HOOK_FILE_NAME, flags, mode);
        return hooks[EEPROM];
    }

    if (strcmp(pathname, "/dev/ttyS0") == 0 || strcmp(pathname, "/dev/tts/0") == 0)
    {
        if (getConfig()->emulateDriveboard == 0 && getConfig()->emulateRideboard == 0 && getConfig()->emulateHW210CardReader == 0 &&
            getConfig()->emulateTouchscreen == 0)
            return _open(getConfig()->serial1Path, flags, mode);

        if (hooks[SERIAL0] != -1 && getConfig()->emulateHW210CardReader && gId != R_TUNED)
        {
            return hooks[SERIAL0];
        }

        hooks[SERIAL0] = _open(HOOK_FILE_NAME, flags, mode);
        log_warn("SERIAL0 Opened %d\n", hooks[SERIAL0]);

        if (getConfig()->emulateHW210CardReader == 1 && gId != R_TUNED)
            cardReaderSetFd(0, hooks[SERIAL0], getConfig()->cardFile1);

        return hooks[SERIAL0];
    }

    if (strcmp(pathname, "/dev/ttyS1") == 0 || strcmp(pathname, "/dev/tts/1") == 0)
    {
        if (getConfig()->emulateDriveboard == 0 && getConfig()->emulateMotionboard == 0 && getConfig()->emulateHW210CardReader == 0 &&
            getConfig()->emulateTouchscreen == 0)
            return _open(getConfig()->serial2Path, flags, mode);

        if (hooks[SERIAL1] != -1 && getConfig()->emulateHW210CardReader && gId != R_TUNED)
        {
            return hooks[SERIAL1];
        }

        hooks[SERIAL1] = _open(HOOK_FILE_NAME, flags, mode);
        log_warn("SERIAL1 opened %d\n", hooks[SERIAL1]);

        if (getConfig()->emulateHW210CardReader == 1)
            cardReaderSetFd(1, hooks[SERIAL1], getConfig()->cardFile2);

        return hooks[SERIAL1];
    }

    if (strcmp(pathname, "/var/tmp/warning") == 0)
    {
        return _open("warning", flags, mode);
    }

    if (strncmp(pathname, "/tmp/", 5) == 0)
    {
        struct stat info;
        if (!(stat("./tmp", &info) == 0 && (info.st_mode & S_IFDIR)))
        {
            mkdir("tmp", 0777);
        }
        return _open(pathname + 1, flags, mode);
    }

    if (strcmp(pathname, "/proc/bus/pci/01/00.0") == 0)
    {
        hooks[PCI_CARD_000] = _open(HOOK_FILE_NAME, flags, mode);
        return hooks[PCI_CARD_000];
    }

    // printf("Open %s\n", pathname);

    return _open(pathname, flags, mode);
}

/**
 * @brief Hook for the open64 function.
 *
 * This function intercepts calls to open64 and redirects them to different
 * files or devices based on the pathname.
 * @param pathname The path to open.
 */
int open64(const char *pathname, int flags, ...)
{
    va_list args;
    va_start(args, flags);
    int mode = va_arg(args, int);
    va_end(args);

    return open(pathname, flags, mode);
}

/**
 * @brief Hook for the fopen function.
 *
 * This function intercepts calls to fopen and redirects them to different
 * files based on the pathname.
 * @param pathname The path to open.
 */
FILE *fopen(const char *restrict pathname, const char *restrict mode)
{
    FILE *(*_fopen)(const char *restrict pathname, const char *restrict mode) = dlsym(RTLD_NEXT, "fopen");

    if (strcmp(pathname, "/proc/net/route") == 0)
    {
        return NULL;
    }

    if (strcmp(pathname, "/root/lindbergrc") == 0)
    {
        return _fopen("lindbergrc", mode);
    }

    if ((strcmp(pathname, "/usr/lib/boot/logo.tga") == 0))
    {
        if (!getConfig()->disableBuiltinLogos)
        {
            fileRead[FILE_LOGO_TGA] = 0;
            fileHooks[FILE_LOGO_TGA] = _fopen(HOOK_FILE_NAME, mode);
            return fileHooks[FILE_LOGO_TGA];
        }
        else
        {
            return _fopen("logo.tga", mode);
        }
    }

    if (strcmp(pathname, "/usr/lib/boot/logo_red.tga") == 0)
    {
        if (!getConfig()->disableBuiltinLogos)
        {
            fileRead[FILE_LOGO_TGA] = 1;
            fileHooks[FILE_LOGO_TGA] = _fopen(HOOK_FILE_NAME, mode);
            return fileHooks[FILE_LOGO_TGA];
        }
        else
        {
            return _fopen("logo_red.tga", mode);
        }
    }

    if (strcmp(pathname, "/usr/lib/boot/LucidaConsole_12.tga") == 0)
    {
        if (!getConfig()->disableBuiltinFont)
        {
            fileRead[FILE_FONT_TGA] = 0;
            fileHooks[FILE_FONT_TGA] = _fopen(HOOK_FILE_NAME, mode);
            return fileHooks[FILE_FONT_TGA];
        }
        else
        {
            return _fopen("LucidaConsole_12.tga", mode);
        }
    }

    if (strcmp(pathname, "/usr/lib/boot/LucidaConsole_12.abc") == 0)
    {
        if (!getConfig()->disableBuiltinFont)
        {
            fileRead[FILE_FONT_ABC] = 0;
            fileHooks[FILE_FONT_ABC] = _fopen(HOOK_FILE_NAME, mode);
            return fileHooks[FILE_FONT_ABC];
        }
        else
        {
            return _fopen("LucidaConsole_12.abc", mode);
        }
    }

    if (strcmp(pathname, "/proc/cpuinfo") == 0)
    {
        fileRead[CPUINFO] = 0;
        fileHooks[CPUINFO] = _fopen(HOOK_FILE_NAME, mode);
        return fileHooks[CPUINFO];
    }

    if (strcmp(pathname, "/usr/lib/boot/SEGA_KakuGothic-DB-Roman_12.tga") == 0)
    {
        if (!getConfig()->disableBuiltinFont)
        {
            fileRead[FILE_FONT_TGA] = 0;
            fileHooks[FILE_FONT_TGA] = _fopen(HOOK_FILE_NAME, mode);
            return fileHooks[FILE_FONT_TGA];
        }
        else
        {
            return _fopen("SEGA_KakuGothic-DB-Roman_12.tga", mode);
        }
    }

    if (strcmp(pathname, "/usr/lib/boot/SEGA_KakuGothic-DB-Roman_12.abc") == 0)
    {
        if (!getConfig()->disableBuiltinFont)
        {
            fileRead[FILE_FONT_ABC] = 0;
            fileHooks[FILE_FONT_ABC] = _fopen(HOOK_FILE_NAME, mode);
            return fileHooks[FILE_FONT_ABC];
        }
        else
        {
            return _fopen("SEGA_KakuGothic-DB-Roman_12.abc", mode);
        }
    }

    if (strcmp(pathname, "/proc/bus/pci/00/1f.0") == 0)
    {
        fileRead[PCI_CARD_1F0] = 0;
        fileHooks[PCI_CARD_1F0] = _fopen(HOOK_FILE_NAME, mode);
        return fileHooks[PCI_CARD_1F0];
    }

    if (strcmp(pathname, "/var/tmp/warning") == 0)
    {
        return _fopen("warning", "wb");
    }

    char *newPathname;
    if ((newPathname = strstr(pathname, "/home/disk0")) != NULL)
    {
        memmove(newPathname + 2, newPathname + 11, strlen(newPathname + 11) + 1);
        memcpy(newPathname, "..", 2);
        pathname = newPathname;
    }

    // Fix for Outrun high scores
    if ((newPathname = strstr(pathname, "/home/disk1")) != NULL && (gGrp == GROUP_OUTRUN || gGrp == GROUP_OUTRUN_TEST))
    {
        pathname = newPathname + 12;
    }

    // This forces LGJ games and ID games to not use the pre-compiled shaders.
    if ((strstr(pathname, "asm_lbg") != NULL) || (strstr(pathname, "asm_gl") != NULL))
    {
        return 0;
    }

    if (cachedShaderFilesLoaded)
    {
        void *addr = __builtin_return_address(0);
        Dl_info info;
        if (!dladdr(addr, &info))
        {
            printf("dladdr failed\n");
            exit(1);
        }
        int idx;
        if ((strcmp(info.dli_fname, "libstdc++.so.5") != 0) && (shaderFileInList(pathname, &idx)))
        {
            if (fileHooks[FILE_RW1] == NULL)
            {
                fileRead[FILE_RW1] = idx;
                fileHooks[FILE_RW1] = _fopen(pathname, mode);
                return fileHooks[FILE_RW1];
            }
            else if (fileHooks[FILE_RW2] == NULL)
            {
                fileRead[FILE_RW2] = idx;
                fileHooks[FILE_RW2] = _fopen(pathname, mode);
                return fileHooks[FILE_RW2];
            }
            else
            {
                printf("Error intercepting fopen.\n");
                exit(1);
            }
        }
    }

    if (strncmp(pathname, "/tmp/", 5) == 0 && gGrp == GROUP_ID5)
    {
        return fopen(pathname + 1, mode);
    }

    if (gId == PRIMEVAL_HUNT)
    {
        if (strstr(pathname, "/data/lua/texture/start_stage") != NULL)
            phShowCursorInGame = true;
        else if (strstr(pathname, "/data/texture/weapon_select/") != NULL)
            phShowCursorInGame = false;
        else if (strstr(pathname, "/data/texture/stage_select/") != NULL)
            phShowCursorInGame = false;
        else if (strstr(pathname, "/data/texture/name_entry/") != NULL)
            phShowCursorInGame = false;
        else if (strstr(pathname, "/data/texture/game_end/") != NULL)
            phShowCursorInGame = false;
        else if (strstr(pathname, "/data/lua/stage/bonus_0") != NULL)
            phShowCursorInGame = true;
    }
    return _fopen(pathname, mode);
}

/**
 * @brief Hook for the fopen64 function.
 *
 * This function intercepts calls to fopen64 and redirects them to different
 * files based on the pathname.
 * @param pathname The path to open.
 */
FILE *fopen64(const char *pathname, const char *mode)
{
    FILE *(*_fopen64)(const char *restrict pathname, const char *restrict mode) = dlsym(RTLD_NEXT, "fopen64");

    if (strcmp(pathname, "/proc/sys/kernel/osrelease") == 0)
    {
        EmulatorConfig *config = getConfig();
        fileRead[OSRELEASE] = 0;
        fileHooks[OSRELEASE] = _fopen64(HOOK_FILE_NAME, mode);
        return fileHooks[OSRELEASE];
    }

    if (strcmp(pathname, "/usr/lib/boot/logo_red.tga") == 0)
    {
        if (!getConfig()->disableBuiltinLogos)
        {
            fileRead[FILE_LOGO_TGA] = 1;
            fileHooks[FILE_LOGO_TGA] = _fopen64(HOOK_FILE_NAME, mode);
            return fileHooks[FILE_LOGO_TGA];
        }
        else
        {
            return _fopen64("logo_red.tga", mode);
        }
    }

    if (strcmp(pathname, "/usr/lib/boot/logo.tga") == 0)
    {
        if (!getConfig()->disableBuiltinLogos)
        {
            fileRead[FILE_LOGO_TGA] = 0;
            fileHooks[FILE_LOGO_TGA] = _fopen64(HOOK_FILE_NAME, mode);
            return fileHooks[FILE_LOGO_TGA];
        }
        else
        {
            return _fopen64("logo.tga", mode);
        }
    }

    if (strcmp(pathname, "/usr/lib/boot/SEGA_KakuGothic-DB-Roman_12.tga") == 0)
    {
        if (!getConfig()->disableBuiltinFont)
        {
            fileRead[FILE_FONT_TGA] = 0;
            fileHooks[FILE_FONT_TGA] = _fopen64(HOOK_FILE_NAME, mode);
            return fileHooks[FILE_FONT_TGA];
        }
        else
        {
            return _fopen64("SEGA_KakuGothic-DB-Roman_12.tga", mode);
        }
    }

    if (strcmp(pathname, "/usr/lib/boot/SEGA_KakuGothic-DB-Roman_12.abc") == 0)
    {
        if (!getConfig()->disableBuiltinFont)
        {
            fileRead[FILE_FONT_ABC] = 0;
            fileHooks[FILE_FONT_ABC] = _fopen64(HOOK_FILE_NAME, mode);
            return fileHooks[FILE_FONT_ABC];
        }
        else
        {
            return _fopen64("SEGA_KakuGothic-DB-Roman_12.abc", mode);
        }
    }

    int idx;

    if (gGrp == GROUP_HUMMER)
    {
        if (shaderFileInList(pathname, &idx))
        {
            hummerExtremeShaderFileIndex = idx;
        }
        return _fopen64(pathname, mode);
    }

    switch (gId)
    {
        case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA:
        case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB:
        case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000:
            if (getConfig()->GPUVendor != NVIDIA_GPU && getConfig()->GPUVendor != ATI_GPU)
            {
                char *filename = basename((char *)pathname);
                if (strstr(filename, "light_") || strstr(filename, "glow_"))
                {
                    char *start = strchr(filename, '_') + 1;
                    char *end = strstr(filename, ".txt");
                    strncpy(vf5StageNameAbbr, start, end - start);
                    vf5StageNameAbbr[end - start] = '\0';
                }
            }
    }
    return _fopen64(pathname, mode);
}

/**
 * @brief Hook for the fclose function.
 *
 * This function intercepts calls to fclose and performs cleanup operations
 * for the file hooks.
 * @param stream The file stream to close.
 */
int fclose(FILE *stream)
{
    int (*_fclose)(FILE *stream) = dlsym(RTLD_NEXT, "fclose");
    for (int i = 0; i < 9; i++)
    {
        if (fileHooks[i] == stream)
        {
            int r = _fclose(stream);
            fileHooks[i] = NULL;
            fileRead[i] = 0;
            if (stream == fileHooks[FILE_FONT_ABC])
                fontABCidx = 0;
            if (stream == fileHooks[FILE_FONT_TGA])
                fontTGAidx = 0;
            if (stream == fileHooks[FILE_LOGO_TGA])
                logoTGAidx = 0;
            return r;
        }
    }
    return _fclose(stream);
}

/**
 * @brief Hook for the openat function.
 *
 * This function intercepts calls to openat and redirects them to different
 * files or devices based on the pathname.
 * @param pathname The path to open.
 */
int openat(int dirfd, const char *pathname, int flags, ...)
{
    int (*_openat)(int dirfd, const char *pathname, int flags) = dlsym(RTLD_NEXT, "openat");
    // printf("openat %s\n", pathname);

    if (strcmp(pathname, "/dev/ttyS0") == 0 || strcmp(pathname, "/dev/ttyS1") == 0 || strcmp(pathname, "/dev/tts/0") == 0 ||
        strcmp(pathname, "/dev/tts/1") == 0)
    {
        return open(pathname, flags);
    }

    return _openat(dirfd, pathname, flags);
}

/**
 * @brief Hook for the close function.
 *
 * This function intercepts calls to close and performs cleanup operations
 * for the file hooks.
 * @param fd The file descriptor to close.
 */
int close(int fd)
{
    int (*_close)(int fd) = dlsym(RTLD_NEXT, "close");

    for (int i = 0; i < (sizeof hooks / sizeof hooks[0]); i++)
    {
        if (hooks[i] == fd)
        {
            hooks[i] = -1;
            return 0;
        }
    }

    return _close(fd);
}

/**
 * @brief Hook for the fgets function.
 *
 * This function intercepts calls to fgets and redirects them to different
 * files based on the stream.
 * @param stream The file stream to read from.
 */
char *fgets(char *str, int n, FILE *stream)
{
    char *(*_fgets)(char *str, int n, FILE *stream) = dlsym(RTLD_NEXT, "fgets");

    if (stream == fileHooks[OSRELEASE])
    {
        char *contents = "mvl";
        strcpy(str, contents);
        return str;
    }

    // This currently doesn't work
    if (stream == fileHooks[CPUINFO])
    {
        char contents[4][256];

        // Pentium 4 HT 3.0E : Prescott 3.0GHz L2 1Mo (SL8JZ, SL7L4, SL7E4, SL88J, SL79L, SL7KB, SL7PM)
        strcpy(contents[0], "processor	: 0");
        strcpy(contents[1], "vendor_id	: GenuineIntel");
        strcpy(contents[2], "model		: 142");
        strcpy(contents[3], "model name	: Intel(R) Pentium(R) CPU 3.00GHz");

        // Celeron D 335 : 2.8GHz NetBurst Prescott-256 (SL8HM, SL7NW, SL7VZ, SL7TJ, SL7DM, SL7L2, SL7C7) si 478 ?
        if (getConfig()->lindberghColour == RED || getConfig()->lindberghColour == REDEX)
            strcpy(contents[3], "model name	: Intel(R) Celeron(R) CPU 2.80GHz");

        if (fileRead[CPUINFO] == 4)
            return NULL;

        strcpy(str, contents[fileRead[CPUINFO]++]);
        return str;
    }

    return _fgets(str, n, stream);
}

/**
 * @brief Hook for the read function.
 *
 * This function intercepts calls to read and redirects them to different
 * files or devices based on the file descriptor.
 * @param fd The file descriptor to read from.
 */
ssize_t read(int fd, void *buf, size_t count)
{
    int (*_read)(int fd, void *buf, size_t count) = dlsym(RTLD_NEXT, "read");
    void *addr = __builtin_return_address(0);
    if (fd == hooks[BASEBOARD])
    {
        return baseboardRead(fd, buf, count);
    }

    if (fd == hooks[SERIAL0] && getConfig()->emulateRideboard)
    {
        return rideboardRead(fd, buf, count);
    }

    if (fd == hooks[SERIAL0] && getConfig()->emulateDriveboard)
    {
        return driveboardRead(fd, buf, count);
    }

    if ((fd == hooks[SERIAL0] || fd == hooks[SERIAL1]) && getConfig()->emulateHW210CardReader)
    {
        return cardReaderRead(fd, buf, count);
    }

    if (fd == hooks[SERIAL0] || fd == hooks[SERIAL1])
    {
        if (gId == PRIMEVAL_HUNT && getConfig()->emulateTouchscreen == 1)
        {
            phRead(fd, buf, count);
            return 1;
        }
        return -1;
    }

    if (fd == hooks[PCI_CARD_000])
    {
        memcpy(buf, pci_000, count);
        return count;
    }

    return _read(fd, buf, count);
}

/**
 * @brief Hook for the fread function.
 *
 * This function intercepts calls to fread and redirects them to different
 * files based on the stream.
 * @param stream The file stream to read from.
 */
size_t fread(void *buf, size_t size, size_t count, FILE *stream)
{
    size_t (*_fread)(void *buf, size_t size, size_t count, FILE *stream) = dlsym(RTLD_NEXT, "fread");

    if (stream == fileHooks[PCI_CARD_1F0])
    {
        memcpy(buf, pci_1f0, size * count);
        return size * count;
    }

    if (stream == fileHooks[FILE_RW1])
    {
        return freadReplace(buf, size, count, fileRead[FILE_RW1]);
    }

    if (stream == fileHooks[FILE_RW2])
    {
        return freadReplace(buf, size, count, fileRead[FILE_RW2]);
    }

    if (stream == fileHooks[FILE_FONT_ABC])
    {
        memcpy(buf, fontABC + fontABCidx, size * count);
        fontABCidx += (size * count);
        return size * count;
    }

    if (stream == fileHooks[FILE_FONT_TGA])
    {
        memcpy(buf, fontTGA + fontTGAidx, size * count);
        fontTGAidx += (size * count);
        return size * count;
    }

    if (stream == fileHooks[FILE_LOGO_TGA])
    {
        if (fileRead[FILE_LOGO_TGA] == 0)
            memcpy(buf, logoLL + logoTGAidx, size * count);
        else
            memcpy(buf, logoLLRed + logoTGAidx, size * count);
        logoTGAidx += (size * count);
        return size * count;
    }

    return _fread(buf, size, count, stream);
}

/**
 * @brief Hook for the ftell function.
 *
 * This function intercepts calls to ftell and redirects them to different
 * files based on the stream.
 * @param stream The file stream to get the position from.
 */
long int ftell(FILE *stream)
{
    long int (*_ftell)(FILE *stream) = dlsym(RTLD_NEXT, "ftell");

    if (stream == fileHooks[FILE_RW1])
    {
        return ftellGetShaderSize(fileRead[FILE_RW1]);
    }
    if (stream == fileHooks[FILE_RW2])
    {
        return ftellGetShaderSize(fileRead[FILE_RW2]);
    }

    if (stream == fileHooks[FILE_FONT_ABC])
    {
        return fontABClen;
    }

    return _ftell(stream);
}

/**
 * @brief Hook for the fseek function.
 *
 * This function intercepts calls to fseek and redirects them to different
 * files based on the stream.
 * @param stream The file stream to seek in.
 */
int fseek(FILE *stream, long int offset, int whence)
{
    int (*_fseek)(FILE *stream, long int offset, int whence) = dlsym(RTLD_NEXT, "fseek");

    if (stream == fileHooks[FILE_FONT_ABC])
    {
        switch (whence)
        {
            case SEEK_CUR:
                break;
            case SEEK_SET:
                fontABCidx = 0;
                break;
            case SEEK_END:
                fontABCidx = fontABClen;
                break;
        }
        return fontABCidx;
    }

    return _fseek(stream, offset, whence);
}

/**
 * @brief Hook for the rewind function.
 *
 * This function intercepts calls to rewind and redirects them to different
 * files based on the stream.
 * @param stream The file stream to rewind.
 */
void rewind(FILE *stream)
{
    void (*_rewind)(FILE *stream) = dlsym(RTLD_NEXT, "rewind");

    if (stream == fileHooks[FILE_FONT_ABC])
    {
        fontABCidx = 0;
        return;
    }

    _rewind(stream);
}

/**
 * @brief Hook for the write function.
 *
 * This function intercepts calls to write and redirects them to different
 * files or devices based on the file descriptor.
 * @param fd The file descriptor to write to.
 */
ssize_t write(int fd, const void *buf, size_t count)
{
    int (*_write)(int fd, const void *buf, size_t count) = dlsym(RTLD_NEXT, "write");
    void *addr = __builtin_return_address(0);
    if (fd == hooks[BASEBOARD])
    {
        return baseboardWrite(fd, buf, count);
    }

    if (fd == hooks[SERIAL0] && getConfig()->emulateRideboard)
    {
        return rideboardWrite(fd, buf, count);
    }

    if (fd == hooks[SERIAL0] && getConfig()->emulateDriveboard)
    {
        return driveboardWrite(fd, buf, count);
    }

    if (fd == hooks[SERIAL1] && getConfig()->emulateDriveboard)
    {
        if (gGrp != GROUP_OUTRUN && gGrp != GROUP_OUTRUN_TEST && gId != R_TUNED)
            return driveboardWrite(fd, buf, count);
    }

    if ((fd == hooks[SERIAL0] || fd == hooks[SERIAL1]) && getConfig()->emulateHW210CardReader)
    {
        return cardReaderWrite(fd, buf, count);
    }

    return _write(fd, buf, count);
}

/**
 * @brief Hook for the ioctl function.
 *
 * This function intercepts calls to ioctl and redirects them to different
 * devices based on the file descriptor.
 * @param fd The file descriptor to perform the ioctl on.
 */
int ioctl(int fd, unsigned long int request, ...)
{
    va_list args;
    va_start(args, request);
    void *argp = va_arg(args, void *);
    va_end(args);

    int (*_ioctl)(int fd, int request, void *data) = dlsym(RTLD_NEXT, "ioctl");

    if (fd == hooks[EEPROM])
    {
        if (request == 0xC04064A0)
            return _ioctl(fd, request, argp);
        return eepromIoctl(fd, request, argp);
    }

    if (fd == hooks[BASEBOARD])
    {
        return baseboardIoctl(fd, request, argp);
    }

    if (fd == hooks[SERIAL0] || fd == hooks[SERIAL1])
    {
        if (request == 0x541b && (gId == R_TUNED) && fd == hooks[SERIAL1])
        {
            uint8_t d = 1;
            memcpy(argp, &d, sizeof(uint8_t));
        }
        return 0;
    }

    // Replace "eth0" with your interface name set in the config file.
    if ((gGrp == GROUP_ID4_EXP || gId == INITIALD_5_EXP_20 || gId == INITIALD_5_EXP_20A ) &&
        (request == SIOCGIFFLAGS || request == SIOCGIFADDR) && getConfig()->enableNetworkPatches && strcmp(getConfig()->nicName, "") != 0)
    {
        struct ifreq *ifr = (struct ifreq *)argp;
        strncpy(ifr->ifr_name, getConfig()->nicName, IFNAMSIZ);
    }

    return _ioctl(fd, request, argp);
}

/**
 * @brief Hook for the tcgetattr function.
 *
 * This function intercepts calls to tcgetattr and redirects them to different
 * devices based on the file descriptor.
 * @param fd The file descriptor to get the attributes from.
 */
int tcgetattr(int fd, struct termios *termios_p)
{
    int (*_tcgetattr)(int fd, struct termios *termios_p) = dlsym(RTLD_NEXT, "tcgetattr");

    if (fd == hooks[SERIAL0] && getConfig()->emulateDriveboard == 1)
        return 0;

    return _tcgetattr(fd, termios_p);
}

/**
 * @brief Hook for the tcsetattr function.
 *
 * This function intercepts calls to tcsetattr and redirects them to different
 * devices based on the file descriptor.
 * @param fd The file descriptor to set the attributes on.
 */
int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
    int (*_tcsetattr)(int fd, int optional_actions, const struct termios *termios_p) = dlsym(RTLD_NEXT, "tcsetattr");

    if (fd == hooks[SERIAL0] && getConfig()->emulateDriveboard == 1)
        return 0;

    return _tcsetattr(fd, optional_actions, termios_p);
}

/**
 * @brief Hook for the cfgetispeed function.
 *
 * This function intercepts calls to cfgetispeed and redirects them to different
 * devices based on the file descriptor.
 * @param termios_p The termios structure to get the input speed from.
 */
speed_t cfgetispeed(const struct termios *termios_p)
{
    speed_t (*_cfgetispeed)(const struct termios *termios_p) = dlsym(RTLD_NEXT, "cfgetispeed");

    if (getConfig()->emulateDriveboard == 1)
        return B9600;

    return _cfgetispeed(termios_p);
}

/**
 * @brief Hook for the cfgetospeed function.
 *
 * This function intercepts calls to cfgetospeed and redirects them to different
 * devices based on the file descriptor.
 * @param termios_p The termios structure to get the output speed from.
 */
speed_t cfgetospeed(const struct termios *termios_p)
{
    speed_t (*_cfgetospeed)(const struct termios *termios_p) = dlsym(RTLD_NEXT, "cfgetospeed");

    if (getConfig()->emulateDriveboard == 1)
        return B9600;

    return _cfgetospeed(termios_p);
}

/**
 * @brief Hook for the cfsetispeed function.
 *
 * This function intercepts calls to cfsetispeed and redirects them to different
 * devices based on the file descriptor.
 * @param termios_p The termios structure to set the input speed on.
 */
int cfsetispeed(struct termios *termios_p, speed_t speed)
{
    int (*_cfsetispeed)(struct termios *termios_p, speed_t speed) = dlsym(RTLD_NEXT, "cfsetispeed");

    if (getConfig()->emulateDriveboard == 1)
        return 0;

    return _cfsetispeed(termios_p, speed);
}

/**
 * @brief Hook for the cfsetospeed function.
 *
 * This function intercepts calls to cfsetospeed and redirects them to different
 * devices based on the file descriptor.
 * @param termios_p The termios structure to set the output speed on.
 */
int cfsetospeed(struct termios *termios_p, speed_t speed)
{
    int (*_cfsetospeed)(struct termios *termios_p, speed_t speed) = dlsym(RTLD_NEXT, "cfsetospeed");

    if (getConfig()->emulateDriveboard == 1)
        return 0;

    return _cfsetospeed(termios_p, speed);
}

/**
 * @brief Hook for the select function.
 *
 * This function intercepts calls to select and redirects them to different
 * devices based on the file descriptor.c7 45 c8 65 74 68 30
 * @param nfds The highest file descriptor number plus one.
 * @param readfds The file descriptors to check for readability.
 */
int select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds, struct timeval *restrict timeout)
{
    int (*_select)(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds,
                   struct timeval *restrict timeout) = dlsym(RTLD_NEXT, "select");

    if (readfds != NULL && FD_ISSET(hooks[BASEBOARD], readfds))
    {
        return baseboardSelect(nfds, readfds, writefds, exceptfds, timeout);
    }

    if (writefds != NULL && FD_ISSET(hooks[BASEBOARD], writefds))
    {
        return baseboardSelect(nfds, readfds, writefds, exceptfds, timeout);
    }

    if ((getConfig()->emulateHW210CardReader == 1 || getConfig()->emulateDriveboard == 1) &&
        (gGrp != GROUP_ID5 && gGrp != GROUP_ID4_EXP && gGrp != GROUP_ID4_JAP))
    {
        return 1;
    }

    return _select(nfds, readfds, writefds, exceptfds, timeout);
}

/**
 * @brief Hook for the system function.
 *
 * This function intercepts calls to system and redirects them to different
 * commands based on the command string.
 * @param command The command to execute.
 */
int system(const char *command)
{
    int (*_system)(const char *command) = dlsym(RTLD_NEXT, "system");

    if (strcmp(command, "lsmod | grep basebd > /dev/null") == 0)
        return 0;

    if (strcmp(command, "cd /tmp/segaboot > /dev/null") == 0)
        return system("cd tmp/segaboot > /dev/null");

    if (strcmp(command, "mkdir /tmp/segaboot > /dev/null") == 0)
        return system("mkdir tmp/segaboot > /dev/null");

    if (strcmp(command, "lspci | grep \"Multimedia audio controller: %Creative\" > /dev/null") == 0)
        return 0;

    if (strcmp(command, "lsmod | grep ctaud") == 0)
        return 0;

    if (strcmp(command, "lspci | grep MPC8272 > /dev/null") == 0)
        return 0;

    if (strcmp(command, "uname -r | grep mvl") == 0)
        return 0;

    if (strstr(command, "hwclock") != NULL)
        return 0;

    if (strstr(command, "losetup") != NULL)
        return 0;

    if (strstr(command, "check_ip.sh") != NULL)
        return 0;

    if (strcmp(command, "ifconfig eth0 10.0.0.1 netmask 255.255.255.0") == 0)
        return 0;

    if (strcmp(command, "ifconfig eth0 10.0.0.2 netmask 255.255.255.0") == 0)
        return 0;

    if (strcmp(command, "touch /tmp/segaboot/test") == 0)
        command = "touch tmp/segaboot/test";

    return _system(command);
}

/**
 * @brief Hook for the strncpy function.
 *
 * This function intercepts calls to strncpy and redirects them to different
 * strings based on the game ID.
 * @param dest The destination string.
 */
char *strncpy(char *dest, const char *src, size_t n)
{
    char *(*_strncpy)(char *dest, const char *src, size_t n) = dlsym(RTLD_NEXT, "strncpy");

    switch (gId)
    {
        case HARLEY_DAVIDSON:
        case RAMBO:
        case RAMBO_CHINA:
        case THE_HOUSE_OF_THE_DEAD_EX:
        case TOO_SPICY:
            if (getConfig()->GPUVendor != NVIDIA_GPU &&
                (strstr(src, "../fs/compiledshader") != NULL || strstr(src, "../fs/compiled") != NULL))
                return _strncpy(dest, "../fs/compiledmesa", n);
            break;
        case QUIZ_AXA:
        case QUIZ_AXA_LIVE:
            if (getConfig()->GPUVendor != NVIDIA_GPU && strstr(src, "../../data/compiledshader") != NULL)
                return _strncpy(dest, "../../data/compiledmesa", n);
            break;
    }
    return _strncpy(dest, src, n);
}

/**
 * @brief Hook for the iopl function.
 *
 * This function intercepts calls to iopl and always returns 0.
 * @param level The I/O privilege level to set.
 * @return Always returns 0.
 */
int iopl(int level)
{
    return 0;
}

/**
 * Hook for the only function provided by kswapapi.so
 * @param p No idea this gets discarded
 */
void kswap_collect(void *p)
{
    return;
}

/**
 * @brief Hook for the powf function.
 *
 * This function intercepts calls to powf and redirects them to pow.
 * @param base The base number.
 * @param exponent The exponent.
 */
float powf(float base, float exponent)
{
    return (float)pow((double)base, (double)exponent);
}

/**
 * @brief Callback function for dl_iterate_phdr.
 *
 * This function is called for each program header in the loaded shared
 * objects. It is used to calculate the CRC32 of the ELF file.
 * @param info Information about the program header.
 */
static int callback(struct dl_phdr_info *info, size_t size, void *data)
{
    if ((info->dlpi_phnum >= 3) && (info->dlpi_phdr[2].p_type == PT_LOAD) && (info->dlpi_phdr[2].p_flags == 5))
    {
        elf_crc = getCrc32((void *)(size_t)(info->dlpi_addr + info->dlpi_phdr[2].p_vaddr + 10), 0x4000);
    }
    return 1;
}

/**
 * @brief Hook for the setenv function.
 *
 * This function intercepts calls to setenv and prevents the game from
 * changing the DISPLAY environment variable.
 * @param name The name of the environment variable.
 */
int setenv(const char *name, const char *value, int overwrite)
{
    int (*_setenv)(const char *name, const char *value, int overwrite) = dlsym(RTLD_NEXT, "setenv");

    if (strcmp(name, "DISPLAY") == 0)
    {
        return 0;
    }

    return _setenv(name, value, overwrite);
}

/**
 * @brief Hook for the getenv function.
 *
 * This function "Fakes" the TEA_DIR environment variable to games that require it to run
 * @param name The name of the environment variable.
 */
char *getenv(const char *name)
{
    char *(*_getenv)(const char *name) = dlsym(RTLD_NEXT, "getenv");

    if (strcmp(name, "TEA_DIR") == 0)
    {
        if (gGrp == GROUP_VT3 || gGrp == GROUP_VT3_TEST || gId == RAMBO || gId == RAMBO_CHINA || gId == TOO_SPICY)
        {
            if (getcwd(envpath, 100) == NULL)
                return "";
            char *ptr = strrchr(envpath, '/');
            if (ptr == NULL)
                return "";
            *ptr = '\0';
            return envpath;
        }
        else
        {
            if (getcwd(envpath, 100) == NULL)
                return "";
            return envpath;
        }
    }

    if (strcmp(name, "__GL_SYNC_TO_VBLANK") == 0)
    {
        return "";
    }
    return _getenv(name);
}

/**
 * @brief Hook for the unsetenv function.
 *
 * This function intercepts calls to unsetenv and prevents the game from
 * unsetting the DISPLAY environment variable.
 * @param name The name of the environment variable.
 */
int unsetenv(const char *name)
{
    int (*_unsetenv)(const char *name) = dlsym(RTLD_NEXT, "unsetenv");

    if (strcmp(name, "DISPLAY") == 0)
    {
        return 0;
    }

    return _unsetenv(name);
}

/**
 * Patches the hardcoded sound card device name
 */
char *__strdup(const char *string)
{
    char *(*___strdup)(const char *string) = dlsym(RTLD_NEXT, "__strdup");
    if (strcmp(string, "plughw:0, 0") == 0)
    {
        return ___strdup("default");
    }
    return ___strdup(string);
}

/**
 * @brief Hook for the localtime_r function.
 *
 * This function intercepts calls to localtime_r and redirects them to different
 * times based on the game ID.
 * @param timep The time to convert.
 */
struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    struct tm *(*_gmtime_r)(const time_t *, struct tm *) = (struct tm * (*)(const time_t *, struct tm *)) dlsym(RTLD_NEXT, "gmtime_r");

    if ((gId == MJ4_REVG || gId == MJ4_EVO) && getConfig()->mj4EnabledAtT == 1)
    {
        time_t target_time = 1735286445;
        struct tm *res = _gmtime_r(&target_time, result);
        return res;
    }
    return _gmtime_r(timep, result);
}

struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
    struct tm *(*_gmtime_r)(const time_t *, struct tm *) = (struct tm * (*)(const time_t *, struct tm *)) dlsym(RTLD_NEXT, "gmtime_r");

    if ((gId == QUIZ_AXA || gId == QUIZ_AXA_LIVE) && getConfig()->mj4EnabledAtT == 1)
    {
        time_t target_time = 1735286445;
        struct tm *res = _gmtime_r(&target_time, result);
        return res;
    }
    return _gmtime_r(timep, result);
}