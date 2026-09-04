#pragma once

#include "custom-types.h"

void InitTerm();

void ShutdownTerm();

int TermHeight();

int TermWidth();

void AddToBuffer(FrameBuffer *buf, int x, int y, uint32_t data);

void ClearBuffer(FrameBuffer *buf);

void PresentBuffer(FrameBuffer *buf);

int TermInput(Camera *cam, Event *ev, int *width, int *height);