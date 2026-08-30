#include "../include/custom-types.h"
#include "../include/keyboard.h"

int ResetKeyboard(Keyboard *keys) {
    keys->w = 0;
    keys->a = 0;
    keys->s = 0;
    keys->d = 0;
    keys->r = 0;
    keys->f = 0;
    keys->i = 0;
    keys->j = 0;
    keys->k = 0;
    keys->l = 0;
    keys->v = 0;

    return 0;
}