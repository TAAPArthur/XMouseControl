#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <time.h>

#include "XMouseControl.h"
#include "config.h"

char keyMap[32];
Master masters[NUMBER_OF_MASTER_DEVICES];
Master *workingMaster;
int numberOfActiveMasters=0;
void computeKeymap(){
	XQueryKeymap(dpy, keyMap);
}

void sleep(long ms){
	struct timespec duration = {
		.tv_sec=ms/1000,
		.tv_nsec=((ms % 1000) * 1e6),
	};
	nanosleep(&duration, NULL);
}
void checkXServerVersion(){
	int opcode, event, error;
	if (!XQueryExtension(dpy, "XInputExtension", &opcode, &event, &error)) {
	   printf("X Input extension not available.\n");
	   exit(1);
	}

	/* Which version of XI2? We support 2.0 */
	int major = 2, minor = 0;
	if (XIQueryVersion(dpy, &major, &minor) == BadRequest) {
	  printf("XI2 not available. Server supports %d.%d\n", major, minor);
	  exit(1);
	}
}
void init(){
	dpy = XOpenDisplay(NULL);
	checkXServerVersion();

	if (!dpy)
		exit(2);

	for (size_t i = 0; i < LEN(keys); i++)
		keys[i].keyCode=XKeysymToKeycode(dpy, keys[i].keySym);
	root = DefaultRootWindow(dpy);

	//XSelectInput(dpy, root, KeyPressMask|KeyReleaseMask);
	grabkeys();

	masters[numberOfActiveMasters].coefficent=1;
	workingMaster=&masters[numberOfActiveMasters];
	numberOfActiveMasters++;


}
void grabkeys(){
	XSelectInput(dpy, root, KeyPressMask|KeyReleaseMask);
	for (size_t i = 0; i < LEN(keys); i++) {
		Key key = keys[i];
		XGrabKey(dpy, key.keyCode, key.mod, root, True, GrabModeAsync, GrabModeAsync);
	}
	/*
	XIEventMask m;

	m.deviceid = XIAllDevices;
	m.mask_len = XIMaskLen(XI_LASTEVENT);
	m.mask = (unsigned char*)calloc(m.mask_len, sizeof(char));

	XISetMask(m.mask, XI_KeyPress);
	XISetMask(m.mask, XI_KeyRelease);
	for (size_t i = 0; i < LEN(keys); i++) {
		Key key = keys[i];
		KeyCode code = key.keyCode;
		XIGrabModifiers modifiers;
		modifiers.modifiers=key.mod;

		XIGrabKeycode(dpy, XIAllDevices, code, root, XIGrabModeAsync, XIGrabModeAsync, False, &m, 1, &modifiers);

	}
	*/
	//printf("grabbing keys\n");

}
void dump(){
	int ndevices;
	XIDeviceInfo *devices, device;

	devices = XIQueryDevice(dpy, XIAllDevices, &ndevices);

	for (int i = 0; i < ndevices; i++) {
		device = devices[i];
		printf("Device %s (id: %d) is a ", device.name, device.deviceid);

		switch(device.use) {
		   case XIMasterPointer: printf("master pointer\n"); break;
		   case XIMasterKeyboard: printf("master keyboard\n"); break;
		   case XISlavePointer: printf("slave pointer\n"); break;
		   case XISlaveKeyboard: printf("slave keyboard\n"); break;
		   case XIFloatingSlave: printf("floating slave\n"); break;
		}

		printf("Device is attached to/paired with %d\n", device.attachment);
	}

	XIFreeDeviceInfo(devices);
}
int main(){
	init();

	while (True){
		//workingMaster
		if(isIdle()){
			detectEvent();
		}
		while(XPending(dpy)){
			detectEvent();
		}


		computeKeymap();
		update(True);
		update(False);
		forceRelease();
		XFlush(dpy);
		fflush(stdout);

		sleep(25);

	}
	printf("exit");

}
int getAssociatedMasterDevice(int deviceId){
	int ndevices;
	XIDeviceInfo *masterDevices;
	int id;
	masterDevices = XIQueryDevice(dpy, deviceId, &ndevices);
	id=masterDevices[0].attachment;
	printf(" ass master info: %d %d %d ",id,ndevices,deviceId);
	XIFreeDeviceInfo(masterDevices);
	masterDevices = XIQueryDevice(dpy, id, &ndevices);
	id=masterDevices[0].attachment;
	XIFreeDeviceInfo(masterDevices);
	printf(" ass master info: %d %d %d ",id,ndevices,deviceId);
	masterDevices = XIQueryDevice(dpy, id, &ndevices);
	id=masterDevices[0].attachment;
	XIFreeDeviceInfo(masterDevices);
	printf(" ass master info: %d %d %d\n ",id,ndevices,deviceId);
	return id;
}
/*
void detectEvent(){
	XEvent event;
	XKeyEvent ev;
	XIDeviceEvent *devev;

	XNextEvent(dpy,&event);
	XGenericEventCookie *cookie = &event.xcookie;
	if(XGetEventData(dpy, cookie)){
		devev = cookie->data;
		if(!workingMaster)
		{
			printf("2\n");
			for(int i=0;i<numberOfActiveMasters;i++){

				if (masters[i].id==devev->deviceid){
					printf("3\n");
					workingMaster=&masters[i];
					break;
				}
			}
			if(!workingMaster){
				printf("3.5");

				getAssociatedMasterDevice(devev->sourceid);
				masters[numberOfActiveMasters].id=getAssociatedMasterDevice(devev->deviceid);
				masters[numberOfActiveMasters].coefficent=1;
				workingMaster=&masters[numberOfActiveMasters];
				numberOfActiveMasters++;
			}
		}
		else
			printf("master already set");
		printf("coefficent %f %f",masters[0].coefficent,workingMaster->coefficent);
		printf("autorepat %d",devev->flags& XIKeyRepeat);
		keypress(devev->detail,devev->mods.effective,cookie->evtype==XI_KeyPress);

	}
	else{
		printf("could not detect device");
		workingMaster=&masters[numberOfActiveMasters];
		ev = event.xkey;
		keypress(ev.keycode,ev.state,event.type==KeyPress);
	}
	XFreeEventData(dpy, cookie);
}
*/

void detectEvent(){
	XEvent event;
	XKeyEvent ev;
	XNextEvent(dpy,&event);
	ev = event.xkey;
	keypress(ev.keycode,ev.state,event.type==KeyPress);
}
int keypress(int keyCode,int mods,Bool press){

	for (size_t i = 0; i < LEN(keys); i++) {
		if (keyCode != keys[i].keyCode) continue;
		if (mods != keys[i].mod)
			continue;

		if (press){
			if (!keys[i].pressfunc)
				break;
			keys[i].pressfunc(keys[i].pressarg);
		}
		else{
			if (!keys[i].releasefunc)
				break;
			//if(isPressed(keys[i].keyCode))
			//	break;

			keys[i].releasefunc(keys[i].releasearg);
		}
		//printf("detected\n");
		keys[i].timeLastRecorded=time(NULL) * 1000;
		return i;
	}
	return -1;

}
void forceRelease(){
	time_t t;
    t = time(NULL) * 1000;
	for (size_t i = 0; i < LEN(keys); i++) {
		Key key = keys[i];
		if (key.timeLastRecorded+1000>=t && key.releasefunc){
			if(!isPressed(key.keyCode)){
				key.releasefunc(key.releasearg);
			}
			else
				key.timeLastRecorded=t;
		}

	}
}
Bool isTupleNotEmpty(Tuple t){
	return t.x!=0 ||t.y!=0;
}
Bool isIdle(){
	for(int i=0;i<numberOfActiveMasters;i++)
		if(!isTupleNotEmpty(masters[i].delta)||
				!isTupleNotEmpty(masters[i].scrollRem)||!isTupleNotEmpty(masters[i].scrollRem))
			return False;
	numberOfActiveMasters=0;
	return True;

}
Bool isPressed(int keycode) {
	int i=keycode/8;
	int pos=1 << (keycode%8);
	return (keyMap[i] & pos);
}






void update(Bool scroll){
	if(calcuateDisplacement(scroll)){
		//printf("mouse info: %d %d %d\n",workingMaster->id,workingMaster->delta.x, workingMaster->delta.y);
		if (scroll)
			request_scrolling();
		else{
			XWarpPointer(dpy, None, None, 0, 0, 0, 0, workingMaster->delta.x, workingMaster->delta.y);
			//XIWarpPointer(dpy,workingMaster->id, None, None, 0, 0, 0, 0, workingMaster->delta.x, workingMaster->delta.y);
		}
	}
}


void cycleMoveOption(){
	workingMaster->moveOption++;
	if (workingMaster->moveOption == RESET)
		workingMaster->moveOption = DEFAULT;

}
void resetCycleMoveOption(){
	workingMaster->moveOption = DEFAULT;
}
Bool calcuateDisplacement( Bool scroll){
	int dir =scroll?workingMaster->scrollDir:workingMaster->mouseDir;
	double xRem =scroll?workingMaster->scrollRem.x:workingMaster->mouseRem.x;
	double yRem =scroll?workingMaster->scrollRem.y:workingMaster->mouseRem.y;
	int xsign = ((dir & RIGHT) ? 1 : 0) - ((dir & LEFT) ? 1 : 0);
	int ysign = ((dir & DOWN) ? 1 : 0) - ((dir & UP) ? 1 : 0) ;

	if (workingMaster->coefficent==0)
		exit(2);

	double value=(scroll? BASE_SCROLL_SPEED: BASE_MOUSE_SPEED) * workingMaster->coefficent ;//* usec / 1e6;

	workingMaster->delta.x=workingMaster->delta.y=0;
	if(xsign){
		double raw=value * xsign + xRem;
		workingMaster->delta.x=(int)raw;

		xRem = raw-workingMaster->delta.x;
		if (scroll)
			workingMaster->scrollRem.x = xRem;
		else
			workingMaster->mouseRem.x = xRem;
	}

	if(ysign){
		double raw=value * ysign + yRem;
		workingMaster->delta.y=(int)raw;
		yRem = raw-workingMaster->delta.y;
		if (scroll)
			workingMaster->scrollRem.y = yRem;
		else
			workingMaster->mouseRem.y = yRem;
	}
	return isTupleNotEmpty(workingMaster->delta);
}


void request_scrolling()
{
	int xbutton = (workingMaster->scrollDir & LEFT) ? SCROLLLEFT : SCROLLRIGHT;
	int ybutton = (workingMaster->scrollDir & UP) ? SCROLLUP : SCROLLDOWN;
	if (!xbutton && !ybutton)
		return;
	for (int i = 0; i < abs(workingMaster->delta.x); i++) {
		XTestFakeButtonEvent(dpy, xbutton, PRESS, CurrentTime);

	}
	XTestFakeButtonEvent(dpy, xbutton, RELEASE, CurrentTime);
	for (int i = 0; i < abs(workingMaster->delta.y); i++) {
		XTestFakeButtonEvent(dpy, ybutton, PRESS, CurrentTime);

	}
	XTestFakeButtonEvent(dpy, ybutton, RELEASE, CurrentTime);
}

void clickpress(const int btn){
	XTestFakeButtonEvent(dpy, btn, True, CurrentTime);
	printf("pressing\n");
}

void clickrelease(const int btn){
	XTestFakeButtonEvent(dpy, btn, False, CurrentTime);
}

void multiplyspeed(const int factor){
	workingMaster->coefficent *= factor;
	if (workingMaster->coefficent>MAX_THRESHOLD)
		workingMaster->coefficent = MAX_THRESHOLD;
}
void dividespeed(const int factor){
	workingMaster->coefficent/= factor;
	if (workingMaster->coefficent<MIN_THRESHOLD)
		workingMaster->coefficent = MIN_THRESHOLD;
}
void mouseAction(Bool scroll, int d, Bool start){
	switch (workingMaster->moveOption){
	case SWAP:
		scroll = !scroll;
		break;
	case ONLY_MOUSE:
		scroll = False;
		break;
	case ONLY_SCROLL:
		scroll = True;
		break;
	}
	//printf("mouse action %d %d %d\n",scroll,workingMaster->scrollDir,workingMaster->mouseDir);
	if (scroll)
		if(start)
			workingMaster->scrollDir|= d;
		else
			workingMaster->scrollDir&= ~ d;
	else
		if(start)
			workingMaster->mouseDir|= d;
		else
			workingMaster->mouseDir&= ~ d;
	//printf("mouse action %d %d %d\n",d,workingMaster->scrollDir,workingMaster->mouseDir);
}
