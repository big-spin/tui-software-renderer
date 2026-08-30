#ifndef TERM_H
#define TERM_H

#include "custom-types.h"

int TermWidth();

int TermHeight();

void EnableRawMode(struct termios *original);

void DisableRawMode(struct termios *original);

void InitTerm(struct termios *original);

void ShutdownTerm(struct termios *original);

void AddToBuffer(FrameBuffer *buf, int x, int y, uint32_t data);

void ClearBuffer(FrameBuffer *buf);

void PresentBuffer(FrameBuffer *buf);

int TermInput(Camera *cam, Event *ev, int *width, int *height);

#endif