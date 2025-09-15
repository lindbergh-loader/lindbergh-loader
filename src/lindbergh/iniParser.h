#pragma once

#include <stdbool.h>

typedef struct
{
    char *key;
    char *value;
} IniKeyValuePair;

typedef struct
{
    char *name;
    IniKeyValuePair *pairs;
    int numPairs;
} IniSection;

typedef struct
{
    IniSection *sections;
    int numSections;
} IniConfig;

IniConfig *iniLoad(const char *filename);
void iniFree(IniConfig *config);
const char *iniGetValue(const IniConfig *config, const char *section, const char *key);
IniSection *iniGetSection(const IniConfig *config, const char *sectionName);
bool iniSetValue(IniConfig *config, const char *section, const char *key, const char *value);
int iniSave(const IniConfig *config, const char *filename);