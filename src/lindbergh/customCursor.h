#pragma once

#include <SDL3/SDL_mouse.h>
#include <stdio.h>

int loadNewCursor(const char *cursorFileName, SDL_Cursor **cursor, int customWidth, int customHeight, int hotX, int hotY);
void loadCursors();
void setCursor(void *cursor);
void hideCursor();
void showCursor();
void showPhCursor();
void CPlayer_Main(int param1);
int hod4VsPrintf(char *str, const char *format, va_list arg);
