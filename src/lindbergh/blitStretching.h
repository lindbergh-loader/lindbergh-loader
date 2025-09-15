#pragma once
#include <stdbool.h>

typedef struct
{
    int W;
    int H;
    int X;
    int Y;
    float gameScale;
} Dest;

void initBlitting();
void blitSetWidthandHeightSize();
int blitInitializeFbo();
void blitStretch();
