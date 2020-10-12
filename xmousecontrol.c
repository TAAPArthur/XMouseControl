#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/keysym.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#define LEN(X) ((int)(sizeof X / sizeof X[0]))
#define MIN_MAX(MIN, MAX, VALUE) (MIN>VALUE?MIN:VALUE>MAX?MAX:VALUE)

static XMouseControlMasterState deviceInfo[NUMBER_OF_MASTER_DEVICES];

Display* dpy = NULL;
Window root;
static struct pollfd xeventFD = {0};

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

void init() {
    dpy = XOpenDisplay(NULL);
    if(!dpy)
        exit(2);
    root = DefaultRootWindow(dpy);
    xeventFD = (struct pollfd) { XConnectionNumber(dpy), POLLIN};
    XSetErrorHandler(handleError);
    for(unsigned int i = 0; i < LEN(bindings); i++) {
        bindings[i].keyCode = XKeysymToKeycode(dpy, bindings[i].keySym);
        grabKey(XIAllMasterDevices, bindings[i].mod, bindings[i].keyCode,
            bindings[i].keyRelease ? XI_KeyReleaseMask : XI_KeyPressMask, IGNORE_MASK);
    }
}

long getTimeSince(struct timespec* start) {
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    if(current.tv_sec  > start->tv_sec)
        return 0;
    long diff = (current.tv_nsec - start->tv_nsec) / 1e6;
    if(diff > XMOUSE_CONTROL_UPDATER_INTERVAL)
        return 0;
    return XMOUSE_CONTROL_UPDATER_INTERVAL - diff;
}

void getEvent(XEvent* event) {
    static struct timespec start;
    int timeout = getTimeSince(&start);
    if(!timeout) {
        xmousecontrolUpdate();
        clock_gettime(CLOCK_MONOTONIC, &start);
        timeout = getTimeSince(&start);
    }
    while(1) {
        if(XEventsQueued(dpy, QueuedAlready)) {
            printf("Loading pending events\n");
            XNextEvent(dpy, event);
            return;
        }
        switch(poll(&xeventFD, 1, timeout)) {
            case 0:
                timeout = XMOUSE_CONTROL_UPDATER_INTERVAL;
                clock_gettime(CLOCK_MONOTONIC, &start);
                if(xmousecontrolUpdate())
                    continue;
            // fallthrough
            case 1:
                XNextEvent(dpy, event);
                return;
            case -1:
                exit(1);
        }
    }
}
void run() {
    XEvent event;
    XIDeviceEvent* devev;
    MasterID active;
    while(1) {
        getEvent(&event);
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
                for(int i = 0; i < LEN(bindings); i++) {
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
}
void removeXMouseControlMask(XMouseControlMasterState* info, int mask) {
    info->mask &= ~mask;
}

void adjustScrollSpeed(XMouseControlMasterState* info, int diff) {
    info->scrollScale = diff == 0 ? 0 : info->scrollScale + diff;
    info->scrollScale = MIN_MAX(1, 256, info->scrollScale);
    _notify(info, 1);
}

void adjustSpeed(XMouseControlMasterState* info, int diff) {
    info->vScale = diff == 0 ? 0 : info->vScale + diff;
    info->vScale = MIN_MAX(1, 256, info->vScale);
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
            update++;
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
    XFlush(dpy);
    return update;
}

int main() {
    signal(SIGCHLD, SIG_IGN);
    init();
    run();
    return 0;
}
