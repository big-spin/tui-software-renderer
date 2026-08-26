#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <math.h>

typedef struct {
    float x1;
    float y1;
    float x2;
    float y2;
} BoundingBox;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} ClipCoords, Vec4;

typedef struct {
    float x;
    float y;
    float z;
} WindowCoords, Fragment, Vec3, NdCoords;

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    Vec3 pos;
    Vec3 nor;
    Vec2 uv;
} Vertex;

typedef struct {
    Vertex vertices[3];
} Triangle;

typedef struct {
    int w;
    int a;
    int s;
    int d;

    int r;
    int f;

    int i;
    int j;
    int k;
    int l;
} Keyboard;

typedef struct {
    int quit;
    int wireframeMode;
    int frameNumber;
    Keyboard keys;
} Event;

typedef struct Object Object;

typedef void (*ObjectFunction)(Object*, Event*);

struct Object {
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    Triangle *mesh;
    int triangleCount;
    ObjectFunction func;
    int hasFunction;
};

typedef struct {
    const char *name;
    const ObjectFunction func;
} NameFunctionPair;

typedef enum {
    X_AXIS,
    Y_AXIS,
    Z_AXIS,
} RotationAxis, MovementAxis;

typedef struct {
    uint32_t *data;
    int width;
    int height;
} FrameBuffer;

typedef struct {
    Vec3 pos;
    float pitch;
    float yaw;
    float speed;
    float rotationSpeed;
} Camera;

typedef enum {
    X11,
    Term,
} Backend;

#endif