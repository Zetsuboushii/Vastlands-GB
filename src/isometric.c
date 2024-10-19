#include "../include/isometric.h"

// Funktion zur Umwandlung von 2D in isometrische Koordinaten
void screen_to_iso(int16_t *x, int16_t *y) {
    int16_t iso_x = (*x - *y) / 2;
    int16_t iso_y = (*x + *y) / 4;
    *x = iso_x;
    *y = iso_y;
}

// Funktion zur Umwandlung von isometrischen Koordinaten in Bildschirmkoordinaten
void iso_to_screen(int16_t *x, int16_t *y) {
    int16_t screen_x = *x + *y;
    int16_t screen_y = (*y - *x) / 2;
    *x = screen_x;
    *y = screen_y;
}
