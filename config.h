#include "XMouseControl.h"
// "Frames" or updates per second.
#define FPS 30
// Pixels per second.
#define BASE_MOUSE_SPEED 14.0
// Events per second.
#define BASE_SCROLL_SPEED 1
#define DELAY 15
#define DEFAULT_MASK Mod3Mask
#define IGNORE_MASK Mod2Mask //set to ingore numlock mod2
#define MAX_THRESHOLD 128.0
#define MIN_THRESHOLD .125

#define NUMBER_OF_MASTER_DEVICES 8



void grabandmove2scroll(const int ignored);

// Movement and scrolling:
// dir can be any of UP, DOWN, LEFT, RIGHT, bitwise or'd together.
void test(const int dir);
void movestart(const int dir);
void movestop(const int dir);
void mouseAction(Bool scroll, int d, Bool start);
void scrollstart(const int dir);
void scrollstop(const int dir);
void cycleMoveOption();
void resetCycleMoveOption();
// factor is a float.
void multiplyspeed(const int factor);
void dividespeed(const int factor);

// Clicking:
// btn can be any value from enum Mouse.
void clickpress(const int btn);
void clickrelease(const int btn);
void togglegrabkeyboard();
void grabkeyboard(); // Wait for keysym to be released, if given.
void ungrabkeyboard();
void toggleLockKeyboard();

void movestart(const int d){
	mouseAction(False, d, True);
}
void movestop(const int d){
	mouseAction(False, d, False);
}
void scrollstart(const int d){
	mouseAction(True, d, True);
}
void scrollstop(const int d){
	mouseAction(True, d, False);
}



Key keys[] = {
// modifier  key	opts	press func	 press arg	release func	release arg
// Enable/disable
//
// Directional control with WASD.

//{DEFAULT_MASK, XK_Tab,cycleDefaultMaster,1},
//{DEFAULT_MASK|ShiftMask, XK_Tab,cycleDefaultMaster,-1},
{DEFAULT_MASK,	XK_f,	cycleMoveOption,	 0},
{DEFAULT_MASK |ShiftMask,	XK_f,	resetCycleMoveOption,	 0},
// Speed multiply/divide.
{DEFAULT_MASK,	XK_z,	dividespeed,	2},
{DEFAULT_MASK,	XK_x,	multiplyspeed,	2},
{DEFAULT_MASK |ShiftMask,	XK_z,	dividespeed,	2,multiplyspeed,	2},
{DEFAULT_MASK|ShiftMask,	XK_x,	multiplyspeed,	2,dividespeed,	2},

// Arrow key Scrolling
{DEFAULT_MASK,	XK_Up,	movestart,	 UP,	movestop,	UP},
{DEFAULT_MASK,	XK_Left,	movestart,	 LEFT,	movestop,	LEFT},
{DEFAULT_MASK,	XK_Down,	movestart,	 DOWN,	movestop,	DOWN},
{DEFAULT_MASK,	XK_Right,	movestart,	 RIGHT,	movestop,	RIGHT},
//WASD Mouse move
{DEFAULT_MASK,	XK_w,	scrollstart,	 UP,	scrollstop,	UP},
{DEFAULT_MASK,	XK_a,	scrollstart,	 LEFT,	scrollstop,	LEFT},
{DEFAULT_MASK,	XK_s,	scrollstart,	 DOWN,	scrollstop,	DOWN},
{DEFAULT_MASK,	XK_d,	scrollstart,	 RIGHT,	scrollstop,	RIGHT},

//Keypad mouse move

{DEFAULT_MASK,	XK_KP_Up,	movestart,	 UP,	movestop,	UP},
{DEFAULT_MASK,	XK_KP_Left,	movestart,	 LEFT,	movestop,	LEFT},
{DEFAULT_MASK,	XK_KP_Down,	movestart,	 DOWN,	movestop,	DOWN},
{DEFAULT_MASK,	XK_KP_Right,	movestart,	 RIGHT,	movestop,	RIGHT},



{DEFAULT_MASK,	XK_c,	clickpress,	BTNRIGHT,	clickrelease,	BTNRIGHT},
{DEFAULT_MASK,	XK_V,	clickpress,	BTNMIDDLE,	clickrelease,	BTNMIDDLE},
{DEFAULT_MASK,	XK_Insert,	clickpress,	BTNMIDDLE,  clickrelease,	BTNMIDDLE},
{DEFAULT_MASK | ShiftMask,	XK_space,	clickpress,	BTNLEFT	},
{DEFAULT_MASK | Mod1Mask,	XK_space,	clickrelease,	BTNLEFT},
{DEFAULT_MASK,	XK_space,	clickpress,	BTNLEFT,	clickrelease,	BTNLEFT},
{DEFAULT_MASK,	XK_Return,	clickpress,	BTNLEFT,	clickrelease,	BTNLEFT},

{DEFAULT_MASK | ShiftMask,	XK_a,	clickpress,	BTNLEFT,	clickrelease,	BTNLEFT},
{DEFAULT_MASK | ShiftMask,	XK_s,	clickpress,	BTNMIDDLE,	clickrelease,	BTNMIDDLE},
{DEFAULT_MASK | ShiftMask,	XK_d,	clickpress,	BTNRIGHT,	clickrelease,	BTNRIGHT},

{DEFAULT_MASK,	XK_KP_Enter,	clickpress,	BTNLEFT,	clickrelease,	BTNLEFT},
{DEFAULT_MASK,	XK_KP_5,	clickpress,	BTNLEFT,	clickrelease,	BTNLEFT},
{DEFAULT_MASK,	XK_KP_Insert,	clickpress,	BTNMIDDLE,  clickrelease,	BTNMIDDLE},



};
