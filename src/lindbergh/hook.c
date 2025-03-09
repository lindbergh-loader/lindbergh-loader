#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <libgen.h>
#include <stdint.h>
#include <time.h>
#endif

#ifndef __i386__
#define __i386__
#endif

#undef __x86_64__
#include <arpa/inet.h>
#include <dlfcn.h>
#include <link.h>
#include <linux/sockios.h>
#include <math.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>
#include <signal.h>
#include <ifaddrs.h>
#include <dirent.h>

#include "baseboard.h"
#include "config.h"
#include "driveboard.h"
#include "eeprom.h"
#include "gpuvendor.h"
#include "input.h"
#include "jvs.h"
#include "patch.h"
#include "pcidata.h"
#include "resolution.h"
#include "rideboard.h"
#include "securityboard.h"
#include "shader_patches.h"
#include "fps_limiter.h"
#include "evdevinput.h"
#include "card_reader.h"
#include "touchscreen.h"
#include "log.h"
#include "resources/font.h"
#include "resources/logo.h"

#define HOOK_FILE_NAME "/dev/zero"

#define BASEBOARD 0
#define EEPROM 1
#define SERIAL0 2
#define SERIAL1 3
#define PCI_CARD_000 4

#define CPUINFO 0
#define OSRELEASE 1
#define PCI_CARD_1F0 2
#define FILE_RW1 3
#define FILE_RW2 4
#define FILE_HARLEY 5
#define FILE_FONT_ABC 6
#define FILE_FONT_TGA 7
#define FILE_LOGO_TGA 8
#define ROUTE 9

int hooks[5] = {-1, -1, -1, -1, -1};
FILE *fileHooks[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
int fileRead[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
char envpath[100];

int fontABCidx = 0;
int fontTGAidx = 0;
int logoTGAidx = 0;

uint32_t elf_crc = 0;

uint32_t gId = 0;

extern int hummerExtremeShaderFileIndex;
extern bool cachedShaderFilesLoaded;
extern char vf5StageNameAbbr[5];
bool phShowCursorInGame = false;

extern fps_limit fpsLimit;
Controllers controllers = {0};

static int callback(struct dl_phdr_info *info, size_t size, void *data);

uint16_t basePortAddress = 0xFFFF;

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
    uint8_t *code = (uint8_t *)(uintptr_t)eip_value; // Use uintptr_t to ensure proper alignment

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

char *checkIDlike()
{
    FILE *file = fopen("/etc/os-release", "r");
    if (!file)
    {
        perror("Failed to open /etc/os-release");
        return NULL;
    }

    char line[256];
    char *result = NULL;

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "ID_LIKE=", 8) == 0)
        {
            char *id_like = strchr(line, '=');
            if (id_like)
            {
                result = strdup(id_like);
                break;
            }
        }
    }
    fclose(file);
    return result;
}

void __attribute__((constructor)) hook_init()
{
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

    char *id_like = checkIDlike();
    if (id_like == NULL)
    {
        log_warn("Unable to get system info on current OS");
    }
    else if (strstr(id_like, "debian") == NULL)
    {
        log_warn("Seems like you're not in debian-like system. There might be unexpected issues.");
    }

    if (getConfig()->fpsLimiter == 1)
    {
        fpsLimit.targetFrameTime = 1000000 / getConfig()->fpsTarget;
        fpsLimit.frameEnd = Clock_now();
    }

    getGPUVendor();

    if (initPatch() != 0)
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

    if (getConfig()->emulateCardreader)
    {
        if (initCardReader() != 0)
            exit(1);
    }

    if (initInput() != 0)
        exit(1);

    if (initControllers(&controllers) != 0)
        exit(1);

    securityBoardSetDipResolution(getConfig()->width, getConfig()->height);

    printf("\nSEGA Lindbergh Emulator\nBy the Lindbergh Development Team 2025\n\n");
    printf("  GAME:        %s\n", getGameName());
    printf("  GAME ID:     %s\n", getGameID());
    printf("  DVP:         %s\n", getDVPName());
    printf("  GPU VENDOR:  %s\n", getConfig()->GPUVendorString);

    for (int i = 0; i < controllers.count; i++)
    {
        if (controllers.controller[i].inUse)
        {
            printf("  CONTROLLER: %s\n", controllers.controller[i].name);
        }
    }
    printf("\n");

    switch (gId)
    {
    case LETS_GO_JUNGLE:
    case LETS_GO_JUNGLE_REVA:
    case LETS_GO_JUNGLE_SPECIAL:
    case AFTER_BURNER_CLIMAX:
    case AFTER_BURNER_CLIMAX_REVA:
    case AFTER_BURNER_CLIMAX_REVB:
    case AFTER_BURNER_CLIMAX_SDX:
    case AFTER_BURNER_CLIMAX_SDX_REVA:
    case AFTER_BURNER_CLIMAX_SE:
    case AFTER_BURNER_CLIMAX_SE_REVA:
    case INITIALD_5_JAP_REVA:
    case INITIALD_5_JAP_REVF:
    case INITIALD_5_EXP_20:
    case INITIALD_5_EXP_20A:
        if (getConfig()->GPUVendor == ATI_GPU)
        {
            printf("WARNING: Game %s is unsupported in AMD GPU with ATI driver\n", getGameName());
        }
    }
}

DIR *opendir(const char *dirname)
{
    DIR *(*_opendir)(const char *dirname) = dlsym(RTLD_NEXT, "opendir");

    switch (gId)
    {
    case INITIALD_5_EXP:
    case INITIALD_5_EXP_20:
    case INITIALD_5_EXP_20A:
    case INITIALD_5_JAP_REVA:
    case INITIALD_5_JAP_REVF:
        if (strcmp(dirname, "/tmp/") == 0)
        {
            return _opendir(dirname + 1);
        }
    }
    return _opendir(dirname);
}

int __xstat64(int ver, const char *path, struct stat64 *stat_buf)
{
    int (*___xstat64)(int ver, const char *path, struct stat64 *stat_buf) = dlsym(RTLD_NEXT, "__xstat64");

    if (strcmp("/var/tmp/warning", path) == 0)
    {
        return ___xstat64(ver, "warning", stat_buf);
    }
    return ___xstat64(ver, path, stat_buf);
}

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
        if (getConfig()->emulateDriveboard == 0 && getConfig()->emulateRideboard == 0 && getConfig()->emulateCardreader == 0 &&
            getConfig()->emulateTouchscreen == 0)
            return _open(getConfig()->serial1Path, flags, mode);

        if (hooks[SERIAL0] != -1 && getConfig()->emulateCardreader && gId != R_TUNED)
        {
            return hooks[SERIAL0];
        }

        hooks[SERIAL0] = _open(HOOK_FILE_NAME, flags, mode);
        printf("Warning: SERIAL0 Opened %d\n", hooks[SERIAL0]);

        if (getConfig()->emulateCardreader == 1 && gId != R_TUNED)
            cardReaderSetFd(0, hooks[SERIAL0], getConfig()->cardFile1);

        return hooks[SERIAL0];
    }

    if (strcmp(pathname, "/dev/ttyS1") == 0 || strcmp(pathname, "/dev/tts/1") == 0)
    {
        if (getConfig()->emulateDriveboard == 0 && getConfig()->emulateMotionboard == 0 && getConfig()->emulateCardreader == 0)
            return _open(getConfig()->serial2Path, flags, mode);

        if (hooks[SERIAL1] != -1 && getConfig()->emulateCardreader && gId != R_TUNED)
        {
            return hooks[SERIAL1];
        }

        hooks[SERIAL1] = _open(HOOK_FILE_NAME, flags, mode);
        log_warn("SERIAL1 opened %d\n", hooks[SERIAL1]);

        if (getConfig()->emulateCardreader == 1)
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

int open64(const char *pathname, int flags, ...)
{
    va_list args;
    va_start(args, flags);
    int mode = va_arg(args, int);
    va_end(args);

    return open(pathname, flags, mode);
}

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

    switch (gId)
    {
    case INITIALD_5_EXP:
    case INITIALD_5_EXP_20:
    case INITIALD_5_EXP_20A:
    case INITIALD_5_JAP_REVA:
    case INITIALD_5_JAP_REVF:
        if (strncmp(pathname, "/tmp/", 5) == 0)
        {
            return fopen(pathname + 1, mode);
        }
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

    // printf("Path= %s\n", pathname);

    return _fopen(pathname, mode);
}

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
    switch (gId)
    {
    case HUMMER:
    case HUMMER_SDLX:
    case HUMMER_EXTREME:
    case HUMMER_EXTREME_MDX:
        if (shaderFileInList(pathname, &idx))
        {
            hummerExtremeShaderFileIndex = idx;
        }
        break;
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

    // printf("fopen64 %s\n", pathname);
    return _fopen64(pathname, mode);
}

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
int openat(int dirfd, const char *pathname, int flags, ...)
{
    int (*_openat)(int dirfd, const char *pathname, int flags) = dlsym(RTLD_NEXT, "openat");
    // printf("openat %s\n", pathname);

    if (strcmp(pathname, "/dev/ttyS0") == 0 || strcmp(pathname, "/dev/ttyS1") == 0 ||
        strcmp(pathname, "/dev/tts/0") == 0 || strcmp(pathname, "/dev/tts/1") == 0)
    {
        return open(pathname, flags);
    }

    return _openat(dirfd, pathname, flags);
}

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

ssize_t read(int fd, void *buf, size_t count)
{
    int (*_read)(int fd, void *buf, size_t count) = dlsym(RTLD_NEXT, "read");

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

    if (fd == hooks[SERIAL0] && getConfig()->emulateCardreader)
    {
        return cardReaderRead(fd, buf, count);
    }

    if (fd == hooks[SERIAL1] && getConfig()->emulateCardreader)
    {
        return cardReaderRead(fd, buf, count);
    }

    // If we don't hook the serial just reply with nothing
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

    // if (stream == fileHooks[FILE_HARLEY])
    // {
    //     return harleyFreadReplace(buf, size, count, fileHooks[FILE_HARLEY]);
    // }

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

ssize_t write(int fd, const void *buf, size_t count)
{
    int (*_write)(int fd, const void *buf, size_t count) = dlsym(RTLD_NEXT, "write");

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
        // printf("Write addr: %p\n", addr);
        return driveboardWrite(fd, buf, count);
    }

    if (fd == hooks[SERIAL1] && getConfig()->emulateDriveboard && gId != R_TUNED)
    {
        return driveboardWrite(fd, buf, count);
    }

    if (fd == hooks[SERIAL0] && getConfig()->emulateCardreader)
    {
        return cardReaderWrite(fd, buf, count);
    }

    if (fd == hooks[SERIAL1] && getConfig()->emulateCardreader)
    {
        return cardReaderWrite(fd, buf, count);
    }

    return _write(fd, buf, count);
}

int ioctl(int fd, unsigned int request, void *data)
{
    int (*_ioctl)(int fd, int request, void *data) = dlsym(RTLD_NEXT, "ioctl");

    if (fd == hooks[EEPROM])
    {
        if (request == 0xC04064A0)
            return _ioctl(fd, request, data);
        return eepromIoctl(fd, request, data);
    }

    if (fd == hooks[BASEBOARD])
    {
        return baseboardIoctl(fd, request, data);
    }

    if (fd == hooks[SERIAL0] || fd == hooks[SERIAL1])
    {
        if (request == 0x541b && gId == R_TUNED && fd == hooks[SERIAL1])
        {
            uint8_t d = 1;
            memcpy(data, &d, sizeof(uint8_t));
        }
        return 0;
    }

    return _ioctl(fd, request, data);
}

int tcgetattr(int fd, struct termios *termios_p)
{
    int (*_tcgetattr)(int fd, struct termios *termios_p) = dlsym(RTLD_NEXT, "tcgetattr");

    if (fd == hooks[SERIAL0] && getConfig()->emulateDriveboard == 1)
        return 0;

    return _tcgetattr(fd, termios_p);
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
    int (*_tcsetattr)(int fd, int optional_actions, const struct termios *termios_p) = dlsym(RTLD_NEXT, "tcsetattr");

    if (fd == hooks[SERIAL0] && getConfig()->emulateDriveboard == 1)
        return 0;

    return _tcsetattr(fd, optional_actions, termios_p);
}

speed_t cfgetispeed(const struct termios *termios_p)
{
    speed_t (*_cfgetispeed)(const struct termios *termios_p) = dlsym(RTLD_NEXT, "cfgetispeed");

    if (getConfig()->emulateDriveboard == 1)
        return B9600;

    return _cfgetispeed(termios_p);
}

speed_t cfgetospeed(const struct termios *termios_p)
{
    speed_t (*_cfgetospeed)(const struct termios *termios_p) = dlsym(RTLD_NEXT, "cfgetospeed");

    if (getConfig()->emulateDriveboard == 1)
        return B9600;

    return _cfgetospeed(termios_p);
}

int cfsetispeed(struct termios *termios_p, speed_t speed)
{
    int (*_cfsetispeed)(struct termios *termios_p, speed_t speed) = dlsym(RTLD_NEXT, "cfsetispeed");

    if (getConfig()->emulateDriveboard == 1)
        return 0;

    return _cfsetispeed(termios_p, speed);
}

int cfsetospeed(struct termios *termios_p, speed_t speed)
{
    int (*_cfsetospeed)(struct termios *termios_p, speed_t speed) = dlsym(RTLD_NEXT, "cfsetospeed");

    if (getConfig()->emulateDriveboard == 1)
        return 0;

    return _cfsetospeed(termios_p, speed);
}

int select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds,
           struct timeval *restrict timeout)
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

    if (getConfig()->emulateCardreader == 1 || getConfig()->emulateDriveboard == 1)
    {
        return 1;
    }

    return _select(nfds, readfds, writefds, exceptfds, timeout);
}

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

    return _system(command);
}

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
 * Hook for function used by Primevil
 * @param base The number to raise to the exponent
 * @param exp The exponent to raise the number to
 * @return The result of raising the number to the exponent
 */
float powf(float base, float exponent)
{
    return (float)pow((double)base, (double)exponent);
}

/*
int sem_wait(sem_t *sem)
{
    int (*original_sem_wait)(sem_t * sem) = dlsym(RTLD_NEXT, "sem_wait");
    return 0;
}
*/

int get_machine_ip(struct sockaddr_in *addr)
{
    struct ifaddrs *ifaddr, *ifa;
    char ip_buffer[INET_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return -1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
        {
            if (inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ip_buffer, sizeof(ip_buffer)))
            {
                if (strcmp(ifa->ifa_name, "lo") != 0)
                {
                    addr->sin_addr.s_addr = inet_addr(ip_buffer);
                    freeifaddrs(ifaddr);
                    return 0;
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return -1;
}

/**
 * Hook function used by Harley Davidson to change IPs to localhost
 * Currently does nothing.
 * @param sockfd
 * @param addr
 * @param addrlen
 * @return
 */
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int (*_connect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = dlsym(RTLD_NEXT, "connect");

    struct sockaddr_in *in_pointer = (struct sockaddr_in *)addr;

    // IP addresses to replace
    const char *specific_ips[] = {"192.168.1.1", "192.168.1.5", "192.168.1.9"};
    int num_specific_ips = sizeof(specific_ips) / sizeof(specific_ips[0]);

    if (getConfig()->crc32 == HARLEY_DAVIDSON)
    {
        char *ip_address = inet_ntoa(in_pointer->sin_addr);
        for (int i = 0; i < num_specific_ips; ++i)
        {
            if (strcmp(ip_address, specific_ips[i]) == 0)
            {
                // Change the IP to connect to 127.0.0.1
                in_pointer->sin_addr.s_addr = inet_addr("127.0.0.1");
                if (getConfig()->showDebugMessages)
                {
                    char *some_addr = inet_ntoa(in_pointer->sin_addr);
                    printf("Connecting to %s\n", some_addr);
                }
                break;
            }
        }
    }

    return _connect(sockfd, addr, addrlen);
}

/**
 * Callback function to get the offset and size of the execution program in memory of the ELF we hook to.
 */
static int callback(struct dl_phdr_info *info, size_t size, void *data)
{
    if ((info->dlpi_phnum >= 3) && (info->dlpi_phdr[2].p_type == PT_LOAD) && (info->dlpi_phdr[2].p_flags == 5))
    {
        elf_crc = get_crc32((void *)(size_t)(info->dlpi_addr + info->dlpi_phdr[2].p_vaddr + 10), 0x4000);
    }
    return 1;
}

/**
 * Stop the game changing the DISPLAY environment variable
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
 * Fake the TEA_DIR environment variable to games that require it to run
 */
char *getenv(const char *name)
{
    char *(*_getenv)(const char *name) = dlsym(RTLD_NEXT, "getenv");

    if (strcmp(name, "TEA_DIR") == 0)
    {
        switch (gId)
        {
        case VIRTUA_TENNIS_3:
        case VIRTUA_TENNIS_3_TEST:
        case VIRTUA_TENNIS_3_REVA:
        case VIRTUA_TENNIS_3_REVA_TEST:
        case VIRTUA_TENNIS_3_REVB:
        case VIRTUA_TENNIS_3_REVB_TEST:
        case VIRTUA_TENNIS_3_REVC:
        case VIRTUA_TENNIS_3_REVC_TEST:
        case RAMBO:
        case TOO_SPICY:
        {
            if (getcwd(envpath, 100) == NULL)
                return "";
            char *ptr = strrchr(envpath, '/');
            if (ptr == NULL)
                return "";
            *ptr = '\0';
            return envpath;
        }
        break;
        default:
        {
            if (getcwd(envpath, 100) == NULL)
                return "";
            return envpath;
        }
        }
    }

    if (strcmp(name, "__GL_SYNC_TO_VBLANK") == 0)
    {
        return "";
    }
    return _getenv(name);
}

/**
 * Stop the game unsetting the DISPLAY environment variable
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

struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    struct tm *(*_localtime_r)(const time_t *, struct tm *) =
        (struct tm * (*)(const time_t *, struct tm *)) dlsym(RTLD_NEXT, "localtime_r");

    if ((gId == MJ4_REVG || gId == MJ4_EVO) && getConfig()->mj4EnabledAtT == 1)
    {
        time_t target_time = 1735286445;
        struct tm *res = _localtime_r(&target_time, result);
        return res;
    }
    return _localtime_r(timep, result);
}
