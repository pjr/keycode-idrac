/*
 * Shared library hack to translate evdev keycodes to old style keycodes.
 */
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>


static int (*real_XNextEvent)(Display *, XEvent *) = NULL;
static KeyCode (*real_XKeysymToKeycode)(Display *, KeySym) = NULL;
static KeySym (*sym_XKeycodeToKeysym)(Display *, KeyCode, int) = NULL;
static int hack_initialised = 0;

#define DEBUG 0

#ifdef DEBUG
static FILE *fd = NULL;
#endif

static void
hack_init(void)
{
	void *h;

	h = dlopen("libX11.so", RTLD_LAZY);
	if (h == NULL) {
		fprintf(stderr, "Unable to open libX11\n");
		_exit(1);
	}

	real_XNextEvent = dlsym(h, "XNextEvent");
	if (real_XNextEvent == NULL) {
		fprintf(stderr, "Unable to find symbol\n");
		_exit(1);
	}

	real_XKeysymToKeycode = dlsym(h, "XKeysymToKeycode");
	if (real_XKeysymToKeycode == NULL) {
		fprintf(stderr, "Unable to find symbol\n");
		_exit(1);
	}

	sym_XKeycodeToKeysym = dlsym(h, "XKeycodeToKeysym");
	if (sym_XKeycodeToKeysym == NULL) {
		fprintf(stderr, "Unable to find symbol\n");
		_exit(1);
	}

#ifdef DEBUG
	if (fd == NULL) {
		fd = fopen("/tmp/keycode-log", "a");
		if (fd == NULL)
			fprintf(stderr, "Unable to open key-log\n");
	}
#endif

	hack_initialised = 1;
}

int
XNextEvent(Display *display, XEvent *event)
{
	int r;

	if (!hack_initialised)
		hack_init();

	r = real_XNextEvent(display, event);

	if (event->type == KeyPress || event->type == KeyRelease) {
		XKeyEvent *keyevent;
		KeySym keysym;

		keyevent = (XKeyEvent *)event;
#ifdef DEBUG
		fprintf(fd, "KeyEvent: %d\n", keyevent->keycode);
		fflush(fd);
#endif

		/* mangle keycodes */
		keysym = sym_XKeycodeToKeysym(display, keyevent->keycode, 0);
		switch (keysym) {
		case XK_Up:
			keyevent->keycode = 98;
			break;
		case XK_Down:
			keyevent->keycode = 104;
			break;
		case XK_Left:
			keyevent->keycode = 100;
			break;
		case XK_Right:
			keyevent->keycode = 102;
			break;
		case XK_Print:
			keyevent->keycode = 111;
			break;
		}
	}

	return r;
}

#ifdef DEBUG
KeyCode
XKeysymToKeycode(Display *display, KeySym keysym)
{
	KeyCode keycode;

	if (!hack_initialised)
		hack_init();

	keycode = real_XKeysymToKeycode(display, keysym);

	fprintf(fd, "XKeysymToKeycode: %d\n", keycode);
	fflush(fd);

	return keycode;
}
#endif
