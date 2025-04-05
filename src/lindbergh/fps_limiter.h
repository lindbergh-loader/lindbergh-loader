#pragma once

typedef struct
{
    long targetFrameTime; // Target frame time in microseconds
    long frameStart;      // Start time of the frame
    long frameEnd;        // End time of the frame
    long sleepTime;       // Sleep time to maintain the frame rate
    long frameOverhead;   // Overhead time in the frame
} fps_limit;

long Clock_now();
void FpsLimiter(fps_limit *stats);
double calculateFps();
