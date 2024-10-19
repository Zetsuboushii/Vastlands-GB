#include "../include/input.h"

void process_input(int8_t *dx, int8_t *dy) {
    *dx = 0;
    *dy = 0;

    if (joypad() & J_UP) {
        *dy = -1;
    }
    if (joypad() & J_DOWN) {
        *dy = 1;
    }
    if (joypad() & J_LEFT) {
        *dx = -1;
    }
    if (joypad() & J_RIGHT) {
        *dx = 1;
    }
}
