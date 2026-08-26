#include "../include/term.h"
#include "../include/math-utils.h"

void EnableRawMode(struct termios *original) {
    struct termios term = *original;

    term.c_lflag &= ~(ECHO | ICANON | ISIG);
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void DisableRawMode(struct termios *original) {
    tcsetattr(STDIN_FILENO, TCSANOW, original);
}

int TermWidth() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}

int TermHeight() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_row;
}

void InitTerm(struct termios *original) {
    EnableRawMode(original);
    
    write(STDOUT_FILENO, "\x1B[H", 3);
    write(STDOUT_FILENO, "\x1B[2J", 4);
    write(STDOUT_FILENO, "\x1B[?25l", 6);
}

void ShutdownTerm(struct termios *original) {
    DisableRawMode(original);

    write(STDOUT_FILENO, "\x1B[2J", 4);
    write(STDOUT_FILENO, "\x1B[?25h", 6);
    write(STDOUT_FILENO, "\x1B[H", 3);
}

void AddToBuffer(FrameBuffer *buf, int x, int y, uint32_t data) {
    buf->data[x + (y * buf->width)] = data;
}

void ClearBuffer(FrameBuffer *buf) {
    for (int i = 0; i < (buf->width * buf->height); i++) buf->data[i] = 0x00000000;
}

void PresentBuffer(FrameBuffer *buf) {
    write(STDOUT_FILENO, "\x1B[H", 3);
    char data[(buf->width * buf->height)];
    for (int i = 0; i < (buf->width * buf->height); i++) {
        if (buf->data[i] == 0x00FFFFFF) data[i] = '@';
        else data[i] = ' ';
    }
    write(STDOUT_FILENO, data, (buf->width * buf->height));
}

void TermInput(Camera *cam, Event *ev) {
    char out[3];

    if(read(STDIN_FILENO, out, sizeof(char) * 3)) {
        Vec3 forward = RotateVec3AroundAxis((Vec3){0.0, 0.0, -cam->speed}, cam->yaw, Y_AXIS);
        Vec3 backward = RotateVec3AroundAxis((Vec3){0.0, 0.0, cam->speed}, cam->yaw, Y_AXIS);
        Vec3 right = RotateVec3AroundAxis((Vec3){cam->speed, 0.0, 0.0}, cam->yaw, Y_AXIS);
        Vec3 left = RotateVec3AroundAxis((Vec3){-cam->speed, 0.0, 0.0}, cam->yaw, Y_AXIS);

        switch (out[0]) {
            case 'q':
                ev->quit = 1;
                return;
            case 'w':
                cam->pos = AddVec3(cam->pos, forward);
                return;
            case 'a':
                cam->pos = AddVec3(cam->pos, left);
                return;
            case 's':
                cam->pos = AddVec3(cam->pos, backward);
                return;
            case 'd':
                cam->pos = AddVec3(cam->pos, right);
                return;
            case 'r':
                cam->pos.y += cam->speed;
                return;
            case 'f':
                cam->pos.y -= cam->speed;
                return;
            case 'i':
                cam->pitch += cam->rotationSpeed;
                return;
            case 'k':
                cam->pitch -= cam->rotationSpeed;
                return;
            case 'j':
                cam->yaw += cam->rotationSpeed;
                return;
            case 'l':
                cam->yaw -= cam->rotationSpeed;
                return;
            case 'v':
                if (ev->wireframeMode == 0) ev->wireframeMode = 1;
                else ev->wireframeMode = 0;
                return;
        }
    }
}