#pragma once

#include "custom-types.h"

Vertex RotateVertexAroundAxis(Vertex vx, float angle, RotationAxis axis);

Vertex RotateVertex(Vertex vx, float x, float y, float z);

Vertex TranslateVertex(Vertex vx, float x, float y, float z);

Vertex ScaleVertex(Vertex vx, float x, float y, float z);

Vertex LocalTransform(Vertex vx, Object obj);

Vertex ViewTransfrom(Vertex vx, Camera cam);

ClipCoords ClipSpaceTransform(Vertex vx, float perspectiveMatrix[4][4]);

NdCoords NormalizeDeviceCoordinates(ClipCoords cc);

WindowCoords WindowTransformation(NdCoords nc, int width, int height);