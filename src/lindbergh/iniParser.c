#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "iniParser.h"
#include "log.h"

/**
 * @brief Trims leading and trailing whitespace from a string in-place.
 * @param str The string to trim.
 * @return A pointer to the beginning of the trimmed string.
 */
static char *trimString(char *str)
{
    char *end;
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return str;
}

/**
 * @brief Loads and parses an INI file into an IniConfig structure.
 * This function reads an INI file, parsing its sections and key-value pairs.
 * It allocates memory for the configuration structure, which must be freed
 * later by calling iniFree().
 * @param filename The path to the INI file.
 * @return A pointer to the newly created IniConfig structure, or NULL on failure.
 */
IniConfig *iniLoad(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        log_warn("Error opening INI file");
        return NULL;
    }
    IniConfig *config = calloc(1, sizeof(IniConfig));
    if (!config)
        return NULL;
    char line[1024];
    IniSection *currentSection = NULL;
    while (fgets(line, sizeof(line), file))
    {
        char *trimmedLine = trimString(line);
        if (trimmedLine[0] == ';' || trimmedLine[0] == '#' || trimmedLine[0] == '\0')
            continue;
        if (trimmedLine[0] == '[' && trimmedLine[strlen(trimmedLine) - 1] == ']')
        {
            config->sections = realloc(config->sections, (config->numSections + 1) * sizeof(IniSection));
            currentSection = &config->sections[config->numSections++];
            currentSection->numPairs = 0;
            currentSection->pairs = NULL;
            char *sectionName = trimmedLine + 1;
            sectionName[strlen(sectionName) - 1] = '\0';
            currentSection->name = strdup(sectionName);
        }
        else if (currentSection && strchr(trimmedLine, '='))
        {
            char *key = trimmedLine;
            char *value = strchr(trimmedLine, '=');
            *value = '\0';
            value++;
            key = trimString(key);
            value = trimString(value);
            currentSection->pairs = realloc(currentSection->pairs, (currentSection->numPairs + 1) * sizeof(IniKeyValuePair));
            IniKeyValuePair *pair = &currentSection->pairs[currentSection->numPairs++];
            pair->key = strdup(key);
            pair->value = strdup(value);
        }
    }
    fclose(file);
    return config;
}

/**
 * @brief Retrieves a section from the INI configuration by its name.
 * @param config Pointer to the INI configuration structure.
 * @param section_name The name of the section to find.
 * @return A pointer to the IniSection if found, otherwise NULL.
 */
IniSection *iniGetSection(const IniConfig *config, const char *sectionName)
{
    if (!config)
        return NULL;
    for (int i = 0; i < config->numSections; i++)
    {
        if (strcmp(config->sections[i].name, sectionName) == 0)
            return &config->sections[i];
    }
    return NULL;
}

/**
 * @brief Frees all memory associated with an IniConfig structure.
 * This includes all sections, key-value pairs, and their names.
 * @param config Pointer to the INI configuration structure to free.
 */
void iniFree(IniConfig *config)
{
    if (!config)
        return;
    for (int i = 0; i < config->numSections; i++)
    {
        for (int j = 0; j < config->sections[i].numPairs; j++)
        {
            free(config->sections[i].pairs[j].key);
            free(config->sections[i].pairs[j].value);
        }
        free(config->sections[i].pairs);
        free(config->sections[i].name);
    }
    free(config->sections);
    free(config);
}

/**
 * @brief Sets a key-value pair in a given section.
 * If the section does not exist, it is created.
 * If the key exists within the section, its value is updated.
 * If the key does not exist, it is created.
 * @param config Pointer to the INI configuration structure.
 * @param section_name The name of the section.
 * @param key The key to set.
 * @param value The value to associate with the key.
 */
bool iniSetValue(IniConfig *config, const char *sectionName, const char *key, const char *value)
{
    if (!config || !sectionName || !key || !value)
    {
        return false;
    }

    // Find the section, or create it if it doesn't exist.
    IniSection *section = iniGetSection(config, sectionName);
    if (!section)
    {
        config->sections = realloc(config->sections, (config->numSections + 1) * sizeof(IniSection));
        section = &config->sections[config->numSections++];
        section->name = strdup(sectionName);
        section->pairs = NULL;
        section->numPairs = 0;
    }

    // Find the key within the section.
    for (int i = 0; i < section->numPairs; i++)
    {
        if (strcmp(section->pairs[i].key, key) == 0)
        {
            // Key found, update the value.
            free(section->pairs[i].value);
            section->pairs[i].value = strdup(value);
            return true;
        }
    }

    // Key not found, add a new key-value pair.
    section->pairs = realloc(section->pairs, (section->numPairs + 1) * sizeof(IniKeyValuePair));
    IniKeyValuePair *pair = &section->pairs[section->numPairs++];
    pair->key = strdup(key);
    pair->value = strdup(value);
    return true;
}

/**
 * @brief Saves the in-memory INI configuration to a file.
 * NOTE: This will overwrite the existing file and will lose any comments
 * or special formatting from the original.
 * @param config Pointer to the INI configuration to save.
 * @param filename The path of the file to save to.
 * @return 0 on success, -1 on failure (e.g., cannot open file for writing).
 */
int iniSave(const IniConfig *config, const char *filename)
{
    if (!config || !filename)
    {
        return -1;
    }

    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Error saving INI file");
        return -1;
    }

    // Write each section and its key-value pairs
    for (int i = 0; i < config->numSections; i++)
    {
        IniSection *section = &config->sections[i];
        fprintf(file, "[%s]\n", section->name);
        for (int j = 0; j < section->numPairs; j++)
        {
            IniKeyValuePair *pair = &section->pairs[j];
            fprintf(file, "%s = %s\n", pair->key, pair->value);
        }
        // Add a blank line between sections for readability
        if (i < config->numSections - 1)
        {
            fprintf(file, "\n");
        }
    }

    fclose(file);
    return 0;
}

const char *iniGetValue(const IniConfig *config, const char *sectionName, const char *key)
{
    IniSection *section = iniGetSection(config, sectionName);
    if (!section)
        return NULL;

    for (int i = 0; i < section->numPairs; i++)
    {
        if (strcmp(section->pairs[i].key, key) == 0)
            return section->pairs[i].value;
    }

    return NULL;
}
