#ifndef X_11_H
#define X_11_H

#include "../include/custom-types.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

void OpenX11Window();

void CloseX11Window();

void SetupXImage(FrameBuffer *buf, int width, int height);

void PresentBufferX11(FrameBuffer *buf);

int X11Input(Camera *cam, Event *ev, int *width, int *height);

#endif