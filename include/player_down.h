#ifndef METASPRITE_player_down_H
#define METASPRITE_player_down_H

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#define player_down_TILE_ORIGIN 0
#define player_down_TILE_W 8
#define player_down_TILE_H 16
#define player_down_WIDTH 16
#define player_down_HEIGHT 16
#define player_down_TILE_COUNT 8
#define player_down_PALETTE_COUNT 1
#define player_down_COLORS_PER_PALETTE 4
#define player_down_TOTAL_COLORS 4
#define player_down_PIVOT_X 8
#define player_down_PIVOT_Y 8
#define player_down_PIVOT_W 16
#define player_down_PIVOT_H 16
extern const metasprite_t* const player_down_metasprites[2];

BANKREF_EXTERN(player_down)

extern const palette_color_t player_down_palettes[4];
extern const uint8_t player_down_tiles[128];

#endif
