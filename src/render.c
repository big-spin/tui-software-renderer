#include "../include/custom-types.h"
#include "../include/transformations.h"
#include "../include/math-utils.h"
#include "../include/term.h"
#include "../include/render.h"

#define Z_NEAR 0.5f
#define Z_FAR 10.0f

void ScanConversion(ClipCoords c0, ClipCoords c1, ClipCoords c2, float *depthBuffer, FrameBuffer *buf, int wireframeMode, int width, int height, float lightRaw) {
    BoundingBox box;

    WindowCoords wc0 = WindowTransformation(NormalizeDeviceCoordinates(c0), width, height);
    WindowCoords wc1 = WindowTransformation(NormalizeDeviceCoordinates(c1), width, height);
    WindowCoords wc2 = WindowTransformation(NormalizeDeviceCoordinates(c2), width, height);
    
    CalculateBoundingBox(wc0, wc1, wc2, &box, width, height);

    float det = ((wc1.y-wc2.y)*(wc0.x-wc2.x)+(wc2.x-wc1.x)*(wc0.y-wc2.y));

    uint32_t light = (uint32_t)(lightRaw * 255);
    uint32_t pixel = (light << 16) | (light << 8) | light;
    
    for (int x = box.x1; x < box.x2; x++) {
        for (int y = box.y1; y < box.y2; y++) {
            float lambda1 = (
                ((wc1.y-wc2.y)*(x-wc2.x)+(wc2.x-wc1.x)*(y-wc2.y)) / det
            );
            float lambda2 = (
                ((wc2.y-wc0.y)*(x-wc2.x)+(wc0.x-wc2.x)*(y-wc2.y)) / det
            );
            float lambda3 = (
                1 - lambda1 - lambda2
            );

            if (!(lambda1 >= 0 && lambda2 >= 0 && lambda3 >= 0)) {
                continue;
            }

            float depth = wc0.z * lambda1 + wc1.z * lambda2 + wc2.z * lambda3;

            if (wireframeMode == 0) {
                if (depth < depthBuffer[x + (y * width)]) {
                    AddToBuffer(buf, x, y, pixel);
                    depthBuffer[x + (y * width)] = depth;
                }
            }
            else if (lambda1 <= 0.05 || lambda2 <= 0.05 || lambda3 <= 0.05) {
                if (depth < depthBuffer[x + (y * width)]) {
                    AddToBuffer(buf, x, y, pixel);
                    depthBuffer[x + (y * width)] = depth;
                }
            }
        }
    }
}

void ClearDepthBuffer(float *buffer, int width, int height) {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            buffer[x + (y * width)] = 1.1;
        }
    }
}

void RenderTriangle(Triangle *triangle, Object *obj, Camera *cam, float *depthBuffer, FrameBuffer *buffer, float perspectiveMatrix[4][4], int wireframeMode, int width, int height) {
    ClipCoords cc[3];

    cc[0] = ClipSpaceTransform(ViewTransfrom(LocalTransform(triangle->vertices[0], *obj), *cam), perspectiveMatrix);
    cc[1] = ClipSpaceTransform(ViewTransfrom(LocalTransform(triangle->vertices[1], *obj), *cam), perspectiveMatrix);
    cc[2] = ClipSpaceTransform(ViewTransfrom(LocalTransform(triangle->vertices[2], *obj), *cam), perspectiveMatrix);

    int clip0 = 0, clip1 = 0, clip2 = 0;
    if (cc[0].w <= Z_NEAR) clip0 = 1;
    if (cc[1].w <= Z_NEAR) clip1 = 1;
    if (cc[2].w <= Z_NEAR) clip2 = 1;

    int clipCount = clip0 + clip1 + clip2;
    int idxClip, idxNonClip, idxNext, idxPrev;

    Vec3 triangleNormal = {
        (triangle->vertices[0].pos.x + triangle->vertices[1].pos.x + triangle->vertices[2].pos.x) / 3,
        (triangle->vertices[0].pos.y + triangle->vertices[1].pos.y + triangle->vertices[2].pos.y) / 3,
        (triangle->vertices[0].pos.z + triangle->vertices[1].pos.z + triangle->vertices[2].pos.z) / 3,
    };
    triangleNormal = RotateVec3(triangleNormal, obj->rotation.x, obj->rotation.y, obj->rotation.z);
    Vec3 lightDirection = {1, -2,3};

    Normalize(&triangleNormal);
    Normalize(&lightDirection);

    float lightValue = 0.3 + 0.7 * (DotProduct(triangleNormal,lightDirection) / 2 + 0.5);

    switch (clipCount) {
        case 0:
            ScanConversion(cc[0], cc[1], cc[2], depthBuffer, buffer, wireframeMode, width, height, lightValue);
            break;
        case 1:
            idxClip = (clip0 == 1) ? 0 : (clip1 == 1) ? 1 : 2;
            idxNext = (idxClip + 1) % 3;
            idxPrev = (idxClip - 1 + 3) % 3;

            float fracA = (Z_NEAR - cc[idxClip].w) / (cc[idxNext].w - cc[idxClip].w);
            float fracB = (Z_NEAR - cc[idxClip].w) / (cc[idxPrev].w - cc[idxClip].w);

            ClipCoords clipPointEdgeA = Lerp(cc[idxClip], cc[idxNext], fracA);
            ClipCoords clipPointEdgeB = Lerp(cc[idxClip], cc[idxPrev], fracB);

            ScanConversion(clipPointEdgeB, clipPointEdgeA, cc[idxPrev], depthBuffer, buffer, wireframeMode, width, height, lightValue);
            ScanConversion(clipPointEdgeA, cc[idxNext], cc[idxPrev], depthBuffer, buffer, wireframeMode, width, height, lightValue);
            break;
        case 2:
            idxNonClip = (clip0 == 0) ? 0 : (clip1 == 0) ? 1 : 2;
            idxNext = (idxNonClip + 1) % 3;
            idxPrev = (idxNonClip - 1 + 3) % 3;
            
            fracA = (Z_NEAR - cc[idxNonClip].w) / (cc[idxNext].w - cc[idxNonClip].w);
            fracB = (Z_NEAR - cc[idxNonClip].w) / (cc[idxPrev].w - cc[idxNonClip].w);

            clipPointEdgeA = Lerp(cc[idxNonClip], cc[idxNext], fracA);
            clipPointEdgeB = Lerp(cc[idxNonClip], cc[idxPrev], fracB);

            ScanConversion(clipPointEdgeB, cc[idxNonClip], clipPointEdgeA, depthBuffer, buffer, wireframeMode, width, height, lightValue);
            break;
        case 3:
            return;
    }
}