#ifndef PK_H
#define PK_H


#include <X11/Xlib.h>
#define LEN(X) (sizeof X / sizeof X[0])

Display *dpy = NULL;
Window root;



typedef struct {
	double x, y;
} Tuple;


typedef struct {
	int mod;
	KeySym keySym;
	void (*pressfunc)(const int );
	const int pressarg;
	void (*releasefunc)(const int );
	const int releasearg;

	KeyCode keyCode;
	long timeLastRecorded[8];
} Key;
typedef struct {
	int ismove2scroll;
	Tuple scrollRem,mouseRem;
	Tuple delta;
	int scrollDir,mouseDir;
	double coefficent;
	int moveOption;
	int id;
	long timeLastRecorded;
} Master;



// Bindable function declarations ("commands").

enum Direction {
	UP    = 1,//(1<<0),
	DOWN  = 2,//(1<<1),
	LEFT  = 4,//(1<<2),
	RIGHT = 8//(1<<3),
};

enum Mouse {
	BTNLEFT = Button1,
	BTNMIDDLE = Button2,
	BTNRIGHT = Button3,
	SCROLLUP = 4,
	SCROLLDOWN = 5,
	SCROLLLEFT = 6,
	SCROLLRIGHT = 7,
};
enum {
	DEFAULT = 0,
	SWAP = 1,
	ONLY_MOUSE = 2,
	ONLY_SCROLL = 3,
	RESET =4
};

enum {
	RELEASE = 0,
	PRESS = 1,
};



void setup();

int getMasterPointerId(XIDeviceEvent *devev,Bool mouseEvent);
void pressButton(const int btn,Bool press);
Bool isPressed(int keycode);
Bool isIdle();
void forceRelease(int n);
void scrollWithMouse(int id);
int keypress(int keyCode,int mods,Bool press);
void grabkeys();
int grabkey(Key *key);
void cleanup();
int saveerror(Display *dpy, XErrorEvent *ee);
void msleep(long ms);
int addDirection(int dir,int d);
int removeDirection(int dir,int d);
Bool calcuateDisplacement(int index, Bool scroll);
void update( Bool scroll);
void detectEvent();
void cycleDefaultMaster(int dir);



#endif
