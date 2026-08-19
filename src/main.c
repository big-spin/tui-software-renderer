#include "../include/math-utils.h"
#include "../include/term.h"
#include "../include/scene-loader.h"
#include "../include/render.h"

#define Z_NEAR 0.1f
#define Z_FAR 10.0f

struct termios original;

int wireframeMode = 0;
int width, height;

float perspectiveMatrix[4][4];

void UpdatePerspectiveMatrix(void) {
    float fov = 25.0 * PI / 180.0;
    float aspect = (float)width / height;

    float fy = 1.0 / tan(fov / 2.0);
    float fx = fy / aspect;

    float zNear = Z_NEAR;
    float zFar = Z_FAR;

    float clip1 = (zFar + zNear) / (zNear - zFar);
    float clip2 = (2 * zFar * zNear) / (zNear - zFar);

    memset(perspectiveMatrix, 0,16 * sizeof(float));

    perspectiveMatrix[0][0] = fx;
    perspectiveMatrix[1][1] = fy;
    perspectiveMatrix[2][2] = clip1;
    perspectiveMatrix[2][3] = clip2;
    perspectiveMatrix[3][2] = -1.0;
}

void Spin(Object *obj, Event *ev) {
    obj->rotation.y += 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        puts("No scene file provided\n");
        return 0;
    }

    NameFunctionPair funcs[1] = {
        {"Spin", Spin},
    };

    Object *scene = NULL;
    int sceneSize = LoadSceneFromFile(argv[1], &scene, funcs, 1);

    if (sceneSize == 0) {
        return 1;
    }

    // Terminal setup
    tcgetattr(STDIN_FILENO, &original);
    InitTerm(&original);

    width = TermWidth();
    height = TermHeight();

    // Initialize perspective matrix
    UpdatePerspectiveMatrix();

    // Text buffer for terminal and depth buffer setup
    TextBuffer buffer = {
        malloc(width * height),
        TermWidth(),
        TermHeight(),
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

    Event ev;
    ev.quit = 0;

    // Main loop
    while (ev.quit != 1) {
        // Update TermWidth() and TermHeight() values if necessary and recalculate everything that depends on those values
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
        ClearDepthBuffer(depthBuffer, width, height);

        // Main rendering
        for (int i = 0; i < sceneSize; i++) {
            // Run any custom object functions
            if (scene[i].hasFunction == 1) {
                scene[i].func(&scene[i], &ev);
            }

            for (int j = 0; j < scene[i].triangleCount; j++) {
                RenderTriangle(&scene[i].mesh[j], &scene[i], &cam, depthBuffer, &buffer, perspectiveMatrix, wireframeMode, width, height);
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
                    ev.quit = 1;
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

        ev.frameNumber++;
    }

    // Memory cleanup and return terminal to original form
    ShutdownTerm(&original);

    free(buffer.data);
    free(depthBuffer);
    
    for (int i = 0; i < sceneSize; i++) {
        free(scene[i].mesh);
    }
    free(scene);

    return 0;
}
