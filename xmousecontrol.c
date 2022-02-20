#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "xutil.h"
#define LEN(X) ((int)(sizeof X / sizeof X[0]))
#define MIN_MAX(MIN, MAX, VALUE) (MIN>VALUE?MIN:VALUE>MAX?MAX:VALUE)

static XMouseControlMasterState deviceInfo[NUMBER_OF_MASTER_DEVICES];

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

static long getTimeSince(struct timespec* start) {
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    if(current.tv_sec  > start->tv_sec)
        return 0;
    long diff = (current.tv_nsec - start->tv_nsec) / 1e6;
    if(diff > XMOUSE_CONTROL_UPDATER_INTERVAL)
        return 0;
    return XMOUSE_CONTROL_UPDATER_INTERVAL - diff;
}

static int processEvents(int timeout) {
    int numEvents;
    if((numEvents = poll(&xeventFD, 1, timeout))) {
        if(xeventFD.revents & (POLLERR | POLLNVAL | POLLHUP)) {
            exit(3);
        }
    }
    return numEvents;
}
static void run() {
    static struct timespec start;
    KeyEvent keyEvent;
    while(1) {
        int timeout = getTimeSince(&start);
        if(!timeout) {
            xmousecontrolUpdate();
            clock_gettime(CLOCK_MONOTONIC, &start);
            timeout = getTimeSince(&start);
        }
        if(!processEvents(timeout)) {
            timeout = XMOUSE_CONTROL_UPDATER_INTERVAL;
            clock_gettime(CLOCK_MONOTONIC, &start);
            if(xmousecontrolUpdate())
                continue;
        }
        unsigned char deviceId= process_event(&keyEvent);
        MasterID masterID = getDeviceMapping(deviceId);
        if(masterID) {
            if(!deviceInfo[masterID].id) {
                resetXMouseControl(&deviceInfo[masterID]);
                deviceInfo[masterID].id = getDeviceMapping(masterID);
            }
            int mod = keyEvent.mod & ~IGNORE_MASK;
            for(int i = 0; i < LEN(bindings); i++) {
                if(bindings[i].keyCode == keyEvent.detail && bindings[i].mod == mod &&
                    bindings[i].keyRelease == keyEvent.keyRelease) {
                    bindings[i].func(deviceInfo + deviceInfo[masterID].id, bindings[i].arg);
                }
            }
        }
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
    grabKeyboardDevice(info->id);
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
    flush();
    return update;
}

int main() {
    signal(SIGCHLD, SIG_IGN);
    int fd = open_connection();
    xeventFD = (struct pollfd) { fd, POLLIN};
    init_bindings(bindings, LEN(bindings), IGNORE_MASK);
    run();
    return 0;
}
