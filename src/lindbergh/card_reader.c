#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "card_reader.h"

#define BLOCKSIZE 8

extern uint32_t gId;

uint8_t cardHeader[] = {0x08, 0x00, 0x00, 0x00, 0x81, 0x00, 0x59, 0xda, 0x00, 0x00, 0x54, 0x4d, 0x50, 0x06, 0x03, 0x23, 0x10, 0x41, 0x62,
                        0xad, 0x00, 0x2b, 0x0b, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x95, 0x71, 0x52, 0x70, 0x00, 0x00,
                        0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd, 0xd2};

SerialFD serialFD[2];
WriteCommand writeCmd[2];
ReadCommand readCmd[2];

void initReadCmd(int fdIdx)
{
    readCmd[fdIdx].cmd = 0;
    readCmd[fdIdx].cmdCompleted = false;
    readCmd[fdIdx].cmdLen = 0;
    readCmd[fdIdx].sentBytes = 0;
    memset(&readCmd[fdIdx].sendBuffer, '\0', 256);
}

void initWriteCmd(int fdIdx)
{
    writeCmd[fdIdx].cmdCompleted = false;
    writeCmd[fdIdx].packetLen = 0;
    writeCmd[fdIdx].recBytes = 0;
    memset(&writeCmd[fdIdx].recBuffer, '\0', sizeof(uint8_t));
}

int initCardReader()
{
    EmulatorConfig *config = getConfig();

    switch (gId)
    {
    case R_TUNED:
    case VIRTUA_TENNIS_3:
    case VIRTUA_TENNIS_3_TEST:
    case VIRTUA_TENNIS_3_REVA:
    case VIRTUA_TENNIS_3_REVA_TEST:
    case VIRTUA_TENNIS_3_REVB:
    case VIRTUA_TENNIS_3_REVB_TEST:
    case VIRTUA_TENNIS_3_REVC:
    case VIRTUA_TENNIS_3_REVC_TEST:
        // Code for supported games
        break;
    default:
        printf("Warning: This Game does not support Card Reader, I will disable the card reader for you.\n");
        config->emulateCardreader = 0;
        return 0;
    }

    if (getConfig()->emulateCardreader && getConfig()->emulateDriveboard && gId != R_TUNED)
    {
        printf("Warning: This Game does not support Card Reader and Driver Board emulation enabled at the same time.\n");
        printf("Please disable Drive Board emulations in the config file.\n");
        return -1;
    }

    initReadCmd(0);
    initReadCmd(1);
    initWriteCmd(0);
    initWriteCmd(1);
    return 0;
}

int carReaderGetFdIdx(int fd)
{
    return (fd == serialFD[0].fd) ? 0 : 1;
}

uint8_t cardReaderGenChecksum(int fdIdx)
{
    int checksum = 0;
    for (int i = 0; i < readCmd[fdIdx].cmdLen - 1; i++)
    {
        checksum ^= readCmd[fdIdx].sendBuffer[i];
    }
    return checksum;
}

void buildOutputBuffer(int fdIdx, uint8_t cmd)
{
    uint16_t packetLen = 0;

    readCmd[fdIdx].sendBuffer[0] = 0x10;
    readCmd[fdIdx].sendBuffer[1] = cmd;
    switch (cmd)
    {
    case 0x10:
    {
        readCmd[fdIdx].sendBuffer[5] = 0x20;
    }
    case 0x11:
    {
        if (writeCmd[fdIdx].recBuffer[5] == 0x04)
        {
            initWriteCmd(fdIdx);
            return;
        }
    }
    case 0x14:
    case 0x15:
    case 0x20:
    {
        if (cmd == 0x20 && writeCmd[fdIdx].recBuffer[5] == 0x00)
            readCmd[fdIdx].sendBuffer[4] = 0x80;
    }
    case 0x26:
    case 0x27:
    {
        packetLen = 0x02;
        readCmd[fdIdx].sendBuffer[3] = packetLen;
    }
    break;
    case 0x21:
    {
        packetLen = 0x0a;
        readCmd[fdIdx].sendBuffer[3] = packetLen;
        readCmd[fdIdx].sendBuffer[6] = 0x81;
        readCmd[fdIdx].sendBuffer[8] = 0x59;
        readCmd[fdIdx].sendBuffer[9] = 0xda;
    }
    break;
    case 0x22:
    {
        packetLen = 0x0a;
        readCmd[fdIdx].sendBuffer[3] = packetLen;
        readCmd[fdIdx].sendBuffer[7] = 0x08;
    }
    break;
    case 0x33:
    {
        packetLen = 0x0a;
        readCmd[fdIdx].sendBuffer[3] = packetLen;
        readCmd[fdIdx].sendBuffer[6] = 0xff;
        readCmd[fdIdx].sendBuffer[7] = 0xff;
    }
    break;
    case 0x34:
    {
        uint16_t blockStart = (writeCmd[fdIdx].recBuffer[6] << 8) | writeCmd[fdIdx].recBuffer[7];
        uint16_t blocksToRead = (writeCmd[fdIdx].recBuffer[8] << 8) | writeCmd[fdIdx].recBuffer[9];
        packetLen = (BLOCKSIZE * blocksToRead) + 2;
        readCmd[fdIdx].sendBuffer[3] = packetLen;
        FILE *crdFile = fopen(serialFD[fdIdx].cardFilename, "rb");
        fseek(crdFile, BLOCKSIZE * blockStart, SEEK_SET);
        fread(readCmd[fdIdx].sendBuffer + 6, 1, packetLen - 2, crdFile);
        fclose(crdFile);
    }
    break;
    case 0x35:
    {
        readCmd[fdIdx].sendBuffer[3] = 0x02;
        uint16_t blockStart = (writeCmd[fdIdx].recBuffer[6] << 8) | writeCmd[fdIdx].recBuffer[7];
        uint16_t blocksToWrite = (writeCmd[fdIdx].recBuffer[8] << 8) | writeCmd[fdIdx].recBuffer[9];
        packetLen = BLOCKSIZE * blocksToWrite;
        FILE *crdFile = fopen(serialFD[fdIdx].cardFilename, "rb+");
        fseek(crdFile, BLOCKSIZE * blockStart, SEEK_SET);
        fwrite(writeCmd[fdIdx].recBuffer + 10, 1, packetLen, crdFile);
        fclose(crdFile);
        packetLen = 0x02;
    }
    break;
    default:
    {
        printf("Command not supported.\n");
    }
    }
    readCmd[fdIdx].cmdLen = packetLen + 5;
    readCmd[fdIdx].sendBuffer[readCmd[fdIdx].cmdLen - 1] = cardReaderGenChecksum(fdIdx);
}

size_t cardReaderWrite(int fd, const void *buf, size_t count)
{
    int fdIdx = carReaderGetFdIdx(fd);
    if (writeCmd[fdIdx].cmdCompleted)
    {
        printf("Something went wrong, a write command was issued after the command was completed.\n");
    }

    if (writeCmd[fdIdx].recBytes == 4)
    {
        writeCmd[fdIdx].packetLen = (writeCmd[fdIdx].recBuffer[2] << 8) | writeCmd[fdIdx].recBuffer[3];
    }

    writeCmd[fdIdx].recBuffer[writeCmd[fdIdx].recBytes] = *((u_int8_t *)buf);
    writeCmd[fdIdx].recBytes++;

    if (writeCmd[fdIdx].recBytes == writeCmd[fdIdx].packetLen + 5)
    {
        writeCmd[fdIdx].cmdCompleted = true;
        initReadCmd(fdIdx);
        buildOutputBuffer(fdIdx, writeCmd[fdIdx].recBuffer[1]);
    }
    return count;
}

size_t cardReaderRead(int fd, void *buf, size_t count)
{
    int fdIdx = carReaderGetFdIdx(fd);
    if (!writeCmd[fdIdx].cmdCompleted || readCmd[fdIdx].cmdCompleted)
        return -1;

    if (readCmd[fdIdx].cmdLen == readCmd[fdIdx].sentBytes)
    {
        readCmd[fdIdx].cmdCompleted = true;
        initWriteCmd(fdIdx);
        return -1;
    }

    memcpy(buf, &readCmd[fdIdx].sendBuffer[readCmd[fdIdx].sentBytes], sizeof(uint8_t));
    readCmd[fdIdx].sentBytes++;
    return 1;
}

void cardReaderSetFd(int serial, int fd, char *fName)
{
    serialFD[serial].fd = fd;
    serialFD[serial].cardFilename = fName;

    if (access(fName, F_OK) != 0)
    {
        FILE *crdFile = fopen(fName, "wb+");
        if (getConfig()->crc32 == R_TUNED)
        {
            cardHeader[0x22] = 0x02;
            cardHeader[0x23] = 0x60;
        }
        fwrite(cardHeader, 1, 56, crdFile);
        uint8_t z = 0;
        fseek(crdFile, 0x7c7, SEEK_CUR);
        fwrite(&z, 1, 1, crdFile);
        fclose(crdFile);
        printf("Cardfile %s created.\n", fName);
    }
}
