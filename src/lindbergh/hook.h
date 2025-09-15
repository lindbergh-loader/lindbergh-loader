#pragma once

typedef enum
{
    BASEBOARD = 0,
    EEPROM = 1,
    SERIAL0 = 2,
    SERIAL1 = 3,
    PCI_CARD_000 = 4
} DeviceType;

typedef enum
{
    CPUINFO = 0,
    OSRELEASE = 1,
    PCI_CARD_1F0 = 2,
    FILE_RW1 = 3,
    FILE_RW2 = 4,
    FILE_HARLEY = 5,
    FILE_FONT_ABC = 6,
    FILE_FONT_TGA = 7,
    FILE_LOGO_TGA = 8,
    ROUTE = 9
} FileTypes;