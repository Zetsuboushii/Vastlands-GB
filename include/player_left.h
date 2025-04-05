#ifndef METASPRITE_player_left_H
#define METASPRITE_player_left_H

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#define player_left_TILE_ORIGIN 0
#define player_left_TILE_W 8
#define player_left_TILE_H 16
#define player_left_WIDTH 16
#define player_left_HEIGHT 16
#define player_left_TILE_COUNT 8
#define player_left_PALETTE_COUNT 1
#define player_left_COLORS_PER_PALETTE 4
#define player_left_TOTAL_COLORS 4
#define player_left_PIVOT_X 8
#define player_left_PIVOT_Y 8
#define player_left_PIVOT_W 16
#define player_left_PIVOT_H 16
extern const metasprite_t* const player_left_metasprites[2];

BANKREF_EXTERN(player_left)

extern const palette_color_t player_left_palettes[4];
extern const uint8_t player_left_tiles[128];

#endif
