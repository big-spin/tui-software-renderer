#include "../include/math-utils.h"

Vec3 MatrixVec3Multiplication(Vec3 vec, int matrixSize, float  matrix[matrixSize][matrixSize]) {
    Vec3 ret = { 0, 0, 0 };

    ret.x += (
        vec.x * matrix[0][0] +
        vec.y * matrix[0][1] +
        vec.z * matrix[0][2]
    );

    ret.y += (
        vec.x * matrix[1][0] +
        vec.y * matrix[1][1] +
        vec.z * matrix[1][2]
    );

    ret.z += (
        vec.x * matrix[2][0] +
        vec.y * matrix[2][1] +
        vec.z * matrix[2][2]
    );

    return ret;
}

Vec4 MatrixVec4Multiplication(Vec4 vec, int matrixSize, float matrix[matrixSize][matrixSize]) {
    Vec4 ret = { 0, 0, 0, 0 };

    ret.x += (
        vec.x * matrix[0][0] +
        vec.y * matrix[0][1] +
        vec.z * matrix[0][2] +
        vec.w * matrix[0][3]
    );

    ret.y += (
        vec.x * matrix[1][0] +
        vec.y * matrix[1][1] +
        vec.z * matrix[1][2] +
        vec.w * matrix[1][3]
    );

    ret.z += (
        vec.x * matrix[2][0] +
        vec.y * matrix[2][1] +
        vec.z * matrix[2][2] +
        vec.w * matrix[2][3]
    );

    ret.w += (
        vec.x * matrix[3][0] +
        vec.y * matrix[3][1] +
        vec.z * matrix[3][2] +
        vec.w * matrix[3][3]
    );

    return ret;
}

Vec3 AddVec3(Vec3 vec1, Vec3 vec2) {
    return (Vec3) {vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z};
}

Vec3 RotateVec3AroundAxis(Vec3 vec, float angle, RotationAxis axis) {
    switch (axis) {
        case X_AXIS:
            return MatrixVec3Multiplication(vec, 3, 
                (float[3][3]){
                    {1, 0, 0},
                    {0, cos(DEG2RAD(angle)), - sin(DEG2RAD(angle))},
                    {0, sin(DEG2RAD(angle)), cos(DEG2RAD(angle))},
                }
            );
        case Y_AXIS:
            return MatrixVec3Multiplication(vec, 3, 
                (float[3][3]){
                    {cos(DEG2RAD(angle)), 0, sin(DEG2RAD(angle))},
                    {0, 1, 0},
                    {- sin(DEG2RAD(angle)), 0, cos(DEG2RAD(angle))},
                }
            );
        case Z_AXIS:
            return MatrixVec3Multiplication(vec, 3, 
                (float[3][3]){
                    {cos(DEG2RAD(angle)), - sin(DEG2RAD(angle)), 0},
                    {sin(DEG2RAD(angle)), cos(DEG2RAD(angle)), 0},
                    {0, 0, 1 },
                }
            );
    }

    return vec;
}

ClipCoords Lerp(ClipCoords p0, ClipCoords p1, float t) {
    ClipCoords out;
    if (t > 1.0) t = 1.0;
    if (t < 0.0) t = 0.0;

    out.x = p0.x + (p1.x - p0.x) * t;
    out.y = p0.y + (p1.y - p0.y) * t;
    out.z = p0.z + (p1.z - p0.z) * t;
    out.w = p0.w + (p1.w - p0.w) * t;

    return out;
}

int max(int a, int b, int c) {
    int _max = a > b ? a : b;
    return _max > c ? _max : c;
}

int min(int a, int b, int c) {
    int _min = a < b ? a : b;
    return _min < c ? _min : c;
}

int clamp(int n, int max, int min) {
    if (n < min) return min;
    if (n > max) return max;
    return n;
}

void CalculateBoundingBox(WindowCoords wc0, WindowCoords wc1, WindowCoords wc2, BoundingBox *bb, int width, int height) {
    int minX, maxX, minY, maxY;

    minX = min(wc0.x, wc1.x, wc2.x);
    maxX = max(wc0.x, wc1.x, wc2.x);
    minY = min(wc0.y, wc1.y, wc2.y);
    maxY = max(wc0.y, wc1.y, wc2.y);

    bb->x1 = clamp(minX, width, 0);
    bb->x2 = clamp(maxX, width, 0);
    bb->y1 = clamp(minY, height, 0);
    bb->y2 = clamp(maxY, height, 0);
}