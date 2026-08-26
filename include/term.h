#ifndef TERM_H
#define TERM_H

#include "../include/custom-types.h"

int TermWidth();

int TermHeight();

void EnableRawMode(struct termios *original);

void DisableRawMode(struct termios *original);

void InitTerm(struct termios *original);

void ShutdownTerm(struct termios *original);

void AddToBuffer(FrameBuffer *buf, int x, int y, uint32_t data);

void ClearBuffer(FrameBuffer *buf);

void PresentBuffer(FrameBuffer *buf);

void TermInput(Camera *cam, Event *ev);

#endif