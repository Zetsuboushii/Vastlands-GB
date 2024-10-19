#ifndef ISOMETRIC_H
#define ISOMETRIC_H

#include <gb/gb.h>

// Funktionen für die isometrische Projektion
void screen_to_iso(int16_t *x, int16_t *y);
void iso_to_screen(int16_t *x, int16_t *y);

#endif
