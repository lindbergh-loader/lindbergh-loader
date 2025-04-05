#pragma once

#include <sys/types.h>

void phCoordinates(int x, int y, int w, int h, int button);
ssize_t phRead(int fd, void *buf, size_t count);
