#include <stdlib.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xinput.h>
#include <xcb/xtest.h>

#include "xmousecontrol.h"
#include "xutil.h"

static xcb_connection_t* dis;
static xcb_window_t root;

static void sendEvent(int type, int detail, MasterID id) {
    const xcb_query_extension_reply_t * reply = xcb_get_extension_data(dis, &xcb_input_id);
    xcb_test_fake_input_checked(dis, reply->first_event + type - XCB_INPUT_KEY_PRESS + 1, detail, XCB_CURRENT_TIME, root, 0,0, id);
}
void sendButtonPress(int button, MasterID id) {
    sendEvent(XCB_INPUT_BUTTON_PRESS, button, id);
}
void sendButtonRelease(int button, MasterID id) {
    sendEvent(XCB_INPUT_BUTTON_RELEASE, button, id);
}

static void warpPointer(short x, short y, xcb_window_t relativeWindow, MasterID id) {
    xcb_input_xi_warp_pointer(dis, XCB_NONE, relativeWindow, 0, 0, 0, 0, x << 16, y << 16, id);
}
void movePointerRelative(short x, short y, int id) {
    warpPointer(x, y, XCB_NONE, id);
}

int grabDevice(MasterID deviceID, int maskValue) {
    xcb_input_xi_grab_device_reply_t* reply;
    reply = xcb_input_xi_grab_device_reply(dis, xcb_input_xi_grab_device(dis, root, XCB_CURRENT_TIME, XCB_NONE, deviceID, XCB_INPUT_GRAB_MODE_22_ASYNC, XCB_INPUT_GRAB_MODE_22_ASYNC, 1,  1, &maskValue), NULL);
    free(reply);
    return reply->status;
}

void grabKeyboardDevice(MasterID deviceID){
    grabDevice(deviceID, XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE | XCB_INPUT_XI_EVENT_MASK_KEY_PRESS);
}

int ungrabDevice(MasterID id) {
    xcb_input_xi_ungrab_device(dis, XCB_CURRENT_TIME, id);
    return 1;
}

void selectEvents(xcb_window_t window, uint32_t maskValue) {
    struct {
        xcb_input_event_mask_t iem;
        xcb_input_xi_event_mask_t xiem;
    } se_mask;
    se_mask.iem.deviceid = XCB_INPUT_DEVICE_ALL;
    se_mask.iem.mask_len = 1;

    se_mask.xiem = maskValue;
    xcb_input_xi_select_events(dis, window, 1, &se_mask.iem);
}

static void grabDetail(MasterID deviceID, uint32_t detail, uint32_t mod, uint32_t maskValue, uint32_t ignoreMod) {
    uint32_t modifiers[2] = {mod, mod | ignoreMod};
    int size = 2;
    xcb_input_grab_type_t grabType = XCB_INPUT_GRAB_TYPE_KEYCODE;
    xcb_input_xi_passive_grab_device_reply_t *reply;
    reply = xcb_input_xi_passive_grab_device_reply(dis, xcb_input_xi_passive_grab_device(dis,XCB_CURRENT_TIME,root,XCB_CURSOR_NONE, detail, deviceID, size, 1, grabType, XCB_INPUT_GRAB_MODE_22_ASYNC, XCB_INPUT_GRAB_MODE_22_ASYNC, XCB_INPUT_GRAB_OWNER_OWNER, &maskValue, modifiers), NULL);
}

int open_connection() {
    dis = xcb_connect(NULL, NULL);
    if(xcb_connection_has_error(dis)) {
        exit(2);
    }

    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (dis));
    xcb_screen_t* screen = iter.data;
    root = screen->root;
    selectEvents(root, XCB_INPUT_XI_EVENT_MASK_HIERARCHY);

    return xcb_get_file_descriptor(dis);
}

void init_bindings(KeyBinding * bindings, int num, int ignore_mask) {
    xcb_key_symbols_t * symbols = xcb_key_symbols_alloc(dis);
    for(unsigned int i = 0; i < num; i++) {
        xcb_keycode_t * codes = xcb_key_symbols_get_keycode(symbols, bindings[i].keySym);
        bindings[i].keyCode = codes[0];
        free(codes);
        grabDetail(XCB_INPUT_DEVICE_ALL_MASTER, bindings[i].keyCode, bindings[i].mod, bindings[i].keyRelease ? XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE: XCB_INPUT_XI_EVENT_MASK_KEY_PRESS, ignore_mask);
    }
    xcb_key_symbols_free(symbols);
}

static int loadGenericEvent(xcb_ge_generic_event_t* event) {
    if(event->extension == xcb_get_extension_data(dis, &xcb_input_id)->major_opcode) {
        return event->event_type - XCB_INPUT_DEVICE_KEY_PRESS;
    }
    return 0;
}

int process_event(KeyEvent* keyEvent) {
    xcb_generic_event_t* event = xcb_wait_for_event(dis);
    int ret = 0;
    if(event->response_type == XCB_GE_GENERIC) {
        int type = loadGenericEvent((void*)event);
        xcb_input_key_press_event_t* devev = (void*)event;
        keyEvent->detail = devev->detail;
        keyEvent->mod = devev->mods.effective;
        keyEvent->keyRelease = (type == XCB_INPUT_DEVICE_KEY_RELEASE);
        ret = devev->deviceid;
    }
    free(event);
    return ret;
}
static char deviceToPointer[255];

static void initilizeDeviceMappings() {
    xcb_input_xi_query_device_cookie_t cookie = xcb_input_xi_query_device(dis, XCB_INPUT_DEVICE_ALL);
    xcb_input_xi_query_device_reply_t *reply = xcb_input_xi_query_device_reply(dis, cookie, NULL);
    xcb_input_xi_device_info_iterator_t  iter = xcb_input_xi_query_device_infos_iterator(reply);

    while(iter.rem){
        xcb_input_xi_device_info_t* device = iter.data;
        const char* deviceName = xcb_input_xi_device_info_name (device);
        switch(device->type) {
            case XCB_INPUT_DEVICE_TYPE_FLOATING_SLAVE:
            case XCB_INPUT_DEVICE_TYPE_MASTER_POINTER:
                deviceToPointer[device->deviceid] = device->deviceid;
                break;
            case XCB_INPUT_DEVICE_TYPE_SLAVE_KEYBOARD:
                deviceToPointer[device->deviceid] = deviceToPointer[device->attachment];
                break;
            case XCB_INPUT_DEVICE_TYPE_MASTER_KEYBOARD:
            case XCB_INPUT_DEVICE_TYPE_SLAVE_POINTER:
                deviceToPointer[device->deviceid] = device->attachment;
                break;
        }
        xcb_input_xi_device_info_next (&iter);
    }
    free(reply);
}

int getDeviceMapping(MasterID id) {
    if(!deviceToPointer[id]) {
        initilizeDeviceMappings();
    }
    return deviceToPointer[id];
}

void flush() {
    xcb_flush(dis);
}
