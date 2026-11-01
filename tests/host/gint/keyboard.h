#ifndef HOST_KEYBOARD_H
#define HOST_KEYBOARD_H

/* Only the signatures and keys consumed by main.c. Not a device emulator. */
enum {
    KEY_UP, KEY_DOWN, KEY_KBD, KEY_X, KEY_Y, KEY_Z, KEY_LEFT, KEY_RIGHT, KEY_SHIFT, KEY_DEL, KEY_EXE, KEY_CLEAR,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_DIV
};
enum { KEYEV_NONE, KEYEV_DOWN, KEYEV_UP, KEYEV_HOLD,KEYEV_OSMENU,
    KEYEV_TOUCH_DOWN,KEYEV_TOUCH_DRAG,KEYEV_TOUCH_UP, GETKEY_NONE = 0 };
typedef struct { unsigned type; int key;unsigned short x,y; } key_event_t;
key_event_t pollevent(void);
key_event_t waitevent(volatile int *timeout);
key_event_t getkey_opt(int options, volatile int *timeout);
int keydown(int key);

#endif

