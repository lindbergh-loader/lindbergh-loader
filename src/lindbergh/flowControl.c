#include <elf.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "flowControl.h"
#include "log.h"

void setVariable(size_t address, size_t value)
{
    int pagesize = sysconf(_SC_PAGE_SIZE);

    size_t *variable = (size_t *)address;

    void *toModify = (void *)(address - (address % pagesize));

    int prot = mprotect(toModify, pagesize, PROT_EXEC | PROT_WRITE);
    if (prot != 0)
    {
        log_error("Error: Cannot unprotect memory region to change variable (%d)\n", prot);
        return;
    }
    *variable = value;
}

void patchMemoryFromString(size_t address, char *value)
{
    size_t size = strlen((void *)value);
    if (size % 2 != 0)
    {
        log_error("Patch sting len should be even.\n");
        exit(EXIT_FAILURE);
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
        log_error("Error: Cannot unprotect memory region to change variable (%d)\n", prot);
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
        log_error("Error: Cannot detour memory region to change variable (%d)\n", prot);
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
        log_error("Error: Cannot detour memory region to change variable (%d)\n", prot);
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

static size_t pageAlignDown(size_t addr)
{
    size_t pagesize = sysconf(_SC_PAGESIZE);
    return addr & ~(pagesize - 1);
}

int patchMemory(void *address, const void *data, size_t size)
{
    size_t page_start = pageAlignDown((size_t)address);
    size_t page_size = size + ((size_t)address - page_start);

    if (mprotect((void *)page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
    {
        log_error("Error in mprotect.");
        return -1;
    }
    memcpy(address, data, size);
    return 0;
}

void *trampolineHook(void *target, void *replacement, size_t saveSize)
{
    void *trampoline = mmap(NULL, saveSize + 5, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (trampoline == MAP_FAILED)
    {
        perror("mmap");
        return NULL;
    }

    uintptr_t src = (uintptr_t)target;
    uintptr_t dst = (uintptr_t)replacement;

    // Save original bytes
    memcpy(trampoline, target, saveSize);

    // Append jmp back after saved bytes
    ((uint8_t *)trampoline)[saveSize] = 0xE9;
    uintptr_t jump_from = (uintptr_t)trampoline + saveSize;
    uintptr_t jump_to = src + saveSize;
    *(uint32_t *)((uint8_t *)trampoline + saveSize + 1) = (uint32_t)(jump_to - (jump_from + 5));

    // Patch original
    uint8_t jmp[5] = {0xE9};
    *(uint32_t *)&jmp[1] = (uint32_t)(dst - src - 5);
    if (patchMemory(target, jmp, sizeof(jmp)) != 0)
    {
        log_error("Error patching original function at %p\n", target);
    }

    return trampoline;
}

uintptr_t getBaseAddress()
{
    FILE *fp = fopen("/proc/self/maps", "r");
    if (!fp)
        return 0;

    uint32_t base = 0;
    fscanf(fp, "%x-", &base);
    fclose(fp);
    return base;
}

void *findStaticFnAddr(const char *functionName)
{
    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd < 0)
        return NULL;

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        close(fd);
        return NULL;
    }

    uint8_t *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (data == MAP_FAILED)
        return NULL;

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    Elf32_Shdr *shdrs = (Elf32_Shdr *)(data + ehdr->e_shoff);
    const char *shstrtab = (const char *)(data + shdrs[ehdr->e_shstrndx].sh_offset);

    Elf32_Shdr *symtabHdr = NULL, *strtabHdr = NULL;

    // Locate .symtab and .strtab
    for (int i = 0; i < ehdr->e_shnum; i++)
    {
        const char *secname = shstrtab + shdrs[i].sh_name;
        if (strcmp(secname, ".symtab") == 0)
        {
            symtabHdr = &shdrs[i];
        }
        else if (strcmp(secname, ".strtab") == 0)
        {
            strtabHdr = &shdrs[i];
        }
    }

    if (!symtabHdr || !strtabHdr)
    {
        munmap(data, st.st_size);
        return NULL;
    }

    // Check if the binary is PIE (ET_DYN)
    int isPie = (ehdr->e_type == ET_DYN);
    uintptr_t base = isPie ? getBaseAddress() : 0;

    Elf32_Sym *symtab = (Elf32_Sym *)(data + symtabHdr->sh_offset);
    const char *strtab = (const char *)(data + strtabHdr->sh_offset);
    int numSyms = symtabHdr->sh_size / sizeof(Elf32_Sym);

    for (int i = 0; i < numSyms; i++)
    {
        const char *sym_name = strtab + symtab[i].st_name;
        if (strcmp(sym_name, functionName) == 0)
        {
            uintptr_t addr = symtab[i].st_value;
            munmap(data, st.st_size);
            return (void *)(base + addr);
        }
    }

    munmap(data, st.st_size);
    return NULL;
}
