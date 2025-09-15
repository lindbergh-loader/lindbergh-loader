#pragma once

/**
 * @brief Generates a default controls.ini file based on the hardcoded default binding arrays.
 * This ensures the generated INI is always in sync with the application's default behavior.
 * @param fileName The name of the file to create (e.g., "controls.ini").
 */
void createDefaultControlsIni(const char *fileName);