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
int workingIndex;
int numberOfActiveMasters=0;


static int
handleError(Display *dpy, XErrorEvent *event)
{
	char buff[100];
	XGetErrorText(dpy,event->error_code,buff,40);
	printf("Ignoring Xlib error: error code %d request code %d %s\n",
			   event->error_code,
			   event->request_code,buff) ;
	return 0;
}

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

	for (size_t i = 0; i < LEN(keys); i++){
		keys[i].keyCode=XKeysymToKeycode(dpy, keys[i].keySym);
		for(int n=0;n<NUMBER_OF_MASTER_DEVICES;n++){
			keys[i].timeLastRecorded[n]=0;
		}
	}
	root = DefaultRootWindow(dpy);

	//XSelectInput(dpy, root, KeyPressMask|KeyReleaseMask);
	XSetErrorHandler(handleError);
	grabkeys();
}
void grabkeys(){
	XIEventMask eventmask;
	unsigned char mask[1] = { 0 }; /* the actual mask */

	eventmask.deviceid = XIAllMasterDevices;
	eventmask.mask_len = sizeof(mask); /* always in bytes */
	eventmask.mask = mask;
	/* now set the mask */
	XISetMask(mask, XI_KeyPress);
	XISetMask(mask, XI_KeyRelease);

	for (size_t i = 0; i < LEN(keys); i++) {
		Key key = keys[i];
		KeyCode code = key.keyCode;
		XIGrabModifiers modifiers;
		modifiers.modifiers=key.mod;
		XIGrabKeycode(dpy, XIAllMasterDevices, code, root, XIGrabModeAsync, XIGrabModeAsync, True, &eventmask, 1, &modifiers);
	}


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
		//workingMaster=None;
		printf("checking idle\n");
		if(isIdle()){
			printf("idle\n");
			detectEvent();
		}
		else
			while(XPending(dpy)){
				detectEvent();
			}

		//computeKeymap();
		update(True);
		update(False);
		//forceRelease();
		XFlush(dpy);
		fflush(stdout);

		//printf("skiiping sleep");

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
	printf("as master info: %d %d %d %s",id,ndevices,deviceId,masterDevices[0].name);
	XIFreeDeviceInfo(masterDevices);
	return id;
}
void setWorkingMaster(XIDeviceEvent *devev){
	int xMasterId=getAssociatedMasterDevice(devev->deviceid);
	if(masters[workingIndex].id!=xMasterId)
	{
		printf("2\n");
		for(int i=0;i<numberOfActiveMasters;i++){

			if (masters[i].id==xMasterId){
				printf("3\n");
				workingIndex=i;
				return;
			}
		}

		masters[numberOfActiveMasters].id=xMasterId;
		printf("3.5 %d %d",numberOfActiveMasters,masters[numberOfActiveMasters].id);
		masters[numberOfActiveMasters].coefficent=1;

		workingIndex=numberOfActiveMasters;

		numberOfActiveMasters++;


	}
}
void detectEvent(){
	XEvent event;
	XKeyEvent ev;
	XIDeviceEvent *devev;

	XNextEvent(dpy,&event);
	XGenericEventCookie *cookie = &event.xcookie;
	if(XGetEventData(dpy, cookie)){
		devev = cookie->data;
		printf("info: %d %d\n",devev->deviceid,devev->sourceid);
		if(devev->deviceid==0){
			XFreeEventData(dpy, cookie);
			return;
		}
		setWorkingMaster(devev);
		printf("coefficent %f %f",masters[0].coefficent,masters[workingIndex].coefficent);
		printf("autorepat %d press:%d \n",devev->flags& XIKeyRepeat,cookie->evtype==XI_KeyPress);
		int keyIndex=keypress(devev->detail,devev->mods.effective,cookie->evtype==XI_KeyPress);
		printf("setting index %d\n",workingIndex);
		if (keyIndex>=0){

			masters[workingIndex].timeLastRecorded=time(NULL) * 1000;
			keys[keyIndex].timeLastRecorded[workingIndex]=time(NULL) * 1000;
			printf("%ld\n",keys[keyIndex].timeLastRecorded[workingIndex]);

		}


	}
	XFreeEventData(dpy, cookie);
	printf("done\n");
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


	}
	else{
		ev = event.xkey;
		keypress(ev.keycode,ev.state,event.type==KeyPress);
	}

}
*/
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

		return i;
	}
	return -1;

}
void forceRelease(int n){
	computeKeymap();
	time_t t;
    t = time(NULL) * 1000;
	for (size_t i = 0; i < LEN(keys); i++) {
		Key key = keys[i];
		if (key.timeLastRecorded[n]+1000>=t && key.releasefunc){
			if(!isPressed(key.keyCode)){
				key.releasefunc(key.releasearg);
			}
			else
				key.timeLastRecorded[n]=t;
		}

	}
}
Bool isTupleNotEmpty(Tuple t){
	return t.x!=0 ||t.y!=0;
}
Bool isIdle(){
	time_t t;
	t = time(NULL) * 1000;
	Bool idle=True;
	for(int i=0;i<numberOfActiveMasters;i++)
		if(masters[i].scrollDir!=0||masters[i].mouseDir!=0){
			if(i==0&&masters[i].timeLastRecorded+1000>=t)
				forceRelease(i);

			printf("master %d is active",i);
			idle=False;
		}

	return idle;

}
Bool isPressed(int keycode) {
	int i=keycode/8;
	int pos=1 << (keycode%8);
	return (keyMap[i] & pos);
}

void update(Bool scroll){
	for(int i=0;i<numberOfActiveMasters;i++)
		if(calcuateDisplacement(i,scroll)){
			printf("mouse info (%d): %d %f %f\n",scroll,masters[i].id,masters[i].delta.x, masters[i].delta.y);
			if (scroll){
				scrollWithMouse(i);
			}
			else{
				//XWarpPointer(dpy, None, None, 0, 0, 0, 0, masters[workingIndex].delta.x, masters[workingIndex].delta.y);
				printf("movinge mouse %d\n",i);
				printf("id: %d",masters[i].id);
				XIWarpPointer(dpy,masters[i].id, None, None, 0, 0, 0, 0, masters[i].delta.x, masters[workingIndex].delta.y);
			}
		}
}


void cycleMoveOption(){
	masters[workingIndex].moveOption++;
	if (masters[workingIndex].moveOption == RESET)
		masters[workingIndex].moveOption = DEFAULT;

}
void resetCycleMoveOption(){
	masters[workingIndex].moveOption = DEFAULT;
}
Bool calcuateDisplacement(int index, Bool scroll){
	int dir =scroll?masters[index].scrollDir:masters[index].mouseDir;
	if(dir==0)
		return False;
	double xRem =scroll?masters[index].scrollRem.x:masters[index].mouseRem.x;
	double yRem =scroll?masters[index].scrollRem.y:masters[index].mouseRem.y;
	int xsign = ((dir & RIGHT) ? 1 : 0) - ((dir & LEFT) ? 1 : 0);
	int ysign = ((dir & DOWN) ? 1 : 0) - ((dir & UP) ? 1 : 0) ;


	if (masters[workingIndex].coefficent==0)
		exit(2);

	double value=(scroll? BASE_SCROLL_SPEED: BASE_MOUSE_SPEED) * masters[index].coefficent ;//* usec / 1e6;

	masters[index].delta.x=masters[index].delta.y=0;
	if(xsign){
		double raw=value * xsign + xRem;
		masters[index].delta.x=(int)raw;

		xRem = raw-masters[index].delta.x;
		if (scroll)
			masters[index].scrollRem.x = xRem;
		else
			masters[index].mouseRem.x = xRem;
	}

	if(ysign){
		double raw=value * ysign + yRem;
		masters[index].delta.y=(int)raw;
		yRem = raw-masters[index].delta.y;
		if (scroll)
			masters[index].scrollRem.y = yRem;
		else
			masters[workingIndex].mouseRem.y = yRem;
	}
	return isTupleNotEmpty(masters[workingIndex].delta);
}


void scrollWithMouse(int id)
{
	int xbutton = (masters[workingIndex].scrollDir & LEFT) ? SCROLLLEFT : SCROLLRIGHT;
	int ybutton = (masters[workingIndex].scrollDir & UP) ? SCROLLUP : SCROLLDOWN;

	printf("id: %d\n",masters[id].id);
	if (!xbutton && !ybutton)
		return;
	pressButton(xbutton,True);
	pressButton(xbutton,False);
	pressButton(ybutton,True);
	pressButton(ybutton,False);
}

void clickpress(const int btn){
	Window w;
	XIGetFocus(dpy,masters[workingIndex].id, &w);
	pressButton(btn,True);
	printf("pressing\n");
}

void clickrelease(const int btn){
	Window w;
	XIGetFocus(dpy,masters[workingIndex].id, &w);
	pressButton(btn,False);
}

void pressButton(const int btn,Bool press){
	XDevice dev;
	dev.device_id = masters[workingIndex].id; /* this is cheating */
	//XTestFakeDeviceButtonEvent(display, &dev, button, True, NULL, 0, 0);
	XTestFakeDeviceButtonEvent(dpy,&dev,btn, press,NULL,0, CurrentTime);
}

void multiplyspeed(const int factor){
	masters[workingIndex].coefficent *= factor;
	if (masters[workingIndex].coefficent>MAX_THRESHOLD)
		masters[workingIndex].coefficent = MAX_THRESHOLD;
}
void dividespeed(const int factor){
	masters[workingIndex].coefficent/= factor;
	if (masters[workingIndex].coefficent<MIN_THRESHOLD)
		masters[workingIndex].coefficent = MIN_THRESHOLD;
}
void mouseAction(Bool scroll, int d, Bool start){
	switch (masters[workingIndex].moveOption){
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
	//printf("mouse action %d %d %d\n",scroll,masters[workingIndex].scrollDir,masters[workingIndex].mouseDir);
	if (scroll)
		if(start)
			masters[workingIndex].scrollDir|= d;
		else
			masters[workingIndex].scrollDir&= ~ d;
	else
		if(start)
			masters[workingIndex].mouseDir|= d;
		else
			masters[workingIndex].mouseDir&= ~ d;
	//printf("mouse action %d %d %d\n",d,masters[workingIndex].scrollDir,masters[workingIndex].mouseDir);
}

void cycleDefaultMaster(int dir){
	printf("cycling\n");
	int id;
	Window w;
	XGetInputFocus(dpy, &w,&id);
	XIGetClientPointer(dpy, w, &id);
	printf("got current client\n");
	int index=0;
	int ndevices;
	XIDeviceInfo *devices, *device;

	devices = XIQueryDevice(dpy, XIMasterPointer, &ndevices);

	for (int i = 0; i < ndevices; i++) {
		device = &devices[i];
		if (device->deviceid==id){
			printf("Device %s (id: %d) at  %d\n", device->name, device->deviceid,i);
			index=((i+dir*2)+ndevices)%ndevices;
			break;
		}
	}
	XIFreeDeviceInfo(devices);
	printf("setting %d %d\n",index,devices[index].deviceid);
	XISetClientPointer(dpy,w,devices[index].deviceid);


}
