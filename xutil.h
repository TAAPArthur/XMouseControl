/**
 * @file test-functions.h
 * @brief Provides methods to simulate key/button presses
 *
 * Wrapper around Xtest
 *
 */
#ifndef MPX_TEST_FUNCTIONS_H_
#define MPX_TEST_FUNCTIONS_H_


typedef int MasterID ;
/**
 * Simulates a button press
 * @param button the button that was pressed
 * @param id the Master pointer id
 */
void sendButtonPress(int button, MasterID id);
/**
 * Simulates a button released
 * @param button the button that was released
 * @param id the Master pointer id
 */
void sendButtonRelease(int button, MasterID id);
/**
 * Moves the mouse specified by id x,y units relative to its current position
 * @param x horizontal displacement
 * @param y vertical displacement
 * @param id
 */
void movePointerRelative(short x, short y, MasterID id) ;


/**
 *
 * Grabs the keyboard or mouse
 * @param deviceID a (non-special) device to grab
 * @param maskValue mask of the events to grab
 * @return 0 on success
 * @see XIGrabDevice
 */
int grabDevice(MasterID deviceID, int maskValue);

void grabKeyboardDevice(MasterID deviceID);
/**
 * Ungrabs the keyboard or mouse
 * Note that id has to be a real (non-special) device id
 * @param id id of the device to ungrab
 * @return 0 on success
 */
int ungrabDevice(MasterID id);
struct KeyBinding;
void init_bindings(struct KeyBinding * bindings, int num, int ignore_mask) ;

int process_event(struct KeyEvent* keyEvent) ;

int getDeviceMapping(MasterID id) ;

int open_connection() ;
void flush() ;

#endif
