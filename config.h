// "Frames" or updates per second.
#define FPS 30
// Pixels per second.
#define BASE_MOUSE_SPEED 14.0
// Events per second.
#define BASE_SCROLL_SPEED 1
#define DELAY 15
#define defaultMask Mod3Mask
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

// Set modifier bits as "internal" while the keyboard is grabbed, that is, the
// xserver still keeps track of their state but doesn't pass them along in key
// events to applications.
//
// By default, set modifier bits will be included in click and scroll events
// generated while the keyboard is grabbed. For example, holding down a shift
// key to make the directional keys scroll instead will send "Shift-scroll"
// events whenever directional keys are pressed unless the ShiftMask bit is
// included here. Note that some modifier keysyms are treated as modifiers even
// if they aren't assigned to any modifier bits.
//
// Modifiers cannot be suppressed for global hotkeys.
//unsigned int internalmods = ShiftMask|ControlMask|Mod1Mask

// See command.h for the list of bindable functions.
//
// Use unshifted keysyms regardless whether shift will be pressed. Eg, use XK_a
// or XK_5 instead of XK_A or XK_percent.
//
// Keys with modifiers can't have release functions, since the order of key
// release is significant.
//
// While the keyboard is grabbed ptrkeys doesn't take modifier bits into
// account when determining a key's binding, similar to how keybindings work
// for video games. Bindings with modifiers aren't active while the keyboard is
// grabbed. Bindings with modifiers must have the GRAB option set.


Key keys[] = {
// modifier  key	opts	press func	 press arg	release func	release arg
// Enable/disable
//
// Directional control with WASD.


{defaultMask,	XK_f,	cycleMoveOption,	 0},
{defaultMask |ShiftMask,	XK_f,	resetCycleMoveOption,	 0},
// Speed multiply/divide.
{defaultMask,	XK_z,	dividespeed,	2},
{defaultMask,	XK_x,	multiplyspeed,	2},
{defaultMask |ShiftMask,	XK_z,	dividespeed,	2,multiplyspeed,	2},
{defaultMask|ShiftMask,	XK_x,	multiplyspeed,	2,dividespeed,	2},

// Arrow key Scrolling
{defaultMask,	XK_Up,	scrollstart,	 UP,	scrollstop,	UP},
{defaultMask,	XK_Left,	scrollstart,	 LEFT,	scrollstop,	LEFT},
{defaultMask,	XK_Down,	scrollstart,	 DOWN,	scrollstop,	DOWN},
{defaultMask,	XK_Right,	scrollstart,	 RIGHT,	scrollstop,	RIGHT},
//WASD Mouse move
{defaultMask,	XK_w,	movestart,	 UP,	movestop,	UP},
{defaultMask,	XK_a,	movestart,	 LEFT,	movestop,	LEFT},
{defaultMask,	XK_s,	movestart,	 DOWN,	movestop,	DOWN},
{defaultMask,	XK_d,	movestart,	 RIGHT,	movestop,	RIGHT},

//Keypad mouse move

{defaultMask,	XK_KP_Up,	movestart,	 UP,	movestop,	UP},
{defaultMask,	XK_KP_Left,	movestart,	 LEFT,	movestop,	LEFT},
{defaultMask,	XK_KP_Down,	movestart,	 DOWN,	movestop,	DOWN},
{defaultMask,	XK_KP_Right,	movestart,	 RIGHT,	movestop,	RIGHT},



{defaultMask,	XK_c,	clickpress,	BTNRIGHT,	clickrelease,	BTNRIGHT},
{defaultMask,	XK_V,	clickpress,	BTNMIDDLE,	clickrelease,	BTNMIDDLE},
{defaultMask,	XK_Insert,	clickpress,	BTNMIDDLE,  clickrelease,	BTNMIDDLE},
{defaultMask | ShiftMask,	XK_space,	clickpress,	BTNLEFT	},
{defaultMask | Mod1Mask,	XK_space,	clickrelease,	BTNLEFT},
{defaultMask,	XK_space,	clickpress,	BTNLEFT,	clickrelease,	BTNLEFT},
{defaultMask,	XK_Return,	clickpress,	BTNLEFT,	clickrelease,	BTNLEFT},

{defaultMask | ShiftMask,	XK_a,	clickpress,	BTNLEFT,	clickrelease,	BTNLEFT},
{defaultMask | ShiftMask,	XK_s,	clickpress,	BTNMIDDLE,	clickrelease,	BTNMIDDLE},
{defaultMask | ShiftMask,	XK_d,	clickpress,	BTNRIGHT,	clickrelease,	BTNRIGHT},

{defaultMask,	XK_KP_Enter,	clickpress,	BTNLEFT,	clickrelease,	BTNLEFT},
{defaultMask,	XK_KP_5,	clickpress,	BTNLEFT,	clickrelease,	BTNLEFT},
{defaultMask,	XK_KP_Insert,	clickpress,	BTNMIDDLE,  clickrelease,	BTNMIDDLE},

};
