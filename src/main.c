#include "../include/math-utils.h"
#include "../include/term.h"
#include "../include/scene-loader.h"

struct termios original;

int width, height;

int wireframeMode = 0;

float perspectiveMatrix[4][4];

// Initialize the perspective matrix to use in Clip Space Transformation.
void UpdatePerspectiveMatrix() {
    float fov = 25.0 * PI / 180.0;
    float aspect = (float)width / height;

    float fy = 1.0 / tan(fov / 2.0);
    float fx = fy / aspect;

    float zNear = 0.001;
    float zFar = 4.0;

    float clip1 = (zFar + zNear) / (zNear - zFar);
    float clip2 = (2 * zFar * zNear) / (zNear - zFar);

    memset(&perspectiveMatrix, 0,16*sizeof(float));

    perspectiveMatrix[0][0] = fx;
    perspectiveMatrix[1][1] = fy;
    perspectiveMatrix[2][2] = clip1;
    perspectiveMatrix[2][3] = clip2;
    perspectiveMatrix[3][2] = -1.0;
}

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

// Move and rotate a vertex based on it's position and rotation.
Vertex LocalTransform(Vertex vx, Object obj) {
    Vertex rotated = RotateVertex(vx, obj.rotation.x, obj.rotation.y, obj.rotation.z);
    return TranslateVertex(rotated, obj.position.x, obj.position.y, obj.position.z);
}

// Move and rotate a vertex based on the camera's position, yaw and pitch.
Vertex ViewTransfrom(Vertex vx, Camera cam) {
    Vertex moved = TranslateVertex(vx, -cam.pos.x, -cam.pos.y, -cam.pos.z);
    return RotateVertex(moved, -cam.pitch, -cam.yaw, 0.0);
}

// Multiply vertex coordinates by perspective matrix -> return clip coordinates.
ClipCoords ClipSpaceTransform(Vertex vx) {
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
WindowCoords WindowTransformation(NdCoords nc) {
    return (WindowCoords) {
        (nc.x + 1) / 2 * width,
        (1.0 - (nc.y + 1) / 2) * height,
        (nc.z + 1) / 2,
    };
}

// Returns max integer out of the three integers given.
int max(int a, int b, int c) {
    int _max = a > b ? a : b;
    return _max > c ? _max : c;
}

// Returns min integer out of the three integers given.
int min(int a, int b, int c) {
    int _min = a < b ? a : b;
    return _min < c ? _min : c;
}

// Clamps an integer value n between a max and min value.
int clamp(int n, int max, int min) {
    if (n < min) return min;
    if (n > max) return max;
    return n;
}

// Calculates the triangle's bounding box for faster scan conversion.
void CalculateBoundingBox(WindowCoords wc[3], BoundingBox *bb) {
    int minX, maxX, minY, maxY;

    minX = min(wc[0].x, wc[1].x, wc[2].x);
    maxX = max(wc[0].x, wc[1].x, wc[2].x);
    minY = min(wc[0].y, wc[1].y, wc[2].y);
    maxY = max(wc[0].y, wc[1].y, wc[2].y);

    bb->x1 = clamp(minX, width, 0);
    bb->x2 = clamp(maxX, width, 0);
    bb->y1 = clamp(minY, height, 0);
    bb->y2 = clamp(maxY, height, 0);
}

// Scan convert a triangle (3 window coordinates -> 1 triangle).
void ScanConversion(WindowCoords *wc, BoundingBox *bb, float *depthBuffer, TextBuffer *buf) {
    WindowCoords wc1 = wc[0];
    WindowCoords wc2 = wc[1];
    WindowCoords wc3 = wc[2];

    float det = ((wc2.y-wc3.y)*(wc1.x-wc3.x)+(wc3.x-wc2.x)*(wc1.y-wc3.y));
    
    for (int x = bb->x1; x < bb->x2; x++) {
        for (int y = bb->y1; y < bb->y2; y++) {
            float lambda1 = (
                ((wc2.y-wc3.y)*(x-wc3.x)+(wc3.x-wc2.x)*(y-wc3.y)) / det
            );
            float lambda2 = (
                ((wc3.y-wc1.y)*(x-wc3.x)+(wc1.x-wc3.x)*(y-wc3.y)) / det
            );
            float lambda3 = (
                1 - lambda1 - lambda2
            );

            if (!(lambda1 >= 0 && lambda2 >= 0 && lambda3 >= 0)) {
                continue;
            }

            float depth = wc1.z * lambda1 + wc2.z * lambda2 + wc3.z * lambda3;

            if (wireframeMode == 0) {
                if (depth < depthBuffer[x + (y * width)]) {
                    AddToBuffer(buf, x, y, '@');
                    depthBuffer[x + (y * width)] = depth;
                }
            }
            else if (lambda1 <= 0.05 || lambda2 <= 0.05 || lambda3 <= 0.05) {
                if (depth < depthBuffer[x + (y * width)]) {
                    AddToBuffer(buf, x, y, '@');
                    depthBuffer[x + (y * width)] = depth;
                }
            }
        }
    }
}

// Initialize every cell with a depth of -4.0
void ClearDepthBuffer(float *buffer) {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            buffer[x + (y * width)] = 1.1;
        }
    }
}

void RenderTriangle(ClipCoords c0, ClipCoords c1, ClipCoords c2, float *depthBuffer, TextBuffer *buffer) {
    WindowCoords finalWC[3];
    BoundingBox box;

    finalWC[0] = WindowTransformation(NormalizeDeviceCoordinates(c0));
    finalWC[1] = WindowTransformation(NormalizeDeviceCoordinates(c1));
    finalWC[2] = WindowTransformation(NormalizeDeviceCoordinates(c2));

    CalculateBoundingBox(finalWC, &box);
    ScanConversion(finalWC, &box, depthBuffer, buffer);
}

void ClipTriangle(Triangle *triangle, Object *obj, Camera *cam, float *depthBuffer, TextBuffer *buffer) {
    ClipCoords cc[3];

    cc[0] = ClipSpaceTransform(ViewTransfrom(LocalTransform(triangle->vertices[0], *obj), *cam));
    cc[1] = ClipSpaceTransform(ViewTransfrom(LocalTransform(triangle->vertices[1], *obj), *cam));
    cc[2] = ClipSpaceTransform(ViewTransfrom(LocalTransform(triangle->vertices[2], *obj), *cam));

    int clip0 = 0, clip1 = 0, clip2 = 0;
    if (cc[0].w <= 0.001) clip0 = 1;
    if (cc[1].w <= 0.001) clip1 = 1;
    if (cc[2].w <= 0.001) clip2 = 1;

    int clipCount = clip0 + clip1 + clip2;
    int idxClip, idxNonClip, idxNext, idxPrev;

    switch (clipCount) {
        case 0:
            RenderTriangle(cc[0], cc[1], cc[2], depthBuffer, buffer);
            break;
        case 1:
            idxClip = (clip0 == 1) ? 0 : (clip1 == 1) ? 1 : 2;
            idxNext = (idxClip + 1) % 3;
            idxPrev = (idxClip - 1 + 3) % 3;

            float fracA = (0.001 - cc[idxClip].w) / (cc[idxNext].w - cc[idxClip].w);
            float fracB = (0.001 - cc[idxClip].w) / (cc[idxPrev].w - cc[idxClip].w);

            ClipCoords clipPointEdgeA = Lerp(cc[idxClip], cc[idxNext], fracA);
            ClipCoords clipPointEdgeB = Lerp(cc[idxClip], cc[idxPrev], fracB);

            RenderTriangle(clipPointEdgeB, clipPointEdgeA, cc[idxPrev], depthBuffer, buffer);
            RenderTriangle(clipPointEdgeA, cc[idxNext], cc[idxPrev], depthBuffer, buffer);
            break;
        case 2:
            idxNonClip = (clip0 == 0) ? 0 : (clip1 == 0) ? 1 : 2;
            idxNext = (idxNonClip + 1) % 3;
            idxPrev = (idxNonClip - 1 + 3) % 3;
            
            fracA = (0.001 - cc[idxNonClip].w) / (cc[idxNext].w - cc[idxNonClip].w);
            fracB = (0.001 - cc[idxNonClip].w) / (cc[idxPrev].w - cc[idxNonClip].w);

            clipPointEdgeA = Lerp(cc[idxNonClip], cc[idxNext], fracA);
            clipPointEdgeB = Lerp(cc[idxNonClip], cc[idxPrev], fracB);

            RenderTriangle(clipPointEdgeB, cc[idxNonClip], clipPointEdgeA, depthBuffer, buffer);
            break;
        case 3:
            return;
    }
}

int main(int argc, char *argv[]) {
    Object *scene = malloc(sizeof(Object));
    int sceneSize = LoadSceneFromFile(argv[1], &scene);

    if (sceneSize == 0) {
        return 1;
    }

    // Terminal setup
    tcgetattr(STDIN_FILENO, &original);
    InitTerm(&original);

    int running = 1;

    width = TermWidth();
    height = TermHeight();

    // Initialize perspective matrix
    UpdatePerspectiveMatrix();

    // Text buffer for terminal and depth buffer setup
    TextBuffer buffer = {
        malloc(width * height),
        width,
        height,
    };

    float *depthBuffer = malloc(width * height * sizeof(float));

    WindowCoords finalWC[3];
    BoundingBox box;

    // Setup camera
    Camera cam = { 
        { 0.0, 0.0, 0.0 },
        0.0,
        0.0,
        0.1,
        1.0,
    };

    // Main loop
    while (running == 1) {
        // Update width and height values if necessary and recalculate everything that depends on those values
        if (width != TermWidth() || height != TermHeight()) {
            width = TermWidth();
            height = TermHeight();

            UpdatePerspectiveMatrix();

            depthBuffer = realloc(depthBuffer, width * height * sizeof(float));

            buffer.data = realloc(buffer.data, width * height * sizeof(char));
            buffer.width = width;
            buffer.height = height;
        }

        // Clear text and depth buffers
        ClearBuffer(&buffer);
        ClearDepthBuffer(depthBuffer);

        // Main rendering
        for (int i = 0; i < sceneSize; i++) {
            for (int j = 0; j < scene[i].triangleCount; j++) {
                ClipTriangle(&scene[i].mesh[j], &scene[i], &cam, depthBuffer, &buffer);
            }
        }

        // Present buffer to terminal
        PresentBuffer(&buffer);

        // Input handling and camera movement + rotation
        char c[3];
        if(PeekInput(c) == OK) {
            Vec3 forward = RotateVec3AroundAxis((Vec3){0.0, 0.0, -cam.speed}, cam.yaw, Y_AXIS);
            Vec3 backward = RotateVec3AroundAxis((Vec3){0.0, 0.0, cam.speed}, cam.yaw, Y_AXIS);
            Vec3 right = RotateVec3AroundAxis((Vec3){cam.speed, 0.0, 0.0}, cam.yaw, Y_AXIS);
            Vec3 left = RotateVec3AroundAxis((Vec3){-cam.speed, 0.0, 0.0}, cam.yaw, Y_AXIS);

            switch (c[0]) {
                case 'q':
                    running = 0;
                    break;
                case 'w':
                    cam.pos = AddVec3(cam.pos, forward);
                    break;
                case 'a':
                    cam.pos = AddVec3(cam.pos, left);
                    break;
                case 's':
                    cam.pos = AddVec3(cam.pos, backward);
                    break;
                case 'd':
                    cam.pos = AddVec3(cam.pos, right);
                    break;
                case 'r':
                    cam.pos.y += cam.speed;
                    break;
                case 'f':
                    cam.pos.y -= cam.speed;
                    break;
                case 'i':
                    cam.pitch += cam.rotationSpeed;
                    break;
                case 'k':
                    cam.pitch -= cam.rotationSpeed;
                    break;
                case 'j':
                    cam.yaw += cam.rotationSpeed;
                    break;
                case 'l':
                    cam.yaw -= cam.rotationSpeed;
                    break;
                case 'v':
                    if (wireframeMode == 0) wireframeMode = 1;
                    else wireframeMode = 0;
                    break;
            }
        }
    }

    // Memory cleanup and return terminal to original form
    ShutdownTerm(&original);

    free(buffer.data);
    free(depthBuffer);
    
    for (int i = 0; i < sceneSize; i++) {
        free(scene[i].mesh);
    }

    return 0;
}
