#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "evdevinput.h"
#include "config.h"
#include "jvs.h"
#include "log.h"

int jvsBits = 10;

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x) ((x) % BITS_PER_LONG)
#define LONG(x) ((x) / BITS_PER_LONG)
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)
#define NAME_ELEMENT(element) [element] = #element

ArcadeInput arcadeInputs[] = {{"TEST_BUTTON", 0, BUTTON_TEST, 1},

                              {"PLAYER_1_BUTTON_START", 1, BUTTON_START, 1},
                              {"PLAYER_1_BUTTON_SERVICE", 1, BUTTON_SERVICE, 1},
                              {"PLAYER_1_BUTTON_UP", 1, BUTTON_UP, 1},
                              {"PLAYER_1_BUTTON_DOWN", 1, BUTTON_DOWN, 1},
                              {"PLAYER_1_BUTTON_LEFT", 1, BUTTON_LEFT, 1},
                              {"PLAYER_1_BUTTON_RIGHT", 1, BUTTON_RIGHT, 1},
                              {"PLAYER_1_BUTTON_1", 1, BUTTON_1, 1},
                              {"PLAYER_1_BUTTON_2", 1, BUTTON_2, 1},
                              {"PLAYER_1_BUTTON_3", 1, BUTTON_3, 1},
                              {"PLAYER_1_BUTTON_4", 1, BUTTON_4, 1},
                              {"PLAYER_1_BUTTON_5", 1, BUTTON_5, 1},
                              {"PLAYER_1_BUTTON_6", 1, BUTTON_6, 1},
                              {"PLAYER_1_BUTTON_7", 1, BUTTON_7, 1},
                              {"PLAYER_1_BUTTON_8", 1, BUTTON_8, 1},
                              {"PLAYER_1_BUTTON_9", 1, BUTTON_9, 1},
                              {"PLAYER_1_BUTTON_10", 1, BUTTON_10, 1},
                              {"PLAYER_1_COIN", 1, COIN, 1},

                              {"PLAYER_2_BUTTON_START", 2, BUTTON_START, 1},
                              {"PLAYER_2_BUTTON_SERVICE", 2, BUTTON_SERVICE, 1},
                              {"PLAYER_2_BUTTON_UP", 2, BUTTON_UP, 1},
                              {"PLAYER_2_BUTTON_DOWN", 2, BUTTON_DOWN, 1},
                              {"PLAYER_2_BUTTON_LEFT", 2, BUTTON_LEFT, 1},
                              {"PLAYER_2_BUTTON_RIGHT", 2, BUTTON_RIGHT, 1},
                              {"PLAYER_2_BUTTON_1", 2, BUTTON_1, 1},
                              {"PLAYER_2_BUTTON_2", 2, BUTTON_2, 1},
                              {"PLAYER_2_BUTTON_3", 2, BUTTON_3, 1},
                              {"PLAYER_2_BUTTON_4", 2, BUTTON_4, 1},
                              {"PLAYER_2_BUTTON_5", 2, BUTTON_5, 1},
                              {"PLAYER_2_BUTTON_6", 2, BUTTON_6, 1},
                              {"PLAYER_2_BUTTON_7", 2, BUTTON_7, 1},
                              {"PLAYER_2_BUTTON_8", 2, BUTTON_8, 1},
                              {"PLAYER_2_BUTTON_9", 2, BUTTON_9, 1},
                              {"PLAYER_2_BUTTON_10", 2, BUTTON_10, 1},
                              {"PLAYER_2_COIN", 2, COIN, 1},

                              {"ANALOGUE_1", 0, ANALOGUE_1, 1},
                              {"ANALOGUE_2", 0, ANALOGUE_2, 1},
                              {"ANALOGUE_3", 0, ANALOGUE_3, 1},
                              {"ANALOGUE_4", 0, ANALOGUE_4, 1},
                              {"ANALOGUE_5", 0, ANALOGUE_5, 1},
                              {"ANALOGUE_6", 0, ANALOGUE_6, 1},
                              {"ANALOGUE_7", 0, ANALOGUE_7, 1},
                              {"ANALOGUE_8", 0, ANALOGUE_8, 1},

                              {"END", 0, 0, 0}};

static const char *const events[EV_MAX + 1] = {
    [0 ... EV_MAX] = NULL, NAME_ELEMENT(EV_SYN),       NAME_ELEMENT(EV_KEY), NAME_ELEMENT(EV_REL), NAME_ELEMENT(EV_ABS),
    NAME_ELEMENT(EV_MSC),  NAME_ELEMENT(EV_LED),       NAME_ELEMENT(EV_SND), NAME_ELEMENT(EV_REP), NAME_ELEMENT(EV_FF),
    NAME_ELEMENT(EV_PWR),  NAME_ELEMENT(EV_FF_STATUS), NAME_ELEMENT(EV_SW),
};

static const int maxval[EV_MAX + 1] = {
    [0 ... EV_MAX] = -1, [EV_SYN] = SYN_MAX, [EV_KEY] = KEY_MAX, [EV_REL] = REL_MAX,
    [EV_ABS] = ABS_MAX,  [EV_MSC] = MSC_MAX, [EV_SW] = SW_MAX,   [EV_LED] = LED_MAX,
    [EV_SND] = SND_MAX,  [EV_REP] = REP_MAX, [EV_FF] = FF_MAX,   [EV_FF_STATUS] = FF_STATUS_MAX,
};

static const char *const absolutes[ABS_MAX + 1] = {
    [0 ... ABS_MAX] = NULL,
    NAME_ELEMENT(ABS_X),
    NAME_ELEMENT(ABS_Y),
    NAME_ELEMENT(ABS_Z),
    NAME_ELEMENT(ABS_RX),
    NAME_ELEMENT(ABS_RY),
    NAME_ELEMENT(ABS_RZ),
    NAME_ELEMENT(ABS_THROTTLE),
    NAME_ELEMENT(ABS_RUDDER),
    NAME_ELEMENT(ABS_WHEEL),
    NAME_ELEMENT(ABS_GAS),
    NAME_ELEMENT(ABS_BRAKE),
    NAME_ELEMENT(ABS_HAT0X),
    NAME_ELEMENT(ABS_HAT0Y),
    NAME_ELEMENT(ABS_HAT1X),
    NAME_ELEMENT(ABS_HAT1Y),
    NAME_ELEMENT(ABS_HAT2X),
    NAME_ELEMENT(ABS_HAT2Y),
    NAME_ELEMENT(ABS_HAT3X),
    NAME_ELEMENT(ABS_HAT3Y),
    NAME_ELEMENT(ABS_PRESSURE),
    NAME_ELEMENT(ABS_DISTANCE),
    NAME_ELEMENT(ABS_TILT_X),
    NAME_ELEMENT(ABS_TILT_Y),
    NAME_ELEMENT(ABS_TOOL_WIDTH),
    NAME_ELEMENT(ABS_VOLUME),
    NAME_ELEMENT(ABS_MISC),
#ifdef ABS_MT_BLOB_ID
    NAME_ELEMENT(ABS_MT_TOUCH_MAJOR),
    NAME_ELEMENT(ABS_MT_TOUCH_MINOR),
    NAME_ELEMENT(ABS_MT_WIDTH_MAJOR),
    NAME_ELEMENT(ABS_MT_WIDTH_MINOR),
    NAME_ELEMENT(ABS_MT_ORIENTATION),
    NAME_ELEMENT(ABS_MT_POSITION_X),
    NAME_ELEMENT(ABS_MT_POSITION_Y),
    NAME_ELEMENT(ABS_MT_TOOL_TYPE),
    NAME_ELEMENT(ABS_MT_BLOB_ID),
#endif
#ifdef ABS_MT_TRACKING_ID
    NAME_ELEMENT(ABS_MT_TRACKING_ID),
#endif
#ifdef ABS_MT_PRESSURE
    NAME_ELEMENT(ABS_MT_PRESSURE),
#endif
#ifdef ABS_MT_SLOT
    NAME_ELEMENT(ABS_MT_SLOT),
#endif
#ifdef ABS_MT_TOOL_X
    NAME_ELEMENT(ABS_MT_TOOL_X),
    NAME_ELEMENT(ABS_MT_TOOL_Y),
    NAME_ELEMENT(ABS_MT_DISTANCE),
#endif

};

static const char *const keys[KEY_MAX + 1] = {
    [0 ... KEY_MAX] = NULL,
    NAME_ELEMENT(KEY_RESERVED),
    NAME_ELEMENT(KEY_ESC),
    NAME_ELEMENT(KEY_1),
    NAME_ELEMENT(KEY_2),
    NAME_ELEMENT(KEY_3),
    NAME_ELEMENT(KEY_4),
    NAME_ELEMENT(KEY_5),
    NAME_ELEMENT(KEY_6),
    NAME_ELEMENT(KEY_7),
    NAME_ELEMENT(KEY_8),
    NAME_ELEMENT(KEY_9),
    NAME_ELEMENT(KEY_0),
    NAME_ELEMENT(KEY_MINUS),
    NAME_ELEMENT(KEY_EQUAL),
    NAME_ELEMENT(KEY_BACKSPACE),
    NAME_ELEMENT(KEY_TAB),
    NAME_ELEMENT(KEY_Q),
    NAME_ELEMENT(KEY_W),
    NAME_ELEMENT(KEY_E),
    NAME_ELEMENT(KEY_R),
    NAME_ELEMENT(KEY_T),
    NAME_ELEMENT(KEY_Y),
    NAME_ELEMENT(KEY_U),
    NAME_ELEMENT(KEY_I),
    NAME_ELEMENT(KEY_O),
    NAME_ELEMENT(KEY_P),
    NAME_ELEMENT(KEY_LEFTBRACE),
    NAME_ELEMENT(KEY_RIGHTBRACE),
    NAME_ELEMENT(KEY_ENTER),
    NAME_ELEMENT(KEY_LEFTCTRL),
    NAME_ELEMENT(KEY_A),
    NAME_ELEMENT(KEY_S),
    NAME_ELEMENT(KEY_D),
    NAME_ELEMENT(KEY_F),
    NAME_ELEMENT(KEY_G),
    NAME_ELEMENT(KEY_H),
    NAME_ELEMENT(KEY_J),
    NAME_ELEMENT(KEY_K),
    NAME_ELEMENT(KEY_L),
    NAME_ELEMENT(KEY_SEMICOLON),
    NAME_ELEMENT(KEY_APOSTROPHE),
    NAME_ELEMENT(KEY_GRAVE),
    NAME_ELEMENT(KEY_LEFTSHIFT),
    NAME_ELEMENT(KEY_BACKSLASH),
    NAME_ELEMENT(KEY_Z),
    NAME_ELEMENT(KEY_X),
    NAME_ELEMENT(KEY_C),
    NAME_ELEMENT(KEY_V),
    NAME_ELEMENT(KEY_B),
    NAME_ELEMENT(KEY_N),
    NAME_ELEMENT(KEY_M),
    NAME_ELEMENT(KEY_COMMA),
    NAME_ELEMENT(KEY_DOT),
    NAME_ELEMENT(KEY_SLASH),
    NAME_ELEMENT(KEY_RIGHTSHIFT),
    NAME_ELEMENT(KEY_KPASTERISK),
    NAME_ELEMENT(KEY_LEFTALT),
    NAME_ELEMENT(KEY_SPACE),
    NAME_ELEMENT(KEY_CAPSLOCK),
    NAME_ELEMENT(KEY_F1),
    NAME_ELEMENT(KEY_F2),
    NAME_ELEMENT(KEY_F3),
    NAME_ELEMENT(KEY_F4),
    NAME_ELEMENT(KEY_F5),
    NAME_ELEMENT(KEY_F6),
    NAME_ELEMENT(KEY_F7),
    NAME_ELEMENT(KEY_F8),
    NAME_ELEMENT(KEY_F9),
    NAME_ELEMENT(KEY_F10),
    NAME_ELEMENT(KEY_NUMLOCK),
    NAME_ELEMENT(KEY_SCROLLLOCK),
    NAME_ELEMENT(KEY_KP7),
    NAME_ELEMENT(KEY_KP8),
    NAME_ELEMENT(KEY_KP9),
    NAME_ELEMENT(KEY_KPMINUS),
    NAME_ELEMENT(KEY_KP4),
    NAME_ELEMENT(KEY_KP5),
    NAME_ELEMENT(KEY_KP6),
    NAME_ELEMENT(KEY_KPPLUS),
    NAME_ELEMENT(KEY_KP1),
    NAME_ELEMENT(KEY_KP2),
    NAME_ELEMENT(KEY_KP3),
    NAME_ELEMENT(KEY_KP0),
    NAME_ELEMENT(KEY_KPDOT),
    NAME_ELEMENT(KEY_ZENKAKUHANKAKU),
    NAME_ELEMENT(KEY_102ND),
    NAME_ELEMENT(KEY_F11),
    NAME_ELEMENT(KEY_F12),
    NAME_ELEMENT(KEY_RO),
    NAME_ELEMENT(KEY_KATAKANA),
    NAME_ELEMENT(KEY_HIRAGANA),
    NAME_ELEMENT(KEY_HENKAN),
    NAME_ELEMENT(KEY_KATAKANAHIRAGANA),
    NAME_ELEMENT(KEY_MUHENKAN),
    NAME_ELEMENT(KEY_KPJPCOMMA),
    NAME_ELEMENT(KEY_KPENTER),
    NAME_ELEMENT(KEY_RIGHTCTRL),
    NAME_ELEMENT(KEY_KPSLASH),
    NAME_ELEMENT(KEY_SYSRQ),
    NAME_ELEMENT(KEY_RIGHTALT),
    NAME_ELEMENT(KEY_LINEFEED),
    NAME_ELEMENT(KEY_HOME),
    NAME_ELEMENT(KEY_UP),
    NAME_ELEMENT(KEY_PAGEUP),
    NAME_ELEMENT(KEY_LEFT),
    NAME_ELEMENT(KEY_RIGHT),
    NAME_ELEMENT(KEY_END),
    NAME_ELEMENT(KEY_DOWN),
    NAME_ELEMENT(KEY_PAGEDOWN),
    NAME_ELEMENT(KEY_INSERT),
    NAME_ELEMENT(KEY_DELETE),
    NAME_ELEMENT(KEY_MACRO),
    NAME_ELEMENT(KEY_MUTE),
    NAME_ELEMENT(KEY_VOLUMEDOWN),
    NAME_ELEMENT(KEY_VOLUMEUP),
    NAME_ELEMENT(KEY_POWER),
    NAME_ELEMENT(KEY_KPEQUAL),
    NAME_ELEMENT(KEY_KPPLUSMINUS),
    NAME_ELEMENT(KEY_PAUSE),
    NAME_ELEMENT(KEY_KPCOMMA),
    NAME_ELEMENT(KEY_HANGUEL),
    NAME_ELEMENT(KEY_HANJA),
    NAME_ELEMENT(KEY_YEN),
    NAME_ELEMENT(KEY_LEFTMETA),
    NAME_ELEMENT(KEY_RIGHTMETA),
    NAME_ELEMENT(KEY_COMPOSE),
    NAME_ELEMENT(KEY_STOP),
    NAME_ELEMENT(KEY_AGAIN),
    NAME_ELEMENT(KEY_PROPS),
    NAME_ELEMENT(KEY_UNDO),
    NAME_ELEMENT(KEY_FRONT),
    NAME_ELEMENT(KEY_COPY),
    NAME_ELEMENT(KEY_OPEN),
    NAME_ELEMENT(KEY_PASTE),
    NAME_ELEMENT(KEY_FIND),
    NAME_ELEMENT(KEY_CUT),
    NAME_ELEMENT(KEY_HELP),
    NAME_ELEMENT(KEY_MENU),
    NAME_ELEMENT(KEY_CALC),
    NAME_ELEMENT(KEY_SETUP),
    NAME_ELEMENT(KEY_SLEEP),
    NAME_ELEMENT(KEY_WAKEUP),
    NAME_ELEMENT(KEY_FILE),
    NAME_ELEMENT(KEY_SENDFILE),
    NAME_ELEMENT(KEY_DELETEFILE),
    NAME_ELEMENT(KEY_XFER),
    NAME_ELEMENT(KEY_PROG1),
    NAME_ELEMENT(KEY_PROG2),
    NAME_ELEMENT(KEY_WWW),
    NAME_ELEMENT(KEY_MSDOS),
    NAME_ELEMENT(KEY_COFFEE),
    NAME_ELEMENT(KEY_DIRECTION),
    NAME_ELEMENT(KEY_CYCLEWINDOWS),
    NAME_ELEMENT(KEY_MAIL),
    NAME_ELEMENT(KEY_BOOKMARKS),
    NAME_ELEMENT(KEY_COMPUTER),
    NAME_ELEMENT(KEY_BACK),
    NAME_ELEMENT(KEY_FORWARD),
    NAME_ELEMENT(KEY_CLOSECD),
    NAME_ELEMENT(KEY_EJECTCD),
    NAME_ELEMENT(KEY_EJECTCLOSECD),
    NAME_ELEMENT(KEY_NEXTSONG),
    NAME_ELEMENT(KEY_PLAYPAUSE),
    NAME_ELEMENT(KEY_PREVIOUSSONG),
    NAME_ELEMENT(KEY_STOPCD),
    NAME_ELEMENT(KEY_RECORD),
    NAME_ELEMENT(KEY_REWIND),
    NAME_ELEMENT(KEY_PHONE),
    NAME_ELEMENT(KEY_ISO),
    NAME_ELEMENT(KEY_CONFIG),
    NAME_ELEMENT(KEY_HOMEPAGE),
    NAME_ELEMENT(KEY_REFRESH),
    NAME_ELEMENT(KEY_EXIT),
    NAME_ELEMENT(KEY_MOVE),
    NAME_ELEMENT(KEY_EDIT),
    NAME_ELEMENT(KEY_SCROLLUP),
    NAME_ELEMENT(KEY_SCROLLDOWN),
    NAME_ELEMENT(KEY_KPLEFTPAREN),
    NAME_ELEMENT(KEY_KPRIGHTPAREN),
    NAME_ELEMENT(KEY_F13),
    NAME_ELEMENT(KEY_F14),
    NAME_ELEMENT(KEY_F15),
    NAME_ELEMENT(KEY_F16),
    NAME_ELEMENT(KEY_F17),
    NAME_ELEMENT(KEY_F18),
    NAME_ELEMENT(KEY_F19),
    NAME_ELEMENT(KEY_F20),
    NAME_ELEMENT(KEY_F21),
    NAME_ELEMENT(KEY_F22),
    NAME_ELEMENT(KEY_F23),
    NAME_ELEMENT(KEY_F24),
    NAME_ELEMENT(KEY_PLAYCD),
    NAME_ELEMENT(KEY_PAUSECD),
    NAME_ELEMENT(KEY_PROG3),
    NAME_ELEMENT(KEY_PROG4),
    NAME_ELEMENT(KEY_SUSPEND),
    NAME_ELEMENT(KEY_CLOSE),
    NAME_ELEMENT(KEY_PLAY),
    NAME_ELEMENT(KEY_FASTFORWARD),
    NAME_ELEMENT(KEY_BASSBOOST),
    NAME_ELEMENT(KEY_PRINT),
    NAME_ELEMENT(KEY_HP),
    NAME_ELEMENT(KEY_CAMERA),
    NAME_ELEMENT(KEY_SOUND),
    NAME_ELEMENT(KEY_QUESTION),
    NAME_ELEMENT(KEY_EMAIL),
    NAME_ELEMENT(KEY_CHAT),
    NAME_ELEMENT(KEY_SEARCH),
    NAME_ELEMENT(KEY_CONNECT),
    NAME_ELEMENT(KEY_FINANCE),
    NAME_ELEMENT(KEY_SPORT),
    NAME_ELEMENT(KEY_SHOP),
    NAME_ELEMENT(KEY_ALTERASE),
    NAME_ELEMENT(KEY_CANCEL),
    NAME_ELEMENT(KEY_BRIGHTNESSDOWN),
    NAME_ELEMENT(KEY_BRIGHTNESSUP),
    NAME_ELEMENT(KEY_MEDIA),
    NAME_ELEMENT(KEY_UNKNOWN),
    NAME_ELEMENT(KEY_OK),
    NAME_ELEMENT(KEY_SELECT),
    NAME_ELEMENT(KEY_GOTO),
    NAME_ELEMENT(KEY_CLEAR),
    NAME_ELEMENT(KEY_POWER2),
    NAME_ELEMENT(KEY_OPTION),
    NAME_ELEMENT(KEY_INFO),
    NAME_ELEMENT(KEY_TIME),
    NAME_ELEMENT(KEY_VENDOR),
    NAME_ELEMENT(KEY_ARCHIVE),
    NAME_ELEMENT(KEY_PROGRAM),
    NAME_ELEMENT(KEY_CHANNEL),
    NAME_ELEMENT(KEY_FAVORITES),
    NAME_ELEMENT(KEY_EPG),
    NAME_ELEMENT(KEY_PVR),
    NAME_ELEMENT(KEY_MHP),
    NAME_ELEMENT(KEY_LANGUAGE),
    NAME_ELEMENT(KEY_TITLE),
    NAME_ELEMENT(KEY_SUBTITLE),
    NAME_ELEMENT(KEY_ANGLE),
    NAME_ELEMENT(KEY_ZOOM),
    NAME_ELEMENT(KEY_MODE),
    NAME_ELEMENT(KEY_KEYBOARD),
    NAME_ELEMENT(KEY_SCREEN),
    NAME_ELEMENT(KEY_PC),
    NAME_ELEMENT(KEY_TV),
    NAME_ELEMENT(KEY_TV2),
    NAME_ELEMENT(KEY_VCR),
    NAME_ELEMENT(KEY_VCR2),
    NAME_ELEMENT(KEY_SAT),
    NAME_ELEMENT(KEY_SAT2),
    NAME_ELEMENT(KEY_CD),
    NAME_ELEMENT(KEY_TAPE),
    NAME_ELEMENT(KEY_RADIO),
    NAME_ELEMENT(KEY_TUNER),
    NAME_ELEMENT(KEY_PLAYER),
    NAME_ELEMENT(KEY_TEXT),
    NAME_ELEMENT(KEY_DVD),
    NAME_ELEMENT(KEY_AUX),
    NAME_ELEMENT(KEY_MP3),
    NAME_ELEMENT(KEY_AUDIO),
    NAME_ELEMENT(KEY_VIDEO),
    NAME_ELEMENT(KEY_DIRECTORY),
    NAME_ELEMENT(KEY_LIST),
    NAME_ELEMENT(KEY_MEMO),
    NAME_ELEMENT(KEY_CALENDAR),
    NAME_ELEMENT(KEY_RED),
    NAME_ELEMENT(KEY_GREEN),
    NAME_ELEMENT(KEY_YELLOW),
    NAME_ELEMENT(KEY_BLUE),
    NAME_ELEMENT(KEY_CHANNELUP),
    NAME_ELEMENT(KEY_CHANNELDOWN),
    NAME_ELEMENT(KEY_FIRST),
    NAME_ELEMENT(KEY_LAST),
    NAME_ELEMENT(KEY_AB),
    NAME_ELEMENT(KEY_NEXT),
    NAME_ELEMENT(KEY_RESTART),
    NAME_ELEMENT(KEY_SLOW),
    NAME_ELEMENT(KEY_SHUFFLE),
    NAME_ELEMENT(KEY_BREAK),
    NAME_ELEMENT(KEY_PREVIOUS),
    NAME_ELEMENT(KEY_DIGITS),
    NAME_ELEMENT(KEY_TEEN),
    NAME_ELEMENT(KEY_TWEN),
    NAME_ELEMENT(KEY_DEL_EOL),
    NAME_ELEMENT(KEY_DEL_EOS),
    NAME_ELEMENT(KEY_INS_LINE),
    NAME_ELEMENT(KEY_DEL_LINE),
    NAME_ELEMENT(KEY_VIDEOPHONE),
    NAME_ELEMENT(KEY_GAMES),
    NAME_ELEMENT(KEY_ZOOMIN),
    NAME_ELEMENT(KEY_ZOOMOUT),
    NAME_ELEMENT(KEY_ZOOMRESET),
    NAME_ELEMENT(KEY_WORDPROCESSOR),
    NAME_ELEMENT(KEY_EDITOR),
    NAME_ELEMENT(KEY_SPREADSHEET),
    NAME_ELEMENT(KEY_GRAPHICSEDITOR),
    NAME_ELEMENT(KEY_PRESENTATION),
    NAME_ELEMENT(KEY_DATABASE),
    NAME_ELEMENT(KEY_NEWS),
    NAME_ELEMENT(KEY_VOICEMAIL),
    NAME_ELEMENT(KEY_ADDRESSBOOK),
    NAME_ELEMENT(KEY_MESSENGER),
    NAME_ELEMENT(KEY_DISPLAYTOGGLE),
#ifdef KEY_SPELLCHECK
    NAME_ELEMENT(KEY_SPELLCHECK),
#endif
#ifdef KEY_LOGOFF
    NAME_ELEMENT(KEY_LOGOFF),
#endif
#ifdef KEY_DOLLAR
    NAME_ELEMENT(KEY_DOLLAR),
#endif
#ifdef KEY_EURO
    NAME_ELEMENT(KEY_EURO),
#endif
#ifdef KEY_FRAMEBACK
    NAME_ELEMENT(KEY_FRAMEBACK),
#endif
#ifdef KEY_FRAMEFORWARD
    NAME_ELEMENT(KEY_FRAMEFORWARD),
#endif
#ifdef KEY_CONTEXT_MENU
    NAME_ELEMENT(KEY_CONTEXT_MENU),
#endif
#ifdef KEY_MEDIA_REPEAT
    NAME_ELEMENT(KEY_MEDIA_REPEAT),
#endif
#ifdef KEY_10CHANNELSUP
    NAME_ELEMENT(KEY_10CHANNELSUP),
#endif
#ifdef KEY_10CHANNELSDOWN
    NAME_ELEMENT(KEY_10CHANNELSDOWN),
#endif
#ifdef KEY_IMAGES
    NAME_ELEMENT(KEY_IMAGES),
#endif
    NAME_ELEMENT(KEY_DEL_EOL),
    NAME_ELEMENT(KEY_DEL_EOS),
    NAME_ELEMENT(KEY_INS_LINE),
    NAME_ELEMENT(KEY_DEL_LINE),
    NAME_ELEMENT(KEY_FN),
    NAME_ELEMENT(KEY_FN_ESC),
    NAME_ELEMENT(KEY_FN_F1),
    NAME_ELEMENT(KEY_FN_F2),
    NAME_ELEMENT(KEY_FN_F3),
    NAME_ELEMENT(KEY_FN_F4),
    NAME_ELEMENT(KEY_FN_F5),
    NAME_ELEMENT(KEY_FN_F6),
    NAME_ELEMENT(KEY_FN_F7),
    NAME_ELEMENT(KEY_FN_F8),
    NAME_ELEMENT(KEY_FN_F9),
    NAME_ELEMENT(KEY_FN_F10),
    NAME_ELEMENT(KEY_FN_F11),
    NAME_ELEMENT(KEY_FN_F12),
    NAME_ELEMENT(KEY_FN_1),
    NAME_ELEMENT(KEY_FN_2),
    NAME_ELEMENT(KEY_FN_D),
    NAME_ELEMENT(KEY_FN_E),
    NAME_ELEMENT(KEY_FN_F),
    NAME_ELEMENT(KEY_FN_S),
    NAME_ELEMENT(KEY_FN_B),
    NAME_ELEMENT(KEY_BRL_DOT1),
    NAME_ELEMENT(KEY_BRL_DOT2),
    NAME_ELEMENT(KEY_BRL_DOT3),
    NAME_ELEMENT(KEY_BRL_DOT4),
    NAME_ELEMENT(KEY_BRL_DOT5),
    NAME_ELEMENT(KEY_BRL_DOT6),
    NAME_ELEMENT(KEY_BRL_DOT7),
    NAME_ELEMENT(KEY_BRL_DOT8),
    NAME_ELEMENT(KEY_BRL_DOT9),
    NAME_ELEMENT(KEY_BRL_DOT10),
#ifdef KEY_NUMERIC_0
    NAME_ELEMENT(KEY_NUMERIC_0),
    NAME_ELEMENT(KEY_NUMERIC_1),
    NAME_ELEMENT(KEY_NUMERIC_2),
    NAME_ELEMENT(KEY_NUMERIC_3),
    NAME_ELEMENT(KEY_NUMERIC_4),
    NAME_ELEMENT(KEY_NUMERIC_5),
    NAME_ELEMENT(KEY_NUMERIC_6),
    NAME_ELEMENT(KEY_NUMERIC_7),
    NAME_ELEMENT(KEY_NUMERIC_8),
    NAME_ELEMENT(KEY_NUMERIC_9),
    NAME_ELEMENT(KEY_NUMERIC_STAR),
    NAME_ELEMENT(KEY_NUMERIC_POUND),
#endif
    NAME_ELEMENT(KEY_BATTERY),
    NAME_ELEMENT(KEY_BLUETOOTH),
    NAME_ELEMENT(KEY_BRIGHTNESS_CYCLE),
    NAME_ELEMENT(KEY_BRIGHTNESS_ZERO),
#ifdef KEY_DASHBOARD
    NAME_ELEMENT(KEY_DASHBOARD),
#endif
    NAME_ELEMENT(KEY_DISPLAY_OFF),
    NAME_ELEMENT(KEY_DOCUMENTS),
    NAME_ELEMENT(KEY_FORWARDMAIL),
    NAME_ELEMENT(KEY_NEW),
    NAME_ELEMENT(KEY_KBDILLUMDOWN),
    NAME_ELEMENT(KEY_KBDILLUMUP),
    NAME_ELEMENT(KEY_KBDILLUMTOGGLE),
    NAME_ELEMENT(KEY_REDO),
    NAME_ELEMENT(KEY_REPLY),
    NAME_ELEMENT(KEY_SAVE),
#ifdef KEY_SCALE
    NAME_ELEMENT(KEY_SCALE),
#endif
    NAME_ELEMENT(KEY_SEND),
    NAME_ELEMENT(KEY_SCREENLOCK),
    NAME_ELEMENT(KEY_SWITCHVIDEOMODE),
#ifdef KEY_UWB
    NAME_ELEMENT(KEY_UWB),
#endif
#ifdef KEY_VIDEO_NEXT
    NAME_ELEMENT(KEY_VIDEO_NEXT),
#endif
#ifdef KEY_VIDEO_PREV
    NAME_ELEMENT(KEY_VIDEO_PREV),
#endif
#ifdef KEY_WIMAX
    NAME_ELEMENT(KEY_WIMAX),
#endif
#ifdef KEY_WLAN
    NAME_ELEMENT(KEY_WLAN),
#endif
#ifdef KEY_RFKILL
    NAME_ELEMENT(KEY_RFKILL),
#endif
#ifdef KEY_MICMUTE
    NAME_ELEMENT(KEY_MICMUTE),
#endif
#ifdef KEY_CAMERA_FOCUS
    NAME_ELEMENT(KEY_CAMERA_FOCUS),
#endif
#ifdef KEY_WPS_BUTTON
    NAME_ELEMENT(KEY_WPS_BUTTON),
#endif
#ifdef KEY_TOUCHPAD_TOGGLE
    NAME_ELEMENT(KEY_TOUCHPAD_TOGGLE),
    NAME_ELEMENT(KEY_TOUCHPAD_ON),
    NAME_ELEMENT(KEY_TOUCHPAD_OFF),
#endif
#ifdef KEY_CAMERA_ZOOMIN
    NAME_ELEMENT(KEY_CAMERA_ZOOMIN),
    NAME_ELEMENT(KEY_CAMERA_ZOOMOUT),
    NAME_ELEMENT(KEY_CAMERA_UP),
    NAME_ELEMENT(KEY_CAMERA_DOWN),
    NAME_ELEMENT(KEY_CAMERA_LEFT),
    NAME_ELEMENT(KEY_CAMERA_RIGHT),
#endif
#ifdef KEY_ATTENDANT_ON
    NAME_ELEMENT(KEY_ATTENDANT_ON),
    NAME_ELEMENT(KEY_ATTENDANT_OFF),
    NAME_ELEMENT(KEY_ATTENDANT_TOGGLE),
    NAME_ELEMENT(KEY_LIGHTS_TOGGLE),
#endif

    NAME_ELEMENT(BTN_0),
    NAME_ELEMENT(BTN_1),
    NAME_ELEMENT(BTN_2),
    NAME_ELEMENT(BTN_3),
    NAME_ELEMENT(BTN_4),
    NAME_ELEMENT(BTN_5),
    NAME_ELEMENT(BTN_6),
    NAME_ELEMENT(BTN_7),
    NAME_ELEMENT(BTN_8),
    NAME_ELEMENT(BTN_9),
    NAME_ELEMENT(BTN_LEFT),
    NAME_ELEMENT(BTN_RIGHT),
    NAME_ELEMENT(BTN_MIDDLE),
    NAME_ELEMENT(BTN_SIDE),
    NAME_ELEMENT(BTN_EXTRA),
    NAME_ELEMENT(BTN_FORWARD),
    NAME_ELEMENT(BTN_BACK),
    NAME_ELEMENT(BTN_TASK),
    NAME_ELEMENT(BTN_TRIGGER),
    NAME_ELEMENT(BTN_THUMB),
    NAME_ELEMENT(BTN_THUMB2),
    NAME_ELEMENT(BTN_TOP),
    NAME_ELEMENT(BTN_TOP2),
    NAME_ELEMENT(BTN_PINKIE),
    NAME_ELEMENT(BTN_BASE),
    NAME_ELEMENT(BTN_BASE2),
    NAME_ELEMENT(BTN_BASE3),
    NAME_ELEMENT(BTN_BASE4),
    NAME_ELEMENT(BTN_BASE5),
    NAME_ELEMENT(BTN_BASE6),
    NAME_ELEMENT(BTN_DEAD),
    NAME_ELEMENT(BTN_C),
#ifdef BTN_SOUTH
    NAME_ELEMENT(BTN_SOUTH),
    NAME_ELEMENT(BTN_EAST),
    NAME_ELEMENT(BTN_NORTH),
    NAME_ELEMENT(BTN_WEST),
#else
    NAME_ELEMENT(BTN_A),
    NAME_ELEMENT(BTN_B),
    NAME_ELEMENT(BTN_X),
    NAME_ELEMENT(BTN_Y),
#endif
    NAME_ELEMENT(BTN_Z),
    NAME_ELEMENT(BTN_TL),
    NAME_ELEMENT(BTN_TR),
    NAME_ELEMENT(BTN_TL2),
    NAME_ELEMENT(BTN_TR2),
    NAME_ELEMENT(BTN_SELECT),
    NAME_ELEMENT(BTN_START),
    NAME_ELEMENT(BTN_MODE),
    NAME_ELEMENT(BTN_THUMBL),
    NAME_ELEMENT(BTN_THUMBR),
    NAME_ELEMENT(BTN_TOOL_PEN),
    NAME_ELEMENT(BTN_TOOL_RUBBER),
    NAME_ELEMENT(BTN_TOOL_BRUSH),
    NAME_ELEMENT(BTN_TOOL_PENCIL),
    NAME_ELEMENT(BTN_TOOL_AIRBRUSH),
    NAME_ELEMENT(BTN_TOOL_FINGER),
    NAME_ELEMENT(BTN_TOOL_MOUSE),
    NAME_ELEMENT(BTN_TOOL_LENS),
    NAME_ELEMENT(BTN_TOUCH),
    NAME_ELEMENT(BTN_STYLUS),
    NAME_ELEMENT(BTN_STYLUS2),
    NAME_ELEMENT(BTN_TOOL_DOUBLETAP),
    NAME_ELEMENT(BTN_TOOL_TRIPLETAP),
#ifdef BTN_TOOL_QUADTAP
    NAME_ELEMENT(BTN_TOOL_QUADTAP),
#endif
    NAME_ELEMENT(BTN_GEAR_DOWN),
    NAME_ELEMENT(BTN_GEAR_UP),

#ifdef BTN_DPAD_UP
    NAME_ELEMENT(BTN_DPAD_UP),
    NAME_ELEMENT(BTN_DPAD_DOWN),
    NAME_ELEMENT(BTN_DPAD_LEFT),
    NAME_ELEMENT(BTN_DPAD_RIGHT),
#endif
#ifdef KEY_ALS_TOGGLE
    NAME_ELEMENT(KEY_ALS_TOGGLE),
#endif
#ifdef KEY_BUTTONCONFIG
    NAME_ELEMENT(KEY_BUTTONCONFIG),
#endif
#ifdef KEY_TASKMANAGER
    NAME_ELEMENT(KEY_TASKMANAGER),
#endif
#ifdef KEY_JOURNAL
    NAME_ELEMENT(KEY_JOURNAL),
#endif
#ifdef KEY_CONTROLPANEL
    NAME_ELEMENT(KEY_CONTROLPANEL),
#endif
#ifdef KEY_APPSELECT
    NAME_ELEMENT(KEY_APPSELECT),
#endif
#ifdef KEY_SCREENSAVER
    NAME_ELEMENT(KEY_SCREENSAVER),
#endif
#ifdef KEY_VOICECOMMAND
    NAME_ELEMENT(KEY_VOICECOMMAND),
#endif
#ifdef KEY_BRIGHTNESS_MIN
    NAME_ELEMENT(KEY_BRIGHTNESS_MIN),
#endif
#ifdef KEY_BRIGHTNESS_MAX
    NAME_ELEMENT(KEY_BRIGHTNESS_MAX),
#endif
#ifdef KEY_KBDINPUTASSIST_PREV
    NAME_ELEMENT(KEY_KBDINPUTASSIST_PREV),
#endif
#ifdef KEY_KBDINPUTASSIST_NEXT
    NAME_ELEMENT(KEY_KBDINPUTASSIST_NEXT),
#endif
#ifdef KEY_KBDINPUTASSIST_PREVGROUP
    NAME_ELEMENT(KEY_KBDINPUTASSIST_PREVGROUP),
#endif
#ifdef KEY_KBDINPUTASSIST_NEXTGROUP
    NAME_ELEMENT(KEY_KBDINPUTASSIST_NEXTGROUP),
#endif
#ifdef KEY_KBDINPUTASSIST_ACCEPT
    NAME_ELEMENT(KEY_KBDINPUTASSIST_ACCEPT),
#endif
#ifdef KEY_KBDINPUTASSIST_CANCEL
    NAME_ELEMENT(KEY_KBDINPUTASSIST_CANCEL),
#endif
#ifdef BTN_TRIGGER_HAPPY
    NAME_ELEMENT(BTN_TRIGGER_HAPPY1),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY11),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY2),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY12),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY3),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY13),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY4),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY14),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY5),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY15),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY6),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY16),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY7),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY17),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY8),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY18),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY9),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY19),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY10),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY20),

    NAME_ELEMENT(BTN_TRIGGER_HAPPY21),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY31),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY22),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY32),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY23),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY33),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY24),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY34),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY25),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY35),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY26),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY36),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY27),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY37),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY28),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY38),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY29),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY39),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY30),
    NAME_ELEMENT(BTN_TRIGGER_HAPPY40),
#endif
#ifdef BTN_TOOL_QUINTTAP
    NAME_ELEMENT(BTN_TOOL_QUINTTAP),
#endif
};

static const char *const *const names[EV_MAX + 1] = {
    [0 ... EV_MAX] = NULL,
    [EV_KEY] = keys,
    [EV_ABS] = absolutes,
};

typedef struct
{
    Controller *controller;
    Controllers *controllers;
    int analogue_deadzone_start[8];
    int analogue_deadzone_middle[8];
    int analogue_deadzone_end[8];
} ThreadArguments;

typedef struct
{
    double start_max;
    double middle_min;
    double middle_max;
    double end_min;
} AnalogueDeadzones;

static inline const char *typename(unsigned int type)
{
    return (type <= EV_MAX && events[type]) ? events[type] : "?";
}

static inline const char *codename(unsigned int type, unsigned int code)
{
    return (type <= EV_MAX && code <= maxval[type] && names[type] && names[type][code]) ? names[type][code] : "?";
}

static int isEventDevice(const struct dirent *dir)
{
    return strncmp("event", dir->d_name, 5) == 0;
}

ControllerStatus stopControllers(Controllers *controllers)
{
    controllers->threadsRunning = 0;
    for (int i = 0; i < controllers->threadIndex; i++)
    {
        pthread_join(controllers->thread[i], NULL);
    }

    free(controllers->controller);
    return CONTROLLER_STATUS_SUCCESS;
}

void normaliseName(char *string)
{
    for (size_t j = 0; j < strlen(string); j++)
    {
        string[j] = toupper(string[j]);
        if (string[j] == ' ' || string[j] == '/' || string[j] == '(' || string[j] == ',' || string[j] == '=' ||
            string[j] == '-' || string[j] == ')')
        {
            string[j] = '_';
        }
    }
}

static ControllerStatus listControllers(Controllers *controllers)
{
    struct dirent **namelist;
    controllers->count = scandir("/dev/input", &namelist, isEventDevice, alphasort);

    controllers->controller = malloc(sizeof(Controller) * controllers->count);
    if (controllers == NULL)
    {
        for (int i = 0; i < controllers->count; i++)
        {
            free(namelist[i]);
        }
        free(namelist);
        return CONTROLLER_STATUS_ERROR;
    }

    for (int i = 0; i < controllers->count; i++)
    {
        snprintf(controllers->controller[i].path, sizeof(controllers->controller[i].path), "%s/%s", "/dev/input",
                 namelist[i]->d_name);
        free(namelist[i]);

        controllers->controller[i].enabled = 0;
        controllers->controller[i].inputCount = 0;
        controllers->controller[i].inUse = 0;

        int controller = open(controllers->controller[i].path, O_RDONLY);
        if (controller < 0)
        {
            strncpy(controllers->controller[i].name, "Unknown", SIZE);
            continue;
        }

        // Get the controllers name
        ioctl(controller, EVIOCGNAME(sizeof(controllers->controller[i].name)), controllers->controller[i].name);
        normaliseName(controllers->controller[i].name);

        // Count how many duplicates there are
        int duplicates = 0;
        for (int j = 0; j < i; j++)
        {
            if (strcmp(controllers->controller[i].name, controllers->controller[j].name) == 0)
            {
                duplicates += 1;
            }
        }
        // Append the controller number
        if (duplicates > 0)
        {
            char number[SIZE];
            sprintf(number, "%d", duplicates + 1);
            strcat(controllers->controller[i].name, "-");
            strcat(controllers->controller[i].name, number);
        }

        // Get the controllers physical location
        ioctl(controller, EVIOCGPHYS(sizeof(controllers->controller[i].physicalLocation)),
              controllers->controller[i].physicalLocation);
        for (size_t j = 0; j < strlen(controllers->controller[i].physicalLocation); j++)
        {
            if (controllers->controller[i].physicalLocation[j] == '/')
            {
                controllers->controller[i].physicalLocation[j] = 0;
                break;
            }
        }

        // Get the capabilities of the controllers
        unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
        memset(bit, 0, sizeof(bit));
        ioctl(controller, EVIOCGBIT(0, EV_MAX), bit[0]);

        if (test_bit(EV_KEY, bit[0]))
        {
            ioctl(controller, EVIOCGBIT(EV_KEY, KEY_MAX), bit[EV_KEY]);
            for (int code = 0; code < KEY_MAX; code++)
            {
                if (test_bit(code, bit[EV_KEY]))
                {
                    if(controllers->controller[i].inputCount >= MAX_INPUTS) { // the number of input is limited
		      fprintf(stderr, "warning, maximum number of inputs reached !\n");
		      break;
                    }

                    controllers->controller[i].enabled = 1;
                    ControllerInput *controllerInput =
                        &controllers->controller[i].inputs[controllers->controller[i].inputCount++];
                    controllerInput->evType = EV_KEY;
                    controllerInput->evCode = code;
                    controllerInput->specialFunction = NO_SPECIAL_FUNCTION;
                    strncpy(controllerInput->inputName, controllers->controller[i].name, SIZE);
                    strcat(controllerInput->inputName, "_");
                    strcat(controllerInput->inputName, codename(EV_KEY, code));
                    normaliseName(controllerInput->inputName);
                    if (snprintf(controllerInput->inputTechName, SIZE, "%s:KEY:%i", controllers->controller[i].path, code) >= 1024)
                    {
                        // hum ok, truncated value
                    }
                    strncpy(controllerInput->inputTechNegName, "-", SIZE); // unassignable value (not "" while some conf are empty)
                }
            }
        }

        if (test_bit(EV_ABS, bit[0]))
        {
            ioctl(controller, EVIOCGBIT(EV_ABS, KEY_MAX), bit[EV_ABS]);
            for (int code = 0; code < KEY_MAX; code++)
            {
                if (test_bit(code, bit[EV_ABS]))
                {
                    controllers->controller[i].enabled = 1;
                    ControllerInput *controllerInput =
                        &controllers->controller[i].inputs[controllers->controller[i].inputCount++];
                    controllerInput->evType = EV_ABS;
                    controllerInput->evCode = code;
                    controllerInput->specialFunction = NO_SPECIAL_FUNCTION;
                    strncpy(controllerInput->inputName, controllers->controller[i].name, SIZE);
                    strcat(controllerInput->inputName, "_");
                    strcat(controllerInput->inputName, codename(EV_ABS, code));
                    normaliseName(controllerInput->inputName);
                    if (snprintf(controllerInput->inputTechName, SIZE, "%s:ABS:%i", controllers->controller[i].path, code) >= 1024)
                    {
                        // hum ok, truncated value
                    }
                    if (snprintf(controllerInput->inputTechNegName, SIZE, "%s:ABS_NEG:%i", controllers->controller[i].path, code) >= 1024)
                    {
                        // hum ok, truncated value
                    }

                    ControllerInput *minControllerInput =
                        &controllers->controller[i].inputs[controllers->controller[i].inputCount++];
                    minControllerInput->evType = EV_ABS;
                    minControllerInput->evCode = code;
                    minControllerInput->specialFunction = ANALOGUE_TO_DIGITAL_MIN;
                    strncpy(minControllerInput->inputName, controllerInput->inputName, SIZE);
                    strcat(minControllerInput->inputName, "_MIN");
                    if (snprintf(minControllerInput->inputTechName, SIZE, "%s:ABS:%i:MIN", controllers->controller[i].path, code) >= 1024)
                    {
                        // hum ok, truncated value
                    }
                    if (snprintf(minControllerInput->inputTechNegName, SIZE, "%s:ABS_NEG:%i:MIN", controllers->controller[i].path, code) >=
                        1024)
                    {
                        // hum ok, truncated value
                    }

                    ControllerInput *maxControllerInput =
                        &controllers->controller[i].inputs[controllers->controller[i].inputCount++];
                    maxControllerInput->evType = EV_ABS;
                    maxControllerInput->evCode = code;
                    maxControllerInput->specialFunction = ANALOGUE_TO_DIGITAL_MAX;
                    strncpy(maxControllerInput->inputName, controllerInput->inputName, SIZE);
                    strcat(maxControllerInput->inputName, "_MAX");
                    if (snprintf(maxControllerInput->inputTechName, SIZE, "%s:ABS:%i:MAX", controllers->controller[i].path, code) >= 1024)
                    {
                        // hum ok, truncated value
                    }
                    if (snprintf(maxControllerInput->inputTechNegName, SIZE, "%s:ABS_NEG:%i:MAX", controllers->controller[i].path, code) >=
                        1024)
                    {
                        // hum ok, truncated value
                    }

                    ControllerInput *shakeControllerInput = &controllers->controller[i].inputs[controllers->controller[i].inputCount++];
                    shakeControllerInput->evType = EV_ABS;
                    shakeControllerInput->evCode = code;
                    shakeControllerInput->specialFunction = ANALOGUE_SHAKE;
                    strncpy(shakeControllerInput->inputName, controllerInput->inputName, SIZE);
                    strcat(shakeControllerInput->inputName, "_SHAKE");
                    if (snprintf(shakeControllerInput->inputTechName, SIZE, "%s:ABS:%i:SHAKE", controllers->controller[i].path, code) >=
                        1024)
                    {
                        // hum ok, truncated value
                    }

                    struct input_absinfo absoluteFeatures;
                    ioctl(controller, EVIOCGABS(code), &absoluteFeatures);
                    controllers->controller[i].absMin[code] = absoluteFeatures.minimum;
                    controllers->controller[i].absMax[code] = absoluteFeatures.maximum;

                }
            }
        }

        close(controller);
    }

    free(namelist);

    // Sort the list of controllers
    for (int i = 0; i < controllers->count - 1; i++)
    {
        for (int j = 0; j < controllers->count - 1 - i; j++)
        {
            Controller tmp;
            if (strcmp(controllers->controller[j].physicalLocation, controllers->controller[j + 1].physicalLocation) >
                0)
            {
                tmp = controllers->controller[j];
                controllers->controller[j] = controllers->controller[j + 1];
                controllers->controller[j + 1] = tmp;
            }
        }
    }

    return CONTROLLER_STATUS_SUCCESS;
}

void *controllerThread(void *_args)
{
    ThreadArguments *args = (ThreadArguments *)_args;

    int fd = open(args->controller->path, O_RDONLY);
    if (fd < 0)
    {
        printf("Error: Failed to open device file descriptor %d \n", fd);
        free(args);
        return 0;
    }

    struct input_event event;
    AnalogueDeadzones analogue_deadzones[8];

    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    fd_set file_descriptor;
    struct timeval tv;

    args->controllers->threadsRunning = 1;

    for (int i = 0; i < 8; ++i) {
        analogue_deadzones[i].start_max = args->analogue_deadzone_start[i] / 100.0;
        analogue_deadzones[i].middle_min = 0.5 - (args->analogue_deadzone_middle[i] / 100.0);
        analogue_deadzones[i].middle_max = 0.5 + (args->analogue_deadzone_middle[i] / 100.0);
        analogue_deadzones[i].end_min = 1.0 - (args->analogue_deadzone_end[i] / 100.0);
    }

    while (args->controllers->threadsRunning)
    {
        FD_ZERO(&file_descriptor);
        FD_SET(fd, &file_descriptor);

        tv.tv_sec = 0;
        tv.tv_usec = 2 * 1000;

        if (select(fd + 1, &file_descriptor, NULL, NULL, &tv) < 1)
            continue;

        if (read(fd, &event, sizeof(event)) == sizeof(event))
        {
            switch (event.type)
            {

            case EV_KEY:
            {
                if (!args->controller->keyTriggers[event.code].enabled)
                    continue;

                if (args->controller->keyTriggers[event.code].isAnalogue)
                {
                    setAnalogue(args->controller->keyTriggers[event.code].channel,
                                event.value == 0 ? 0 : 1 * (pow(2, jvsBits) - 1));
                    continue;
                }

                if(args->controller->keyTriggers[event.code].isCoin == 1) {
                  if(event.value == 1)
                    incrementCoin(args->controller->keyTriggers[event.code].player, 1);
                } else
                  setSwitch(args->controller->keyTriggers[event.code].player,
                            args->controller->keyTriggers[event.code].channel, event.value == 0 ? 0 : 1);

            }
            break;

            case EV_ABS:
            {
                double scaled =
                    ((double)event.value - (double)args->controller->absMin[event.code]) /
                    ((double)args->controller->absMax[event.code] - (double)args->controller->absMin[event.code]);

                if(args->controller->absTriggers[event.code].isNeg == 1) {
                  scaled = 1.0 - scaled;
                }

                if (args->controller->absTriggers[event.code].enabled)
                {
                    int channel = args->controller->absTriggers[event.code].channel;

		    if (args->controller->absTriggers[event.code].isAnalogue) {
		      // Deadzone handling
		      if (scaled < analogue_deadzones[channel].start_max)
                        scaled = 0.0;
		      if (scaled > analogue_deadzones[channel].middle_min && scaled < analogue_deadzones[channel].middle_max)
                        scaled = 0.5;
		      if (scaled > analogue_deadzones[channel].end_min)
                        scaled = 1.0;
		      setAnalogue(channel, scaled * (pow(2, jvsBits) - 1));
		    } else {
		      setSwitch(args->controller->absTriggers[event.code].player,
				args->controller->absTriggers[event.code].channel, scaled < 0.8 ? 0 : 1);
		    }
                }

                if(event.value <= ((args->controller->absMin[event.code] + args->controller->absMax[event.code]) / 2)) {
                  if (args->controller->absTriggers[event.code].minEnabled)
                    {
                      int channel = args->controller->absTriggers[event.code].minChannel;

		      if (args->controller->absTriggers[event.code].isAnalogue) {
			setAnalogue(channel, scaled < 0.2 ? 0.0 : pow(2, jvsBits) * 1.0);
		      } else {
			setSwitch(args->controller->absTriggers[event.code].minPlayer,
				  args->controller->absTriggers[event.code].minChannel, scaled < 0.2 ? 1 : 0);
		      }
                    }
                }

                if(event.value >= ((args->controller->absMin[event.code] + args->controller->absMax[event.code]) / 2)) {
                  if (args->controller->absTriggers[event.code].maxEnabled)
                    {
                       int channel = args->controller->absTriggers[event.code].maxChannel;

		       if (args->controller->absTriggers[event.code].isAnalogue) {
			 setAnalogue(channel, scaled > 0.8 ? pow(2, jvsBits) : 0.0);
		       } else {
			setSwitch(args->controller->absTriggers[event.code].maxPlayer,
				  args->controller->absTriggers[event.code].maxChannel, scaled > 0.8 ? 1 : 0);
		      }
                    }
                }
		if (args->controller->absTriggers[event.code].shakeEnabled)
		  {
		    int channel = args->controller->absTriggers[event.code].shakeChannel;
		    if( (scaled > args->controller->absTriggers[event.code].shakePreviousScaled &&  scaled - args->controller->absTriggers[event.code].shakePreviousScaled > 0.1) ||
			(scaled < args->controller->absTriggers[event.code].shakePreviousScaled && -scaled + args->controller->absTriggers[event.code].shakePreviousScaled > 0.1)) {
		      setAnalogue(channel, 0);
		    } else {
		      setAnalogue(channel, pow(2, jvsBits) / 2);
		    }
		    args->controller->absTriggers[event.code].shakePreviousScaled = scaled;
		  }
            }
            break;

            default:
                break;
            }
        }
    }

    close(fd);
    free(args);

    return NULL;
}

char *getMapping(char *mapping)
{
    EmulatorConfig *config = getConfig();

    // Test button
    if (strcmp(mapping, config->arcadeInputs.test) == 0)
        return "TEST_BUTTON";
    if (strcmp(mapping, config->arcadeInputs.player1_coin) == 0)
        return "PLAYER_1_COIN";
    if (strcmp(mapping, config->arcadeInputs.player2_coin) == 0)
        return "PLAYER_2_COIN";

    // Player 1 controls
    if (strcmp(mapping, config->arcadeInputs.player1_button_start) == 0)
        return "PLAYER_1_BUTTON_START";
    if (strcmp(mapping, config->arcadeInputs.player1_button_service) == 0)
        return "PLAYER_1_BUTTON_SERVICE";
    if (strcmp(mapping, config->arcadeInputs.player1_button_up) == 0)
        return "PLAYER_1_BUTTON_UP";
    if (strcmp(mapping, config->arcadeInputs.player1_button_down) == 0)
        return "PLAYER_1_BUTTON_DOWN";
    if (strcmp(mapping, config->arcadeInputs.player1_button_left) == 0)
        return "PLAYER_1_BUTTON_LEFT";
    if (strcmp(mapping, config->arcadeInputs.player1_button_right) == 0)
        return "PLAYER_1_BUTTON_RIGHT";
    if (strcmp(mapping, config->arcadeInputs.player1_button_1) == 0)
        return "PLAYER_1_BUTTON_1";
    if (strcmp(mapping, config->arcadeInputs.player1_button_2) == 0)
        return "PLAYER_1_BUTTON_2";
    if (strcmp(mapping, config->arcadeInputs.player1_button_3) == 0)
        return "PLAYER_1_BUTTON_3";
    if (strcmp(mapping, config->arcadeInputs.player1_button_4) == 0)
        return "PLAYER_1_BUTTON_4";
    if (strcmp(mapping, config->arcadeInputs.player1_button_5) == 0)
        return "PLAYER_1_BUTTON_5";
    if (strcmp(mapping, config->arcadeInputs.player1_button_6) == 0)
        return "PLAYER_1_BUTTON_6";
    if (strcmp(mapping, config->arcadeInputs.player1_button_7) == 0)
        return "PLAYER_1_BUTTON_7";
    if (strcmp(mapping, config->arcadeInputs.player1_button_8) == 0)
        return "PLAYER_1_BUTTON_8";
    if (strcmp(mapping, config->arcadeInputs.player1_button_9) == 0)
        return "PLAYER_1_BUTTON_9";
    if (strcmp(mapping, config->arcadeInputs.player1_button_10) == 0)
        return "PLAYER_1_BUTTON_10";

    // Player 2 controls
    if (strcmp(mapping, config->arcadeInputs.player2_button_start) == 0)
        return "PLAYER_2_BUTTON_START";
    if (strcmp(mapping, config->arcadeInputs.player2_button_service) == 0)
        return "PLAYER_2_BUTTON_SERVICE";
    if (strcmp(mapping, config->arcadeInputs.player2_button_up) == 0)
        return "PLAYER_2_BUTTON_UP";
    if (strcmp(mapping, config->arcadeInputs.player2_button_down) == 0)
        return "PLAYER_2_BUTTON_DOWN";
    if (strcmp(mapping, config->arcadeInputs.player2_button_left) == 0)
        return "PLAYER_2_BUTTON_LEFT";
    if (strcmp(mapping, config->arcadeInputs.player2_button_right) == 0)
        return "PLAYER_2_BUTTON_RIGHT";
    if (strcmp(mapping, config->arcadeInputs.player2_button_1) == 0)
        return "PLAYER_2_BUTTON_1";
    if (strcmp(mapping, config->arcadeInputs.player2_button_2) == 0)
        return "PLAYER_2_BUTTON_2";
    if (strcmp(mapping, config->arcadeInputs.player2_button_3) == 0)
        return "PLAYER_2_BUTTON_3";
    if (strcmp(mapping, config->arcadeInputs.player2_button_4) == 0)
        return "PLAYER_2_BUTTON_4";
    if (strcmp(mapping, config->arcadeInputs.player2_button_5) == 0)
        return "PLAYER_2_BUTTON_5";
    if (strcmp(mapping, config->arcadeInputs.player2_button_6) == 0)
        return "PLAYER_2_BUTTON_6";
    if (strcmp(mapping, config->arcadeInputs.player2_button_7) == 0)
        return "PLAYER_2_BUTTON_7";
    if (strcmp(mapping, config->arcadeInputs.player2_button_8) == 0)
        return "PLAYER_2_BUTTON_8";
    if (strcmp(mapping, config->arcadeInputs.player2_button_9) == 0)
        return "PLAYER_2_BUTTON_9";
    if (strcmp(mapping, config->arcadeInputs.player2_button_10) == 0)
        return "PLAYER_2_BUTTON_10";

    // Analogue inputs
    if (strcmp(mapping, config->arcadeInputs.analogue_1) == 0)
        return "ANALOGUE_1";
    if (strcmp(mapping, config->arcadeInputs.analogue_2) == 0)
        return "ANALOGUE_2";
    if (strcmp(mapping, config->arcadeInputs.analogue_3) == 0)
        return "ANALOGUE_3";
    if (strcmp(mapping, config->arcadeInputs.analogue_4) == 0)
        return "ANALOGUE_4";
    if (strcmp(mapping, config->arcadeInputs.analogue_5) == 0)
        return "ANALOGUE_5";
    if (strcmp(mapping, config->arcadeInputs.analogue_6) == 0)
        return "ANALOGUE_6";
    if (strcmp(mapping, config->arcadeInputs.analogue_7) == 0)
        return "ANALOGUE_7";
    if (strcmp(mapping, config->arcadeInputs.analogue_8) == 0)
        return "ANALOGUE_8";

    return NULL;
}

ControllerStatus getArcadeInputByName(char *name, ArcadeInput *input)
{
    int index = 0;
    char *tempName = arcadeInputs[index].name;
    while (strcmp(tempName, "END") != 0)
    {
        if (strcmp(tempName, name) == 0)
        {
            strncpy(input->name, arcadeInputs[index].name, SIZE);
            input->channel = arcadeInputs[index].channel;
            input->player = arcadeInputs[index].player;
            return CONTROLLER_STATUS_SUCCESS;
        }
        index = index + 1;
        tempName = arcadeInputs[index].name;
    }
    return CONTROLLER_STATUS_ERROR;
}

ControllerStatus startControllerThreads(Controllers *controllers)
{
    // Don't start EVDEV threads if SDL/X11-only mode is enabled
    if (getConfig()->inputMode == 1) {
        return CONTROLLER_STATUS_SUCCESS;
    }

    for (int i = 0; i < controllers->count; i++)
    {
        if (!controllers->controller[i].enabled)
            continue;

        for (int j = 0; j < ABS_MAX; j++)
        {
            controllers->controller[i].absTriggers[j].enabled = 0;
            controllers->controller[i].absTriggers[j].minEnabled = 0;
            controllers->controller[i].absTriggers[j].maxEnabled = 0;
            controllers->controller[i].absTriggers[j].shakeEnabled = 0;
            controllers->controller[i].absTriggers[j].isNeg = 0;
	    controllers->controller[i].absTriggers[j].isAnalogue = 1;
        }

        for (int j = 0; j < KEY_MAX; j++)
        {
            controllers->controller[i].keyTriggers[j].enabled = 0;
            controllers->controller[i].keyTriggers[j].isAnalogue = 0;
            controllers->controller[i].keyTriggers[j].isCoin = 0;
        }

        for (int j = 0; j < controllers->controller[i].inputCount; j++)
        {
            char *mapping = getMapping(controllers->controller[i].inputs[j].inputName);
            int negabs = 0;

            if (mapping == NULL) {
              // give a 2nd chance with a techninal mapping
              mapping = getMapping(controllers->controller[i].inputs[j].inputTechName);
              if (mapping == NULL) {
                // give a 3rd change with negativ technical mapping
                mapping = getMapping(controllers->controller[i].inputs[j].inputTechNegName);
                negabs = 1;
                if (mapping == NULL) {
                  continue;
                }
              }
            }

            ArcadeInput input = {0};
            ControllerStatus status = getArcadeInputByName(mapping, &input);
            if (status != CONTROLLER_STATUS_SUCCESS)
            {
                log_warn("Couldn't find arcade mapping with title %s\n", mapping);
                continue;
            }

            switch (controllers->controller[i].inputs[j].evType)
            {
            case EV_ABS:
            {
                switch (controllers->controller[i].inputs[j].specialFunction)
                {
                case NO_SPECIAL_FUNCTION:
                {
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].enabled = 1;
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].channel =
                        input.channel;
                    strncpy(controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].name,
			    input.name, SIZE);
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].player =
                        input.player;
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].isNeg = negabs;
		    if (strstr(input.name, "ANALOGUE") == NULL)
		      controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].isAnalogue = 0;
                }
                break;

                case ANALOGUE_TO_DIGITAL_MAX:
                {
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].maxEnabled = 1;
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].maxChannel =
                        input.channel;
                    strncpy(controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].maxName,
			    input.name, SIZE);
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].maxPlayer =
                        input.player;
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].isNeg = negabs;
		    if (strstr(input.name, "ANALOGUE") == NULL)
		      controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].isAnalogue = 0;
                }
                break;

                case ANALOGUE_TO_DIGITAL_MIN:
                {
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].minEnabled = 1;
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].minChannel =
                        input.channel;
                    strncpy(controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].minName,
			    input.name, SIZE);
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].minPlayer =
                        input.player;
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].isNeg = negabs;
		    if (strstr(input.name, "ANALOGUE") == NULL)
		      controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].isAnalogue = 0;
                }
                break;

                case ANALOGUE_SHAKE:
                {
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].shakeEnabled = 1;
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].shakeChannel =
                        input.channel;
                    strncpy(controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].shakeName,
			    input.name, SIZE);
                    controllers->controller[i].absTriggers[controllers->controller[i].inputs[j].evCode].shakePlayer =
                        input.player;
                }
                break;

                default:
                    break;
                }
            }
            break;

            case EV_KEY:
            {
                controllers->controller[i].keyTriggers[controllers->controller[i].inputs[j].evCode].enabled = 1;
                controllers->controller[i].keyTriggers[controllers->controller[i].inputs[j].evCode].channel =
                    input.channel;
                strncpy(controllers->controller[i].keyTriggers[controllers->controller[i].inputs[j].evCode].name,
			input.name, SIZE);
                controllers->controller[i].keyTriggers[controllers->controller[i].inputs[j].evCode].player =
                    input.player;

                if (strstr(input.name, "ANALOGUE") != NULL)
                    controllers->controller[i].keyTriggers[controllers->controller[i].inputs[j].evCode].isAnalogue = 1;

                if (strcmp(input.name, "PLAYER_1_COIN") == 0)
                {
                    controllers->controller[i].keyTriggers[controllers->controller[i].inputs[j].evCode].isCoin = 1;
                }

                if (strcmp(input.name, "PLAYER_2_COIN") == 0)
                {
                    controllers->controller[i].keyTriggers[controllers->controller[i].inputs[j].evCode].isCoin = 1;
                }
            }
            break;

            default:
            {
                printf("Warining: Unknown mapping type\n");
            }
            }
        }

        int controllerHasInputsEnabled = 0;
        for (int j = 0; j < KEY_MAX; j++)
        {
            if (controllers->controller[i].keyTriggers[j].enabled)
                controllerHasInputsEnabled = 1;
        }

        for (int j = 0; j < ABS_MAX; j++)
            if (controllers->controller[i].absTriggers[j].enabled)
                controllerHasInputsEnabled = 1;

        if (!controllerHasInputsEnabled)
            continue;

        EmulatorConfig *config = getConfig();
        ThreadArguments *args = malloc(sizeof(ThreadArguments));
        args->controller = &controllers->controller[i];
        args->controllers = controllers;
        memcpy(args->analogue_deadzone_start, config->arcadeInputs.analogue_deadzone_start, sizeof(config->arcadeInputs.analogue_deadzone_start));
        memcpy(args->analogue_deadzone_middle, config->arcadeInputs.analogue_deadzone_middle, sizeof(config->arcadeInputs.analogue_deadzone_middle));
        memcpy(args->analogue_deadzone_end, config->arcadeInputs.analogue_deadzone_end, sizeof(config->arcadeInputs.analogue_deadzone_end));
        pthread_create(&controllers->thread[controllers->threadIndex++], NULL, controllerThread, args);
        controllers->controller[i].inUse = 1;
    }

    return CONTROLLER_STATUS_SUCCESS;
}

ControllerStatus initControllers(Controllers *controllers)
{
    jvsBits = getJVSIO()->capabilities.analogueInBits;

    controllers->threadIndex = 0;

    if (listControllers(controllers) != CONTROLLER_STATUS_SUCCESS)
    {
        return CONTROLLER_STATUS_ERROR;
    }

    if (startControllerThreads(controllers) != CONTROLLER_STATUS_SUCCESS)
    {
        return CONTROLLER_STATUS_ERROR;
    }

    return CONTROLLER_STATUS_SUCCESS;
}
