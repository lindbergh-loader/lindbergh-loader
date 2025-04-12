#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct
{
    int fd;
    char *cardFilename;

} SerialFD;

typedef struct
{
    bool cmdCompleted;
    uint16_t packetLen;
    uint8_t recBytes;
    uint8_t recBuffer[256];
} WriteCommand;

typedef struct
{
    uint8_t cmd;
    bool cmdCompleted;
    uint8_t cmdLen;
    uint8_t sentBytes;
    uint8_t sendBuffer[256];
} ReadCommand;

int initCardReader();
size_t cardReaderWrite(int fd, const void *buf, size_t count);
size_t cardReaderRead(int fd, void *buf, size_t count);
void cardReaderSetFd(int serial, int fd, char *fName);
