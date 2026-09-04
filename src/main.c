#include "../include/term.h"
#include "../include/scene-loader.h"
#include "../include/render.h"
#include "../include/math-utils.h"

#ifndef NO_X11
#include "../include/x11.h" 
#endif

int width, height;

int main(int argc, char *argv[]) {
    Backend backend = Term;

    if (argc < 2) {
        puts("No scene file provided\n");
        return 0;
    } else if (argc == 3 && strcmp(argv[2], "X11") == 0) {
	backend = X11;
    }

    if (backend == Term) {
        InitTerm();
    }
    #ifndef NO_X11
    else OpenX11Window();
    #endif

    width = (backend == Term ? TermWidth() : 640);
    height = (backend == Term ? TermHeight() : 480);


    FrameBuffer buffer = {
	malloc(width * height * sizeof(uint32_t)),
        width,
	height,
    };

    float *depthBuffer = malloc(width * height * sizeof(float));

    #ifndef NO_X11
    if (backend == X11) SetupXImage(&buffer, 640, 480);
    #endif

    Object *scene = NULL;

    int sceneSize = LoadSceneFromFile(argv[1], &scene, NULL, 0);
    if (sceneSize == 0) return 1;

    Camera cam = { 
        .pos={ 0.0, 0.0, 0.0 },
        .pitch=0.0,
        .yaw=0.0,
        .speed=80.0,
        .rotationSpeed=250.0,
    };

    Event ev;

    ev.quit = 0;
    ev.wireframeMode = 0;

    struct timespec deltaTimeClock;
    double deltaTime;

    clock_gettime(CLOCK_MONOTONIC, &deltaTimeClock);

    while (ev.quit != 1) {
        ClearBuffer(&buffer);
        ClearDepthBuffer(depthBuffer, width, height);

        for (int i = 0; i < sceneSize; i++) {
            if (scene[i].hasFunction == 1) {
                scene[i].func(&scene[i], &ev);
            }

            for (int j = 0; j < scene[i].triangleCount; j++) {
                RenderTriangle(&scene[i].mesh[j], &scene[i], &cam, depthBuffer, &buffer, ev.wireframeMode, width, height);
            }
        }

        int inputResult = 0;

        if (backend == Term) {
            PresentBuffer(&buffer);
            inputResult = TermInput(&cam, &ev, &width, &height);
        }
        #ifndef NO_X11
        else {
            PresentBufferX11(&buffer);
            inputResult = X11Input(&cam, &ev, &width, &height);
        }
        #endif

        if (inputResult == -1) return 1;

        if (inputResult == 1) {
            depthBuffer = realloc(depthBuffer, width * height * sizeof(float));

            buffer.data = realloc(buffer.data, width * height * sizeof(uint32_t));
            buffer.width = width;
            buffer.height = height;

            #ifndef NO_X11
            if (backend == X11) SetupXImage(&buffer, width, height);
            #endif
        }

        Vec3 forward = RotateVec3AroundAxis((Vec3){0.0, 0.0, -cam.speed * deltaTime}, cam.yaw, Y_AXIS);
        Vec3 backward = RotateVec3AroundAxis((Vec3){0.0, 0.0, cam.speed * deltaTime}, cam.yaw, Y_AXIS);
        Vec3 right = RotateVec3AroundAxis((Vec3){cam.speed * deltaTime, 0.0, 0.0}, cam.yaw, Y_AXIS);
        Vec3 left = RotateVec3AroundAxis((Vec3){-cam.speed * deltaTime, 0.0, 0.0}, cam.yaw, Y_AXIS);

        if (ev.keys.w == 1) cam.pos = AddVec3(cam.pos, forward);
        if (ev.keys.a == 1) cam.pos = AddVec3(cam.pos, left);
        if (ev.keys.s == 1) cam.pos = AddVec3(cam.pos, backward);
        if (ev.keys.d == 1) cam.pos = AddVec3(cam.pos, right);

        if (ev.keys.r == 1) cam.pos.y += cam.speed * deltaTime;
        if (ev.keys.f == 1) cam.pos.y -= cam.speed * deltaTime;

        if (ev.keys.i == 1) cam.pitch += cam.rotationSpeed * deltaTime;
        if (ev.keys.j == 1) cam.yaw += cam.rotationSpeed * deltaTime;
        if (ev.keys.k == 1) cam.pitch -= cam.rotationSpeed * deltaTime;
        if (ev.keys.l == 1) cam.yaw -= cam.rotationSpeed * deltaTime;

        if (ev.keys.v == 1) ev.wireframeMode = (ev.wireframeMode == 0 ? 1 : 0);

        ev.frameNumber++;

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        deltaTime = (double)(now.tv_sec - deltaTimeClock.tv_sec) + (double)(now.tv_nsec - deltaTimeClock.tv_nsec) / 1000000000;

        deltaTimeClock = now;
    }

    if (backend == Term) {
        ShutdownTerm();
        free(buffer.data);
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