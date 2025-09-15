#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "config.h"
#include "fpsLimiter.h"

FpsLimit fpsLimit;
double lastTime = 0.0;
int frameCount = 0;
double fps = 0.0;

void initFpsLimiter()
{
    if (getConfig()->fpsLimiter == 1)
    {
        fpsLimit.targetFrameTime = 1000000 / getConfig()->fpsTarget;
        fpsLimit.frameEnd = clockNow();
    }
}

double getTimeInMilliseconds()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000.0) + (time.tv_usec / 1000.0);
}

double getTimeInSeconds()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec / 1000000.0;
}

double calculateFps()
{
    double currentTime = getTimeInSeconds();
    double deltaTime = currentTime - lastTime;
    frameCount++;
    if (deltaTime >= 1.0)
    {
        fps = frameCount / deltaTime;
        frameCount = 0;
        lastTime = currentTime;
    }
    return fps;
}

long clockNow()
{
    struct timeval time_now;
    gettimeofday(&time_now, NULL);
    return time_now.tv_sec * 1000000L + time_now.tv_usec;
}

void frameTiming()
{
    fpsLimit.frameStart = clockNow();
    fpsLimiter(&fpsLimit);
    fpsLimit.frameEnd = clockNow();
}

void fpsLimiter(FpsLimit *stats)
{
    stats->sleepTime = stats->targetFrameTime - (stats->frameStart - stats->frameEnd);

    if (stats->sleepTime > stats->frameOverhead)
    {
        long adjustedSleep = stats->sleepTime - stats->frameOverhead;

        usleep(adjustedSleep);

        stats->frameOverhead = (clockNow() - stats->frameStart) - adjustedSleep;

        if (stats->frameOverhead > stats->targetFrameTime / 2)
        {
            stats->frameOverhead = 0;
        }
    }
}