#include <pthread.h>

#include <linux/input-event-codes.h>

#define SIZE 300
#define CONTROLLER_THREAD_MAX 256
#define MAX_INPUTS 512

typedef struct
{
    char *arcade[SIZE];
    char *pc[SIZE];
    int count;
} ControllerMapping;

typedef struct
{
    char name[SIZE];
    int player;
    int channel;
    int enabled;

    char minName[SIZE];
    int minPlayer;
    int minChannel;
    int minEnabled;

    char maxName[SIZE];
    int maxPlayer;
    int maxChannel;
    int maxEnabled;

    int isAnalogue;
    int isNeg; // reversed axis

    char shakeName[SIZE];
    int shakeChannel;
    int shakeEnabled;
    int shakePlayer;
    double shakePreviousScaled;

    int isCoin;
} ArcadeInput;

typedef enum
{
    CONTROLLER_STATUS_SUCCESS = 0,
    CONTROLLER_STATUS_ERROR = 1
} ControllerStatus;

typedef enum {
    NO_SPECIAL_FUNCTION = 0,
    ANALOGUE_TO_DIGITAL_MAX,
    ANALOGUE_TO_DIGITAL_MIN,
    DIGITAL_TO_ANALOGUE,
    ANALOGUE_SHAKE
} SpecialFunction;

typedef struct
{
    char inputName[SIZE];
    char inputTechName[SIZE];
    char inputTechNegName[SIZE];
    int evType;
    int evCode;
    SpecialFunction specialFunction;
} ControllerInput;

typedef struct
{
    char name[SIZE];
    char path[SIZE];
    char physicalLocation[SIZE];
    int absMax[ABS_MAX];
    int absMin[ABS_MAX];

    ControllerInput inputs[MAX_INPUTS];
    int inputCount;

    ArcadeInput keyTriggers[KEY_MAX];
    ArcadeInput absTriggers[ABS_MAX];

    int enabled;
    int inUse;

} Controller;

typedef struct
{
    Controller *controller;
    int count;

    pthread_t thread[CONTROLLER_THREAD_MAX];
    int threadIndex;
    int threadsRunning;
} Controllers;

ControllerStatus initControllers(Controllers *controllers);
ControllerStatus stopControllers(Controllers *controllers);

