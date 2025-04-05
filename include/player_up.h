#ifndef METASPRITE_player_up_H
#define METASPRITE_player_up_H

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#define player_up_TILE_ORIGIN 0
#define player_up_TILE_W 8
#define player_up_TILE_H 16
#define player_up_WIDTH 16
#define player_up_HEIGHT 16
#define player_up_TILE_COUNT 4
#define player_up_PALETTE_COUNT 1
#define player_up_COLORS_PER_PALETTE 4
#define player_up_TOTAL_COLORS 4
#define player_up_PIVOT_X 8
#define player_up_PIVOT_Y 8
#define player_up_PIVOT_W 16
#define player_up_PIVOT_H 16
extern const metasprite_t* const player_up_metasprites[2];

BANKREF_EXTERN(player_up)

extern const palette_color_t player_up_palettes[4];
extern const uint8_t player_up_tiles[64];

#endif
