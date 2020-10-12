#include <X11/keysym.h>
#include "xmousecontrol.h"
#include "xutil.h"

#define BASE_MOUSE_SPEED 10
#define BASE_SCROLL_SPEED 1
#define XMOUSE_CONTROL_UPDATER_INTERVAL 30
#define NUMBER_OF_MASTER_DEVICES  255
#define DEFAULT_MOD_MASK Mod3Mask
#define IGNORE_MASK Mod2Mask

#define PAIR(MASK,KEY,KP,A1,KR,A2)\
    {MASK, KEY, KP, A1,0 }, \
    {MASK, KEY, KR, A2, 1}

#define BINDING(MASK, KEY, BUTTON_MASK)\
    PAIR(MASK, KEY, addXMouseControlMask, BUTTON_MASK, removeXMouseControlMask, BUTTON_MASK)

KeyBinding bindings[] = {
    // Directional control with WASD.
    BINDING(DEFAULT_MOD_MASK, XK_w, SCROLL_UP_MASK),
    BINDING(DEFAULT_MOD_MASK, XK_a, SCROLL_LEFT_MASK),
    BINDING(DEFAULT_MOD_MASK, XK_s, SCROLL_DOWN_MASK),
    BINDING(DEFAULT_MOD_MASK, XK_d, SCROLL_RIGHT_MASK),
    BINDING(DEFAULT_MOD_MASK, XK_Up, MOVE_UP_MASK),
    BINDING(DEFAULT_MOD_MASK, XK_Left, MOVE_LEFT_MASK),
    BINDING(DEFAULT_MOD_MASK, XK_Down, MOVE_DOWN_MASK),
    BINDING(DEFAULT_MOD_MASK, XK_Right, MOVE_RIGHT_MASK),

    BINDING(DEFAULT_MOD_MASK, XK_k, SCROLL_UP_MASK),
    BINDING(DEFAULT_MOD_MASK, XK_l, SCROLL_RIGHT_MASK),
    BINDING(DEFAULT_MOD_MASK, XK_j, SCROLL_DOWN_MASK),
    BINDING(DEFAULT_MOD_MASK, XK_h, SCROLL_LEFT_MASK),
    BINDING(DEFAULT_MOD_MASK | ShiftMask, XK_k, MOVE_UP_MASK),
    BINDING(DEFAULT_MOD_MASK | ShiftMask, XK_l, MOVE_RIGHT_MASK),
    BINDING(DEFAULT_MOD_MASK | ShiftMask, XK_j, MOVE_DOWN_MASK),
    BINDING(DEFAULT_MOD_MASK | ShiftMask, XK_h, MOVE_LEFT_MASK),

    {DEFAULT_MOD_MASK, XK_Hyper_L, removeXMouseControlMask, -1, 0},

    {DEFAULT_MOD_MASK,	XK_e, adjustScrollSpeed, 1},
    {DEFAULT_MOD_MASK | ShiftMask,	XK_e, adjustScrollSpeed, -1},
    {DEFAULT_MOD_MASK | Mod1Mask,	XK_e, adjustSpeed, 1},
    {DEFAULT_MOD_MASK | Mod1Mask | ShiftMask,	XK_e, adjustSpeed, -1},

    {DEFAULT_MOD_MASK,	XK_q, resetXMouseControl},

    {DEFAULT_MOD_MASK, XK_c, clickButton, Button2},
    {DEFAULT_MOD_MASK, XK_x, clickButton, Button3},
    {DEFAULT_MOD_MASK, XK_space, clickButton, Button1},
    {DEFAULT_MOD_MASK, XK_Return, clickButton, Button2},
    {DEFAULT_MOD_MASK | ShiftMask,	XK_space, clickButton, Button3},

    {DEFAULT_MOD_MASK,	XK_Tab, grabKeyboard},
    {DEFAULT_MOD_MASK | ShiftMask,	XK_Tab, ungrabKeyboard},
};
