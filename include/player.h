
#ifndef METASPRITE_player_H
#define METASPRITE_player_H

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#define player_TILE_ORIGIN 0
#define player_TILE_W 8
#define player_TILE_H 16
#define player_WIDTH 16
#define player_HEIGHT 16
#define player_TILE_COUNT 4
#define player_PALETTE_COUNT 1
#define player_COLORS_PER_PALETTE 4
#define player_TOTAL_COLORS 4
#define player_PIVOT_X 8
#define player_PIVOT_Y 8
#define player_PIVOT_W 16
#define player_PIVOT_H 16
extern const metasprite_t* const player_metasprites[4];

BANKREF_EXTERN(player)

extern const palette_color_t player_palettes[4];
extern const uint8_t player_tiles[64];

#endif
