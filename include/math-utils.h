#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "custom-types.h"

#define PI 3.1415926

#define DEG2RAD(x) ((x)*PI / 180.0f)

Vec3 MatrixVec3Multiplication(Vec3 vec, int matrixSize, float matrix[matrixSize][matrixSize]);

Vec4 MatrixVec4Multiplication(Vec4 vec, int matrixSize, float matrix[matrixSize][matrixSize]);

Vec3 AddVec3(Vec3 vec1, Vec3 vec2);

Vec3 RotateVec3AroundAxis(Vec3 vec, float angle, RotationAxis axis);

ClipCoords Lerp(ClipCoords p0, ClipCoords p1, float t);

int max(int a, int b, int c);

int min(int a, int b, int c);

int clamp(int n, int max, int min);

void CalculateBoundingBox(WindowCoords wc0, WindowCoords wc1, WindowCoords wc2, BoundingBox *bb, int width, int height);
#endif