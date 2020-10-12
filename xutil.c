#include <X11/extensions/XInput2.h>
#include <X11/extensions/XTest.h>

#include "xutil.h"
extern Display* dpy ;
extern Window root;

void sendButtonPress(int button, MasterID id) {
    XDevice dev = {.device_id = id};
    XTestFakeDeviceButtonEvent(dpy, &dev, button, 1, NULL, 0, CurrentTime);
}
void sendButtonRelease(int button, MasterID id) {
    XDevice dev = {.device_id = id};
    XTestFakeDeviceButtonEvent(dpy, &dev, button, 0, NULL, 0, CurrentTime);
}

void movePointerRelative(short x, short y, MasterID id) {
    XIWarpPointer(dpy, id, None, None, 0, 0, 0, 0, x, y);
    //xcb_input_xi_warp_pointer(dis, None, relativeWindow, 0, 0, 0, 0, x << 16, y << 16, id);
}

int grabDevice(MasterID deviceID, uint32_t maskValue) {
    XIEventMask eventMask = {deviceID, 4, (unsigned char*)& maskValue};
    return XIGrabDevice(dpy, deviceID, root, CurrentTime, None, GrabModeAsync,
            GrabModeAsync, 1, &eventMask);
}
int ungrabDevice(MasterID id) {
    return XIUngrabDevice(dpy, id, 0);
}

int grabKey(MasterID deviceID, uint32_t mod, uint32_t detail, uint32_t maskValue, uint32_t ignoreMod) {
    XIEventMask eventMask = {deviceID, 2, (unsigned char*)& maskValue};
    XIGrabModifiers modifiers[2] = {{mod}, {mod | ignoreMod}};
    return XIGrabKeycode(dpy, deviceID, detail, root, XIGrabModeAsync, XIGrabModeAsync,
            1, &eventMask, 2, modifiers);
}
