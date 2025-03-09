#ifndef CUSTOM_CURSORS_H
#define CUSTOM_CURSORS_H

int loadNewCursor(const char *cursorFileName, void **cursor, int customWidth, int customHeight);
void loadCursors();
void setCursor(void *cursor);
void hideCursor();
void showCursor();
void showPhCursor();
#endif
