#include "../include/x11.h"
#include "../include/render.h"
#include "../include/keyboard.h"

static Display *display;
static int screen;
static Window window;
static GC gc;
static XImage *img;
static XEvent event;
static FrameBuffer *bufCopy;

static float sens = 0.005;

static int pointerCaptured = 0;

void OpenX11Window() {
    display = XOpenDisplay(NULL);

    screen = DefaultScreen(display);

    window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        0,
        0,
        640,
        480,
        0,
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );

    XMapWindow(display, window);
    XFlush(display);

    gc = XCreateGC(display, window, 0, NULL);

    XSelectInput(display, window, KeyPressMask | StructureNotifyMask | PointerMotionMask);

    XWindowChanges changes;
    changes.width = 640;
    changes.height = 480;
    changes.stack_mode = Above;

    XConfigureWindow(display, window, CWWidth | CWHeight | CWStackMode, &changes);

    XGrabPointer(display, window, 1, PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    pointerCaptured = 1;
}

void CloseX11Window() {
    XDestroyImage(img);
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

void SetupXImage(FrameBuffer *buf, int width, int height) {
    img = XCreateImage(
        display,
        DefaultVisual(display, screen),
        DefaultDepth(display, screen),
        ZPixmap,
        0,
        (char *)buf->data,
        width,
        height,
        32,
        0
    );
}

void PresentBufferX11(FrameBuffer *buf) {
    XPutImage(display, window, gc, img, 0, 0, 0, 0, buf->width, buf->height);
    XFlush(display);
}

int X11Input(Camera *cam, Event *ev, int *width, int *height) {
    ResetKeyboard(&ev->keys);

    int resized = 0;
    while (XPending(display)) {
        XNextEvent(display, &event);
        if (event.type == ConfigureNotify) {
            *width = event.xconfigure.width;
            *height = event.xconfigure.height;

            resized = 1;
        } else if (pointerCaptured == 1 && event.type == MotionNotify) {
            int dx = event.xmotion.x - (*width / 2);
            int dy = event.xmotion.y - (*height / 2);

            if (dx != 0 || dy != 0) {
                cam->yaw -= dx * sens;
                cam->pitch -= dy * sens;
            }
        } else if (event.type == KeyPress) {
            KeySym key = XLookupKeysym(&event.xkey, 0);

            switch (key) {
                case XK_q:
                    ev->quit = 1;
                    break;
                case XK_w:
                    ev->keys.w = 1;
                    break;
                case XK_a:
                    ev->keys.a = 1;
                    break;
                case XK_s:
                    ev->keys.s = 1;
                    break;
                case XK_d:
                    ev->keys.d = 1;
                    break;
                case XK_r:
                    ev->keys.r = 1;
                    break;
                case XK_f:
                    ev->keys.f = 1;
                    break;
                case XK_i:
                    ev->keys.i = 1;
                    break;
                case XK_k:
                    ev->keys.k = 1;
                    break;
                case XK_j:
                    ev->keys.j = 1;
                    break;
                case XK_l:
                    ev->keys.l = 1;
                    break;
                case XK_v:
                    ev->keys.v = 1;
                    break;
                case XK_Escape:
                    if (pointerCaptured == 0) {
                        XGrabPointer(display, window, 1, PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
                        pointerCaptured = 1;
                    } else {
                        XUngrabPointer(display, CurrentTime);
                        pointerCaptured = 0;
                    }
            }
        }
    }

    if (pointerCaptured == 1) XWarpPointer(display, None, window, 0, 0, 0, 0, *width / 2, *height / 2);
    if (resized == 1) UpdatePerspectiveMatrix(*width, *height);

    return resized;
}