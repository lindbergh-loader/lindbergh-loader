#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <libgen.h>
#include <time.h>
#endif

#ifndef __i386__
#define __i386__
#endif

#undef __x86_64__
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
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

#include "hook.h"

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
#include "log.h"

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

int hooks[5] = {-1, -1, -1, -1, -1};
FILE *fileHooks[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
int fileRead[6] = {0, 0, 0, 0, 0, 0};
char envpath[100];

uint32_t elf_crc = 0;

extern int hummerExtremeShaderFileIndex;
extern bool cachedShaderFilesLoaded;
extern char vf5StageNameAbbr[5];

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
    uint8_t *code = (uint8_t *)ctx->uc_mcontext.gregs[REG_EIP];

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

void __attribute__((constructor)) hook_init()
{

    // Get offsets of the Game's ELF and calculate CRC32.
    dl_iterate_phdr(callback, NULL);

    // Implement SIGSEGV handler
    struct sigaction act;
    act.sa_sigaction = handleSegfault;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &act, NULL);

    initConfig();

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

    if (getConfig()->GPUVendor == ATI_GPU
        && ((getConfig()->crc32 == LETS_GO_JUNGLE) || (getConfig()->crc32 == LETS_GO_JUNGLE_REVA) ||
        (getConfig()->crc32 == LETS_GO_JUNGLE_SPECIAL) || (getConfig()->crc32 == AFTER_BURNER_CLIMAX) ||
        (getConfig()->crc32 == AFTER_BURNER_CLIMAX_REVA) || (getConfig()->crc32 == AFTER_BURNER_CLIMAX_REVB) ||
        (getConfig()->crc32 == AFTER_BURNER_CLIMAX_SDX) || (getConfig()->crc32 == AFTER_BURNER_CLIMAX_SDX_REVA) ||
        (getConfig()->crc32 == AFTER_BURNER_CLIMAX_SE) || (getConfig()->crc32 == AFTER_BURNER_CLIMAX_SE_REVA) ||
        (getConfig()->crc32 == INITIALD_5_JAP_REVA) || (getConfig()->crc32 == INITIALD_5_JAP_REVF) ||
        (getConfig()->crc32 == INITIALD_5_EXP_30) || (getConfig()->crc32 == INITIALD_5_EXP_40))) {
            printf("WARNING: Game %s is unsupported in AMD GPU with ATI driver\n",getGameName());
    }
    if (getConfig()->lgjRenderWithMesa &&
        ((getConfig()->crc32 == LETS_GO_JUNGLE) || (getConfig()->crc32 == LETS_GO_JUNGLE_REVA) ||
         (getConfig()->crc32 == LETS_GO_JUNGLE_SPECIAL)))
    {
        getConfig()->GPUVendor = AMD_GPU;
    }

    printf("  ################################## \n\n");
}

DIR *opendir(const char *dirname)
{
    DIR *(*_opendir)(const char *dirname) = dlsym(RTLD_NEXT, "opendir");

    int gId = getConfig()->crc32;
    if (gId == INITIALD_5_EXP_30 || gId == INITIALD_5_EXP_40 || gId == INITIALD_5_JAP_REVA ||
        gId == INITIALD_5_JAP_REVF)
    {
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
        if (getConfig()->emulateDriveboard == 0 && getConfig()->emulateRideboard == 0)
            return _open(getConfig()->serial1Path, flags, mode);

        if (hooks[SERIAL0] != -1)
            return -1;

        hooks[SERIAL0] = _open(HOOK_FILE_NAME, flags, mode);
        log_warn("SERIAL0 Opened %d\n", hooks[SERIAL0]);
        return hooks[SERIAL0];
    }

    if (strcmp(pathname, "/dev/ttyS1") == 0 || strcmp(pathname, "/dev/tts/1") == 0)
    {
        if (getConfig()->emulateMotionboard == 0)
            return _open(getConfig()->serial2Path, flags, mode);

        if (hooks[SERIAL1] != -1)
            return -1;

        hooks[SERIAL1] = _open(HOOK_FILE_NAME, flags, mode);
        log_warn("SERIAL1 opened %d\n", hooks[SERIAL1]);
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

    if (strcmp(pathname, "/root/lindbergrc") == 0)
    {
        return _fopen("lindbergrc", mode);
    }

    if ((strcmp(pathname, "/usr/lib/boot/logo.tga") == 0) || (strcmp(pathname, "/usr/lib/boot/logo.tga") == 0))
    {
        return _fopen("logo.tga", mode);
    }

    if (strcmp(pathname, "/usr/lib/boot/LucidaConsole_12.tga") == 0)
    {
        return _fopen("LucidaConsole_12.tga", mode);
    }

    if (strcmp(pathname, "/usr/lib/boot/LucidaConsole_12.abc") == 0)
    {
        return _fopen("LucidaConsole_12.abc", mode);
    }

    if (strcmp(pathname, "/proc/cpuinfo") == 0)
    {
        fileRead[CPUINFO] = 0;
        fileHooks[CPUINFO] = _fopen(HOOK_FILE_NAME, mode);
        return fileHooks[CPUINFO];
    }

    if (strcmp(pathname, "/usr/lib/boot/logo_red.tga") == 0)
    {
        return _fopen("logo_red.tga", mode);
    }
    if (strcmp(pathname, "/usr/lib/boot/SEGA_KakuGothic-DB-Roman_12.tga") == 0)
    {
        return _fopen("SEGA_KakuGothic-DB-Roman_12.tga", mode);
    }

    if (strcmp(pathname, "/usr/lib/boot/SEGA_KakuGothic-DB-Roman_12.abc") == 0)
    {
        return _fopen("SEGA_KakuGothic-DB-Roman_12.abc", mode);
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

    if (strcmp("./../fs/data/cse_credit.bnk", pathname) == 0)
    {
        void *addr = __builtin_return_address(0);
        fileHooks[FILE_HARLEY] = _fopen(pathname, mode);
        return fileHooks[FILE_HARLEY];
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

    int gId = getConfig()->crc32;
    if (gId == INITIALD_5_EXP_30 || gId == INITIALD_5_EXP_40 || gId == INITIALD_5_JAP_REVA ||
        gId == INITIALD_5_JAP_REVF)
    {
        if (strncmp(pathname, "/tmp/", 5) == 0)
        {
            return fopen(pathname + 1, mode);
        }
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
        return _fopen64("logo_red.tga", mode);
    }

    if (strcmp(pathname, "/usr/lib/boot/logo.tga") == 0)
    {
        return _fopen64("logo.tga", mode);
    }

    if (strcmp(pathname, "/usr/lib/boot/SEGA_KakuGothic-DB-Roman_12.tga") == 0)
    {
        return _fopen64("SEGA_KakuGothic-DB-Roman_12.tga", mode);
    }

    if (strcmp(pathname, "/usr/lib/boot/SEGA_KakuGothic-DB-Roman_12.abc") == 0)
    {
        return _fopen64("SEGA_KakuGothic-DB-Roman_12.abc", mode);
    }

    if ((getConfig()->crc32 == HUMMER) || (getConfig()->crc32 == HUMMER_SDLX) ||
        (getConfig()->crc32 == HUMMER_EXTREME) || (getConfig()->crc32 == HUMMER_EXTREME_MDX))
    {
        int idx;
        if (shaderFileInList(pathname, &idx))
        {
            hummerExtremeShaderFileIndex = idx;
        }
    }

    if ((getConfig()->crc32 == VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA || getConfig()->crc32 == VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB ||
         getConfig()->crc32 == VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000) &&
        (getConfig()->GPUVendor != NVIDIA_GPU && getConfig()->GPUVendor != ATI_GPU))
    {
        char *filename = basename((char *)pathname);
        if (strstr(filename, "light_") || strstr(filename, "glow_"))
        {
            printf("Stage: %s\n", filename);
            char *start = strchr(filename, '_') + 1;
            char *end = strstr(filename, ".txt");
            strncpy(vf5StageNameAbbr, start, end - start);
            vf5StageNameAbbr[end - start] = '\0';
        }
    }

    // printf("fopen64 %s\n", pathname);
    return _fopen64(pathname, mode);
}

int fclose(FILE *stream)
{
    int (*_fclose)(FILE *stream) = dlsym(RTLD_NEXT, "fclose");
    for (int i = 0; i < 7; i++)
    {
        if (fileHooks[i] == stream)
        {
            int r = _fclose(stream);
            fileHooks[i] = NULL;
            fileRead[i] = 0;
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

    // If we don't hook the serial just reply with nothing
    if (fd == hooks[SERIAL0] || fd == hooks[SERIAL1])
    {
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

    if (stream == fileHooks[FILE_HARLEY])
    {
        return harleyFreadReplace(buf, size, count, fileHooks[FILE_HARLEY]);
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

    return _ftell(stream);
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
        return driveboardWrite(fd, buf, count);
    }

    return _write(fd, buf, count);
}

int ioctl(int fd, unsigned int request, void *data)
{
    int (*_ioctl)(int fd, int request, void *data) = dlsym(RTLD_NEXT, "ioctl");

    // Attempt to stop access to the ethernet ports
    if ((request == SIOCSIFADDR) || (request == SIOCSIFFLAGS) || (request == SIOCSIFHWADDR) ||
        (request == SIOCSIFHWBROADCAST) || (request == SIOCDELRT) || (request == SIOCADDRT) ||
        (request == SIOCSIFNETMASK))
    {
        errno = ENXIO;
        return -1;
    }

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

    // Just accept any IOCTL on serial ports and ignore it
    if (fd == hooks[SERIAL0] || fd == hooks[SERIAL1])
    {
        return 0;
    }

    return _ioctl(fd, request, data);
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
 * Function to calculate CRC32 checksum in memory.
 */
uint32_t get_crc32(const char *s, size_t n)
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
    int gId = getConfig()->crc32;
    if ((strcmp(name, "TEA_DIR") == 0) &&
        ((gId == VIRTUA_TENNIS_3) || (gId == VIRTUA_TENNIS_3_TEST) || (gId == VIRTUA_TENNIS_3_REVA) ||
         (gId == VIRTUA_TENNIS_3_REVA_TEST) || (gId == VIRTUA_TENNIS_3_REVB) || (gId == VIRTUA_TENNIS_3_REVB_TEST) ||
         (gId == VIRTUA_TENNIS_3_REVC) || (gId == VIRTUA_TENNIS_3_REVC_TEST) || ((gId == RAMBO)) || (gId == TOO_SPICY)))
    {
        if (getcwd(envpath, 100) == NULL)
            return "";
        char *ptr = strrchr(envpath, '/');
        if (ptr == NULL)
            return "";
        *ptr = '\0';
        return envpath;
    }
    else if (strcmp(name, "TEA_DIR") == 0)
    {
        if (getcwd(envpath, 100) == NULL)
            return "";
        return envpath;
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
