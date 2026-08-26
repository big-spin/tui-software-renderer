#include "../include/x11.h"
#include "../include/math-utils.h"

static Display *display;
static int screen;
static Window window;
static GC gc;
static XImage *img;
static XEvent event;

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

    XSelectInput(display, window, KeyPressMask | StructureNotifyMask);
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
    int resized = 0;
    while (XPending(display)) {
        XNextEvent(display, &event);
        if (event.type == ConfigureNotify) {
            *width = event.xconfigure.width;
            *height = event.xconfigure.height;

            resized = 1;
        }
        else if (event.type == KeyPress) {
            KeySym key = XLookupKeysym(&event.xkey, 0);

            Vec3 forward = RotateVec3AroundAxis((Vec3){0.0, 0.0, -cam->speed}, cam->yaw, Y_AXIS);
            Vec3 backward = RotateVec3AroundAxis((Vec3){0.0, 0.0, cam->speed}, cam->yaw, Y_AXIS);
            Vec3 right = RotateVec3AroundAxis((Vec3){cam->speed, 0.0, 0.0}, cam->yaw, Y_AXIS);
            Vec3 left = RotateVec3AroundAxis((Vec3){-cam->speed, 0.0, 0.0}, cam->yaw, Y_AXIS);

            switch (key) {
                case XK_q:
                    ev->quit = 1;
                    break;
                case XK_w:
                    cam->pos = AddVec3(cam->pos, forward);
                    break;
                case XK_a:
                    cam->pos = AddVec3(cam->pos, left);
                    break;
                case XK_s:
                    cam->pos = AddVec3(cam->pos, backward);
                    break;
                case XK_d:
                    cam->pos = AddVec3(cam->pos, right);
                    break;
                case XK_r:
                    cam->pos.y += cam->speed;
                    break;
                case XK_f:
                    cam->pos.y -= cam->speed;
                    break;
                case XK_i:
                    cam->pitch += cam->rotationSpeed;
                    break;
                case XK_k:
                    cam->pitch -= cam->rotationSpeed;
                    break;
                case XK_j:
                    cam->yaw += cam->rotationSpeed;
                    break;
                case XK_l:
                    cam->yaw -= cam->rotationSpeed;
                    break;
                case XK_v:
                    if (ev->wireframeMode == 0) ev->wireframeMode = 1;
                    else ev->wireframeMode = 0;
                    break;
            }
        }
    }

    return resized;
}