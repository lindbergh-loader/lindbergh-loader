#include <sys/types.h>

int initPatch();
void setVariable(size_t address, size_t value);
void patchMemory(size_t address, char *value);
void detourFunction(size_t address, void *function);
void replaceCallAtAddress(size_t address, void *function);
void stubReturn();
int stubRetOne();
int stubRetMinusOne();
char stubRetZeroChar();

int patchedPrintf(char *format,...);
int patchedPuts(char *s);