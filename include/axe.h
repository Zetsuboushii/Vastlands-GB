
#ifndef METASPRITE_axe_H
#define METASPRITE_axe_H

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#define axe_TILE_ORIGIN 0
#define axe_TILE_W 8
#define axe_TILE_H 16
#define axe_WIDTH 16
#define axe_HEIGHT 16
#define axe_TILE_COUNT 8
#define axe_PALETTE_COUNT 1
#define axe_COLORS_PER_PALETTE 4
#define axe_TOTAL_COLORS 4
#define axe_PIVOT_X 8
#define axe_PIVOT_Y 8
#define axe_PIVOT_W 16
#define axe_PIVOT_H 16
extern const metasprite_t* const axe_metasprites[2];

BANKREF_EXTERN(axe)

extern const palette_color_t axe_palettes[4];
extern const uint8_t axe_tiles[128];

#endif
