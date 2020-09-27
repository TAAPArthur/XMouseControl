#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"
#include "threads.h"
#define LEN(X) (sizeof X / sizeof X[0])
#define NUMBER_OF_MASTER_DEVICES  8
#define MAX(A,B)(A>B?A:B)
#define MIN(A,B)(A<B?A:B)

static XMouseControlMasterState deviceInfo[NUMBER_OF_MASTER_DEVICES];
static ThreadSignaler signaler = THREAD_SIGNALER_INITIALIZER;

unsigned int BASE_MOUSE_SPEED = 10;
unsigned int BASE_SCROLL_SPEED = 1;
unsigned int XMOUSE_CONTROL_UPDATER_INTERVAL = 30;

static void notify(char* summary, char* body) {
    if(!fork()) {
        char* const args[] = {"/usr/bin/notify-send", "-h", "string:x-canonical-private-synchronous:XMouseControl", "-a", "XMouseControl", summary, body, NULL};
        execv("/usr/bin/notify-send", args);
    }
}
static inline void _notify(XMouseControlMasterState* info, int scroll) {
    char summary[32];
    sprintf(summary, "XMouseControl %d", info->id);
    char body[64];
    if(scroll)
        sprintf(body, "Scroll speed %d", info->scrollScale);
    else
        sprintf(body, "Pointer Speed %d", info->vScale);
    notify(summary, body);
}

Display* dpy = NULL;
Window root;

static void forceReset(MasterID id) {
    int ndevices;
    XIDeviceInfo* devices, device;
    devices = XIQueryDevice(dpy, id, &ndevices);
    for(int i = 0; i < ndevices; i++) {
        device = devices[i];
        switch(device.use) {
            case XIMasterPointer:
                deviceInfo[device.deviceid].id = device.deviceid;
                deviceInfo[device.attachment].id = device.deviceid;
                break;
            case XIMasterKeyboard:
                deviceInfo[device.attachment].id = device.attachment;
                deviceInfo[device.deviceid].id = device.attachment;
                break;
        }
    }
    XIFreeDeviceInfo(devices);
}

static int handleError(Display* dpy, XErrorEvent* event) {
    char buff[100];
    XGetErrorText(dpy, event->error_code, buff, 40);
    printf("Ignoring Xlib error: error code %d request code %d %s\n",
        event->error_code,
        event->request_code, buff) ;
    forceReset(XIAllMasterDevices);
    return 0;
}

void checkXServerVersion() {
    int opcode, event, error;
    if(!XQueryExtension(dpy, "XInputExtension", &opcode, &event, &error)) {
        printf("X Input extension not available.\n");
        exit(1);
    }
    /* Which version of XI2? We support 2.0 */
    int major = 2, minor = 0;
    if(XIQueryVersion(dpy, &major, &minor) == BadRequest) {
        printf("XI2 not available. Server supports %d.%d\n", major, minor);
        exit(1);
    }
}

void init() {
    dpy = XOpenDisplay(NULL);
    if(!dpy)
        exit(2);
    checkXServerVersion();
    root = DefaultRootWindow(dpy);
    XSetErrorHandler(handleError);
    for(unsigned int i = 0; i < LEN(bindings); i++) {
        bindings[i].keyCode = XKeysymToKeycode(dpy, bindings[i].keySym);
        grabKey(XIAllMasterDevices, bindings[i].mod, bindings[i].keyCode,
            bindings[i].keyRelease ? XI_KeyReleaseMask : XI_KeyPressMask, IGNORE_MASK);
    }
}
MasterID active;
void run() {
    XEvent event;
    XIDeviceEvent* devev;
    while(1) {
        XNextEvent(dpy, &event);
        XGenericEventCookie* cookie = &event.xcookie;
        if(XGetEventData(dpy, cookie)) {
            devev = cookie->data;
            if(devev->deviceid != 0) {
                active = devev->deviceid;
                if(!deviceInfo[active].id) {
                    resetXMouseControl(&deviceInfo[active]);
                    forceReset(deviceInfo[active].id);
                }
                int mods = devev->mods.effective & ~IGNORE_MASK;
                for(size_t i = 0; i < LEN(bindings); i++) {
                    if(bindings[i].keyCode == devev->detail && bindings[i].mod == mods &&
                        bindings[i].keyRelease == (cookie->evtype == XI_KeyRelease)) {
                        bindings[i].func(deviceInfo + active, bindings[i].arg);
                    }
                }
            }
        }
        XFreeEventData(dpy, cookie);
    }
}

void resetXMouseControl(XMouseControlMasterState* info) {
    info->scrollScale = BASE_SCROLL_SPEED;
    info->vScale = BASE_MOUSE_SPEED ;
    info->mask = 0;
}
void addXMouseControlMask(XMouseControlMasterState* info, int mask) {
    info->mask |= mask;
    signalThread(&signaler);
}
void removeXMouseControlMask(XMouseControlMasterState* info, int mask) {
    info->mask &= ~mask;
}

void adjustScrollSpeed(XMouseControlMasterState* info, int diff) {
    info->scrollScale = diff == 0 ? 0 : info->scrollScale + diff;
    info->scrollScale = MAX(1, MIN(info->scrollScale, 256));
    _notify(info, 1);
}

void adjustSpeed(XMouseControlMasterState* info, int diff) {
    info->vScale = diff == 0 ? 0 : info->vScale + diff;
    info->vScale = MAX(1, MIN(info->vScale, 256));
    _notify(info, 0);
}

void clickButton(XMouseControlMasterState* info, int button) {
    sendButtonPress(button, info->id);
    sendButtonRelease(button, info->id);
}

void grabKeyboard(XMouseControlMasterState* info) {
    grabDevice(info->id, XI_KeyReleaseMask | XI_KeyPressMask);
}
void ungrabKeyboard(XMouseControlMasterState* info) {
    ungrabDevice(info->id);
}
#define _IS_SET(info,A,B)\
       ((info->mask & (A|B)) && (((info->mask & A)?1:0) ^ ((info->mask&B)?1:0)))
int xmousecontrolUpdate(void) {
    char update = 0;
    for(int i = 0; i < NUMBER_OF_MASTER_DEVICES; i++) {
        XMouseControlMasterState* info = deviceInfo + i;
        if(info->id && info->mask) {
            update++  ;
            if(_IS_SET(info, SCROLL_RIGHT_MASK, SCROLL_LEFT_MASK))
                for(int i = 0; i < info->scrollScale; i++)
                    clickButton(info, info->mask & SCROLL_RIGHT_MASK ? SCROLL_RIGHT : SCROLL_LEFT);
            if(_IS_SET(info, SCROLL_UP_MASK, SCROLL_DOWN_MASK))
                for(int i = 0; i < info->scrollScale; i++) {
                    clickButton(info, info->mask & SCROLL_UP_MASK ? SCROLL_UP : SCROLL_DOWN);
                }
            double deltaX = 0, deltaY = 0;
            if(_IS_SET(info, MOVE_RIGHT_MASK, MOVE_LEFT_MASK))
                deltaX = info->mask & MOVE_RIGHT_MASK ? info->vScale : -info->vScale;
            if(_IS_SET(info, MOVE_UP_MASK, MOVE_DOWN_MASK))
                deltaY = info->mask & MOVE_DOWN_MASK ? info->vScale : -info->vScale;
            if(deltaX || deltaY)
                movePointerRelative(deltaX, deltaY, info->id);
        }
    }
    return update;
}

void* runXMouseControl() {
    while(1) {
        int update = xmousecontrolUpdate();
        XFlush(dpy);
        sleepOrWait(&signaler, update, XMOUSE_CONTROL_UPDATER_INTERVAL);
    }
    return NULL;
}

int main() {
    signal(SIGCHLD, SIG_IGN);
    if(!XInitThreads())
        exit(2);
    init();
    spawnThread(runXMouseControl);
    run();
    return 0;
}
