#pragma once

#include <stdio.h>

void setVariable(size_t address, size_t value);
void patchMemoryFromString(size_t address, char *value);
void detourFunction(size_t address, void *function);
void replaceCallAtAddress(size_t address, void *function);
void stubReturn();
int stubRetZero();
int stubRetOne();
int stubRetMinusOne();
char stubRetZeroChar();
void *trampolineHook(void *target, void *replacement, size_t saveSize);
void *findStaticFnAddr(const char *functionName);