#pragma once

#include <SDL3/SDL_events.h>
#include <stdint.h>
#include <sys/types.h>

void phCoordinates(int x, int y, int w, int h, int button);
ssize_t phRead(int fd, void *buf, size_t count);
void phTouchScreenCursor(int mX, int mY, int *motX, int *motY);
bool phIsInsideTouchScreen(int mX, int mY, int *x, int *y);
void phTouchClick(int x, int y, int type);
