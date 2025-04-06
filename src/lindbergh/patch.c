#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <time.h>
#define GL_GLEXT_PROTOTYPES
#include "patch.h"

#include <GL/glx.h>

#include <dlfcn.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "resolution.h"
#include "securityboard.h"
#include "shader_patches.h"
#include "glxhooks.h"
#include "log.h"
#include "customcursor.h"

extern uint32_t gId;
char elfID[4];
void *amDipswContextAddr;
extern void *glVertex3fPatchRetAddrABC[4];
extern void *glVertex3fNoPatchRetAddrABC[4];
void *hod4CursorHideShowCAVEAddress;

extern void *customCursor;
extern void *phTouchCursor;
extern void *blankCursor;

void setVariable(size_t address, size_t value)
{
    int pagesize = sysconf(_SC_PAGE_SIZE);

    size_t *variable = (size_t *)address;

    void *toModify = (void *)(address - (address % pagesize));

    int prot = mprotect(toModify, pagesize, PROT_EXEC | PROT_WRITE);
    if (prot != 0)
    {
        printf("Error: Cannot unprotect memory region to change variable (%d)\n", prot);
        return;
    }
    *variable = value;
}

void patchMemory(size_t address, char *value)
{
    size_t size = strlen((void *)value);
    if (size % 2 != 0)
    {
        printf("Patch value should be even.\n");
        exit(1);
    }

    char buf[size / 2];
    char tmpchr[3];
    char *p = value;
    for (int i = 0; i < size; i++)
    {
        memcpy(tmpchr, p, 2);
        tmpchr[2] = '\0';
        buf[i] = (int)strtol(tmpchr, NULL, 16);
        p += 2;
    }

    int pagesize = sysconf(_SC_PAGE_SIZE);

    void *toModify = (void *)(address - (address % pagesize));

    int prot = mprotect(toModify, pagesize, PROT_EXEC | PROT_WRITE);
    if (prot != 0)
    {
        printf("Error: Cannot unprotect memory region to change variable (%d)\n", prot);
        return;
    }

    memcpy((void *)address, buf, size / 2);
}

void detourFunction(size_t address, void *function)
{
    int pagesize = sysconf(_SC_PAGE_SIZE);

    void *toModify = (void *)(address - (address % pagesize));

    int prot = mprotect(toModify, pagesize, PROT_EXEC | PROT_WRITE);
    if (prot != 0)
    {
        printf("Error: Cannot detour memory region to change variable (%d)\n", prot);
        return;
    }

    uint32_t jumpAddress = (function - (void *)address) - 5;

    // Build the assembly to make the function jump
    char cave[5] = {0xE9, 0x00, 0x00, 0x00, 0x00};
    cave[4] = (jumpAddress >> (8 * 3)) & 0xFF;
    cave[3] = (jumpAddress >> (8 * 2)) & 0xFF;
    cave[2] = (jumpAddress >> (8 * 1)) & 0xFF;
    cave[1] = (jumpAddress) & 0xFF;

    memcpy((void *)address, cave, 5);
}

void replaceCallAtAddress(size_t address, void *function)
{
    uint32_t offset = function - (void *)(address)-5;

    int pagesize = sysconf(_SC_PAGE_SIZE);

    void *toModify = (void *)(address - (address % pagesize));

    int prot = mprotect(toModify, pagesize, PROT_EXEC | PROT_WRITE);
    if (prot != 0)
    {
        printf("Error: Cannot detour memory region to change variable (%d)\n", prot);
        return;
    }

    uint32_t callAddress = (function - (void *)address) - 5;

    char cave[5] = {0xE8, 0x00, 0x00, 0x00, 0x00};
    cave[4] = (callAddress >> (8 * 3)) & 0xFF;
    cave[3] = (callAddress >> (8 * 2)) & 0xFF;
    cave[2] = (callAddress >> (8 * 1)) & 0xFF;
    cave[1] = (callAddress) & 0xFF;

    memcpy((void *)address, cave, 5);
}

int stubRetZero()
{
    return 0;
}

void stubReturn()
{
    return;
}

int stubRetOne()
{
    return 1;
}

int stubRetMinusOne()
{
    return -1;
}

char stubRetZeroChar()
{
    return 0x00;
}

int amDongleInit()
{
    return 0;
}

int amDongleIsAvailable()
{
    return 1;
}

int amDongleUpdate()
{
    return 0;
}

int amDongleUserInfoEx(int a, int b, char *_arcadeContext)
{
    char *gameID;
    switch (gId)
    {
    case RAMBO:
        memcpy(_arcadeContext, "SBQL", 4);
        break;
    case HUMMER_EXTREME:
    case HUMMER_EXTREME_MDX:
        memcpy(_arcadeContext, "SBST", 4);
        break;
    case GHOST_SQUAD_EVOLUTION:
        memcpy(_arcadeContext, getConfig()->gameID, 4);
        break;
    case INITIALD_4_REVA:
    case INITIALD_4_REVB:
    case INITIALD_4_REVC:
    case INITIALD_4_REVD:
    case INITIALD_4_REVG:
    case INITIALD_4_EXP_REVB:
    case INITIALD_4_EXP_REVC:
    case INITIALD_4_EXP_REVD:
    case INITIALD_5_JAP_REVA:
    case INITIALD_5_JAP_REVF:
    case INITIALD_5_EXP:
    case INITIALD_5_EXP_20:
    case INITIALD_5_EXP_20A:
        memcpy(_arcadeContext, elfID, 4);
        break;
    default:
        break;
    }
    return 0;
}

int amDipswInit()
{
    uint32_t *amLibContext = (uint32_t *)amDipswContextAddr;
    *amLibContext = 1;
    return 0;
}

int amDipswExit()
{
    return 0;
}

int amDipswSetLed()
{
    return 0;
}

int amDongleIsDevelop()
{
    return 1;
}

int amDongleDecryptEx()
{
    return 0;
}

int amDongleSetIv()
{
    return 0;
}

void print_binary(unsigned int number)
{
    if (number >> 1)
    {
        print_binary(number >> 1);
    }
    putc((number & 1) ? '1' : '0', stdout);
}

int amDipswGetData(uint8_t *dip)
{
    uint8_t result;
    uint32_t data;

    securityBoardIn(0x38, &data);

    result = 0x00;
    if ((~data & 0x10) != 0)
        result |= 0x01; // Dip 7
    if ((~data & 0x20) != 0)
        result |= 0x02; // Dip 8
    if ((~data & 0x40) != 0)
        result |= 0x04; // Dip 1
    if ((~data & 0x80) != 0)
        result |= 0x08; // Dip 2
    if ((~data & 0x100) != 0)
        result |= 0x10; // Rotation Dip 3
    if ((~data & 0x200) != 0)
        result |= 0x20; // Resolution Dip 4
    if ((~data & 0x400) != 0)
        result |= 0x40; // Resolution Dip 5
    if ((~data & 0x800) != 0)
        result |= 0x80; // Resolution Dip 6
    *dip = result;
    return 0;
}

void _putConsole(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char output[1000];
    vsprintf(output, format, args);
    printf("%s", output);
    va_end(args);
    printf("\n");
}

void _putConsoleSeparate(char *separator, char *str)
{
    if (!str)
        return;

    while (*str)
    {
        if (*str != ' ')
            putchar(*str);
        else
            putchar('\n');
        str++;
    }
    putchar('\n');
}

// Original snprintf in Outrun crashes sometimes so I replaced it
int or2snprintf(char *s, size_t n, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    if (format == NULL)
        return 0;

    int ret = vsnprintf(s, n, format, args);
    va_end(args);

    return ret;
}

// printf hook, used by OR2
int patchedPrintf(char *format, ...)
{
    if (format == NULL)
        return 0;

    va_list args;
    va_start(args, format);
    int ret = logVA_game(format, args);
    va_end(args);

    return ret;
}

// puts hook, used by OR2
int patchedPuts(char *s)
{
    if (s == NULL)
        return 0;

    // Puts appends a new line by default, we add it ourselves
    int ret = log_game("%s\n", s);

    return ret;
}

__attribute__((naked)) void hod4CursorHideShowFixCAVE(int param1)
{
    __asm__ volatile("push %%edi\n\tpush %%esi\n\tpush %%ebx\n\tsub $0x18, %%esp\n\tjmp *%0\n\t" ::"m"(hod4CursorHideShowCAVEAddress));
}

uint32_t hod4PrevValue;
bool hod4InTutorial = false;

void hod4CursorHideShow(int param1)
{

    uint32_t value = *(unsigned int *)(uintptr_t)(param1 + 0x38);
    if (!getConfig()->hideCursor)
    {

        switch (value)
        {
        case 0:
        {
            if (hod4PrevValue != 3 || hod4InTutorial)
            {
                hideCursor();
            }
            hod4PrevValue = 0;
        }
        break;
        case 3:
        {
            if (!hod4InTutorial)
            {
                showCursor();
                hod4PrevValue = 3;
            }
        }
        break;
        case 4:
        {
            hod4InTutorial = false;
        }
        case 2:
        case 5:
        case 11:
        {
            hideCursor();
        }
        }
    }
    hod4CursorHideShowFixCAVE(param1);
}

int hod4VsPrintf(char *str, const char *format, va_list arg)
{
    int (*_vsprintf)(char *str, const char *format, va_list arg) = dlsym(RTLD_NEXT, "vsprintf");

    int res = _vsprintf(str, format, arg);

    if (strcmp(str, "free : tutorial\n") == 0)
    {
        hod4InTutorial = false;
    }

    if (strcmp(str, "entry : tutorial\n") == 0)
    {
        hod4InTutorial = true;
    }

    return res;
}

int initPatch()
{
    EmulatorConfig *config = getConfig();
    int GPUVendor = getConfig()->GPUVendor;

    switch (gId)
    {
    case AFTER_BURNER_CLIMAX: // DVP-0009
    {
        // Security
        detourFunction(0x081e2c78, amDongleInit);
        detourFunction(0x081e2fc6, amDongleIsAvailable);
        detourFunction(0x081e2ef2, amDongleUpdate);
        patchMemory(0x08064cfe, "9090909090"); // check_security_error  Call set_error
        patchMemory(0x08064d04, "00");
        // Fixes
        detourFunction(0x081e2bae, amDipswGetData);
        detourFunction(0x081e2c26, amDipswSetLed);

        amDipswContextAddr = (void *)0x0a0ca0a8;
        detourFunction(0x081e2a9c, amDipswInit);
        detourFunction(0x081e2b38, amDipswExit);

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804e1e4, gl_ProgramStringARB);
        }
    }
    break;
    case AFTER_BURNER_CLIMAX_REVA: // DVP-0009A
    {
        // Security
        detourFunction(0x081e33c0, amDongleInit);
        detourFunction(0x081e370e, amDongleIsAvailable);
        detourFunction(0x081e363a, amDongleUpdate);
        patchMemory(0x08064cf6, "9090909090");
        patchMemory(0x08064cfc, "00");
        // Fixes
        detourFunction(0x081e32f6, amDipswGetData);
        detourFunction(0x081e336e, amDipswSetLed);

        amDipswContextAddr = (void *)0x0a0ca168;
        detourFunction(0x081e31e4, amDipswInit);
        detourFunction(0x081e3280, amDipswExit);

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804e1e4, gl_ProgramStringARB);
        }
    }
    break;
    case AFTER_BURNER_CLIMAX_REVB: // DVP-0009B
    {
        // Security
        detourFunction(0x081e3424, amDongleInit);
        detourFunction(0x081e3772, amDongleIsAvailable);
        detourFunction(0x081e369e, amDongleUpdate);
        patchMemory(0x08064cfe, "9090909090");
        patchMemory(0x08064d04, "00");
        // Fixes
        detourFunction(0x081e335a, amDipswGetData);
        detourFunction(0x081e33d2, amDipswSetLed);

        amDipswContextAddr = (void *)0x0a0ca168;
        detourFunction(0x081e3248, amDipswInit);
        detourFunction(0x081e32e4, amDipswExit);

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804e1ec, gl_ProgramStringARB);
        }
    }
    break;
    case AFTER_BURNER_CLIMAX_SDX: // DVP-0018
    {
        // Security
        detourFunction(0x081e4980, amDongleInit);
        detourFunction(0x081e4cce, amDongleIsAvailable);
        detourFunction(0x081e4bfa, amDongleUpdate);
        detourFunction(0x08064d76, stubRetZero);
        // Fixes
        detourFunction(0x081e48b6, amDipswGetData);
        detourFunction(0x081e492e, amDipswSetLed);

        amDipswContextAddr = (void *)0x0a0ccc28;
        detourFunction(0x081e47a4, amDipswInit);
        detourFunction(0x081e4840, amDipswExit);

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804e244, gl_ProgramStringARB);
        }
    }
    break;
    case AFTER_BURNER_CLIMAX_SDX_REVA: // DVP-0018A
    {
        // Security
        detourFunction(0x081e4968, amDongleInit);
        detourFunction(0x081e4cb6, amDongleIsAvailable);
        detourFunction(0x081e4be2, amDongleUpdate);
        patchMemory(0x08064dc4, "9090909090");
        patchMemory(0x08064dca, "00");
        // Fixes
        detourFunction(0x081e489e, amDipswGetData);
        detourFunction(0x081e4916, amDipswSetLed);

        amDipswContextAddr = (void *)0x0a0ccc28;
        detourFunction(0x081e478c, amDipswInit);
        detourFunction(0x081e4828, amDipswExit);

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804e24c, gl_ProgramStringARB);
        }
    }
    break;
    case AFTER_BURNER_CLIMAX_SE: // DVP-0031
    {
        // Security
        detourFunction(0x081e3470, amDongleInit);
        detourFunction(0x081e37be, amDongleIsAvailable);
        detourFunction(0x081e36ea, amDongleUpdate);
        patchMemory(0x08064cf6, "9090909090");
        patchMemory(0x08064cfc, "00");
        // Fixes
        detourFunction(0x081e33a6, amDipswGetData);
        detourFunction(0x081e341e, amDipswSetLed);

        amDipswContextAddr = (void *)0x0a0ca168;
        detourFunction(0x081e3294, amDipswInit);
        detourFunction(0x081e3330, amDipswExit);

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804e1e4, gl_ProgramStringARB);
        }
    }
    break;
    case AFTER_BURNER_CLIMAX_SE_REVA: // DVP-0031A
    {
        // Security
        detourFunction(0x081e3458, amDongleInit);
        detourFunction(0x081e37a6, amDongleIsAvailable);
        detourFunction(0x081e36d2, amDongleUpdate);
        patchMemory(0x08064cfe, "9090909090");
        patchMemory(0x08064d04, "00");
        // Fixes
        detourFunction(0x081e338e, amDipswGetData);
        detourFunction(0x081e3406, amDipswSetLed);

        amDipswContextAddr = (void *)0x0a0ca168;
        detourFunction(0x081e327c, amDipswInit);
        detourFunction(0x081e3318, amDipswExit);

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804e1ec, gl_ProgramStringARB);
        }
    }
    break;
    case GHOST_SQUAD_EVOLUTION:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug
            detourFunction(0x080984fe, _putConsole);
        }
        // Security
        detourFunction(0x08183046, amDongleInit);
        detourFunction(0x08181a91, amDongleIsAvailable);
        detourFunction(0x081824f5, amDongleUpdate);
        detourFunction(0x080e7ee8, stubRetOne); // stub check_dongle_validate
        patchMemory(0x080e7db2, "01");
        // Fixes
        amDipswContextAddr = (void *)0x0aeafc28; // Address of amDipswContext
        detourFunction(0x08181824, amDipswInit);
        detourFunction(0x081818a8, amDipswExit);
        detourFunction(0x0818191d, amDipswGetData);
        detourFunction(0x08181994, stubRetZero);
        patchMemory(0x080f37dd, "75"); // patch to prevent memory assignment error
        // Stop the extra inputs by removing io_glut_init
        detourFunction(0x080e7f94, stubRetZero);

        cacheModedShaderFiles();
        if (GPUVendor != NVIDIA_GPU)
        {
            gsEvoElfShaderPatcher();
            detourFunction(0x0804c334, gl_ProgramStringARB);
        }
    }
    break;
    case HARLEY_DAVIDSON:
    {
        // Security
        detourFunction(0x08395ede, amDongleInit);
        detourFunction(0x083949b6, amDongleIsDevelop);
        detourFunction(0x0818ba08, stubRetZero);
        detourFunction(0x0818b9ce, stubRetZero);

        // Fixes
        amDipswContextAddr = (void *)0x0abe35a8;
        detourFunction(0x0839472c, amDipswInit);
        detourFunction(0x083947b0, amDipswExit);
        detourFunction(0x08394825, amDipswGetData);
        detourFunction(0x08395da5, amDongleUserInfoEx);

        if (GPUVendor != NVIDIA_GPU)
        {
            loadLibCg();
            cacheModedShaderFiles();
            cacheNnstdshader();
            patchMemory(0x082646b3, "01");                             // Compile using CGC
            replaceCallAtAddress(0x082d48ac, compileWithCGC);
            replaceCallAtAddress(0x082d4b3b, compileWithCGC);
        }
        // Turns off the opponent marker at higher resolutions because it is out of place.
        if (getConfig()->width > 1360)
            detourFunction(0x080f19a6, stubRetZero);
    }
    break;
    case HUMMER:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x0a6e9fac, 2); // amBackupDebugLevel
            setVariable(0x0a6e9fc0, 2); // amCreditDebugLevel
            setVariable(0x0a6ea218, 2); // amDipswDebugLevel
            setVariable(0x0a6ea21c, 2); // amDongleDebugLevel
            setVariable(0x0a6ea220, 2); // amEepromDebugLevel
            setVariable(0x0a6ea224, 2); // amHwmonitorDebugLevel
            setVariable(0x0a6ea228, 2); // amJvsDebugLevel
            setVariable(0x0a6ea22c, 2); // amLibDebugLevel
            setVariable(0x0a6ea230, 2); // amMiscDebugLevel
            setVariable(0x0a6ea234, 2); // amOsinfoDebugLevel
            setVariable(0x0a6ea238, 2); // amSysDataDebugLevel
            setVariable(0x0a6ea240, 2); // bcLibDebugLevel
        }

        // Security
        detourFunction(0x083dcbf6, amDongleInit);
        detourFunction(0x083db641, amDongleIsAvailable);
        detourFunction(0x083dc0a5, amDongleUpdate);
        detourFunction(0x083db65e, amDongleIsDevelop);
        detourFunction(0x083dcabd, amDongleUserInfoEx);
        detourFunction(0x080f4e04, stubRetMinusOne);   // checkError
        detourFunction(0x083dc5aa, amDongleDecryptEx); // Return 0
        detourFunction(0x083dbddd, amDongleSetIv);     // Retrun 0

        // Security Board
        amDipswContextAddr = (void *)0x0a6f2c88;
        detourFunction(0x083db3d4, amDipswInit);
        detourFunction(0x083db458, amDipswExit);
        detourFunction(0x083db4cd, amDipswGetData);
        detourFunction(0x083db544, amDipswSetLed);

        // Networking
        detourFunction(0x083e339d, stubRetZero); // amOsinfoModifyNetworkAdr
        detourFunction(0x083e3a2f, stubRetZero); // amOsinfoModifyNetworkProperty

        // While we can't get into the test menu, immidiately return from the
        // call to clSteerErrorDisp::run that complains about the calibration
        // values.
        patchMemory(0x082e8f48, "C3");
        // Fixes Black screen after finishing the race
        patchMemory(0x08078b4c, "C3");

        detourFunction(0x082b5e4a, stubReturn); // adx_err_callback
        detourFunction(0x082d199c, stubReturn); // clSerialLindbergh::send
        detourFunction(0x082d1ab4, stubReturn); // clSerialLindbergh::receive

        // Shader patching
        cacheModedShaderFiles();
        detourFunction(0x0804f480, gl_XGetProcAddressARB);
    }
    break;
    case HUMMER_EXTREME:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x0a79a834, 2); // amAdtecDebugLevel
            setVariable(0x0a79a838, 2); // amAtaDebugLevel
            setVariable(0x0a79a83c, 2); // amBackupDebugLevel
            setVariable(0x0a79a840, 2); // amCreditDebugLevel
            setVariable(0x0a79aa98, 2); // amDipswDebugLevel
            setVariable(0x0a79aa9c, 2); // amDongleDebugLevel
            setVariable(0x0a79aaa0, 2); // amEepromDebugLevel
            setVariable(0x0a79aaa4, 2); // amHwmonitorDebugLevel
            setVariable(0x0a79aaa8, 2); // amJvsDebugLevel
            setVariable(0x0a79aaac, 2); // amLibDebugLevel
            setVariable(0x0a79aab0, 2); // amMiscDebugLevel
            setVariable(0x0a79aab4, 2); // amOsinfoDebugLevel
            setVariable(0x0a79a830, 2); // amsLibDebugLevel
            setVariable(0x0a79aab8, 2); // amSysDataDebugLevel
            setVariable(0x0a79aac0, 2); // bcLibDebugLevel
        }

        // Security
        detourFunction(0x0831c0d1, amDongleInit);
        detourFunction(0x0831a95d, amDongleIsAvailable);
        detourFunction(0x0831b47e, amDongleUpdate);
        detourFunction(0x0831a97a, amDongleIsDevelop);
        detourFunction(0x0831bf97, amDongleUserInfoEx);
        detourFunction(0x0831668c, stubRetZero);       // amsInit
        detourFunction(0x08170654, stubRetMinusOne);   // checkError
        detourFunction(0x0831b982, amDongleDecryptEx); // Return 0
        detourFunction(0x0831b08b, amDongleSetIv);     // Retrun 0
        patchMemory(0x082b4d51, "00");
        patchMemory(0x082b5163, "00");
        patchMemory(0x082b5306, "00");
        patchMemory(0x082b570f, "00");
        patchMemory(0x082b57c8, "909090909090");

        // Security Board
        amDipswContextAddr = (void *)0x0a7aefe8;
        detourFunction(0x0831a6f0, amDipswInit);
        detourFunction(0x0831a774, amDipswExit);
        detourFunction(0x0831a7e9, amDipswGetData);
        detourFunction(0x0831a85f, amDipswSetLed);

        // Networking
        detourFunction(0x08323886, stubRetZero); // amOsinfoModifyNetworkAdr
        detourFunction(0x0832386b, stubRetZero); // amOsinfoModifyNetworkProperty

        // While we can't get into the test menu, immidiately return from the
        // call to clSteerErrorDisp::run that complains about the calibration
        // values.
        patchMemory(0x080e8b40, "C3");
        // Fixes Black screen after finishing the race
        patchMemory(0x0811d0c0, "C3");

        // Shader patching
        cacheModedShaderFiles();
        detourFunction(0x0807a9dc, gl_XGetProcAddressARB);
    }
    break;
    case HUMMER_EXTREME_MDX:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x0a7ae7b4, 2); // amAdtecDebugLevel
            setVariable(0x0a7ae7b8, 2); // amAtaDebugLevel
            setVariable(0x0a7ae7bc, 2); // amBackupDebugLevel
            setVariable(0x0a7ae7c0, 2); // amCreditDebugLevel
            setVariable(0x0a7aea18, 2); // amDipswDebugLevel
            setVariable(0x0a7aea1c, 2); // amDongleDebugLevel
            setVariable(0x0a7aea20, 2); // amEepromDebugLevel
            setVariable(0x0a7aea24, 2); // amHwmonitorDebugLevel
            setVariable(0x0a7aea28, 2); // amJvsDebugLevel
            setVariable(0x0a7aea2c, 2); // amLibDebugLevel
            setVariable(0x0a7aea30, 2); // amMiscDebugLevel
            setVariable(0x0a7aea34, 2); // amOsinfoDebugLevel
            setVariable(0x0a7ae7b0, 2); // amsLibDebugLevel
            setVariable(0x0a7aea38, 2); // amSysDataDebugLevel
            setVariable(0x0a7aea40, 2); // bcLibDebugLevel
        }

        // Security
        detourFunction(0x0832b411, amDongleInit);
        detourFunction(0x08329c9d, amDongleIsAvailable);
        detourFunction(0x0832a7be, amDongleUpdate);
        detourFunction(0x08329cba, amDongleIsDevelop);
        detourFunction(0x0832b2d7, amDongleUserInfoEx);
        detourFunction(0x083259cc, stubRetZero);       // amsInit
        detourFunction(0x0817a0b6, stubRetMinusOne);   // checkError
        detourFunction(0x0832acc2, amDongleDecryptEx); // Return 0
        detourFunction(0x0832a3cb, amDongleSetIv);     // Retrun 0
        patchMemory(0x082c2465, "00");
        patchMemory(0x082c2877, "00");
        patchMemory(0x082c2a16, "00");
        patchMemory(0x082c2e1f, "00");
        patchMemory(0x082c2ed8, "909090909090");

        patchMemory(0x080de537, "909090909090");

        // Security Board
        amDipswContextAddr = (void *)0x0a7c2f68;
        detourFunction(0x08329a30, amDipswInit);
        detourFunction(0x08329ab4, amDipswExit);
        detourFunction(0x08329b29, amDipswGetData);
        detourFunction(0x08329b9f, amDipswSetLed);

        // Networking
        detourFunction(0x08332bc6, stubRetZero); // amOsinfoModifyNetworkAdr
        detourFunction(0x08332bab, stubRetZero); // amOsinfoModifyNetworkProperty

        // While we can't get into the test menu, immidiately return from the
        // call to clSteerErrorDisp::run that complains about the calibration
        // values.
        patchMemory(0x080ebe20, "C3");
        // Fixes Black screen after finishing the race
        patchMemory(0x081260f0, "C3");

        // Shader patching
        cacheModedShaderFiles();
        detourFunction(0x0807a9dc, gl_XGetProcAddressARB);
    }
    break;
    case HUMMER_SDLX:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x0a6eac0c, 2); // amBackupDebugLevel
            setVariable(0x0a6eac20, 2); // amCreditDebugLevel
            setVariable(0x0a6eae78, 2); // amDipswDebugLevel
            setVariable(0x0a6eae7c, 2); // amDongleDebugLevel
            setVariable(0x0a6eae80, 2); // amEepromDebugLevel
            setVariable(0x0a6eae84, 2); // amHwmonitorDebugLevel
            setVariable(0x0a6eae88, 2); // amJvsDebugLevel
            setVariable(0x0a6eae8c, 2); // amLibDebugLevel
            setVariable(0x0a6eae90, 2); // amMiscDebugLevel
            setVariable(0x0a6eae94, 2); // amOsinfoDebugLevel
            setVariable(0x0a6eae98, 2); // amSysDataDebugLevel
            setVariable(0x0a6eaea0, 2); // bcLibDebugLevel
        }

        // Security
        detourFunction(0x083dd7d2, amDongleInit);
        detourFunction(0x083dc21d, amDongleIsAvailable);
        detourFunction(0x083dcc81, amDongleUpdate);
        detourFunction(0x083dc23a, amDongleIsDevelop);
        detourFunction(0x083dd699, amDongleUserInfoEx);
        detourFunction(0x080f4ea8, stubRetMinusOne);   // checkError
        detourFunction(0x083dd186, amDongleDecryptEx); // Return 0
        detourFunction(0x083dc9b9, amDongleSetIv);     // Retrun 0

        // Security Board
        amDipswContextAddr = (void *)0x0a6f38e8;
        detourFunction(0x083dbfb0, amDipswInit);
        detourFunction(0x083dc034, amDipswExit);
        detourFunction(0x083dc0a9, amDipswGetData);
        detourFunction(0x083dc120, amDipswSetLed);

        // Networking
        detourFunction(0x083e3f79, stubRetZero); // amOsinfoModifyNetworkAdr
        detourFunction(0x083e460b, stubRetZero); // amOsinfoModifyNetworkProperty

        // While we can't get into the test menu, immidiately return from the
        // call to clSteerErrorDisp::run that complains about the calibration
        // values.
        patchMemory(0x082e99ec, "C3");
        // Fixes Black screen after finishing the race
        patchMemory(0x08078b84, "C3");

        detourFunction(0x082b650a, stubReturn); // adx_err_callback
        detourFunction(0x082d2414, stubReturn); // clSerialLindbergh::send
        detourFunction(0x082d252c, stubReturn); // clSerialLindbergh::receive

        // Shader patching
        cacheModedShaderFiles();
        detourFunction(0x0804f480, gl_XGetProcAddressARB);
    }
    break;
    case INITIALD_4_EXP_REVB:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08d93b70, 2);                      // amBackupDebugLevel
            setVariable(0x08d93b80, 2);                      // amCreditDebugLevel
            setVariable(0x08d93dd8, 2);                      // amDipswDebugLevel
            setVariable(0x08d93ddc, 2);                      // amDongleDebugLevel
            setVariable(0x08d93de0, 2);                      // amEepromDebugLevel
            setVariable(0x08d93de4, 2);                      // amHwmonitorDebugLevel
            setVariable(0x08d93de8, 2);                      // amJvsDebugLevel
            setVariable(0x08d93dec, 2);                      // amLibDebugLevel
            setVariable(0x08d93df0, 2);                      // amMiscDebugLevel
            setVariable(0x08d93df8, 2);                      // amSysDataDebugLevel
            setVariable(0x08d93e00, 2);                      // bcLibDebugLevel
            setVariable(0x08d93df4, 2);                      // amOsinfoDebugLevel
            setVariable(0x08d93e04, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x0854a778, _putConsoleSeparate); // Debug Messages (crashes game)
        }
        // Security
        detourFunction(0x0870ee36, amDongleInit);
        detourFunction(0x0870d881, amDongleIsAvailable);
        detourFunction(0x0870e2e5, amDongleUpdate);
        detourFunction(0x0870ecfd, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x087c0a61, 4); // Gets gameID from the ELF

        // Fixes
        amDipswContextAddr = (void *)0x08dc2d08;
        detourFunction(0x0870d614, amDipswInit);
        detourFunction(0x0870d698, amDipswExit);
        detourFunction(0x0870d70d, amDipswGetData);
        detourFunction(0x0870d784, stubRetZero); // amDipswSetLed
        detourFunction(0x0822fdde, stubRetOne);  // isEthLinkUp
        // patchMemory(0x082ddfcd, "e954010000");   // tickWaitDHCP
        patchMemory(0x082df619, "eb60");        // tickInitAddress
        detourFunction(0x0821a1fa, stubRetOne); // Skip Kickback initialization
        setVariable(0x0857f0e9, 0x000126e9);    // Avoid Full Screen set from Game
        patchMemory(0x087bc56c, "ab");          // Skips initialization

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08078860, gl_MultiTexCoord2fARB);
            detourFunction(0x080788d0, gl_Color4ub);
            detourFunction(0x08078b00, gl_Vertex3f);
            detourFunction(0x08079200, gl_TexCoord2f);
            detourFunction(0x080792a0, cg_GLIsProfileSupported);
            patchMemory(0x08549c10, "9090");
            patchMemory(0x0856e843, "31C090"); // cgCreateProgram args argument to 0;
            cacheModedShaderFiles();
            detourFunction(0x08079730, gl_XGetProcAddressARB);
            detourFunction(0x080794d0, gl_ProgramParameters4fvNV);
        }
        detourFunction(0x08264e7a, stubRetOne); // isExistNewerSource (forces shader recompilation)
    }
    break;
    case INITIALD_4_EXP_REVC:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08d93b70, 2);                      // amBackupDebugLevel
            setVariable(0x08d93b80, 2);                      // amCreditDebugLevel
            setVariable(0x08d93dd8, 2);                      // amDipswDebugLevel
            setVariable(0x08d93ddc, 2);                      // amDongleDebugLevel
            setVariable(0x08d93de0, 2);                      // amEepromDebugLevel
            setVariable(0x08d93de4, 2);                      // amHwmonitorDebugLevel
            setVariable(0x08d93de8, 2);                      // amJvsDebugLevel
            setVariable(0x08d93dec, 2);                      // amLibDebugLevel
            setVariable(0x08d93df0, 2);                      // amMiscDebugLevel
            setVariable(0x08d93df8, 2);                      // amSysDataDebugLevel
            setVariable(0x08d93e00, 2);                      // bcLibDebugLevel
            setVariable(0x08d93df4, 2);                      // amOsinfoDebugLevel
            setVariable(0x08d93e04, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x0854a4f8, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x0870ebb6, amDongleInit);
        detourFunction(0x0870d601, amDongleIsAvailable);
        detourFunction(0x0870e065, amDongleUpdate);
        detourFunction(0x0870ea7d, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x08337197, 4); // Gets gameID from the ELF

        // Fixes
        amDipswContextAddr = (void *)0x08dc2d08;
        detourFunction(0x0870d394, amDipswInit);
        detourFunction(0x0870d418, amDipswExit);
        detourFunction(0x0870d48d, amDipswGetData);
        detourFunction(0x0870d504, stubRetZero); // amDipswSetLed
        detourFunction(0x0822feae, stubRetOne);  // isEthLinkUp
        // patchMemory(0x082dde0d, "e954010000");   // tickWaitDHCP
        patchMemory(0x082df459, "eb60");        // tickInitAddress
        detourFunction(0x0821a2aa, stubRetOne); // Skip Kickback initialization
        setVariable(0x0857ee69, 0x000126e9);    // Avoid Full Screen set from Game
        patchMemory(0x087bc2ec, "eb");          // Skips initialization

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08078860, gl_MultiTexCoord2fARB);
            detourFunction(0x080788d0, gl_Color4ub);
            detourFunction(0x08078b00, gl_Vertex3f);
            detourFunction(0x08079200, gl_TexCoord2f);
            detourFunction(0x080792a0, cg_GLIsProfileSupported);
            patchMemory(0x0854b4a0, "9090");
            patchMemory(0x085700d3, "31C090"); // cgCreateProgram args argument to 0;
            cacheModedShaderFiles();
            detourFunction(0x08079730, gl_XGetProcAddressARB);
            detourFunction(0x080794d0, gl_ProgramParameters4fvNV);
        }
        detourFunction(0x08264f62, stubRetOne); // isExistNewerSource (forces shader recompilation)
    }
    break;
    case INITIALD_4_EXP_REVD:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08d972d0, 2);                      // amBackupDebugLevel
            setVariable(0x08d972e0, 2);                      // amCreditDebugLevel
            setVariable(0x08d97538, 2);                      // amDipswDebugLevel
            setVariable(0x08d9753c, 2);                      // amDongleDebugLevel
            setVariable(0x08d97540, 2);                      // amEepromDebugLevel
            setVariable(0x08d97544, 2);                      // amHwmonitorDebugLevel
            setVariable(0x08d97548, 2);                      // amJvsDebugLevel
            setVariable(0x08d9754c, 2);                      // amLibDebugLevel
            setVariable(0x08d97550, 2);                      // amMiscDebugLevel
            setVariable(0x08d97554, 2);                      // amSysDataDebugLevel
            setVariable(0x08d97558, 2);                      // bcLibDebugLevel
            setVariable(0x08d97560, 2);                      // amOsinfoDebugLevel
            setVariable(0x08d97564, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x0854c008, _putConsoleSeparate); // Debug Messages (crashes game)
        }
        // Security
        detourFunction(0x087106e6, amDongleInit);
        detourFunction(0x0870f131, amDongleIsAvailable);
        detourFunction(0x0870fb95, amDongleUpdate);
        detourFunction(0x087105ad, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x08338ce7, 4); // Gets gameID from the ELF

        // Fixes
        amDipswContextAddr = (void *)0x08dc6928;
        detourFunction(0x0870eec4, amDipswInit);
        detourFunction(0x0870ef48, amDipswExit);
        detourFunction(0x0870efbd, amDipswGetData);
        detourFunction(0x0870f034, stubRetZero); // amDipswSetLed
        detourFunction(0x08230fde, stubRetOne);  // isEthLinkUp
        patchMemory(0x082df87d, "e954010000");   // tickWaitDHCP
        patchMemory(0x082e0ec9, "eb60");         // tickInitAddress
        detourFunction(0x0821b3ea, stubRetOne);  // Skip Kickback initialization
        setVariable(0x08580979, 0x000126e9);     // Avoid Full Screen set from Game
        patchMemory(0x087beb6c, "5b");           // Skips initialization

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x080788ec, gl_MultiTexCoord2fARB);
            detourFunction(0x0807895c, gl_Color4ub);
            detourFunction(0x08078b8c, gl_Vertex3f);
            detourFunction(0x0807928c, gl_TexCoord2f);
            detourFunction(0x0807932c, cg_GLIsProfileSupported);
            patchMemory(0x0854b4a0, "9090");
            patchMemory(0x085700d3, "31C090"); // cgCreateProgram args argument to 0;
            cacheModedShaderFiles();
            detourFunction(0x080797dc, gl_XGetProcAddressARB);
            detourFunction(0x0807955c, gl_ProgramParameters4fvNV);
        }
        detourFunction(0x08266098, stubRetOne); // isExistNewerSource (forces shader recompilation)
    }
    break;
    case INITIALD_4_REVA:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08d71470, 2);                      // amBackupDebugLevel
            setVariable(0x08d71480, 2);                      // amCreditDebugLevel
            setVariable(0x08d716d8, 2);                      // amDipswDebugLevel
            setVariable(0x08d716dc, 2);                      // amDongleDebugLevel
            setVariable(0x08d716e0, 2);                      // amEepromDebugLevel
            setVariable(0x08d716e4, 2);                      // amHwmonitorDebugLevel
            setVariable(0x08d716e8, 2);                      // amJvsDebugLevel
            setVariable(0x08d716ec, 2);                      // amLibDebugLevel
            setVariable(0x08d716f0, 2);                      // amMiscDebugLevel
            setVariable(0x08d716f8, 2);                      // amSysDataDebugLevel
            setVariable(0x08d71700, 2);                      // bcLibDebugLevel
            setVariable(0x08d716f4, 2);                      // amOsinfoDebugLevel
            setVariable(0x08d71704, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x08524c88, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x086e20fa, amDongleInit);
        detourFunction(0x086e0b45, amDongleIsAvailable);
        detourFunction(0x086e15a9, amDongleUpdate);
        detourFunction(0x086e1fc1, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x8318835, 4); // Gets gameID from the ELF
        //  Fixes
        amDipswContextAddr = (void *)0x08da0628; // Address of amDipswContext
        detourFunction(0x086e08d8, amDipswInit);
        detourFunction(0x086e095c, amDipswExit);
        detourFunction(0x086e09d1, amDipswGetData);
        detourFunction(0x086e0a48, amDipswSetLed); // amDipswSetLED

        detourFunction(0x0821e52c, stubRetOne); // isEthLinkUp
        patchMemory(0x082cb222, "c0270900");    // tickInitStoreNetwork
        patchMemory(0x082cb4e9, "e950010000");  // tickWaitDHCP
        patchMemory(0x082ccc68, "eb");          // Skip Kickback initialization
        patchMemory(0x08799acc, "62");          // Skips initialization
        patchMemory(0x08799adc, "df");          // Skips initialization
        setVariable(0x085593c9, 0x000126e9);    // Avoid Full Screen set from Game

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08078980, gl_MultiTexCoord2fARB);
            detourFunction(0x080789f0, gl_Color4ub);
            detourFunction(0x08078c10, gl_Vertex3f);
            detourFunction(0x08079330, gl_TexCoord2f);
            detourFunction(0x080793d0, cg_GLIsProfileSupported);
            patchMemory(0x085241ea, "9090");
            cacheModedShaderFiles();
            detourFunction(0x08079870, gl_XGetProcAddressARB);
            detourFunction(0x08079600, gl_ProgramParameters4fvNV);
        }
        patchMemory(0x08548cb3, "31C090");      // cgCreateProgram args argument to 0;
        detourFunction(0x0825637a, stubRetOne); // isExistNewerSource (forces shader recompilation)
    }
    break;
    case INITIALD_4_REVB:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08d71750, 2);                      // amBackupDebugLevel
            setVariable(0x08d71760, 2);                      // amCreditDebugLevel
            setVariable(0x08d719b8, 2);                      // amDipswDebugLevel
            setVariable(0x08d719bc, 2);                      // amDongleDebugLevel
            setVariable(0x08d719c0, 2);                      // amEepromDebugLevel
            setVariable(0x08d719c4, 2);                      // amHwmonitorDebugLevel
            setVariable(0x08d719c8, 2);                      // amJvsDebugLevel
            setVariable(0x08d719cc, 2);                      // amLibDebugLevel
            setVariable(0x08d719d0, 2);                      // amMiscDebugLevel
            setVariable(0x08d719d8, 2);                      // amSysDataDebugLevel
            setVariable(0x08d719e0, 2);                      // bcLibDebugLevel
            setVariable(0x08d719d4, 2);                      // amOsinfoDebugLevel
            setVariable(0x08d719e4, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x08524ec8, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x086e2336, amDongleInit);
        detourFunction(0x086e0d81, amDongleIsAvailable);
        detourFunction(0x086e17e5, amDongleUpdate);
        detourFunction(0x086e21fd, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x08318a45, 4); // Gets gameID from the ELF
        //  Fixes
        amDipswContextAddr = (void *)0x08da0908; // Address of amDipswContext
        detourFunction(0x086e0b14, amDipswInit);
        detourFunction(0x086e0b98, amDipswExit);
        detourFunction(0x086e0c0d, amDipswGetData);
        detourFunction(0x086e0c84, amDipswSetLed); // amDipswSetLED

        detourFunction(0x0821e6dc, stubRetOne); // isEthLinkUp
        patchMemory(0x082cb412, "c0270900");    // tickInitStoreNetwork
        patchMemory(0x082cb6d9, "e950010000");  // tickWaitDHCP
        patchMemory(0x082cce58, "EB");          // Skip Kickback initialization
        patchMemory(0x08799d8c, "52");          // Skips initialization
        patchMemory(0x08799d9c, "cf");          // Skips initialization
        setVariable(0x08559609, 0x000126e9);    // Avoid Full Screen set from Game

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x080789a4, gl_MultiTexCoord2fARB);
            detourFunction(0x08078a14, gl_Color4ub);
            detourFunction(0x08078c34, gl_Vertex3f);
            detourFunction(0x08079354, gl_TexCoord2f);
            detourFunction(0x080793f4, cg_GLIsProfileSupported);
            patchMemory(0x0852442a, "9090");
            cacheModedShaderFiles();
            detourFunction(0x08079894, gl_XGetProcAddressARB);
            detourFunction(0x08079624, gl_ProgramParameters4fvNV);
        }
        patchMemory(0x08548ef3, "31C090");      // cgCreateProgram args argument to 0;
        detourFunction(0x0825653a, stubRetOne); // isExistNewerSource (forces shader recompilation)
    }
    break;
    case INITIALD_4_REVC:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08d7c8f0, 2);                      // amBackupDebugLevel
            setVariable(0x08d7c900, 2);                      // amCreditDebugLevel
            setVariable(0x08d7cb58, 2);                      // amDipswDebugLevel
            setVariable(0x08d7cb5c, 2);                      // amDongleDebugLevel
            setVariable(0x08d7cb60, 2);                      // amEepromDebugLevel
            setVariable(0x08d7cb64, 2);                      // amHwmonitorDebugLevel
            setVariable(0x08d7cb68, 2);                      // amJvsDebugLevel
            setVariable(0x08d7cb6c, 2);                      // amLibDebugLevel
            setVariable(0x08d7cb70, 2);                      // amMiscDebugLevel
            setVariable(0x08d7cb78, 2);                      // amSysDataDebugLevel
            setVariable(0x08d7cb80, 2);                      // bcLibDebugLevel
            setVariable(0x08d7cb74, 2);                      // amOsinfoDebugLevel
            setVariable(0x08d7cb84, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x0852d738, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x086eabd6, amDongleInit);
        detourFunction(0x086e9621, amDongleIsAvailable);
        detourFunction(0x086ea085, amDongleUpdate);
        detourFunction(0x086eaa9d, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x08795338, 4); // Gets gameID from the ELF
        //  Fixes
        amDipswContextAddr = (void *)0x08dabaa8; // Address of amDipswContext
        detourFunction(0x086e93b4, amDipswInit);
        detourFunction(0x086e9438, amDipswExit);
        detourFunction(0x086e94ad, amDipswGetData);
        detourFunction(0x086e9524, amDipswSetLed); // amDipswSetLED

        detourFunction(0x0821f0dc, stubRetOne); // isEthLinkUp
        patchMemory(0x082cfe42, "c0270900");    // tickInitStoreNetwork
        patchMemory(0x082d0109, "e950010000");  // tickWaitDHCP
        patchMemory(0x082d1888, "EB");          // Skip Kickback initialization
        patchMemory(0x087a2eec, "82");          // Skips initialization
        patchMemory(0x087a2efc, "ff");          // Skips initialization
        setVariable(0x08561e79, 0x000126e9);    // Avoid Full Screen set from Game

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x080789a4, gl_MultiTexCoord2fARB);
            detourFunction(0x08078a14, gl_Color4ub);
            detourFunction(0x08078c34, gl_Vertex3f);
            detourFunction(0x08079354, gl_TexCoord2f);
            detourFunction(0x080793f4, cg_GLIsProfileSupported);
            patchMemory(0x0852cc9a, "9090");
            cacheModedShaderFiles();
            detourFunction(0x08079894, gl_XGetProcAddressARB);
            detourFunction(0x08079624, gl_ProgramParameters4fvNV);
        }
        patchMemory(0x08551763, "31C090");      // cgCreateProgram args argument to 0;
        detourFunction(0x08256ff4, stubRetOne); // isExistNewerSource (forces shader recompilation)
    }
    break;
    case INITIALD_4_REVD:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08d798b0, 2);                      // amBackupDebugLevel
            setVariable(0x08d798c0, 2);                      // amCreditDebugLevel
            setVariable(0x08d79b18, 2);                      // amDipswDebugLevel
            setVariable(0x08d79b1c, 2);                      // amDongleDebugLevel
            setVariable(0x08d79b20, 2);                      // amEepromDebugLevel
            setVariable(0x08d79b24, 2);                      // amHwmonitorDebugLevel
            setVariable(0x08d79b28, 2);                      // amJvsDebugLevel
            setVariable(0x08d79b2c, 2);                      // amLibDebugLevel
            setVariable(0x08d79b30, 2);                      // amMiscDebugLevel
            setVariable(0x08d79b38, 2);                      // amSysDataDebugLevel
            setVariable(0x08d79b40, 2);                      // bcLibDebugLevel
            setVariable(0x08d79b34, 2);                      // amOsinfoDebugLevel
            setVariable(0x08d79b44, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x0852add8, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x086e8276, amDongleInit);
        detourFunction(0x086e6cc1, amDongleIsAvailable);
        detourFunction(0x086e7725, amDongleUpdate);
        detourFunction(0x086e813d, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x087929d8, 4); // Gets gameID from the ELF
        //  Fixes
        amDipswContextAddr = (void *)0x08da8a68; // Address of amDipswContext
        detourFunction(0x086e6a54, amDipswInit);
        detourFunction(0x086e6ad8, amDipswExit);
        detourFunction(0x086e6b4d, amDipswGetData);
        detourFunction(0x086e6bc4, amDipswSetLed); // amDipswSetLED

        detourFunction(0x0821f5cc, stubRetOne); // isEthLinkUp
        patchMemory(0x082cd3b2, "c0270900");    // tickInitStoreNetwork
        patchMemory(0x082cd679, "e950010000");  // tickWaitDHCP
        patchMemory(0x082cedf8, "EB");          // Skip Kickback initialization
        patchMemory(0x087a024c, "f2");          // Skips initialization
        patchMemory(0x087a025c, "6f");          // Skips initialization
        setVariable(0x0855f519, 0x000126e9);    // Avoid Full Screen set from Game

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x080789a4, gl_MultiTexCoord2fARB);
            detourFunction(0x08078a14, gl_Color4ub);
            detourFunction(0x08078c34, gl_Vertex3f);
            detourFunction(0x08079354, gl_TexCoord2f);
            detourFunction(0x080793f4, cg_GLIsProfileSupported);
            patchMemory(0x0852a33a, "9090");
            cacheModedShaderFiles();
            detourFunction(0x08079894, gl_XGetProcAddressARB);
            detourFunction(0x08079624, gl_ProgramParameters4fvNV);
        }
        patchMemory(0x0854ee03, "31C090");      // cgCreateProgram args argument to 0;
        detourFunction(0x08257470, stubRetOne); // isExistNewerSource (forces shader recompilation)
    }
    break;
    case INITIALD_4_REVG:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08dde130, 2);                      // amBackupDebugLevel
            setVariable(0x08dde140, 2);                      // amCreditDebugLevel
            setVariable(0x08dde398, 2);                      // amDipswDebugLevel
            setVariable(0x08dde39c, 2);                      // amDongleDebugLevel
            setVariable(0x08dde3a0, 2);                      // amEepromDebugLevel
            setVariable(0x08dde3a4, 2);                      // amHwmonitorDebugLevel
            setVariable(0x08dde3a8, 2);                      // amJvsDebugLevel
            setVariable(0x08dde3ac, 2);                      // amLibDebugLevel
            setVariable(0x08dde3b0, 2);                      // amMiscDebugLevel
            setVariable(0x08dde3b8, 2);                      // amSysDataDebugLevel
            setVariable(0x08dde3c0, 2);                      // bcLibDebugLevel
            setVariable(0x08dde3b4, 2);                      // amOsinfoDebugLevel
            setVariable(0x08dde3c4, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x08564ea8, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x0872c8d6, amDongleInit);
        detourFunction(0x0872b321, amDongleIsAvailable);
        detourFunction(0x0872bd85, amDongleUpdate);
        detourFunction(0x0872c79d, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x087da56c, 4); // Gets gameID from the ELF
        //  Fixes
        amDipswContextAddr = (void *)0x08e0d7a8; // Address of amDipswContext
        detourFunction(0x0872b0b4, amDipswInit);
        detourFunction(0x0872b138, amDipswExit);
        detourFunction(0x0872b1ad, amDipswGetData);
        detourFunction(0x0872b224, amDipswSetLed); // amDipswSetLED

        detourFunction(0x0823813a, stubRetOne); // isEthLinkUp
        patchMemory(0x082f4396, "c0270900");    // tickInitStoreNetwork
        patchMemory(0x082f4efb, "e94d010000");  // tickWaitDHCP
        patchMemory(0x082f6d5b, "EB");          // Skip Kickback initialization
        patchMemory(0x087e9eac, "c2");          // Skips initialization
        patchMemory(0x087e9ebc, "3f36");        // Skips initialization
        setVariable(0x08599819, 0x000126e9);    // Avoid Full Screen set from Game

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0807a298, gl_MultiTexCoord2fARB);
            detourFunction(0x0807a308, gl_Color4ub);
            detourFunction(0x0807a538, gl_Vertex3f);
            detourFunction(0x0807ac48, gl_TexCoord2f);
            detourFunction(0x0807ace8, cg_GLIsProfileSupported);
            patchMemory(0x08564340, "9090");
            cacheModedShaderFiles();
            detourFunction(0x0807b1b8, gl_XGetProcAddressARB);
            detourFunction(0x0807af28, gl_ProgramParameters4fvNV);
        }
        patchMemory(0x08588f73, "31C090");      // cgCreateProgram args argument to 0;
        detourFunction(0x08271cec, stubRetOne); // isExistNewerSource (forces shader recompilation)
    }
    break;
    case INITIALD_5_EXP:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x093a93f0, 2);                      // amBackupDebugLevel
            setVariable(0x093a9400, 2);                      // amCreditDebugLevel
            setVariable(0x093a9658, 2);                      // amDipswDebugLevel
            setVariable(0x093a965c, 2);                      // amDongleDebugLevel
            setVariable(0x093a9660, 2);                      // amEepromDebugLevel
            setVariable(0x093a9664, 2);                      // amHwmonitorDebugLevel
            setVariable(0x093a9668, 2);                      // amJvsDebugLevel
            setVariable(0x093a966c, 2);                      // amLibDebugLevel
            setVariable(0x093a9670, 2);                      // amMiscDebugLevel
            setVariable(0x093a9678, 2);                      // amSysDataDebugLevel
            setVariable(0x093a9680, 2);                      // bcLibDebugLevel
            setVariable(0x093a9674, 2);                      // amOsinfoDebugLevel
            setVariable(0x093a9684, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x08745e28, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x08911dca, amDongleInit);
        detourFunction(0x08910815, amDongleIsAvailable);
        detourFunction(0x08911279, amDongleUpdate);
        detourFunction(0x08911c91, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x084e2407, 4); // Get gameID from the ELF
        // Fixes
        amDipswContextAddr = (void *)0x093d94c8;
        detourFunction(0x089105a8, amDipswInit);
        detourFunction(0x0891062c, amDipswExit);
        detourFunction(0x089106a1, amDipswGetData);
        detourFunction(0x08910718, amDipswSetLed); // amDipswSetLed
        detourFunction(0x08322248, stubRetOne);    // isEthLinkUp
        detourFunction(0x083084a2, stubRetOne);    // Skip Kickback initialization
        patchMemory(0x084429d9, "eb60");           // tickInitAddress
        patchMemory(0x0843fb34, "C0270900");       // tickInitStoreNetwork
        detourFunction(0x084deddc, stubRetZero);   // doesNeedRollerCleaning
        detourFunction(0x084dedf8, stubRetZero);   // doesNeedStockerCleaning
        patchMemory(0x0843f5ad, "c7c300030000");   // Moves Initializing text
        patchMemory(0x089e314c, "47f3");           // Skips initialization
        patchMemory(0x08789a49, "e92601000090");   // Prevents Full Screen set from the game

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0807a3f0, gl_MultiTexCoord2fARB);
            detourFunction(0x0807a470, gl_Color4ub);
            detourFunction(0x0807a6b0, gl_Vertex3f);
            detourFunction(0x0807ae10, gl_TexCoord2f);
            detourFunction(0x0807aed0, cg_GLIsProfileSupported);
            patchMemory(0x087450ef, "9090");
            cacheModedShaderFiles();
        }

        detourFunction(0x08389544, stubRetOne); // isExistNewerSource
        detourFunction(0x0807b370, gl_XGetProcAddressARB);
        patchMemory(0x08744f2e, "00"); // Fix cutscenes
    }
    break;
    case INITIALD_5_EXP_20:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x093f6fa0, 2);                      // amBackupDebugLevel
            setVariable(0x093f6fc0, 2);                      // amCreditDebugLevel
            setVariable(0x093f7218, 2);                      // amDipswDebugLevel
            setVariable(0x093f721c, 2);                      // amDongleDebugLevel
            setVariable(0x093f7220, 2);                      // amEepromDebugLevel
            setVariable(0x093f7224, 2);                      // amHwmonitorDebugLevel
            setVariable(0x093f7228, 2);                      // amJvsDebugLevel
            setVariable(0x093f722c, 2);                      // amLibDebugLevel
            setVariable(0x093f7230, 2);                      // amMiscDebugLevel
            setVariable(0x093f7238, 2);                      // amSysDataDebugLevel
            setVariable(0x093f7240, 2);                      // bcLibDebugLevel
            setVariable(0x093f7234, 2);                      // amOsinfoDebugLevel
            setVariable(0x093f7244, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x08762978, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x0893dd25, amDongleInit);
        detourFunction(0x0893c5b1, amDongleIsAvailable);
        detourFunction(0x0893d0d2, amDongleUpdate);
        detourFunction(0x0893dbeb, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x084fdec2, 4); // Get gameID from the ELF
        patchMemory(0x084fde9c, "00");        // Patch KEYCHECK
        // Fixes
        amDipswContextAddr = (void *)0x09427128;
        detourFunction(0x0893c344, amDipswInit);
        detourFunction(0x0893c3c8, amDipswExit);
        detourFunction(0x0893c43d, amDipswGetData);
        detourFunction(0x0893c4b3, amDipswSetLed); // amDipswSetLed
        detourFunction(0x0832fca6, stubRetOne);    // isEthLinkUp
        patchMemory(0x08456348, "e991000000");     // tickWaitDHCP
        patchMemory(0x0845843b, "eb60");           // tickInitAddress
        patchMemory(0x08455584, "C0270900");       // tickInitStoreNetwork
        detourFunction(0x08943eb6, stubRetZero);   // amOsinfoExecDhcpNic
        detourFunction(0x085135e0, stubRetZero);   // isUseServerBox
        patchMemory(0x084592be, "30B5E3");         // seqInitWheel
        detourFunction(0x084fa3ae, stubRetZero);   // doesNeedRollerCleaning
        detourFunction(0x084fa3ca, stubRetZero);   // doesNeedStockerCleaning
        patchMemory(0x08455001, "c7c300030000");   // Moves Initializing text
        patchMemory(0x08513a10, "00");             // Network init just once
        patchMemory(0x08a0f78c, "95");             // Skips initialization
        patchMemory(0x087a6599, "e92601000090");   // Prevents Full Screen set from the game

        // amsInit
        detourFunction(0x089382e0, stubRetZero); // Eliminates amsInit Function

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0807a6c0, gl_MultiTexCoord2fARB);
            detourFunction(0x0807a740, gl_Color4ub);
            detourFunction(0x0807a990, gl_Vertex3f);
            detourFunction(0x0807b130, gl_TexCoord2f);
            detourFunction(0x0807b1f0, cg_GLIsProfileSupported);
            patchMemory(0x08761c3f, "9090");
            cacheModedShaderFiles();
        }

        detourFunction(0x08397224, stubRetOne); // isExistNewerSource
        detourFunction(0x0807b6c0, gl_XGetProcAddressARB);
        patchMemory(0x08761a7e, "00"); // Fix cutscenes
    }
    break;
    case INITIALD_5_EXP_20A:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x093f6fa0, 2);                      // amBackupDebugLevel
            setVariable(0x093f6fc0, 2);                      // amCreditDebugLevel
            setVariable(0x093f7218, 2);                      // amDipswDebugLevel
            setVariable(0x093f721c, 2);                      // amDongleDebugLevel
            setVariable(0x093f7220, 2);                      // amEepromDebugLevel
            setVariable(0x093f7224, 2);                      // amHwmonitorDebugLevel
            setVariable(0x093f7228, 2);                      // amJvsDebugLevel
            setVariable(0x093f722c, 2);                      // amLibDebugLevel
            setVariable(0x093f7230, 2);                      // amMiscDebugLevel
            setVariable(0x093f7238, 2);                      // amSysDataDebugLevel
            setVariable(0x093f7240, 2);                      // bcLibDebugLevel
            setVariable(0x093f7234, 2);                      // amOsinfoDebugLevel
            setVariable(0x093f7244, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x08762bc8, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x0893df75, amDongleInit);
        detourFunction(0x0893c801, amDongleIsAvailable);
        detourFunction(0x0893d322, amDongleUpdate);
        detourFunction(0x0893de3b, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x084fe0f2, 4); // Get gameID from the ELF
        patchMemory(0x084fe0cc, "00");        // Patch KEYCHECK
        // Fixes
        amDipswContextAddr = (void *)0x09427128;
        detourFunction(0x0893c594, amDipswInit);
        detourFunction(0x0893c618, amDipswExit);
        detourFunction(0x0893c68d, amDipswGetData);
        detourFunction(0x0893c703, amDipswSetLed); // amDipswSetLed
        detourFunction(0x0832fe46, stubRetOne);    // isEthLinkUp
        patchMemory(0x084566d8, "e991000000");     // tickWaitDHCP
        patchMemory(0x084587cb, "eb60");           // tickInitAddress
        patchMemory(0x08455914, "C0270900");       // tickInitStoreNetwork
        detourFunction(0x08944106, stubRetZero);   // amOsinfoExecDhcpNic
        detourFunction(0x08513810, stubRetZero);   // isUseServerBox
        patchMemory(0x0845964e, "40b3e3");         // seqInitWheel
        detourFunction(0x084fa5de, stubRetZero);   // doesNeedRollerCleaning
        detourFunction(0x084fa5fa, stubRetZero);   // doesNeedStockerCleaning
        patchMemory(0x08455391, "c7c300030000");   // Moves Initializing text
        patchMemory(0x08513c40, "00");             // Network init just once
        patchMemory(0x08a0f9cc, "25");             // Skips initialization
        patchMemory(0x087a67e9, "e92601000090");   // Prevents Full Screen set from the game

        // amsInit
        detourFunction(0x08938530, stubRetZero); // Eliminates amsInit Function

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0807a6c0, gl_MultiTexCoord2fARB);
            detourFunction(0x0807a740, gl_Color4ub);
            detourFunction(0x0807a990, gl_Vertex3f);
            detourFunction(0x0807b130, gl_TexCoord2f);
            detourFunction(0x0807b1f0, cg_GLIsProfileSupported);
            patchMemory(0x08761e8f, "9090");
            cacheModedShaderFiles();
        }

        detourFunction(0x083973a4, stubRetOne); // isExistNewerSource
        detourFunction(0x0807b6c0, gl_XGetProcAddressARB);
        patchMemory(0x08761cce, "00"); // Fix cutscenes
    }
    break;
    case INITIALD_5_JAP_REVA: // ID5 - DVP-0070A
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x093a92d0, 2);                      // amBackupDebugLevel
            setVariable(0x093a92e0, 2);                      // amCreditDebugLevel
            setVariable(0x093a9538, 2);                      // amDipswDebugLevel
            setVariable(0x093a953c, 2);                      // amDongleDebugLevel
            setVariable(0x093a9540, 2);                      // amEepromDebugLevel
            setVariable(0x093a9544, 2);                      // amHwmonitorDebugLevel
            setVariable(0x093a9548, 2);                      // amJvsDebugLevel
            setVariable(0x093a954c, 2);                      // amLibDebugLevel
            setVariable(0x093a9550, 2);                      // amMiscDebugLevel
            setVariable(0x093a9558, 2);                      // amSysDataDebugLevel
            setVariable(0x093a9560, 2);                      // bcLibDebugLevel
            setVariable(0x093a9554, 2);                      // amOsinfoDebugLevel
            setVariable(0x093a9564, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x08745238, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x089111da, amDongleInit);
        detourFunction(0x0890fc25, amDongleIsAvailable);
        detourFunction(0x08910689, amDongleUpdate);
        detourFunction(0x089110a1, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x084e1707, 4); // Get gameID from the ELF
        amDipswContextAddr = (void *)0x093d93a8;
        detourFunction(0x0890f9b8, amDipswInit);
        detourFunction(0x0890fa3c, amDipswExit);
        detourFunction(0x0890fab1, amDipswGetData);
        detourFunction(0x0890fb28, amDipswSetLed); // amDipswSetLed
        detourFunction(0x08321968, stubRetOne);    // isEthLinkUp
        patchMemory(0x0843f068, "c0270900");       // tickInitStoreNetwork
        // patchMemory(0x0843fed0, "e98d000000");  // tickWaitDHCP
        detourFunction(0x08307b62, stubRetOne);  // Skip Kickback initialization
        detourFunction(0x084de0dc, stubRetZero); // doesNeedRollerCleaning
        detourFunction(0x084de0f8, stubRetZero); // doesNeedStockerCleaning
        patchMemory(0x089e308c, "BA");           // Skips initialization
        patchMemory(0x08788e59, "e92601000090"); // Prevents Full Screen set from the game

        patchMemory(0x08441f99, "eb60"); // tickInitAddress

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0807a3f0, gl_MultiTexCoord2fARB);
            detourFunction(0x0807a470, gl_Color4ub);
            detourFunction(0x0807a6b0, gl_Vertex3f);
            detourFunction(0x0807ae10, gl_TexCoord2f);
            detourFunction(0x0807aed0, cg_GLIsProfileSupported);
            patchMemory(0x087444ff, "9090");
            cacheModedShaderFiles();
        }

        detourFunction(0x08388cb4, stubRetOne); // isExistNewerSource
        detourFunction(0x0807b370, gl_XGetProcAddressARB);
        patchMemory(0x0874433e, "00"); // Fix cutscenes
    }
    break;
    case INITIALD_5_JAP_REVF: // ID5 - DVP-0070F
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x093e52d4, 2);                      // amBackupDebugLevel
            setVariable(0x093e52e0, 2);                      // amCreditDebugLevel
            setVariable(0x093e5538, 2);                      // amDipswDebugLevel
            setVariable(0x093e553c, 2);                      // amDongleDebugLevel
            setVariable(0x093e5540, 2);                      // amEepromDebugLevel
            setVariable(0x093e5544, 2);                      // amHwmonitorDebugLevel
            setVariable(0x093e5548, 2);                      // amJvsDebugLevel
            setVariable(0x093e554c, 2);                      // amLibDebugLevel
            setVariable(0x093e5550, 2);                      // amMiscDebugLevel
            setVariable(0x093e5558, 2);                      // amSysDataDebugLevel
            setVariable(0x093e5560, 2);                      // bcLibDebugLevel
            setVariable(0x093e5554, 2);                      // amOsinfoDebugLevel
            setVariable(0x093e5564, 0x0FFFFFFF);             // s_logMask
            detourFunction(0x0875f0a8, _putConsoleSeparate); // Debug Messages
        }
        // Security
        detourFunction(0x08937b71, amDongleInit);
        detourFunction(0x089363fd, amDongleIsAvailable);
        detourFunction(0x08936f1e, amDongleUpdate);
        detourFunction(0x08937a37, amDongleUserInfoEx);
        memcpy(elfID, (void *)0x084fa2e2, 4); // Get gameID from the ELF
        patchMemory(0x084fa2bc, "00");        // Patch KEYCHECK
        // Fixes
        amDipswContextAddr = (void *)0x09415388;
        detourFunction(0x08936190, amDipswInit);
        detourFunction(0x08936214, amDipswExit);
        detourFunction(0x08936289, amDipswGetData);
        detourFunction(0x089362ff, amDipswSetLed); // amDipswSetLed
        detourFunction(0x08330368, stubRetOne);    // isEthLinkUp
        patchMemory(0x084519ee, "C0270900");       // tickInitStoreNetwork
        // patchMemory(0x08452848, "e991000000");     // tickWaitDHCP
        detourFunction(0x08316392, stubRetOne);  // Skip Kickback initialization
        detourFunction(0x084f67ce, stubRetZero); // doesNeedRollerCleaning
        detourFunction(0x084f67ea, stubRetZero); // doesNeedStockerCleaning

        patchMemory(0x08451501, "c7c300030000"); // Moves Initializing text
        patchMemory(0x0850ffd0, "00");           // Network init just once
        patchMemory(0x08a098ac, "95");           // Skips initialization
        patchMemory(0x087a2cc9, "e92601000090"); // Prevents Full Screen set from the game

        patchMemory(0x0845493b, "eb60");         // tickInitAddress
        detourFunction(0x0893dd02, stubRetZero); // amOsinfoExecDhcpNic
        detourFunction(0x0850fba0, stubRetZero); // isUseServerBox

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0807a42c, gl_MultiTexCoord2fARB);
            detourFunction(0x0807a4ac, gl_Color4ub);
            detourFunction(0x0807a6ec, gl_Vertex3f);
            detourFunction(0x0807ae4c, gl_TexCoord2f);
            detourFunction(0x0807af0c, cg_GLIsProfileSupported);
            patchMemory(0x0875e36f, "9090");
            cacheModedShaderFiles();
        }

        detourFunction(0x08397aa4, stubRetOne); // isExistNewerSource
        detourFunction(0x0807b3bc, gl_XGetProcAddressARB);
        patchMemory(0x0875e1ae, "00"); // Fix cutscenes
    }
    break;
    case LETS_GO_JUNGLE:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08c083a4, 2);              // amBackupDebugLevel
            setVariable(0x08c083c0, 2);              // amCreditDebugLevel
            setVariable(0x08c08618, 2);              // amDipswDebugLevel
            setVariable(0x08c0861c, 2);              // amDongleDebugLevel
            setVariable(0x08c08620, 2);              // amEepromDebugLevel
            setVariable(0x08c08624, 2);              // amHwmonitorDebugLevel
            setVariable(0x08c08628, 2);              // amJvsDebugLevel
            setVariable(0x08c0862c, 2);              // amLibDebugLevel
            setVariable(0x08c08630, 2);              // amMiscDebugLevel
            setVariable(0x08c08638, 2);              // amSysDataDebugLevel
            setVariable(0x08c08640, 2);              // bcLibDebugLevel
            setVariable(0x08c08634, 2);              // amOsinfoDebugLevel
            setVariable(0x08c08644, 0x0FFFFFFF);     // s_logMask
            detourFunction(0x08074a8c, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x084e50d8, amDongleInit);
        detourFunction(0x084e5459, amDongleIsAvailable);
        detourFunction(0x084e537d, amDongleUpdate);
        patchMemory(0x0807b76a, "9090"); // Patch initializeArcadeBackup
        // Fixes
        amDipswContextAddr = (void *)0x08c3bba8; // Address of amDipswContext
        detourFunction(0x084e4efc, amDipswInit);
        detourFunction(0x084e4f98, amDipswExit);
        detourFunction(0x084e500e, amDipswGetData);
        detourFunction(0x084e5086, amDipswSetLed);
        patchMemory(0x0840d858, "9090"); // No full screen

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU || getConfig()->lgjRenderWithMesa)
        {
            detourFunction(0x080716e4, gl_MultiTexCoord2fARB);
            detourFunction(0x08071754, gl_Color4ub);
            detourFunction(0x08071984, gl_Vertex3f);
            detourFunction(0x08071fc4, gl_TexCoord2f);
            detourFunction(0x08072054, cg_GLIsProfileSupported);
            detourFunction(0x08071da4, gl_ProgramStringARB);
            cacheModedShaderFiles();
        }
        detourFunction(0x08072214, gl_ProgramParameters4fvNV);
        // patchMemory(0x083ef88e, "909090909090");
        detourFunction(0x08072454, gl_XGetProcAddressARB);
    }
    break;
    case LETS_GO_JUNGLE_REVA:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08c10604, 2);              // amBackupDebugLevel
            setVariable(0x08c10620, 2);              // amCreditDebugLevel
            setVariable(0x08c10878, 2);              // amDipswDebugLevel
            setVariable(0x08c1087c, 2);              // amDongleDebugLevel
            setVariable(0x08c10880, 2);              // amEepromDebugLevel
            setVariable(0x08c10884, 2);              // amHwmonitorDebugLevel
            setVariable(0x08c10888, 2);              // amJvsDebugLevel
            setVariable(0x08c1088c, 2);              // amLibDebugLevel
            setVariable(0x08c10890, 2);              // amMiscDebugLevel
            setVariable(0x08c10898, 2);              // amSysDataDebugLevel
            setVariable(0x08c108a0, 2);              // bcLibDebugLevel
            setVariable(0x08c10894, 2);              // amOsinfoDebugLevel
            setVariable(0x08c108a4, 0x0FFFFFFF);     // s_logMask
            detourFunction(0x08074a8c, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x084e9fbc, amDongleInit);
        detourFunction(0x084ea378, amDongleIsAvailable);
        detourFunction(0x084ea29c, amDongleUpdate);
        patchMemory(0x0807b86a, "9090"); // Patch initializeArcadeBackup
        // Fixes
        amDipswContextAddr = (void *)0x08c43e08; // Address of amDipswContext
        detourFunction(0x084e9de0, amDipswInit);
        detourFunction(0x084e9e7c, amDipswExit);
        detourFunction(0x084e9ef2, amDipswGetData);
        detourFunction(0x084e9f6a, amDipswSetLed);
        patchMemory(0x084125f0, "9090"); // No full screen

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU || getConfig()->lgjRenderWithMesa)
        {
            detourFunction(0x08071790, gl_MultiTexCoord2fARB);
            detourFunction(0x08071800, gl_Color4ub);
            detourFunction(0x08071a30, gl_Vertex3f);
            detourFunction(0x08072080, gl_TexCoord2f);
            detourFunction(0x08072110, cg_GLIsProfileSupported);
            detourFunction(0x08071e50, gl_ProgramStringARB);
            cacheModedShaderFiles();
        }
        detourFunction(0x080722e0, gl_ProgramParameters4fvNV);
        // patchMemory(0x083f4626, "909090909090");
        detourFunction(0x08072520, gl_XGetProcAddressARB);
    }
    break;
    case LETS_GO_JUNGLE_SPECIAL:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x08c453e4, 2);              // amBackupDebugLevel
            setVariable(0x08c45400, 2);              // amCreditDebugLevel
            setVariable(0x08c45658, 2);              // amDipswDebugLevel
            setVariable(0x08c4565c, 2);              // amDongleDebugLevel
            setVariable(0x08c45660, 2);              // amEepromDebugLevel
            setVariable(0x08c45664, 2);              // amHwmonitorDebugLevel
            setVariable(0x08c45668, 2);              // amJvsDebugLevel
            setVariable(0x08c4566c, 2);              // amLibDebugLevel
            setVariable(0x08c45670, 2);              // amMiscDebugLevel
            setVariable(0x08c45678, 2);              // amSysDataDebugLevel
            setVariable(0x08c45680, 2);              // bcLibDebugLevel
            setVariable(0x08c45674, 2);              // amOsinfoDebugLevel
            setVariable(0x08c45684, 0x0FFFFFFF);     // s_logMask
            detourFunction(0x08075012, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x08510320, amDongleInit);
        detourFunction(0x085106dc, amDongleIsAvailable);
        detourFunction(0x08510600, amDongleUpdate);
        patchMemory(0x0807e609, "909090909090");
        // Fixes
        amDipswContextAddr = (void *)0x08c78be8; // Address of amDipswContext
        detourFunction(0x08510144, amDipswInit);
        detourFunction(0x085101e0, amDipswExit);
        detourFunction(0x08510256, amDipswGetData);
        detourFunction(0x085102ce, amDipswSetLed);
        patchMemory(0x08438954, "9090"); // No full screen

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU || getConfig()->lgjRenderWithMesa)
        {
            detourFunction(0x080718e0, gl_MultiTexCoord2fARB);
            detourFunction(0x08071950, gl_Color4ub);
            detourFunction(0x08071b80, gl_Vertex3f);
            detourFunction(0x08072210, gl_TexCoord2f);
            detourFunction(0x080722a0, cg_GLIsProfileSupported);
            detourFunction(0x08071fd0, gl_ProgramStringARB);
            cacheModedShaderFiles();
        }
        detourFunction(0x08072480, gl_ProgramParameters4fvNV);
        patchMemory(0x0841a98a, "909090909090");
        detourFunction(0x080726c0, gl_XGetProcAddressARB);
    }
    break;
    case MJ4_EVO:
    {
        detourFunction(0x08840b61, amDongleInit);
        detourFunction(0x0883f3ed, amDongleIsAvailable);
        detourFunction(0x0883f40a, amDongleIsDevelop);
        detourFunction(0x0883ff0e, amDongleUpdate);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x0af8ff3c; // Address of amDipswContext
        detourFunction(0x0883f180, amDipswInit);
        detourFunction(0x0883f204, amDipswExit);
        detourFunction(0x0883f279, amDipswGetData);
        detourFunction(0x0883f2ef, amDipswSetLed);

        patchMemory(0x080ea0d1, "909090909090");
        detourFunction(0x080e327c, stubRetZero); // skip checks
        detourFunction(0x080ee4fa, stubRetZero);
        patchMemory(0x080ea0bf, "75");
        patchMemory(0x08c035b8, "00");

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08051314, gl_ProgramStringARB);
        }
    }
    break;
    case MJ4_REVG:
    {
        setVariable(0x0acf85d4, 2);
        setVariable(0x0acf85d0, 2);
        setVariable(0x0acf85cc, 2);
        setVariable(0x0acf85c8, 2);
        setVariable(0x0acf85c0, 2);
        setVariable(0x0acf85bc, 2);

        // Security
        detourFunction(0x08710c0a, amDongleInit);
        detourFunction(0x0870f6c5, amDongleIsAvailable);
        detourFunction(0x0870f6e2, amDongleIsDevelop);
        detourFunction(0x087100bb, amDongleUpdate);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x0ad224fc; // Address of amDipswContext
        detourFunction(0x0870f458, amDipswInit);
        detourFunction(0x0870f4dc, amDipswExit);
        detourFunction(0x0870f551, amDipswGetData);
        detourFunction(0x0870f5c7, amDipswSetLed);

        // detourFunction(0x080e286a, stubRetZero); // skip checks
        // detourFunction(0x080ed7f8, stubRetZero);
        // patchMemory(0x080e954b, "75");
        // patchMemory(0x08a4cb90, "00");
        // patchMemory(0x080e955d, "909090909090"); // test skip countdown

        // setVariable(0x08534a33, 30);
        // setVariable(0x08534a42, 2);

        patchMemory(0x080e2891, "eb");
        patchMemory(0x080ea049, "05");
        patchMemory(0x08a4cb90, "00");

        patchMemory(0x0872bc4d, "2e2f72616d2f746f2e74787400");     // Patch /home/disk2 folder
        patchMemory(0x0872bc64, "2e2f72616d2f66726f6d2e74787400"); // Patch /home/disk2 folder
        patchMemory(0x0872bc7d, "2e2f72616d2f2e746d7000");         // Patch /home/disk2 folder
        patchMemory(0x0872bccd, "2e2f72616d00");                   // Patch /home/disk2 folder
        patchMemory(0x0872bcdd, "2e2f666f6f3100");                 // Patch /home/disk2 folder
        patchMemory(0x0872bcee, "2e2f666f6f3200");                 // Patch /home/disk2 folder
        patchMemory(0x0872f3de, "2e2f72616d2f746d7000");           // Patch /home/disk2 folder
        patchMemory(0x08732fd3, "2e2f00");                         // Patch /home/disk2 folder

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08050ebc, gl_ProgramStringARB);
        }
    }
    break;
    case OUTRUN_2_SP_SDX:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug Messages
            setVariable(0x0893a2cc, 2); // amBackupDebugLevel
            setVariable(0x0893a2e0, 2); // amCreditDebugLevel
            setVariable(0x0893a538, 2); // amDipswDebugLevel
            setVariable(0x0893a53c, 2); // amDongleDebugLevel
            setVariable(0x0893a540, 2); // amEepromDebugLevel
            setVariable(0x0893a544, 2); // amHwmonitorDebugLevel
            setVariable(0x0893a548, 2); // amJvsDebugLevel
            setVariable(0x0893a54c, 2); // amLibDebugLevel
            setVariable(0x0893a550, 2); // amMiscDebugLevel
            setVariable(0x0893a554, 2); // amOsinfoDebugLevel
            setVariable(0x0893a558, 2); // amSysDataDebugLevel
            setVariable(0x0893a560, 2); // bcLibDebugLevel
        }
        // Security
        detourFunction(0x08190f0c, amDongleInit);
        detourFunction(0x0819128d, amDongleIsAvailable);
        detourFunction(0x081911b1, amDongleUpdate);
        detourFunction(0x081912ad, amDongleIsDevelop);
        // Fixes
        amDipswContextAddr = (void *)0x0896a108;
        detourFunction(0x08190d30, amDipswInit);
        detourFunction(0x08190dcc, amDipswExit);
        detourFunction(0x08190e42, amDipswGetData);
        detourFunction(0x08190eba, amDipswSetLed);

        // Taken from original patched OUTRUN 2 SP SDX by Android and improved a little
        patchMemory(0x08105383, "e91f000000");
        patchMemory(0x081095ff, "9090");
        patchMemory(0x08109603, "9090");
        patchMemory(0x08109609, "77");

        // Shader Patches
        detourFunction(0x0804ca98, gl_ProgramStringARB);
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804cf38, glGenProgramsARB);             // Replaces glGenProgramsNV
            detourFunction(0x0804c548, gl_BindProgramNV);             // Replaces glBindProgramNV
            detourFunction(0x0804c788, gl_LoadProgramNV);             // Replaces glLoadProgramNV
            detourFunction(0x0804c4e8, gl_DeleteProgramsNV);          // Replaces glDeleteProgramsNV
            detourFunction(0x0804c938, glProgramEnvParameter4fvARB);  // Replaces glProgramParameter4fvNV
            detourFunction(0x0804ce18, glProgramEnvParameters4fvEXT); // Replaces glProgramParameters4fvNV
            detourFunction(0x0804cec8, glProgramEnvParameter4fARB);   // Replaces glProgramParameter4fNV
            detourFunction(0x0804cb98, glIsProgramARB);               // Replaces glIsProgramNV
            detourFunction(0x0804c568, gl_EndOcclusionQueryNV);       // Replaces glEndOcclusionQueryNV
            detourFunction(0x0804d498, glGetQueryObjectuivARB);       // Replaces glGetOcclusionQueryuivNV
            detourFunction(0x0804d648, glGenQueriesARB);              // Replaces glGenOcclusionQueriesNV
            detourFunction(0x0804d0a8, gl_BeginOcclusionQueryNV);     // Replaces glBeginOcclusionQueryNV
        }
        detourFunction(0x0804d148, or2snprintf); // Fixes a bug in snprintf libc 2.39??
    }
    break;
    case OUTRUN_2_SP_SDX_REVA:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug Messages
            setVariable(0x0893a24c, 2); // amBackupDebugLevel
            setVariable(0x0893a260, 2); // amCreditDebugLevel
            setVariable(0x0893a4b8, 2); // amDipswDebugLevel
            setVariable(0x0893a4bc, 2); // amDongleDebugLevel
            setVariable(0x0893a4c0, 2); // amEepromDebugLevel
            setVariable(0x0893a4c4, 2); // amHwmonitorDebugLevel
            setVariable(0x0893a4c8, 2); // amJvsDebugLevel
            setVariable(0x0893a4cc, 2); // amLibDebugLevel
            setVariable(0x0893a4d0, 2); // amMiscDebugLevel
            setVariable(0x0893a4d4, 2); // amOsinfoDebugLevel
            setVariable(0x0893a4d8, 2); // amSysDataDebugLevel
            setVariable(0x0893a4e0, 2); // bcLibDebugLevel
        }
        // Output/logs
        detourFunction(0x0804c9a8, patchedPuts);
        detourFunction(0x0804cfe8, patchedPrintf);
        // Security
        detourFunction(0x08190e80, amDongleInit);
        detourFunction(0x08191201, amDongleIsAvailable);
        detourFunction(0x08191125, amDongleUpdate);
        detourFunction(0x08191221, amDongleIsDevelop);
        // Fixes
        amDipswContextAddr = (void *)0x0896A088;
        detourFunction(0x08190ca4, amDipswInit);
        detourFunction(0x08190D40, amDipswExit);
        detourFunction(0x08190db6, amDipswGetData);
        detourFunction(0x08190e2e, amDipswSetLed);

        // Taken from original patched OUTRUN 2 SP SDX by Android and improved a little
        patchMemory(0x08105317, "e91f000000");
        patchMemory(0x08109593, "9090");
        patchMemory(0x08109597, "9090");
        patchMemory(0x0810959D, "77");

        // Shader Patches
        detourFunction(0x0804ca98, gl_ProgramStringARB);
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804cf38, glGenProgramsARB);             // Replaces glGenProgramsNV
            detourFunction(0x0804c548, gl_BindProgramNV);             // Replaces glBindProgramNV
            detourFunction(0x0804c788, gl_LoadProgramNV);             // Replaces glLoadProgramNV
            detourFunction(0x0804c4e8, gl_DeleteProgramsNV);          // Replaces glDeleteProgramsNV
            detourFunction(0x0804c938, glProgramEnvParameter4fvARB);  // Replaces glProgramParameter4fvNV
            detourFunction(0x0804ce18, glProgramEnvParameters4fvEXT); // Replaces glProgramParameters4fvNV
            detourFunction(0x0804cec8, glProgramEnvParameter4fARB);   // Replaces glProgramParameter4fNV
            detourFunction(0x0804cb98, glIsProgramARB);               // Replaces glIsProgramNV
            detourFunction(0x0804c568, gl_EndOcclusionQueryNV);       // Replaces glEndOcclusionQueryNV
            detourFunction(0x0804d498, glGetQueryObjectuivARB);       // Replaces glGetOcclusionQueryuivNV
            detourFunction(0x0804d648, glGenQueriesARB);              // Replaces glGenOcclusionQueriesNV
            detourFunction(0x0804d0a8, gl_BeginOcclusionQueryNV);     // Replaces glBeginOcclusionQueryNV
        }
        detourFunction(0x0804d148, or2snprintf); // Fixes a bug in snprintf libc 2.39??

        // Only apply these patches if skipOutrunCabinetCheck is enabled
        if (config->skipOutrunCabinetCheck)
        {
            // Bypass checks for tower and Force Feedback
            detourFunction(0x08103eaa, stubRetOne);
            detourFunction(0x08105d88, stubRetOne);
        }
    }
    break;
    case OUTRUN_2_SP_SDX_REVA_TEST:
    case OUTRUN_2_SP_SDX_REVA_TEST2:
    {
        // Security
        detourFunction(0x08066220, amDongleInit);
        detourFunction(0x080665a1, amDongleIsAvailable);
        detourFunction(0x080664c5, amDongleUpdate);
        detourFunction(0x080665c1, amDongleIsDevelop);

        // Fixes
        amDipswContextAddr = (void *)0x080980e8; // Address of amDipswContext
        detourFunction(0x08066044, amDipswInit);
        detourFunction(0x080660e0, amDipswExit);
        detourFunction(0x08066156, amDipswGetData);
        detourFunction(0x080661ce, amDipswSetLed); // Stub amDipswSetLed
        detourFunction(0x08066044, amDipswInit);

        // Patch to allow selection of all cabinet types
        patchMemory(0x08054ac4, "9090909090");
    }
    break;
    case OUTRUN_2_SP_SDX_TEST:
    {
        // Security
        detourFunction(0x08066204, amDongleInit);
        detourFunction(0x08066585, amDongleIsAvailable);
        detourFunction(0x080664a9, amDongleUpdate);
        detourFunction(0x080665a5, amDongleIsDevelop);

        // Fixes
        amDipswContextAddr = (void *)0x080980e8; // Address of amDipswContext
        detourFunction(0x08066028, amDipswInit);
        detourFunction(0x080660c4, amDipswExit);
        detourFunction(0x0806613a, amDipswGetData);
        detourFunction(0x080661b2, amDipswSetLed); // Stub amDipswSetLed
        detourFunction(0x08066028, amDipswInit);

        // Patch to allow selection of all cabinet types
        patchMemory(0x08054a88, "9090909090");
    }
    break;
    case PRIMEVAL_HUNT:
    {
        // Security
        detourFunction(0x08141770, amDongleInit);
        detourFunction(0x08140229, amDongleIsAvailable);
        detourFunction(0x08140c1f, amDongleUpdate);
        patchMemory(0x08055264, "EB");
        // Fixes
        amDipswContextAddr = (void *)0x0a96d488; // Address of amDipswContext
        detourFunction(0x0813ffbc, amDipswInit);
        detourFunction(0x08140040, amDipswExit);
        detourFunction(0x081400b5, amDipswGetData);
        detourFunction(0x0814012c, stubRetZero);
        patchMemory(0x08052cb2, "9090909090");
        cacheModedShaderFiles();
    }
    break;
    case RAMBO:
    {
        // Security
        detourFunction(0x082c4746, amDongleInit);
        detourFunction(0x082c3201, amDongleIsAvailable);
        detourFunction(0x082c3bf7, amDongleUpdate);
        detourFunction(0x082c460d, amDongleUserInfoEx);
        detourFunction(0x082c321e, amDongleIsDevelop);
        detourFunction(0x080e4262, stubRetZero);
        detourFunction(0x080e3e94, stubRetZero);
        patchMemory(0x080e3f1d, "EB");
        patchMemory(0x080e3f79, "01");
        patchMemory(0x080e3e2e, "EB");
        // Fixes
        amDipswContextAddr = (void *)0x082c2f94;
        detourFunction(0x082c2f94, amDipswInit);
        detourFunction(0x082c3018, amDipswExit);
        detourFunction(0x082c308d, amDipswGetData);
        detourFunction(0x082c3103, amDipswSetLed);
        cacheModedShaderFiles();
        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            loadLibCg();
            cacheNnstdshader();
            patchMemory(0x080b3d71, "01");                             // Compile using CGC
            replaceCallAtAddress(0x082015bf, compileWithCGC);
            replaceCallAtAddress(0x082012c1, compileWithCGC);
        }
    }
    break;
    case R_TUNED:
    {
        // Security
        detourFunction(0x08366846, amDongleInit);
        detourFunction(0x08365301, amDongleIsAvailable);
        detourFunction(0x08365cf7, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x08578be8;
        detourFunction(0x08365094, amDipswInit);
        detourFunction(0x08365118, amDipswExit);
        detourFunction(0x08365203, amDipswSetLed);
        detourFunction(0x0836518d, amDipswGetData);
        patchMemory(0x8388C90, "2e2f72616d2f746f2e74787400");     // Patch /home/disk2 folder
        patchMemory(0x8388CA7, "2e2f72616d2f66726f6d2e74787400"); // Patch /home/disk2 folder
        patchMemory(0x8388CC0, "2e2f72616d2f2e746d7000");         // Patch /home/disk2 folder
        patchMemory(0x8388D10, "2e2f72616d00");                   // Patch /home/disk2 folder
        patchMemory(0x8388D20, "2e2f666f6f3100");                 // Patch /home/disk2 folder
        patchMemory(0x8388D31, "2e2f666f6f3200");                 // Patch /home/disk2 folder
        patchMemory(0x83C979E, "2e2f00");                         // Patch /home/disk2 folder
        patchMemory(0x838C498, "2e2f524f4d5f4c414e472f656e00");   // Patch /home/disk0 folder

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x080507e8, gl_ProgramStringARB);
        }
        // Disable Force Feedback check?
        patchMemory(0x0817f8d9, "891c2490909090");
        patchMemory(0x0817f8e5, "ebc5");
    }
    break;
    case SEGABOOT_2_4_SYM:
    {
        if (config->showDebugMessages == 1)
        {
            setVariable(0x0808da48, 2);          // amAdtecDebugLevel
            setVariable(0x0808cf8c, 2);          // amAtaDebugLevel
            setVariable(0x0808cf90, 2);          // amBackupDebugLevel
            setVariable(0x0808cf94, 2);          // amChunkDataDebugLevel
            setVariable(0x0808cfa0, 2);          // amCreditDebugLevel
            setVariable(0x0808d1f8, 2);          // amDipswDebugLevel
            setVariable(0x0808d1fc, 2);          // amDiskDebugLevel
            setVariable(0x0808d200, 2);          // amDongleDebugLevel
            setVariable(0x0808d204, 2);          // amEepromDebugLevel
            setVariable(0x0808d208, 2);          // amHmDebugLevel
            setVariable(0x0808d210, 2);          // amJvsDebugLevel
            setVariable(0x0808d214, 2);          // amLibDebugLevel
            setVariable(0x0808d218, 2);          // amMiscDebugLevel
            setVariable(0x0808d21c, 2);          // amSysDataDebugLevel
            setVariable(0x0808d220, 2);          // bcLibDebugLevel
            setVariable(0x0808cf58, 2);          // g_DebugLevel
            setVariable(0x0808d224, 0x0FFFFFFF); // logmask
        }

        detourFunction(0x0805e8b0, amDongleInit);
        detourFunction(0x0805ebc3, amDongleIsAvailable);
        detourFunction(0x0805eb2a, amDongleUpdate);
        detourFunction(0x0805c30b, amDipswGetData);
    }
    break;
    case SEGA_RACE_TV:
    {
        // Security
        detourFunction(0x084d5b40, amDongleInit);
        detourFunction(0x084d45f9, amDongleIsAvailable);
        detourFunction(0x084d4fef, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x08df9cec;
        detourFunction(0x084d438c, amDipswInit);
        detourFunction(0x084d4410, amDipswExit);
        detourFunction(0x084d44fc, amDipswSetLed);
        detourFunction(0x084d4485, amDipswGetData);
        patchMemory(0x0804edb2, "9090909090"); //__intel_new_proc_init_P
        detourFunction(0x08178e6c, stubRetOne); // _ZN7am_ctrl18SetNetworkAddrEth0Ejj.

        detourFunction(0x0804e594, gl_XGetProcAddressARB);
        srtvElfShaderPatcher();

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            patchMemory(0x082d3ce9, "9090"); // Force GL_NVX_conditional_render
        }
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_REVA:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug Messages
            setVariable(0x0a737c60, 2);          // amBackupDebugLevel
            setVariable(0x0a737c64, 2);          // amChunkDataDebugLevel
            setVariable(0x0a737c80, 2);          // amCreditDebugLevel
            setVariable(0x0a737ed8, 2);          // amDipswDebugLevel
            setVariable(0x0a737edc, 2);          // amDiskDebugLevel
            setVariable(0x0a737ee0, 2);          // amDongleDebugLevel
            setVariable(0x0a737ee4, 2);          // amEepromDebugLevel
            setVariable(0x0a737ee8, 2);          // amHmDebugLevel
            setVariable(0x0a737ef0, 2);          // amJvsDebugLevel
            setVariable(0x0a737f14, 2);          // amLibDebugLevel
            setVariable(0x0a737f18, 2);          // amMiscDebugLevel
            setVariable(0x0a737f1c, 2);          // amSysDataDebugLevel
            setVariable(0x0a737f20, 2);          // bcLibDebugLevel
            setVariable(0x0a737f24, 0x0FFFFFFF); // s_logMask
        }
        // Security
        detourFunction(0x08320178, amDongleInit);
        detourFunction(0x08320459, amDongleIsAvailable);
        detourFunction(0x083203c0, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x0a7686cc; // Address of amDipswContext
        detourFunction(0x0831dccc, amDipswInit);
        detourFunction(0x0831dd61, amDipswExit);
        detourFunction(0x0831ddd7, amDipswGetData);
        detourFunction(0x0831de4f, amDipswSetLed);

        // Patch comparison check
        patchMemory(0x081b502a, "c7420400000001");

        // Patch fullscreen
        detourFunction(0x08376405, stubRetZero);

        // CPU patch to support AMD processors
        patchMemory(0x0804d034, "9090909090"); //__intel_new_proc_init_P

        // Shader Fixes
        cacheModedShaderFiles();

        //  test GLX
        //  detourFunction(0x0804c8b4, gl_XGetProcAddressARB); // glXGetProcAddressARB
        //  detourFunction(0x0804ce44, glXToSDLSwapBuffers);   // glXSwapBuffers
        //  detourFunction(0x0828ac7c, glxSDLmyCreateWindow);  // CreateWindow

        // detourFunction(0x08195fd8, stubReturn); // CMouseMgr::Update
        // patchMemory(0x0828a92a, "9090909090");  // IzPadServerExecute
        // patchMemory(0x0828a936, "9090909090");  // IzMouseServerExecute
        // patchMemory(0x0828a942, "9090909090");  // IzKeybdServerExecute

        if (!getConfig()->hideCursor)
        {
            hod4CursorHideShowCAVEAddress = (void *)0x08161c98 + 6;
            detourFunction(0x08161c98, hod4CursorHideShow);
            detourFunction(0x0804be14, hod4VsPrintf);
        }

        if (getConfig()->cpuFreqGhz != 0.0f)
        {
            uint32_t cpuFreq = (uint32_t)(getConfig()->cpuFreqGhz * 998000000.0);
            setVariable(0x0828ea13, cpuFreq);
        }
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_REVA_TEST:
    {
        detourFunction(0x080677a0, amDongleInit);
        detourFunction(0x08067a81, amDongleIsAvailable);
        detourFunction(0x080679e8, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x080a7a48; // Address of amDipswContext
        detourFunction(0x08067548, amDipswInit);
        detourFunction(0x080675dd, amDipswExit);
        detourFunction(0x08067653, amDipswGetData);
        detourFunction(0x080676cb, amDipswSetLed);

        // CPU patch to support AMD processors
        patchMemory(0x0804e46c, "9090909090"); //__intel_new_proc_init_P
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_REVB:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug Messages
            setVariable(0x0a715c40, 2);          // amBackupDebugLevel
            setVariable(0x0a715c44, 2);          // amChunkDataDebugLevel
            setVariable(0x0a715c60, 2);          // amCreditDebugLevel
            setVariable(0x0a715eb8, 2);          // amDipswDebugLevel
            setVariable(0x0a715ebc, 2);          // amDiskDebugLevel
            setVariable(0x0a715ec0, 2);          // amDongleDebugLevel
            setVariable(0x0a715ec4, 2);          // amEepromDebugLevel
            setVariable(0x0a715ec8, 2);          // amHmDebugLevel
            setVariable(0x0a715ed0, 2);          // amJvsDebugLevel
            setVariable(0x0a715ed4, 2);          // amLibDebugLevel
            setVariable(0x0a715ed8, 2);          // amMiscDebugLevel
            setVariable(0x0a715edc, 2);          // amSysDataDebugLevel
            setVariable(0x0a715ee0, 2);          // bcLibDebugLevel
            setVariable(0x0a715ee4, 0x0FFFFFFF); // s_logMask
        }
        // Security
        detourFunction(0x0831ad24, amDongleInit);
        detourFunction(0x0831b037, amDongleIsAvailable);
        detourFunction(0x0831af9e, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x0a74688c; // Address of amDipswContext
        detourFunction(0x08318674, amDipswInit);
        detourFunction(0x08318709, amDipswExit);
        detourFunction(0x0831877f, amDipswGetData);
        detourFunction(0x083187f7, amDipswSetLed);

        // Patch comparison check
        patchMemory(0x081b58be, "c7420400000001");

        // Patch fullscreen
        detourFunction(0x083723a9, stubRetZero);

        // CPU patch to support AMD processors
        patchMemory(0x0804d054, "9090909090"); //__intel_new_proc_init_P

        // Shader Fixes
        cacheModedShaderFiles();

        // test GLX
        // detourFunction(0x0804c8e4, gl_XGetProcAddressARB); // glXGetProcAddressARB
        // detourFunction(0x0804ce64, glXToSDLSwapBuffers);   // glXSwapBuffers
        // detourFunction(0x08285604, glxSDLmyCreateWindow);

        // detourFunction(0x081968b8, stubReturn); // CMouseMgr::Update
        // patchMemory(0x082852b2, "9090909090");  // IzPadServerExecute
        // patchMemory(0x082852be, "9090909090");  // IzMouseServerExecute
        // patchMemory(0x082852ca, "9090909090");  // IzKeybdServerExecute

        if (!getConfig()->hideCursor)
        {
            hod4CursorHideShowCAVEAddress = (void *)0x08162494 + 6;
            detourFunction(0x08162494, hod4CursorHideShow);
            detourFunction(0x0804be04, hod4VsPrintf);
        }

        if (getConfig()->cpuFreqGhz != 0.0f)
        {
            uint32_t cpuFreq = (uint32_t)(getConfig()->cpuFreqGhz * 998000000.0);
            setVariable(0x08289353, cpuFreq);
        }
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_REVB_TEST:
    {
        detourFunction(0x0806228c, amDongleInit);
        detourFunction(0x0806259f, amDongleIsAvailable);
        detourFunction(0x08062506, amDongleUpdate);
        // //  Fixes
        amDipswContextAddr = (void *)0x080980e8; // Address of amDipswContext
        detourFunction(0x08062034, amDipswInit);
        detourFunction(0x080620c9, amDipswExit);
        detourFunction(0x0806213f, amDipswGetData);
        detourFunction(0x080621b7, amDipswSetLed);

        // CPU patch to support AMD processors
        patchMemory(0x08049f4d, "9090909090"); //__intel_new_proc_init_P
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_REVC:
    {
        // Security
        detourFunction(0x0831ad04, amDongleInit);
        detourFunction(0x0831b017, amDongleIsAvailable);
        detourFunction(0x0831af7e, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x0a74688c; // Address of amDipswContext
        detourFunction(0x08318654, amDipswInit);
        detourFunction(0x083186e9, amDipswExit);
        detourFunction(0x0831875f, amDipswGetData);
        detourFunction(0x083187d7, amDipswSetLed);

        // Patch comparison check
        patchMemory(0x081b58be, "c7420400000001");

        // Patch fullscreen
        detourFunction(0x08372391, stubRetZero);

        // CPU patch to support AMD processors
        patchMemory(0x0804d054, "9090909090"); //__intel_new_proc_init_P

        // Shader Fixes
        cacheModedShaderFiles();

        // test GLX
        // detourFunction(0x0804c8e4, gl_XGetProcAddressARB); // glXGetProcAddressARB
        // detourFunction(0x0804ce64, glXToSDLSwapBuffers);   // glXSwapBuffers
        // detourFunction(0x08285604, glxSDLmyCreateWindow);

        // detourFunction(0x081968b8, stubReturn); // CMouseMgr::Update
        // patchMemory(0x082852b2, "9090909090");  // IzPadServerExecute
        // patchMemory(0x082852be, "9090909090");  // IzMouseServerExecute
        // patchMemory(0x082852ca, "9090909090");  // IzKeybdServerExecute

        if (!getConfig()->hideCursor)
        {
            hod4CursorHideShowCAVEAddress = (void *)0x08162494 + 6;
            detourFunction(0x08162494, hod4CursorHideShow);
            detourFunction(0x0804be04, hod4VsPrintf);
        }

        if (getConfig()->cpuFreqGhz != 0.0f)
        {
            uint32_t cpuFreq = (uint32_t)(getConfig()->cpuFreqGhz * 998000000.0);
            setVariable(0x08289353, cpuFreq);
        }
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_REVC_TEST:
    {
        detourFunction(0x0806226c, amDongleInit);
        detourFunction(0x0806257f, amDongleIsAvailable);
        detourFunction(0x080624e6, amDongleUpdate);
        //  Fixes
        amDipswContextAddr = (void *)0x080980c8; // Address of amDipswContext
        detourFunction(0x08062014, amDipswInit);
        detourFunction(0x080620a9, amDipswExit);
        detourFunction(0x0806211f, amDipswGetData);
        detourFunction(0x08062197, amDipswSetLed);

        // CPU patch to support AMD processors
        patchMemory(0x08049f4d, "9090909090"); //__intel_new_proc_init_P
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL:
    {
        detourFunction(0x082faafc, amDongleInit);
        detourFunction(0x082fae0f, amDongleIsAvailable);
        detourFunction(0x082fad76, amDongleUpdate);
        patchMemory(0x08194db9, "9090");
        patchMemory(0x08194dc1, "01");
        // Fixes
        amDipswContextAddr = (void *)0x0a6c32ec; // Address of amDipswContext
        detourFunction(0x082f844c, amDipswInit);
        detourFunction(0x082f84e1, amDipswExit);
        detourFunction(0x082f8557, amDipswGetData);
        detourFunction(0x082f85cf, amDipswSetLed);

        // CPU patch to support AMD processors
        patchMemory(0x0804d1d4, "9090909090"); //__intel_new_proc_init_P

        // Shader Fixes
        cacheModedShaderFiles();
        if (getConfig()->cpuFreqGhz != 0.0f)
        {
            uint32_t cpuFreq = (uint32_t)(getConfig()->cpuFreqGhz * 998000000.0);
            setVariable(0x0826829b, cpuFreq);
        }
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB:
    {
        detourFunction(0x08363438, amDongleInit);
        detourFunction(0x0836374b, amDongleIsAvailable);
        detourFunction(0x083636b2, amDongleUpdate);
        patchMemory(0x081f9491, "9090");
        patchMemory(0x081f9499, "01");
        // Fixes
        amDipswContextAddr = (void *)0x0a6f3f0c; // Address of amDipswContext
        detourFunction(0x08360d88, amDipswInit);
        detourFunction(0x08360e1d, amDipswExit);
        detourFunction(0x08360e93, amDipswGetData);
        detourFunction(0x08360f0b, amDipswSetLed);

        // CPU patch to support AMD processors
        patchMemory(0x0804d6c4, "9090909090"); //__intel_new_proc_init_P

        // Shader Fixes
        cacheModedShaderFiles();
        if (getConfig()->cpuFreqGhz != 0.0f)
        {
            uint32_t cpuFreq = (uint32_t)(getConfig()->cpuFreqGhz * 998000000.0);
            setVariable(0x082d019d, cpuFreq);
        }
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_REVB_TEST:
    {
        detourFunction(0x0806e914, amDongleInit);
        detourFunction(0x0806ec27, amDongleIsAvailable);
        detourFunction(0x0806eb8e, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x080adfe8; // Address of amDipswContext
        detourFunction(0x0806e6bc, amDipswInit);
        detourFunction(0x0806e751, amDipswExit);
        detourFunction(0x0806e7c7, amDipswGetData);
        detourFunction(0x0806e83f, amDipswSetLed);

        // CPU patch to support AMD processors
        patchMemory(0x08054820, "9090909090"); //__intel_new_proc_init_P
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_4_SPECIAL_TEST:
    {
        detourFunction(0x0806e9ec, amDongleInit);
        detourFunction(0x0806ecff, amDongleIsAvailable);
        detourFunction(0x0806ec66, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x080b2e88; // Address of amDipswContext
        detourFunction(0x0806e794, amDipswInit);
        detourFunction(0x0806e829, amDipswExit);
        detourFunction(0x0806e89f, amDipswGetData);
        detourFunction(0x0806e917, amDipswSetLed);

        // CPU patch to support AMD processors
        patchMemory(0x08054bf0, "9090909090"); //__intel_new_proc_init_P
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_EX:
    {
        detourFunction(0x084ba886, amDongleInit);
        detourFunction(0x084b9341, amDongleIsAvailable);
        detourFunction(0x084b9d37, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x0aa79c0c;
        detourFunction(0x084b6970, amDipswInit);
        detourFunction(0x084b69f4, amDipswExit);
        detourFunction(0x084b6a69, amDipswGetData);
        detourFunction(0x084b6adf, amDipswSetLed);
        detourFunction(0x084ba23b, stubRetOne); // amDongleDecryptEx
        patchMemory(0x081896c2, "01");

        // CPU patch to support AMD processors
        patchMemory(0x0804e4a7, "9090909090"); //__intel_new_proc_init_P

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            loadLibCg();
            cacheModedShaderFiles();
            cacheNnstdshader();
            replaceCallAtAddress(0x08370aa7, compileWithCGC);
            replaceCallAtAddress(0x08370bb6, compileWithCGC);
        }

        // test GLX
        // detourFunction(0x0804dcc4, gl_XGetProcAddressARB); // glXGetProcAddressARB
        // detourFunction(0x0804e304, glXToSDLSwapBuffers);   // glXSwapBuffers
        // detourFunction(0x0835220a, glxSDLmyCreateWindow);  // CreateWindow
    }
    break;
    case THE_HOUSE_OF_THE_DEAD_EX_TEST:
    {
        detourFunction(0x08078996, amDongleInit);
        detourFunction(0x08077451, amDongleIsAvailable);
        detourFunction(0x08077e47, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x080c5f88; // Address of amDipswContext
        detourFunction(0x080771e4, amDipswInit);
        detourFunction(0x08077268, amDipswExit);
        detourFunction(0x080772dd, amDipswGetData);
        detourFunction(0x08077353, amDipswSetLed);

        // CPU patch to support AMD processors
        patchMemory(0x0804a042, "9090909090"); //__intel_new_proc_init_P
    }
    break;
    case TOO_SPICY:
    {
        // Security
        detourFunction(0x0831cf02, amDongleInit);
        detourFunction(0x0831b94d, amDongleIsAvailable);
        detourFunction(0x0831c3b1, amDongleUpdate);
        patchMemory(0x081f7f8f, "9090");
        patchMemory(0x081f7fa7, "9090909090909090");
        patchMemory(0x081f7fb0, "01");
        patchMemory(0x081f7fb4, "909090");
        patchMemory(0x08210f27, "9090");
        patchMemory(0x08210f3c, "9090909090909090");
        patchMemory(0x08210f45, "01");
        patchMemory(0x08210f49, "909090");
        patchMemory(0x082ef5d0, "9090909090");
        // Fixes
        amDipswContextAddr = (void *)0x0c8ed0cc;
        detourFunction(0x08318f84, amDipswInit);
        detourFunction(0x08319008, amDipswExit);
        detourFunction(0x0831907d, amDipswGetData);
        detourFunction(0x083190f4, amDipswSetLed);
        detourFunction(0x081f7dba, stubRetOne); // CheckApplicationValid

        // CPU patch to support AMD processors
        patchMemory(0x08202a51, "9090909090"); //__intel_new_proc_init_P

        // unlock characters
        detourFunction(0x081cbcd2, stubRetOne);

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            loadLibCg();
            cacheModedShaderFiles();
            cacheNnstdshader();
            patchMemory(0x081ef9d8, "01"); // Force the game to recompile the shaders using cgc
            replaceCallAtAddress(0x08259fe5, compileWithCGC);
            replaceCallAtAddress(0x0825a089, compileWithCGC);
        }
    }
    break;
    case TOO_SPICY_TEST:
    {
        // Security
        detourFunction(0x0806a286, amDongleInit);
        detourFunction(0x08068cd1, amDongleIsAvailable);
        detourFunction(0x08069735, amDongleUpdate);
        patchMemory(0x08056989, "9090");
        patchMemory(0x080569a7, "01");
        patchMemory(0x080569ab, "909090");
        // Fixes
        amDipswContextAddr = (void *)0x080a9988; // Address of amDipswContext
        detourFunction(0x08068a64, amDipswInit);
        detourFunction(0x08068ae8, amDipswExit);
        detourFunction(0x08068b5d, amDipswGetData);
        detourFunction(0x08068bd4, amDipswSetLed);

        // CPU patch to support AMD processors
        patchMemory(0x080564c8, "9090909090"); //__intel_new_proc_init_P
    }
    break;
    case VIRTUA_FIGHTER_5:
    {
        // Security
        detourFunction(0x084580e8, amDongleInit);
        detourFunction(0x08458362, amDongleUpdate);
        detourFunction(0x08458436, amDongleIsAvailable);
        detourFunction(0x08458456, amDongleIsDevelop);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x0b3fcd08; // Address of amDipswContext
        detourFunction(0x08457f0c, amDipswInit);
        detourFunction(0x08457fa8, amDipswExit);
        detourFunction(0x0845801e, amDipswGetData);
        detourFunction(0x08458096, amDipswSetLed);
        detourFunction(0x08085dea, stubRetZero); // Stub returns 0
        detourFunction(0x08097624, stubRetZero); // Stub returns 0
        detourFunction(0x082f3892, stubRetZero); // Stub returns 0
        detourFunction(0x082f5dac, stubRetZero); // Stub returns 0
        patchMemory(0x080a378f, "b800000000");

        patchMemory(0x847ED73, "2e2f72616d2f746573743100"); // Patch /home/disk2 folder
        patchMemory(0x847ED89, "2e2f72616d2f746573743200"); // Patch /home/disk2 folder
        patchMemory(0x847EE44, "2e2f72616d00");             // Patch /home/disk2 folder
        patchMemory(0x847EE54, "2e2f72616d2f746d7000");     // Patch /home/disk2 folder
        patchMemory(0x85180C9, "2e2f00");                   // Patch /home/disk2 folder
        patchMemory(0x847C2B0, "2e2f00");                   // Patch /home/disk1 folder

        detourFunction(0x080862d0, stubRetZero);
        patchMemory(0x080a368b, "02");
        patchMemory(0x080a36de, "9090909090");
        patchMemory(0x080a35ed, "ffffffff");
        patchMemory(0x080a3620, "ffffffff");
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08051b9c, gl_ProgramStringARB);
        }
    }
    break;
    case VIRTUA_FIGHTER_5_EXPORT:
    {
        // Security
        detourFunction(0x084fca4c, amDongleInit);
        detourFunction(0x084fcd2c, amDongleUpdate);
        detourFunction(0x084fce08, amDongleIsAvailable);
        detourFunction(0x084fce28, amDongleIsDevelop);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x0bd8df28; // Address of amDipswContext
        detourFunction(0x084fc870, amDipswInit);
        detourFunction(0x084fc90c, amDipswExit);
        detourFunction(0x084fc982, amDipswGetData);
        detourFunction(0x084fc9fa, amDipswSetLed);
        detourFunction(0x08094d28, stubRetZero);
        patchMemory(0x080a29a5, "EB");

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0805160c, gl_ProgramStringARB);
        }
    }
    break;
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVA:
    {
        // Security
        detourFunction(0x0888f4ba, amDongleInit);
        detourFunction(0x0888df75, amDongleIsAvailable);
        detourFunction(0x0888e96b, amDongleUpdate);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x093a4848; // Address of amDipswContext
        detourFunction(0x0888dd08, amDipswInit);
        detourFunction(0x0888dd8c, amDipswExit);
        detourFunction(0x0888de01, amDipswGetData);
        detourFunction(0x0888de77, amDipswSetLed); // Stub amDipswSetLed

        patchMemory(0x080e2bad, "EB");
        patchMemory(0x081082cd, "EB");
        patchMemory(0x08122598, "05");
        detourFunction(0x08122c56, stubReturn);
        patchMemory(0x88AB02A, "2e2f72616d2f746f2e74787400");     // Patch /home/disk2 folder
        patchMemory(0x88AB041, "2e2f72616d2f66726f6d2e74787400"); // Patch /home/disk2 folder
        patchMemory(0x88AB05A, "2e2f72616d2f2e746d7000");         // Patch /home/disk2 folder
        patchMemory(0x88AB08D, "2e2f72616d00");                   // Patch /home/disk2 folder
        patchMemory(0x88AB0C1, "2e2f666f6f3100");                 // Patch /home/disk2 folder
        patchMemory(0x88AB0D2, "2e2f666f6f3200");                 // Patch /home/disk2 folder
        patchMemory(0x89C8F24, "2e2f00");                         // Patch /home/disk2 folder
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08052aa4, gl_ProgramStringARB);
        }
        if (GPUVendor != NVIDIA_GPU || GPUVendor == ATI_GPU)
        {
            hookVf5FSExposure(0x08148279, 0x0814844a, 0x08148b42, 0x08148d52);
        }
    }
    break;
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB:
    {
        // Security
        detourFunction(0x088af07d, amDongleInit);
        detourFunction(0x088ad909, amDongleIsAvailable);
        detourFunction(0x088ae42a, amDongleUpdate);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x091c6f28; // Address of amDipswContext
        detourFunction(0x088ad69c, amDipswInit);
        detourFunction(0x088ad720, amDipswExit);
        detourFunction(0x088ad795, amDipswGetData);
        detourFunction(0x088ad80b, amDipswSetLed); // Stub amDipswSetLed

        patchMemory(0x080e4c9b, "EB");
        patchMemory(0x08108444, "EB");
        patchMemory(0x081229c0, "05");
        detourFunction(0x08123240, stubReturn);
        patchMemory(0x88CAC4A, "2e2f72616d2f746f2e74787400");     // Patch /home/disk2 folder
        patchMemory(0x88CAC61, "2e2f72616d2f66726f6d2e74787400"); // Patch /home/disk2 folder
        patchMemory(0x88CAC7A, "2e2f72616d2f2e746d7000");         // Patch /home/disk2 folder
        patchMemory(0x88CACAD, "2e2f72616d00");                   // Patch /home/disk2 folder
        patchMemory(0x88CACBD, "2e2f666f6f3100");                 // Patch /home/disk2 folder
        patchMemory(0x88CACCE, "2e2f666f6f3200");                 // Patch /home/disk2 folder
        patchMemory(0x89DE5AA, "2e2f00");                         // Patch /home/disk2 folder
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08052840, gl_ProgramStringARB);
        }
        if (GPUVendor != NVIDIA_GPU || GPUVendor == ATI_GPU)
        {
            hookVf5FSExposure(0x08148a47, 0x08148c18, 0x08149310, 0x08149520);
        }
    }
    break;
    case VIRTUA_FIGHTER_5_FINAL_SHOWDOWN_REVB_6000:
    {
        // Security
        detourFunction(0x088b1866, amDongleInit);
        detourFunction(0x088b0321, amDongleIsAvailable);
        detourFunction(0x088b0d17, amDongleUpdate);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x093ce7c8; // Address of amDipswContext
        detourFunction(0x088b00b4, amDipswInit);
        detourFunction(0x088b0138, amDipswExit);
        detourFunction(0x088b01ad, amDipswGetData);
        detourFunction(0x088b0223, amDipswSetLed); // Stub amDipswSetLed

        patchMemory(0x080e4ff5, "EB");
        patchMemory(0x081084d5, "EB");
        patchMemory(0x081229f4, "05");
        detourFunction(0x0812326e, stubReturn);
        patchMemory(0x88CD42A, "2e2f72616d2f746f2e74787400");     // Patch /home/disk2 folder
        patchMemory(0x88CD441, "2e2f72616d2f66726f6d2e74787400"); // Patch /home/disk2 folder
        patchMemory(0x88CD45A, "2e2f72616d2f2e746d7000");         // Patch /home/disk2 folder
        patchMemory(0x88CD48D, "2e2f72616d00");                   // Patch /home/disk2 folder
        patchMemory(0x88CD49D, "2e2f666f6f3100");                 // Patch /home/disk2 folder
        patchMemory(0x88CD4AE, "2e2f666f6f3200");                 // Patch /home/disk2 folder
        patchMemory(0x89ED9C4, "2e2f00");                         // Patch /home/disk2 folder

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08052b20, gl_ProgramStringARB);
        }
        if (GPUVendor != NVIDIA_GPU || GPUVendor == ATI_GPU)
        {
            hookVf5FSExposure(0x081489f7, 0x08148bc8, 0x081492c0, 0x081494d0);
        }
    }
    break;
    case VIRTUA_FIGHTER_5_R:
    {
        // Security
        detourFunction(0x08740e1a, amDongleInit);
        detourFunction(0x0873f8d5, amDongleIsAvailable);
        detourFunction(0x0873f8f2, amDongleIsDevelop);
        detourFunction(0x087402cb, amDongleUpdate);
        // detourFunction(0x08740ce1, amDongleUserInfoEx);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x09109ac8; // Address of amDipswContext
        detourFunction(0x0873f668, amDipswInit);
        detourFunction(0x0873f6ec, amDipswExit);
        detourFunction(0x0873f761, amDipswGetData);
        detourFunction(0x0873f7d7, amDipswSetLed);
        detourFunction(0x080c6f88, stubRetZero);
        detourFunction(0x080e6924, stubRetZero);
        detourFunction(0x084d4190, stubRetZero);
        detourFunction(0x084d5604, stubRetZero);
        patchMemory(0x080ff2c7, "b800000000");

        detourFunction(0x080c6700, stubRetZero);
        patchMemory(0x080fef09, "02");
        patchMemory(0x080fef53, "909090909090");
        patchMemory(0x0810030c, "ffffffff");
        patchMemory(0x08100313, "ffffffff");

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08052a68, gl_ProgramStringARB);
        }
    }
    break;
    case VIRTUA_FIGHTER_5_R_REVD:
    {
        // Security
        detourFunction(0x0880024e, amDongleInit);
        detourFunction(0x087fed09, amDongleIsAvailable);
        detourFunction(0x087fed26, amDongleIsDevelop);
        detourFunction(0x087ff6ff, amDongleUpdate);
        // detourFunction(0x08740ce1, amDongleUserInfoEx);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x092870a8; // Address of amDipswContext
        detourFunction(0x087fea9c, amDipswInit);
        detourFunction(0x087feb20, amDipswExit);
        detourFunction(0x087feb95, amDipswGetData);
        detourFunction(0x087fec0b, amDipswSetLed);
        detourFunction(0x080dc6ee, stubRetZero);
        detourFunction(0x080feee8, stubRetZero);
        detourFunction(0x0853f942, stubRetZero);
        detourFunction(0x08540996, stubRetZero);
        patchMemory(0x081181d8, "b800000000");

        detourFunction(0x080dbdc4, stubRetZero);
        patchMemory(0x08117e56, "02");
        patchMemory(0x08117ea0, "909090909090");
        patchMemory(0x08119204, "ffffffff");
        patchMemory(0x0811920b, "ffffffff");

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08052a84, gl_ProgramStringARB);
        }
    }
    break;
    case VIRTUA_FIGHTER_5_R_REVG:
    {
        // Security
        detourFunction(0x0885ab46, amDongleInit);
        detourFunction(0x08859601, amDongleIsAvailable);
        detourFunction(0x0885961e, amDongleIsDevelop);
        detourFunction(0x08859ff7, amDongleUpdate);
        // detourFunction(0x08740ce1, amDongleUserInfoEx);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x09351b08; // Address of amDipswContext
        detourFunction(0x08859394, amDipswInit);
        detourFunction(0x08859418, amDipswExit);
        detourFunction(0x0885948d, amDipswGetData);
        detourFunction(0x08859503, amDipswSetLed);
        detourFunction(0x080dc9d6, stubRetZero);
        detourFunction(0x080fe46c, stubRetZero);
        detourFunction(0x0856a01c, stubRetZero);
        detourFunction(0x0856aab0, stubRetZero);
        patchMemory(0x08118358, "b800000000");

        detourFunction(0x080dc0ac, stubRetZero);
        patchMemory(0x08117fd6, "02");
        patchMemory(0x08118020, "909090909090");
        patchMemory(0x08119384, "ffffffff");
        patchMemory(0x0811938b, "ffffffff");

        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08052a70, gl_ProgramStringARB);
        }
    }
    break;
    case VIRTUA_FIGHTER_5_REVA:
    {
        // Security
        detourFunction(0x084620a8, amDongleInit);
        detourFunction(0x0846234d, amDongleUpdate);
        detourFunction(0x08462429, amDongleIsAvailable);
        detourFunction(0x08462449, amDongleIsDevelop);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x0b40b1a8; // Address of amDipswContext
        detourFunction(0x08461ecc, amDipswInit);
        detourFunction(0x08461f68, amDipswExit);
        detourFunction(0x08461fde, amDipswGetData);
        detourFunction(0x08462056, amDipswSetLed);
        detourFunction(0x080884e2, stubRetZero); // Stub returns 0
        detourFunction(0x0809a1a0, stubRetZero); // Stub returns 0
        detourFunction(0x082fa9fc, stubRetZero); // Stub returns 0
        detourFunction(0x082fce2c, stubRetZero); // Stub returns 0
        patchMemory(0x080a6407, "b800000000");

        // patchMemory(0x, "2e2f72616d2f746573743100"); // Patch /home/disk2 folder
        // patchMemory(0x, "2e2f72616d2f746573743200"); // Patch /home/disk2 folder
        patchMemory(0x08489b3b, "2e2f72616d00");         // Patch /home/disk2 folder
        patchMemory(0x08489bf4, "2e2f72616d2f746d7000"); // Patch /home/disk2 folder
        patchMemory(0x0852328a, "2e2f00");               // Patch /home/disk2 folder
        patchMemory(0x084870d9, "2e2f00");               // Patch /home/disk1 folder

        detourFunction(0x080889c8, stubRetZero);
        patchMemory(0x080a6303, "02");
        patchMemory(0x080a6356, "9090909090");
        patchMemory(0x080a6265, "ffffffff");
        patchMemory(0x080a6298, "ffffffff");
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08051c4c, gl_ProgramStringARB);
        }
    }
    break;
    case VIRTUA_FIGHTER_5_REVB:
    {
        // Security
        detourFunction(0x0850fd5c, amDongleInit);
        detourFunction(0x08510001, amDongleUpdate);
        detourFunction(0x085100dd, amDongleIsAvailable);
        detourFunction(0x085100fd, amDongleIsDevelop);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x0bd36c68; // Address of amDipswContext
        detourFunction(0x0850fb80, amDipswInit);
        detourFunction(0x0850fc1c, amDipswExit);
        detourFunction(0x0850fc92, amDipswGetData);
        detourFunction(0x0850fd0a, amDipswSetLed);
        detourFunction(0x080936d8, stubRetZero); // Stub returns 0
        detourFunction(0x080a800c, stubRetZero); // Stub returns 0
        detourFunction(0x08376746, stubRetZero); // Stub returns 0
        detourFunction(0x08378a38, stubRetZero); // Stub returns 0
        patchMemory(0x080b5733, "b800000000");

        patchMemory(0x08533c38, "2e2f72616d00");         // Patch /home/disk2 folder
        patchMemory(0x08538741, "2e2f72616d2f746d7000"); // Patch /home/disk2 folder
        patchMemory(0x085e5993, "2e2f00");               // Patch /home/disk2 folder
        patchMemory(0x08535239, "2e2f00");               // Patch /home/disk1 folder

        detourFunction(0x08093be0, stubRetZero);
        patchMemory(0x080B562F, "02");
        patchMemory(0x080b5682, "9090909090");
        patchMemory(0x080B5591, "ffffffff");
        patchMemory(0x080B55C4, "ffffffff");
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08052088, gl_ProgramStringARB);
        }
    }
    break;
    case VIRTUA_FIGHTER_5_REVE: // Also the public REV C version
    {
        // Security
        detourFunction(0x085c6010, amDongleInit);
        detourFunction(0x085c62f0, amDongleUpdate);
        detourFunction(0x085c63cc, amDongleIsAvailable);
        detourFunction(0x085c63ec, amDongleIsDevelop);

        // Fixes and patches to bypss network check
        amDipswContextAddr = (void *)0x0d4d2b08; // Address of amDipswContext
        detourFunction(0x085c5e34, amDipswInit);
        detourFunction(0x085c5ed0, amDipswExit);
        detourFunction(0x085c5f46, amDipswGetData);
        detourFunction(0x085c5fbe, amDipswSetLed); // Stub amDipswSetLed
        detourFunction(0x080b3426, stubRetZero);   // Stub returns 0
        detourFunction(0x080cb6d4, stubRetZero);   // Stub returns 0
        detourFunction(0x0840889e, stubRetZero);   // Stub returns 0
        detourFunction(0x0840ab90, stubRetZero);   // Stub returns 0
        patchMemory(0x080e1743, "EB");
        patchMemory(0x080e17af, "b800000000"); // Patch IDK what

        patchMemory(0x085eb718, "2e2f72616d00");         // Patch /home/disk2 folder
        patchMemory(0x085f1b41, "2e2f72616d2f746d7000"); // Patch /home/disk2 folder
        patchMemory(0x086a196b, "2e2f00");               // Patch /home/disk2 folder
        patchMemory(0x085ee110, "2e2f00");               // Patch /home/disk1 folder

        detourFunction(0x080b3938, stubRetZero); // ok
        patchMemory(0x080e1663, "02");
        patchMemory(0x080e16b6, "9090909090");
        patchMemory(0x080e15bd, "ffffffff");
        patchMemory(0x080e15f7, "ffffffff");
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x08052fbc, gl_ProgramStringARB);
        }
        //
        patchMemory(0x080e173c, "B810000000");
    }
    break;
    case VIRTUA_TENNIS_3:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug
            detourFunction(0x080a0240, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x0831c07c, amDongleInit);
        detourFunction(0x0831c38f, amDongleIsAvailable);
        detourFunction(0x0831c2f6, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x08907388;
        detourFunction(0x0831be24, amDipswInit);
        detourFunction(0x0831beb9, amDipswExit);
        detourFunction(0x0831bf2f, amDipswGetData);
        detourFunction(0x0831bfa7, stubRetZero);
        setVariable(0x0827a7c7, 0x34891beb); // Disable Fullscreen set from the game

        detourFunction(0x08177acc, stubRetOne); // Patch seterror

        // Mesa
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804d458, cg_GLGetLatestProfile);
            detourFunction(0x0804dcd8, cg_GLIsProfileSupported);
            detourFunction(0x0804d468, gl_Color4ub);
            detourFunction(0x0804d628, gl_Vertex3f);
            detourFunction(0x0804dc58, gl_TexCoord2f);
            patchMemory(0x082744f6, "31FF90");
            cacheModedShaderFiles();
            // Had to intercept some CPP file methods.
            replaceCallAtAddress(0x0819ca67, vt3_open);
            replaceCallAtAddress(0x0819caa6, vt3_tellg);
            replaceCallAtAddress(0x0819caf4, vt3_read);
            replaceCallAtAddress(0x0819cb3a, vt3_close);
        }
    }
    break;
    case VIRTUA_TENNIS_3_REVA:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug
            detourFunction(0x080a026c, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x0831c120, amDongleInit);
        detourFunction(0x0831c433, amDongleIsAvailable);
        detourFunction(0x0831c39a, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x08908308;
        detourFunction(0x0831bec8, amDipswInit);
        detourFunction(0x0831bf5d, amDipswExit);
        detourFunction(0x0831bfd3, amDipswGetData);
        detourFunction(0x0831c04b, stubRetZero);
        setVariable(0x0827a7f7, 0x34891beb);    // Disable Fullscreen set from the game
        detourFunction(0x08177afc, stubRetOne); // Patch seterror

        // Mesa
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804d458, cg_GLGetLatestProfile);
            detourFunction(0x0804dcd8, cg_GLIsProfileSupported);
            detourFunction(0x0804d468, gl_Color4ub);
            detourFunction(0x0804d628, gl_Vertex3f);
            detourFunction(0x0804dc58, gl_TexCoord2f);
            patchMemory(0x08274526, "31FF90");
            cacheModedShaderFiles();
            // Had to intercept some CPP file methods.
            replaceCallAtAddress(0x0819ca97, vt3_open);
            replaceCallAtAddress(0x0819cad6, vt3_tellg);
            replaceCallAtAddress(0x0819cb24, vt3_read);
            replaceCallAtAddress(0x0819cb6a, vt3_close);
        }
    }
    break;
    case VIRTUA_TENNIS_3_REVA_TEST:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug
            detourFunction(0x08054c64, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x0815f580, amDongleInit);
        detourFunction(0x0815f893, amDongleIsAvailable);
        detourFunction(0x0815f7fa, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x085e492c; // Address of amDipswContext
        detourFunction(0x0815ced0, amDipswInit);
        detourFunction(0x0815cf65, amDipswExit);
        detourFunction(0x0815cfdb, amDipswGetData);
        detourFunction(0x0815d053, amDipswSetLed);
        patchMemory(0x0806fd8f, "EB"); // Avoid Full Screen set from Game

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804ce54, cg_GLIsProfileSupported);
            detourFunction(0x0804c724, gl_Color4ub);
            detourFunction(0x0804c8b4, gl_Vertex3f);
            detourFunction(0x0804cdc4, gl_TexCoord2f);
            patchMemory(0x0806a01a, "31FF90");
            cacheModedShaderFiles();
        }
    }
    break;
    case VIRTUA_TENNIS_3_REVB:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug
            detourFunction(0x080a0336, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x0831c30c, amDongleInit);
        detourFunction(0x0831c61f, amDongleIsAvailable);
        detourFunction(0x0831c586, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x089085a8;
        detourFunction(0x0831c0b4, amDipswInit);
        detourFunction(0x0831c149, amDipswExit);
        detourFunction(0x0831c1bf, amDipswGetData);
        detourFunction(0x0831c237, stubRetZero);
        setVariable(0x0827a9e3, 0x34891beb);    // Disable Fullscreen set from the game
        detourFunction(0x08177bf0, stubRetOne); // Patch seterror

        // Mesa
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804d458, cg_GLGetLatestProfile);
            detourFunction(0x0804dcd8, cg_GLIsProfileSupported);
            detourFunction(0x0804d468, gl_Color4ub);
            detourFunction(0x0804d628, gl_Vertex3f);
            detourFunction(0x0804dc58, gl_TexCoord2f);
            patchMemory(0x08274712, "31FF90");
            cacheModedShaderFiles();
            // Had to intercept some CPP file methods.
            replaceCallAtAddress(0x0819cc83, vt3_open);
            replaceCallAtAddress(0x0819ccc2, vt3_tellg);
            replaceCallAtAddress(0x0819cd10, vt3_read);
            replaceCallAtAddress(0x0819cd56, vt3_close);
        }
    }
    break;
    case VIRTUA_TENNIS_3_REVB_TEST:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug
            detourFunction(0x08054d14, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x0815f224, amDongleInit);
        detourFunction(0x0815f537, amDongleIsAvailable);
        detourFunction(0x0815f49e, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x085e46cc; // Address of amDipswContext
        detourFunction(0x0815cb74, amDipswInit);
        detourFunction(0x0815cc09, amDipswExit);
        detourFunction(0x0815cc7f, amDipswGetData);
        detourFunction(0x0815ccf7, amDipswSetLed);
        patchMemory(0x0806fa33, "EB"); // Avoid Full Screen set from Game

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804ce54, cg_GLIsProfileSupported);
            detourFunction(0x0804c724, gl_Color4ub);
            detourFunction(0x0804c8b4, gl_Vertex3f);
            detourFunction(0x0804cdc4, gl_TexCoord2f);
            patchMemory(0x0806a0ca, "31FF90");
            cacheModedShaderFiles();
        }
    }
    break;
    case VIRTUA_TENNIS_3_REVC:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug
            detourFunction(0x080a0362, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x0831c724, amDongleInit);
        detourFunction(0x0831ca37, amDongleIsAvailable);
        detourFunction(0x0831c99e, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x08902908;
        detourFunction(0x0831c4cc, amDipswInit);
        detourFunction(0x0831c561, amDipswExit);
        detourFunction(0x0831c5d7, amDipswGetData);
        detourFunction(0x0831c64f, stubRetZero);
        setVariable(0x0827ae1b, 0x34891beb);    // Disable Fullscreen set from the game
        detourFunction(0x08177c1c, stubRetOne); // Patch seterror

        // Mesa
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804d458, cg_GLGetLatestProfile);
            detourFunction(0x0804dcd8, cg_GLIsProfileSupported);
            detourFunction(0x0804d468, gl_Color4ub);
            detourFunction(0x0804d628, gl_Vertex3f);
            detourFunction(0x0804dc58, gl_TexCoord2f);
            patchMemory(0x0827473e, "31FF90");
            // patchMemory(0x08173d17, "9090");
            // detourFunction(0x0804da38, gl_ProgramStringARB);
            cacheModedShaderFiles();
            // Had to intercept some CPP file methods.
            replaceCallAtAddress(0x0819ccaf, vt3_open);
            replaceCallAtAddress(0x0819ccee, vt3_tellg);
            replaceCallAtAddress(0x0819cd3c, vt3_read);
            replaceCallAtAddress(0x0819cd82, vt3_close);
        }
        // test GLX
        // patchMemory(0x0827173b, "909090909090");
        // detourFunction(0x0804e048, gl_XGetProcAddressARB); // glXGetProcAddressARB
        // detourFunction(0x0804e608, glXToSDLSwapBuffers);   // glXSwapBuffers
        // detourFunction(0x0827ae42, glxSDLmyCreateWindow);
    }
    break;
    case VIRTUA_TENNIS_3_REVC_TEST:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug
            detourFunction(0x08054d14, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x0815f610, amDongleInit);
        detourFunction(0x0815f923, amDongleIsAvailable);
        detourFunction(0x0815f88a, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x085e492c; // Address of amDipswContext
        detourFunction(0x0815cf60, amDipswInit);
        detourFunction(0x0815cff5, amDipswExit);
        detourFunction(0x0815d06b, amDipswGetData);
        detourFunction(0x0815d0e3, amDipswSetLed);
        patchMemory(0x0806fe3f, "EB"); // Avoid Full Screen set from Game

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804ce54, cg_GLIsProfileSupported);
            detourFunction(0x0804c724, gl_Color4ub);
            detourFunction(0x0804c8b4, gl_Vertex3f);
            detourFunction(0x0804cdc4, gl_TexCoord2f);
            patchMemory(0x0806a0ca, "31FF90");
            cacheModedShaderFiles();
        }
    }
    break;
    case VIRTUA_TENNIS_3_TEST:
    {
        if (config->showDebugMessages == 1)
        {
            // Debug
            detourFunction(0x08054c64, _putConsole); // Debug Messages
        }
        // Security
        detourFunction(0x0815f060, amDongleInit);
        detourFunction(0x0815f373, amDongleIsAvailable);
        detourFunction(0x0815f2da, amDongleUpdate);
        // Fixes
        amDipswContextAddr = (void *)0x085e4a6c; // Address of amDipswContext
        detourFunction(0x0815c9b0, amDipswInit);
        detourFunction(0x0815ca45, amDipswExit);
        detourFunction(0x0815cabb, amDipswGetData);
        detourFunction(0x0815cb33, amDipswSetLed);
        patchMemory(0x0806f983, "EB"); // Avoid Full Screen set from Game

        // Mesa Patches
        if (GPUVendor != NVIDIA_GPU)
        {
            detourFunction(0x0804ce54, cg_GLIsProfileSupported);
            detourFunction(0x0804c724, gl_Color4ub);
            detourFunction(0x0804c8b4, gl_Vertex3f);
            detourFunction(0x0804cdc4, gl_TexCoord2f);
            patchMemory(0x0806a01a, "31FF90");
            cacheModedShaderFiles();
        }
    }
    break;
    default:
        break;
    }
    return 0;
}
