#include "../include/term.h"
#include "../include/render.h"
#include "../include/keyboard.h"

void EnableRawMode(struct termios *original) {
    struct termios term = *original;

    term.c_lflag &= ~(ECHO | ICANON | ISIG);
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

    UpdatePerspectiveMatrix(TermWidth(), TermHeight());
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

char LightLevelToChar(uint32_t light) {
    int lightValue = (int)(light / 1677721);

    if (lightValue < 0) lightValue = 0;
    if (lightValue > 9) lightValue = 9;

    char ramp[10] = {'.', ':', '-', '=', '+', '*', '#', '%', '@', '$'};

    return ramp[lightValue];
}

void PresentBuffer(FrameBuffer *buf) {
    write(STDOUT_FILENO, "\x1B[H", 3);
    char data[(buf->width * buf->height)];
    for (int i = 0; i < (buf->width * buf->height); i++) {
        if (buf->data[i] != 0x00000000) data[i] = LightLevelToChar(buf->data[i]);
        else data[i] = ' ';
    }
    write(STDOUT_FILENO, data, (buf->width * buf->height));
}

int TermInput(Camera *cam, Event *ev, int *width, int *height) {
    int resized = 0;

    if (*width != TermWidth() || *height != TermHeight()) {
        *width = TermWidth();
        *height = TermHeight();

        UpdatePerspectiveMatrix(*width, *height);

        resized = 1;
    }

    ResetKeyboard(&ev->keys);

    struct pollfd pollCall = {
        STDIN_FILENO,
        POLLIN,
        POLLIN,
    };

    int result = poll(&pollCall, 1, 1);
    if (result < 0) {
        puts("Error polling STDIN_FILENO for input");
        return -1;
    } else if (result == 0 || !(pollCall.revents & POLLIN)) {
        return resized;
    }

    char out;
    ssize_t bytesRead = read(STDIN_FILENO, &out, sizeof(char) * 1);
    if (bytesRead == -1) {
        puts("Error reading STDIN_FILENO");
        return -1;
    }

    if (bytesRead == 1) {
        switch (out) {
            case 'q':
                ev->quit = 1;
                break;
            case 'w':
                ev->keys.w = 1;
                break;
            case 'a':
                ev->keys.a = 1;
                break;
            case 's':
                ev->keys.s = 1;
                break;
            case 'd':
                ev->keys.d = 1;
                break;
            case 'r':
                ev->keys.r = 1;
                break;
            case 'f':
                ev->keys.f = 1;
                break;
            case 'i':
                ev->keys.i = 1;
                break;
            case 'k':
                ev->keys.k = 1;
                break;
            case 'j':
                ev->keys.j = 1;
                break;
            case 'l':
                ev->keys.l = 1;
                break;
            case 'v':
                ev->keys.v = 1;
                break;
        }
    }

    return resized;
}