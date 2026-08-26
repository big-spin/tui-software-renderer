#include "../include/custom-types.h"
#include "../include/math-utils.h"

// Rotate a vertex by angle around an axis (x, y or z).
Vertex RotateVertexAroundAxis(Vertex vx, float angle, RotationAxis axis) {
    Vertex out = vx;

    switch (axis) {
        case X_AXIS:
            out.pos = MatrixVec3Multiplication(vx.pos, 3, 
                (float[3][3]){
                    {1, 0, 0},
                    {0, cos(DEG2RAD(angle)), - sin(DEG2RAD(angle))},
                    {0, sin(DEG2RAD(angle)), cos(DEG2RAD(angle))},
                }
            );
            break;
        case Y_AXIS:
            out.pos = MatrixVec3Multiplication(vx.pos, 3, 
                (float[3][3]){
                    {cos(DEG2RAD(angle)), 0, sin(DEG2RAD(angle))},
                    {0, 1, 0},
                    {- sin(DEG2RAD(angle)), 0, cos(DEG2RAD(angle))},
                }
            );
            break;
        case Z_AXIS:
            out.pos = MatrixVec3Multiplication(vx.pos, 3, 
                (float[3][3]){
                    {cos(DEG2RAD(angle)), - sin(DEG2RAD(angle)), 0},
                    {sin(DEG2RAD(angle)), cos(DEG2RAD(angle)), 0},
                    {0, 0, 1 },
                }
            );
            break;
    }

    return out;
}

// Rotate a vertex around all axis at once.
Vertex RotateVertex(Vertex vx, float x, float y, float z) {
    return RotateVertexAroundAxis(RotateVertexAroundAxis(RotateVertexAroundAxis(vx, z, Z_AXIS), y, Y_AXIS), x, X_AXIS);
}

// Move a vertex in all 3 axis at once.
Vertex TranslateVertex(Vertex vx, float x, float y, float z) {
    Vertex out = vx;

    out.pos.x += x;
    out.pos.y += y;
    out.pos.z += z;

    return out;
}

Vertex ScaleVertex(Vertex vx, float x, float y, float z) {
    vx.pos.x *= x;
    vx.pos.y *= y;
    vx.pos.z *= z;

    return vx;
}

// Move and rotate a vertex based on it's position and rotation.
Vertex LocalTransform(Vertex vx, Object obj) {
    Vertex scaled = ScaleVertex(vx, obj.scale.x, obj.scale.y, obj.scale.z);
    Vertex rotated = RotateVertex(scaled, obj.rotation.x, obj.rotation.y, obj.rotation.z);
    return TranslateVertex(rotated, obj.position.x, obj.position.y, obj.position.z);
}

// Move and rotate a vertex based on the camera's position, yaw and pitch.
Vertex ViewTransfrom(Vertex vx, Camera cam) {
    Vertex moved = TranslateVertex(vx, -cam.pos.x, -cam.pos.y, -cam.pos.z);
    return RotateVertex(moved, -cam.pitch, -cam.yaw, 0.0);
}

// Multiply vertex coordinates by perspective matrix -> return clip coordinates.
ClipCoords ClipSpaceTransform(Vertex vx, float perspectiveMatrix[4][4]) {
    return MatrixVec4Multiplication((Vec4){ vx.pos.x, vx.pos.y, vx.pos.z, 1.0}, 4, perspectiveMatrix);
}

// Divide cc's x, y and z components by w -> return normalized device coordinates.
NdCoords NormalizeDeviceCoordinates(ClipCoords cc) {
    return (NdCoords){
        cc.x / cc.w,
        cc.y / cc.w,
        cc.z / cc.w,
    };
}

// Convert normalized device coordinates to window coordinates -> return window coordinates.
WindowCoords WindowTransformation(NdCoords nc, int width, int height) {
    return (WindowCoords) {
        (nc.x + 1) / 2 * width,
        (1.0 - (nc.y + 1) / 2) * height,
        (nc.z + 1) / 2,
    };
}