#ifndef RENDER_H
#define RENDER_H

#include "../include/custom-types.h"

void ScanConversion(ClipCoords c0, ClipCoords c1, ClipCoords c2, float *depthBuffer, TextBuffer *buf, int wireframeMode, int width, int height);

void ClearDepthBuffer(float *buffer, int width, int height);

void RenderTriangle(Triangle *triangle, Object *obj, Camera *cam, float *depthBuffer, TextBuffer *buffer, float perspectiveMatrix[4][4], int wireframeMode, int width, int height);

#endif