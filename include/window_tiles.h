#ifndef __window_tiles_h_
#define __window_tiles_h_

#include <gb/gb.h>
#include <gbdk/platform.h>

/* Externe Deklarationen, damit andere .c-Dateien darauf zugreifen können: */
extern const unsigned char black_tile[16];  // 1 Tile = 16 Bytes
extern const unsigned char arrow_tile[16];  // 1 Tile = 16 Bytes

/* 96 Zeichen, je 16 Bytes => 2D-Array */
extern const unsigned char char_sprites[96][16];

#endif
