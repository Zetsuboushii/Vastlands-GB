
#ifndef METASPRITE_palettes_H
#define METASPRITE_palettes_H

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#define palettes_TILE_ORIGIN 0
#define palettes_TILE_W 8
#define palettes_TILE_H 16
#define palettes_WIDTH 16
#define palettes_HEIGHT 16
#define palettes_TILE_COUNT 6
#define palettes_PALETTE_COUNT 1
#define palettes_COLORS_PER_PALETTE 4
#define palettes_TOTAL_COLORS 4
#define palettes_PIVOT_X 8
#define palettes_PIVOT_Y 8
#define palettes_PIVOT_W 16
#define palettes_PIVOT_H 16
extern const metasprite_t* const palettes_metasprites[4];

BANKREF_EXTERN(palettes)

extern const palette_color_t palettes_palettes[4];
extern const uint8_t palettes_tiles[96];

#endif
