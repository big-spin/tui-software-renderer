#pragma once

#include "custom-types.h"

int LoadMeshFromFile(char *path, Triangle *mesh);

Vertex GetVertexFromText(char *text);
Triangle GetTriangleFromText(char *text, Vertex *vertices);