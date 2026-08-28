#include "../include/math-utils.h"
#include "../include/term.h"
#include "../include/scene-loader.h"
#include "../include/render.h"

#ifndef NO_X11
#include "../include/x11.h" 
#endif

#define Z_NEAR 0.1f
#define Z_FAR 10.0f

struct termios original;

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
    Backend backend = Term;

    if (argc < 2) {
        puts("No scene file provided\n");
        return 0;
    }
    else if(argc == 3 && strcmp(argv[2], "X11") == 0) {
        backend = X11;
    }

    NameFunctionPair funcs[1] = {
        {"Spin", Spin},
    };

    Object *scene = NULL;
    int sceneSize = LoadSceneFromFile(argv[1], &scene, funcs, 1);

    if (sceneSize == 0) {
        return 1;
    }

    if (backend == Term) {
        tcgetattr(STDIN_FILENO, &original);
        InitTerm(&original);

        width = TermWidth();
        height = TermHeight();
    }
    #ifndef NO_X11
    else {
        OpenX11Window();
        width = 640;
        height = 480;
    }
    #endif

    // Initialize perspective matrix
    UpdatePerspectiveMatrix();

    FrameBuffer buffer;
    buffer.data = malloc(width * height * sizeof(uint32_t));

    buffer.width = TermWidth();
    buffer.height = TermHeight();

    #ifndef NO_X11
    if (backend == X11) {
        buffer.width = 640;
        buffer.height = 480;

        SetupXImage(&buffer, 640, 480);
    }
    #endif

    float *depthBuffer = malloc(width * height * sizeof(float));

    WindowCoords finalWC[3];
    BoundingBox box;

    // Setup camera
    Camera cam = { 
        { 0.0, 0.0, 0.0 },
        0.0,
        0.0,
        0.5,
    };

    if (backend == Term) {
        cam.rotationSpeed = 2.0;
    }
    #ifndef NO_X11
    else {
        cam.rotationSpeed = 0.01;
    }
    #endif

    Event ev;
    ev.quit = 0;
    ev.wireframeMode = 0;

    // Main loop
    while (ev.quit != 1) {
        // Update width and height values if necessary and recalculate everything that depends on those values
        if (backend == Term && (width != TermWidth() || height != TermHeight())) {
            width = TermWidth();
            height = TermHeight();

            UpdatePerspectiveMatrix();

            depthBuffer = realloc(depthBuffer, width * height * sizeof(float));

            buffer.data = realloc(buffer.data, width * height * sizeof(uint32_t));
            buffer.width = width;
            buffer.height = height;
            puts("Resizing terminal");
        }

        // Clear frame and depth buffers
        ClearBuffer(&buffer);
        ClearDepthBuffer(depthBuffer, width, height);

        // Main rendering
        for (int i = 0; i < sceneSize; i++) {
            // Run any custom object functions
            if (scene[i].hasFunction == 1) {
                scene[i].func(&scene[i], &ev);
            }

            for (int j = 0; j < scene[i].triangleCount; j++) {
                RenderTriangle(&scene[i].mesh[j], &scene[i], &cam, depthBuffer, &buffer, perspectiveMatrix, ev.wireframeMode, width, height);
            }
        }

        // Present buffer
        if (backend == Term) PresentBuffer(&buffer);
        #ifndef NO_X11
        else if (backend == X11) PresentBufferX11(&buffer);
        #endif

        // Input handling and camera movement + rotation
        if (backend == Term) TermInput(&cam, &ev);
        #ifndef NO_X11
        else if (X11Input(&cam, &ev, &width, &height) == 1) {
            UpdatePerspectiveMatrix();

            depthBuffer = realloc(depthBuffer, width * height * sizeof(float));

            buffer.data = realloc(buffer.data, width * height * sizeof(uint32_t));
            buffer.width = width;
            buffer.height = height;

            SetupXImage(&buffer, width, height);
        }
        #endif

        ev.frameNumber++;
    }

    // Final cleanup
    if (backend == Term) {
        ShutdownTerm(&original);
    }
    #ifndef NO_X11
    else CloseX11Window();
    #endif

    free(depthBuffer);
    
    for (int i = 0; i < sceneSize; i++) {
        free(scene[i].mesh);
    }
    free(scene);

    return 0;
}
